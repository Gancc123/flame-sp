#include "cluster/csd_mgmt.h"

#include <cassert>

namespace flame {

/**
 * CsdManager
 */

int CsdManager::init() {
    // pass: 从MetaStore加载所有CSD信息
}

int CsdManager::csd_register(const csd_reg_attr_t& attr, CsdHandle** hp) {
    assert(hp != nullptr);
    // pass
}

int CsdManager::csd_unregister(uint64_t csd_id) {
    // pass
}

int CsdManager::csd_sign_up(const csd_sgup_attr_t& attr) {
    // pass
}

int CsdManager::csd_sign_out(uint64_t csd_id) {
    // pass
}

int CsdManager::csd_heart_beat(uint64_t csd_id) {

}

int CsdManager::csd_stat_update(uint64_t csd_id, uint32_t stat) {

}

int CsdManager::csd_health_update(uint64_t csd_id, const csd_hlt_sub_t& hlt) {

}

CsdHandle* CsdManager::find(uint64_t csd_id) {
    // pass
}

int CsdManager::create_csd_handle__(uint64_t csd_id) {
    WriteLocker(csd_map_lock_);
    auto it = csd_map_.find(csd_id);
    if (it == csd_map_.end())
        csd_map_[csd_id] = new CsdHandle(ms_, csd_id);
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
    // pass
}

int CsdHandle::save_meta() {
    // pass   
}

int CsdHandle::save_health() {
    // pass
}

} // namespace flame