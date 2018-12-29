#ifndef FLAME_CLUSTER_MY_MGMT_H
#define FLAME_CLUSTER_MY_MGMT_H

#include "work/timer_work.h"
#include "cluster/clt_mgmt.h"
#include "common/thread/spin_lock.h"
#include "include/retcode.h"
#include "util/utime.h"

namespace flame {

struct node_stat_item_t {
    uint32_t stat;
    utime_t latime;
};

class MyClusterMgmt;

class CheckStatWork : public WorkEntry {
public:
    CheckStatWork(MyClusterMgmt* mgmt) : mgmt_(mgmt) {}

    virtual void entry() override;
private:
    MyClusterMgmt* mgmt_;
}; // class CheckStatusWork

class MyClusterMgmt : public  ClusterMgmt {
public:
    MyClusterMgmt(
        FlameContext* fct, 
        const std::shared_ptr<CsdManager>& csdm, 
        const std::shared_ptr<TimerWorker>& tw, 
        utime_t hb_check
    ) : ClusterMgmt(fct, csdm), tw_(tw), hb_check_(hb_check) {}

    virtual int init() override {
        tw_->push_cycle(std::shared_ptr<WorkEntry>(new CheckStatWork(this)), hb_check_);
        return RC_SUCCESS;
    }

    virtual int update_stat(uint64_t node_id, uint32_t stat) override;
    void stat_check();

    friend class CheckStatWork;

private:
    std::shared_ptr<TimerWorker> tw_;
    utime_t hb_check_;

    std::map<uint64_t, node_stat_item_t> stmap_;
    SpinLock stmap_lock_;

    bool stmap_update__(uint64_t node_id, uint32_t stat);
}; // class MyClusterMgmt

} // namespace flame

#endif // FLAME_CLUSTER_MY_MGMT_H