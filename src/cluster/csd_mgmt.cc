#include "cluster/csd_mgmt.h"

#include <cassert>

namespace flame {

/**
 * CsdManager
 */

int CsdManager::init() {
    // pass: 从MetaStore加载所有CSD信息
    // 初始化next_csd_id
    std::list<csd_meta_t> res_list;
    int r = ms_->get_csd_ms()->list_all(res_list);
    if (r)return r;
    
    uint64_t max_id = 0;

    for (auto it = res_list.begin(); it != res_list.end(); ++it) {
        CsdHandle* hdl = create_csd_handle__(it->csd_id);
        hdl->set_as_save();
        CsdObject* obj = hdl->get();
        obj->get_meta() = *it;
        if (max_id < it->csd_id) max_id = it->csd_id;
    }

    next_csd_id_ = max_id + 1;

    std::list<csd_health_meta_t> hlt_list;
    r = ms_->get_csd_health_ms()->list_all(hlt_list);
    if (r) return r;

    for (auto it = hlt_list.begin(); it != hlt_list.end(); ++it) {
        auto mit = csd_map_.find(it->csd_id);
        if (mit != csd_map_.end()) {
            CsdObject* obj = hdl->get();
            obj->get_hlt() = *it;
        }
    }

    return SUCCESS;
}

int CsdManager::csd_register(const csd_reg_attr_t& attr, CsdHandle** hp) {
    assert(hp != nullptr);
    WriteLocker(csd_map_lock_);

    uint64_t new_id = next_csd_id_.fetch_add(1);

    CsdHandle* hdl = create_csd_handle__(new_id);
    hdl->set_as_new();
    
    CsdObject* obj = hdl->write_and_lock();
    csd_meta_t& meta = obj->get_meta();

    meta.csd_id = new_id;
    meta.name = attr.csd_name;
    meta.size = attr.size;
    meta.io_addr = attr.io_addr;
    meta.admin_addr = attr.admin_addr;
    meta.stat = attr.stat;
    meta.ctime = utime_t::now();
    meta.latime = utime_t::now();

    *hp = hdl;

    return SUCCESS;
}

int CsdManager::csd_unregister(uint64_t csd_id) {
    WriteLocker(csd_map_lock_);
    auto it = csd_map_.find(csd_id);
    if (it == csd_map_.end()) {
        return OBJ_NOT_EXIST;
    }
    CsdHandle* hdl = *it;
    hdl->set_as_trim();

    ms_->get_csd_ms()->remove(csd_id);

    csd_map_.erase(it);

    return SUCCESS;
}

int CsdManager::csd_sign_up(const csd_sgup_attr_t& attr) {
    ReadLocker(csd_map_lock_);
    auto it = csd_map_.find(csd_id);
    if (it == csd_map_.end()) {
        return OBJ_NOT_EXIST;
    }
    CsdHandle* hdl = *it;
    if (!hdl->stat_is_new()) {
        hdl->set_as_dirty();
    }
    CsdObject* obj = hdl->write_and_lock();
    obj->set_stat(attr.stat);
    obj->set_io_addr(attr.io_addr);
    obj->set_admin_addr(attr.admin_addr);
    return SUCCESS;
}

int CsdManager::csd_sign_out(uint64_t csd_id) {
    ReadLocker(csd_map_lock_);
    auto it = csd_map_.find(csd_id);
    if (it == csd_map_.end()) {
        return OBJ_NOT_EXIST;
    }
    CsdHandle* hdl = *it;
    if (!hdl->stat_is_new()) {
        hdl->set_as_dirty();
    }
    CsdObject* obj = hdl->write_and_lock();
    obj->set_stat(CSD_STAT_DOWN);
    return SUCCESS;
}

int CsdManager::csd_heart_beat(uint64_t csd_id) {
    ReadLocker(csd_map_lock_);
    auto it = csd_map_.find(csd_id);
    if (it == csd_map_.end()) {
        return OBJ_NOT_EXIST;
    }
    CsdHandle* hdl = *it;
    if (!hdl->stat_is_new()) {
        hdl->set_as_dirty();
    }
    CsdObject* obj = hdl->write_and_lock();
    obj->set_stat(CSD_STAT_ACTIVE);
    return SUCCESS;
}

int CsdManager::csd_stat_update(uint64_t csd_id, uint32_t stat) {
    ReadLocker(csd_map_lock_);
    auto it = csd_map_.find(csd_id);
    if (it == csd_map_.end()) {
        return OBJ_NOT_EXIST;
    }
    CsdHandle* hdl = *it;
    if (!hdl->stat_is_new()) {
        hdl->set_as_dirty();
    }
    CsdObject* obj = hdl->write_and_lock();
    obj->set_stat(stat);
    return SUCCESS;
}

int CsdManager::csd_health_update(uint64_t csd_id, const csd_hlt_sub_t& hlt) {
    ReadLocker(csd_map_lock_);
    auto it = csd_map_.find(csd_id);
    if (it == csd_map_.end()) {
        return OBJ_NOT_EXIST;
    }
    CsdHandle* hdl = *it;
    if (!hdl->stat_is_new()) {
        hdl->set_as_dirty();
    }
    CsdObject* obj = hdl->write_and_lock();
    obj->set_size(hlt.size);
    obj->set_alloced(hlt.alloced);
    obj->set_used(hlt.used);
    obj->get_hlt().hlt_meta = hlt.hlt_meta;
   
    return SUCCESS;
}

CsdHandle* CsdManager::find(uint64_t csd_id) {
    ReadLocker(csd_map_lock_);
    auto it = csd_map_.find(csd_id);
    if (it == csd_map_.end()) {
        return nullptr;
    }
    return *it;
}

CsdHandle* CsdManager::create_csd_handle__(uint64_t csd_id) {
    auto it = csd_map_.find(csd_id);
    CsdHandle *ret;
    if (it == csd_map_.end()) {
        ret = new CsdHandle(ms_, csd_id);
        csd_map_[csd_id] = ret;
    }
    else {
        ret = *it;
    }
    return ret;
}

/**
 * CsdHandle
 */

CsdObject* CsdHandle::get() const {
    if (obj_readable__())
        return obj_;
}

CsdObject* CsdHandle::read_and_lock() {
    if (!obj_readable__())
        return nullptr;
    lock_.rdlock();
    return obj_;
}

CsdObject* CsdHandle::read_try_lock() {
    if (obj_readable__() && lock_.try_rdlock())
        return obj_;
    return nullptr;
}

CsdObject* CsdHandle::write_and_lock() {
    if (!obj_readable__())
        return nullptr;
    lock_.wrlock();
    return obj_;
}

CsdObject* CsdHandle::write_try_lock() {
    if (obj_readable__() && lock_.try_wrlock())
        return obj_;
    return nullptr;
}

int CsdHandle::save() {
    if(!obj_readable__())
        return FAILD;

    if (stat_.load() == CSD_OBJ_STAT_NEW) {
        ms_->get_csd_ms()->create(obj_->get_meta());
        ms_->get_csd_health_ms()->create(obj_->get_hlt());

    } else if (stat_.load() == CSD_OBJ_STAT_DIRT) {
        ms_->get_csd_ms()->update(obj_->get_meta());
        ms_->get_csd_health_ms()->update(obj_->get_hlt());

        set_as_save();
    } else {
        return SUCCESS；
    }

    return SUCCESS;
}

int CsdHandle::load() {
    if (stat_ != CSD_OBJ_STAT_LOAD)
        return OBJ_NOT_EXIST;
    
    int r;
    if ((r = ms_->get_csd_ms()->get(obj_->get_meta(), csd_id_)) != 0)
        return r;
    
    if ((r = ms_->get_csd_health_ms()->get(obj_->get_hlt(), csd_id_)) != 0)
        return r;
    
    stat_ = CSD_OBJ_STAT_SVAE;

    return SUCCESS;
}

} // namespace flame