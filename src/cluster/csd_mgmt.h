#ifndef FLAME_CLUSTER_H
#define FLAME_CLUSTER_H

#include "common/context.h"
#include "metastore/metastore.h"
#include "include/objects.h"
#include "common/thread/rw_lock.h"
#include "include/internal.h"
#include "cluster/clt_mgmt.h"

#include <cstdint>
#include <atomic>
#include <memory>
#include <map>

namespace flame {

class CsdHandle;

class CsdManager final {
public:
    CsdManager(FlameContext* fct, const std::shared_ptr<MetaStore>& ms, 
        const std::shared_ptr<ClusterMgmt>& clt_mgmt) 
    : fct_(fct), ms_(ms), clt_mgmt_(clt_mgmt) {}
    
    ~CsdManager() {}

    int init();

    int csd_register(const csd_reg_attr_t& attr, CsdHandle** hp);
    int csd_unregister(uint64_t csd_id);
    int csd_sign_up(const csd_sgup_attr_t& attr);
    int csd_sign_out(uint64_t csd_id);

    int csd_heart_beat(uint64_t csd_id);
    int csd_stat_update(uint64_t csd_id, uint32_t stat);
    int csd_health_update(uint64_t csd_id, const csd_hlt_sub_t& hlt);

    CsdHandle* find(uint64_t csd_id);

private:
    FlameContext* fct_;
    std::shared_ptr<MetaStore> ms_;
    std::shared_ptr<ClusterMgmt> clt_mgmt_;

    /**
     * CsdHandle 存储管理
     */
    std::map<uint64_t, CsdHandle*> csd_map_;
    RWLock csd_map_lock_;

    int create_csd_handle__(uint64_t csd_id);
}; // class CsdManager

class CsdHandle final {
public:
    CsdHandle(const std::shared_ptr<MetaStore>& ms, uint64_t csd_id) 
    : csd_id_(csd_id), stat_(CSD_OBJ_STAT_LOAD), ms_(ms) {}

    ~CsdHandle() {}

    int stat() const { return stat_.load(); }

    CsdObject* get() const;
    CsdObject* read_and_lock();
    CsdObject* read_try_lock();
    CsdObject* write_and_lock();
    CsdObject* write_try_lock();
    void unlock() { lock_.unlock(); }

    /**
     * @brief 持久化 CsdObject
     * 
     * @return int 0 => success, 1 => faild
     */
    int save();

    /**
     * @brief 单独持久化元数据部分
     * 
     * @return int 
     */
    int save_meta();

    /**
     * @brief 单独持久化健康部分
     * 
     * @return int 
     */
    int save_health();

private:
    uint64_t csd_id_;
    CsdObject* obj_ {nullptr};
    
    enum CsdObjStat {
        CSD_OBJ_STAT_NEW  = 0,  // 新创建的状态，尚未保存
        CSD_OBJ_STAT_LOAD = 1,  // 处于加载状态
        CSD_OBJ_STAT_SVAE = 2,  // 已保存状态
        CSD_OBJ_STAT_DIRT = 3,  // 数据脏，等待保存
        CSD_OBJ_STAT_TRIM = 4   // 等待删除
    };
    std::atomic<int> stat_ {CSD_OBJ_STAT_LOAD};

    std::shared_ptr<MetaStore> ms_;
    RWLock lock_;

    bool obj_readable__() const {
        int stat = stat_.load();
        if (stat != CSD_OBJ_STAT_LOAD || stat != CSD_OBJ_STAT_TRIM)
            return true;
        return false;
    }
}; // class CsdHandle

} // namespace flame

#endif // FLAME_CLUSTER_H