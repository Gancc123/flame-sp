#ifndef FLAME_CLUSTER_H
#define FLAME_CLUSTER_H

#include "common/context.h"
#include "metastore/metastore.h"
#include "include/objects.h"
#include "common/thread/rw_lock.h"

#include <cstdint>
#include <atomic>
#include <memory>
#include <map>

namespace flame {

class CsdHandle;

class ClusterManager final {
public:
    ClusterManager(FlameContext* fct, const std::shared_ptr<MetaStore>& ms) 
    : fct_(fct), ms_(ms) {}
    
    ~ClusterManager() {}

private:
    FlameContext* fct_;
    std::shared_ptr<MetaStore> ms_;
    std::map<uint64_t, CsdHandle> csd_map_;
}; // class Cluster

class CsdHandle final {
public:
    CsdHandle(const std::shared_ptr<MetaStore>& ms, uint64_t csd_id) 
    : csd_id_(csd_id), stat_(CSD_OBJ_STAT_LOAD), ms_(ms) {}

    ~CsdHandle() {}

    int stat() const { return stat_.load(); }

    CsdObject* get();
    CsdObject* read_and_lock();
    CsdObject* read_try_lock();
    CsdObject* write_and_lock();
    CsdObject* write_try_lock();
    void unlock() { lock_.unlock(); }

private:
    uint64_t csd_id_;
    CsdObject* obj_ {nullptr};
    
    enum CsdObjStat {
        CSD_OBJ_STAT_NEW  = 0,
        CSD_OBJ_STAT_LOAD = 1,
        CSD_OBJ_STAT_SVAE = 2,
        CSD_OBJ_STAT_DIRT = 3,
        CSD_OBJ_STAT_TRIM = 4
    };
    std::atomic<int> stat_ {CSD_OBJ_STAT_LOAD};

    std::shared_ptr<MetaStore> ms_;
    
    RWLock lock_;
}; // class CsdHandle

} // namespace flame

#endif // FLAME_CLUSTER_H