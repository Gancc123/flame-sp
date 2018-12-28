#include "mgr/chk/chk_mgmt.h"

namespace flame {


int ChkManager::chunk_bulk_create(std::map<uint64_t, std::list<uint64_t>>& chk_list, volume_meta_t& vol_req) {
   bool flag = false;
   for (auto it = chk_list.begin(); it != chk_list.end(); ++it) {
       std::list<chunk_meta_t> chk_meta_list;
       std::map<uint64_t, chunk_meta_t> chk_meta_map;
       std::list<chunk_health_meta_t> chk_hlt_list;
       std::set<chunk_health_meta_t> chk_hlt_map;
       for (auto lit = it->second.begin(); lit != it->second.end(); ++lit) {
           chunk_meta_t mt;
           chunk_id_t id(*lit);
           mt.chk_id = *lit;
           mt.vol_id = vol_req.vol_id;
           mt.index = id.get_index();
           mt.stat = CHK_STAT_CREATING;
           mt.spolicy = vol_req.spolicy;
           mt.ctime = utime_t::now();
           // 选主副本，sub_id为0的默认为主副本
           mt.primary = (mt.vol_id << 20) | ((mt.index & 0xFFFFULL) << 4);
           mt.size = vol_req.chk_sz;
           mt.csd_id = it->first;
           mt.csd_mtime = mt.ctime;
           mt.dst_id = it->first;
           mt.dst_ctime = mt.ctime;
           chk_meta_list.push_back(mt);
           chk_meta_map.insert(make_pair(mt.chk_id, mt));

           chunk_health_meta_t ht;
           ht.chk_id = *lit;
           ht.stat = CHK_STAT_CREATING;
           ht.size = vol_req.chk_sz;
           chk_hlt_list.push_back(ht);
           chk_hlt_map.insert(make_pair(ht.chk_id, ht));
       }

       // 调用数据库的接口批量创建chunk和chunk_hlt,状态都为创建中
       ms_->get_csd_ms()->create_bulk(chk_meta_list);
       ms_->get_chunk_health_ms()->create(chk_hlt_list);

       // 调用CsdHandle通过grpc向csd发送创建chunk的请求
       //fct_->log()->lerror("creating chunk faild csd(%llu) chk(%llu)", );
       std::list<chunk_bulk_res_t> res;
       chunk_create_attr_t cattr;
       cattr.flags = vol_req.flags;
       cattr.spolicy = vol_req.spolicy;
       cattr.size = vol_req.chk_sz;
       csd_mgmt_->find(it->first)->get_client()->chunk_create(res, cattr, it->second);

       // 如果创建成功修改对应的chunk的状态
       for (auto rit = res.begin(); rit != res.end(); ++rit) {
           if (rit->res == 0) {
              auto sit = chk_meta_map.find(rit->first);
              if (sit != chk_meta_map.end()) {
                  chunk_meta_t mt = sit->second;
                  mt.stat = CHK_STAT_CREATED;
                  ms_->get_chunk_ms()->update(mt);
              }

              auto shit = chk_hlt_map.find(rit->first);
              if (shit != chk_hlt_map.end()) {
                  chunk_health_meta_t ht = shit->second;
                  ht.stat = CHK_STAT_CREATED;
                  ms_->get_chunk_health_ms()->update(ht);
              }
           }
           else {
               flag = true;
           }
       }

   }

    // 只要有一个chunk创建失败flag就为true
    return flag;
}

// 先在数据库中更新相应chunk的状态为删除中,csd删除了chunk之后再在数据库中删除对应的chunk
int ChkManager::chunk_remove_by_volid(const uint64_t& vol_id) {
    int r;
    std::list<chunk_meta_t> chk_list;
    r = ms_->get_chunk_ms()->list(chk_list, vol_id, 0, 0);
    if (r) return r;

    std::map<uint64_t, std::list<uint64_t>> csd_chk_list;
    for (auto it = chk_list.begin(); it != chk_list.end(); ++it) {
        it->stat = CHK_STAT_DELETING;
        ms_->get_chunk_ms()->update(*it);
        chunk_health_meta_t chk_hlt_temp;
        ms_->get_chunk_health_ms()->get(chk_hlt_temp, it->chk_id);
        chk_hlt_temp.stat = CHK_STAT_DELETING;
        ms_->get_chunk_health_ms()->update(chk_hlt_temp);

        // 以csd_id为单位记录要删除的chunk
        csd_chk_list[it->csd_id].push_back(it->chk_id);
    }

    // 调用CsdHandle通过grpc向csd发送删除chunk的请求
    // 如果csd返回删除成功，在数据库中删除chunk和chunk_hlt的信息
    bool flag = false;
    for (auto it = csd_chk_list.begin(); it != csd_chk_list.end(); ++it) {
        std::list<chunk_bulk_res_t> res;
        csd_mgmt_->find(it->first)->get_client()->chunk_remove(res, it->second);

        for (auto rit = res.begin(); rit != res.end(); ++rit) {
            if (rit->res == 0) {
                ms_->get_chunk_ms()->remove(rit->chk_id);
                ms_->get_chunk_health_ms()->remove(rit->chk_id);
            }
            else {
                flag = true;
            }
        }
    }
    
    // 只要有一个chunk删除失败flag就为true
    return flag;
}

int ChkManager::chunk_get_maps(std::list<chunk_meta_t>& chk_list, const uint64_t& vol_id) {
    int r = ms_->get_chunk_ms()->list(chk_list, vol_id);
    if (r) return r;

    return SUCCESS;
}

int ChkManager::chunk_get_set(std::list<chunk_meta_t>& res_list, const std::list<uint64_t>& chk_ids) {
    int r = ms_->get_chunk_ms()->list(res_list, chk_ids);
    if (r) return r;

    return SUCCESS;
}

int ChkManager::chunk_get_related(std::list<chunk_meta_t>& res_list, const uint64_t& chk_id) {
    chunkid_t tid(chk_id);
    uint64_t vol_id = tid.get_vol_id();
    uint16_t index_id = tid.get_index();
    int r = ms_->get_chunk_ms()->list_cg(res_list, vol_id, index_id);
    return r;
}

int ChkManager::chunk_get_related(std::list<chunk_meta_t>& res_list, const uint64_t& vol_id, const uint16_t& index) {
    int r = ms_->get_chunk_ms()->list_cg(res_list, vol_id, index);
    return r;
}

int ChkManager::chunk_push_status(const std::list<chk_push_attr_t>& chk_list) {
    for (auto it = chk_list.begin(); it != chk_list.end(); ++it) {
        chunk_meta_t mt;
        ms_->get_chunk_ms()->get(mt, it->chk_id);
        mt.stat = it->stat;
        mt.csd_id = it->csd_id;
        mt.dst_id = it->dst_id;
        mt.dst_mtime = it->dst_mtime;

        ms_->get_chunk_ms()->update(mt);
    }

    return SUCCESS;
}

int ChkManager::chunk_push_health(const std::list<chk_hlt_attr_t>& chk_hlt_list) {
    for (auto it = chk_hlt_list.begin(); it != chk_hlt_list.end(); ++it) {
        chunk_health_meta_t hmt;
        ms_->get_chunk_health_ms()->get(hmt, it->chk_id);

        hmt.write_count = hmt.write_count / 2 + it->hlt_meta.last_write;
        hmt.read_count = hmt.read_count / 2 + it->hlt_meta.last_read;

        hmt.size = it->size;
        hmt.stat = it->stat;
        hmt.used = it->used;
        hmt.csd_used = it->csd_used;
        hmt.dst_used = it->dst_used;
        hmt.hlt_meta.last_time = it->hlt_meta.last_time;
        hmt.hlt_meta.last_write = it->hlt_meta.last_write;
        hmt.hlt_meta.last_read = it->hlt_meta.last_read;
        hmt.hlt_meta.last_latency = it->hlt_meta.last_latency;
        hmt.hlt_meta.last_alloc = it->hlt_meta.last_alloc;

        ms_->get_chunk_health_ms()->update(hmt);
    }

    return SUCCESS;
}

int ChkManager::chunk_get_hot(coonst std::map<uint64_t, uint64_t>& res, const uint64_t& csd_id, const uint16_t& limit, const uint32_t& spolicy_num) {
    int r = ms_->get_hot_chunk(res, csd_id, limit, spolicy_num);
    return r;
}

int ChkManager::chunk_record_move(const chunk_move_attr_t& chk) {
    chunk_meta_t mt;

    ms_->get_chunk_ms()->get(mt, chk.chk_id);

    if (chk.signal == 1) { // 通知迁移，记录chunk迁移的目标地址dst_id
        mt.dst_id = chk.dst_id;
    } else if(chk.signal == 2) { // 强制迁移，迁移完成，修改chunk的src_id == dst_id
        mt.csd_id = chk.dst_id;
    }

    ms_->get_chunk_ms()->update(mt);
}

} // namespace flame