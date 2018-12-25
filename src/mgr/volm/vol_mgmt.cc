#include "mgr/vol/vol_mgmt.h"

namespace flame {

int VolManager::init_vg_map() {
    std::list<volume_group_meta_t> res_list;
    int r = ms_->get_vg_ms()->list(res_list, 0, 0);
    if (r) return r;

    for (auto it = res_list.begin(); it != res_list.end(); ++it) {
        vg_map_[it->name] = it->vg_id;
    }

    return SUCCESS;
}

int VolManager::vg_map_find(const std::string& vg_name, uint64_t& vg_id) {
    WriteLocker lok(vg_map_);
    int r = 0;

    auto it = vg_map_.find(vg_name);
    if (it != vg_map_.end()) {
        vg_id = it->second;
    }
    else {
        volume_group_meta_t vg;
        r = ms_->get_vg_ms()->get(vg, vg_name);
        if (r) return r;
        vg_map_[vg_name] = vg.vg_id;
        vg_id = vg.vg_id;
    }

    return SUCCESS;
}

/**
 * volume group
*/

int VolManager::volume_group_list(std::list<volume_group_meta_t>& res_list, const uint32_t& offset, const uint32_t& limit) {
    int r = ms_->get_vg_ms()->list(res_list, offset, limit);

    return r;
}

int VolManager::volume_group_create(const std::string& name) {
    volume_group_meta_t vg;
    vg.name = name;
    vg.ctime = utime_t::now().to_msec();
    int r = ms_->get_vg_ms()->create_and_get(vg);

    if (r == 0) {
        WriteLocker lok(vg_map_lock_);
        vg_map_[name] = vg.vg_id;
    }

    return r;
}

int VolManager::volume_group_remove(const std::string& name) {
    volume_group_meta_t vg;
    inr r = ms_->get_vg_ms()->get(vg, name);
    if (r) return r;

    if (vg.volumes <= 0) {
        return FAILD;
    }
    else {
        r = ms_->get_vg_ms->remove(name);

        if (r == 0) {
            WriteLocker lok(vg_map_lock_);
            auto it = vg_map_.find(name);
            if (it != vg_map_.end()) vg_map_.erase(it);
        }

        return r;
    }
}

int VolManager::volume_group_rename(const std::string& old_name, const std::string& new_name) {
    int r = ms_->get_vg_ms()->rename(old_name, new_name);

    if (r == 0) {
        WriteLocker lok(vg_map_lock_);
        auto it = vg_map_.find(old_name);
        if (it != vg_map_.end()) {
            vg_map_[new_name] = it->second;
            vg_map_.erase(it);
        }
    }

    return r;
}

/**
 * volume
 */

int VolManager::volume_list(std::list<volume_meta_t>& res_list, const std::string& vg_name, const uint32_t& offset, const uint32_t& limit) {
    uint64_t vg_id;
    int r;

    r = vg_map_find(vg_name, vg_id);
    if (r) return r;

    r = ms_->get_volume_ms()->list(res_list, vg_id, offset, limit);

    return r;
}

// 在vg中增加volumes的计数
int VolManager::volume_create(const std::string& vg_name, volume_meta_t& vol_req) {
    uint64_t vg_id;
    int r;

    r = vg_map_find(vg_name, vg_id);
    if (r) return r;

    // 在数据库中创建volume，并get到vol_id
    vol_req.vg_id = vg_id;
    vol_req.stat = VOL_STAT_CREATING; // volume的状态为创建中！
    r = ms_->get_volume_ms()->create_and_get(vol_req);
    if (r) {
        fct_->log()->lerror("insert volume into database faild vg(%s) volume(%s) ", vg_name, vol_req.name);
        return r;
    }

    // 调用静态数据布局算法计算volume的chunk所属csd
    std::map<uint64_t, std::list<uint64_t>> chk_res;
    r = layout_->static_layout(chk_res, vol_req);
    if (r) {
        fct_->log()->lerror("static layout error vg(%s) volume(%s)", vg_name, vol_req.name);
        return r;
    }
    
    
    r = chk_mgr_->create_chunk_bulk(chk_res, vol_req);
    if (r) {
        fct_->log()->lerror("create chunk bulk error vg(%s) volume(%s)", vg_name, vol_req.name);
        return r;
    }

    // todo: 在vg中增加volumes的计数
    vol_req.stat = VOL_STAT_CREATED;
    r = ms_->get_volume_ms()->update(vol_req);
    if (r) {
        fct_->log()->lerror("update volume stat to creating success error vg(%s) volume(%s)", vg_name, vol_req.name);
        return r;
    }
    
    volume_group_meta_t vg;
    r = ms_->get_vg_ms()->get(vg, vg_id);
    vg.volumes += 1;
    r = ms_->get_vg_ms()->update(vg);

    return SUCCESS;
}

// 在vg中减少volumes的计数
int VolManager::volume_remove(const std::string& vg_name, const std::string& vol_name) {
    uint64_t vg_id;
    int r;

    r = vg_map_find(vg_name, vg_id);
    if (r) return r;

    volume_meta_t vol;
    r = ms_->get_volume_ms()->get(vol, vg_id, vol_name);
    if (r) return r;
    if (vol.stat == VOL_STAT_CREATING || vol.stat == VOL_STAT_DELETING) {
        return FAILD;
    }

    // 将volume的状态改为deleting
    vol.stat = VOL_STAT_DELETING;
    r = ms_->get_volume_ms()->update(vol);

    r = chk_mgmt_->chunk_remove_by_volid(vol.vol_id);
    // 考虑volume的chunk数为0的时候也会返回错误，因为在数据库找不到chunk的列表
    if (r) return r;

    // csd那边删除chunk成功之后再数据库中删除volume的记录
    r = ms_->get_volume_ms()->remove(vol.vol_id);
    if (r) return r;

    return SUCCESS;
}


int VolManager::volume_rename(const std::string& vg_name, const std::string& vol_old_name, const std::string& vol_new_name) {
    uint64_t vg_id;
    int r;

    r = vg_map_find(vg_name, vg_id);
    if (r) return r;

   // 如果volume的状态为创建中或者删除中则不能进行重命名的操作
    volume_meta_t vol;
    r = ms_->get_volume_ms()->get(vol, vg_id, vol_old_name);
    if (r) return r;
    if (vol.stat == VOL_STAT_CREATING || vol.stat == VOL_STAT_DELETING) {
        return FAILD;
    }    

    r = ms_->get_volume_ms()->rename(vg_id, vol_old_name, vol_new_name);
    if (r) return r;

    return SUCCESS;
}

int VolManager::volume_get(volume_meta_t& vol_res, const std::string& vg_name, const std::string& vol_name) {
    uint64_t vg_id;
    int r;

    r = vg_map_find(vg_name, vg_id);
    if (r) return r;

    r = ms_->get_volume_ms()->get(vol_res, vg_id, vol_name);
    if (r) return r;

    return SUCCESS;
}

int VolManager::volume_resize(const std::string& vg_name, const std::string& vol_name, const uint64_t& new_size) {
    uint64_t vg_id;
    int r;

    r = vg_map_find(vg_name, vg_id);
    if (r) return r;

    volume_meta_t vol;
    r = ms_->get_volume_ms()->get(vol, vg_id, vol_name);
    if (r) return r;
    if (vol.stat == VOL_STAT_CREATING || vol.stat == VOL_STAT_DELETING) {
        return FAILD;
    }   

    vol.size = new_size;
    r = ms_->get_volume_ms()->update(vol);
    if (r) return r;

    return SUCCESS;
}

int VolManager::volume_open(const uint64_t& gw_id, const std::string& vg_name, const std::string& vol_name) {
    return 0;
}

int VolManager::volume_close(const uint64_t& gw_id, const std::string& vg_name, const std::string& vol_name) {
    return 0;
}

int VolManager::volume_lock(const uint64_t& gw_id, const std::string& vg_name, const std::string& vol_name) {
    return 0;
}

int VolManager::volume_unlock(const uint64_t& gw_id, const std::string& vg_name, const std::string& vol_name) {
    return 0;
}

} // namespace flame