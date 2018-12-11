#ifndef FLAME_CLUSTER_MY_MGMT_H
#define FLAME_CLUSTER_MY_MGMT_H

#include "work/timer_work.h"
#include "cluster/clt_mgmt.h"
#include "common/thread/spin_lock.h"

namespace flame {

struct csd_stat_attr_t {
    uint32_t stat    {0};
    uint64_t latime  {0};
}; // struct csd_stat_attr_t

class MyClusterMgmt;

class CheckStatWork : public WorkEntry {
public:
    CheckStatWork(MyClusterMgmt* mgmt)
    : mgmt_(mgmt) {}

    virtual void entry() override ;
private:
    MyClusterMgmt* mgmt_;
}; // class CheckStatusWork


class MyClusterMgmt : public  ClusterMgmt {
public:
    MyClusterMgmt(TimerWorker* tw, utime_t wait_time, utime_t durable_time)
    : tw_(tw),  wait_time_(wait_time), durable_time_(durable_time) {}

    virtual int init() override {
        tw_->push_cycle(std::shared_ptr<WorkEntry>(new CheckStatWork(this)), wait_time_);
    }

    virtual int update_stat(uint64_t node_id, uint32_t stat) override {
        bool flag = false; // 标志csd状态是否发生变化
        csd_stat_attr_t res;
        if (stmap_get__(res, node_id)) {
            if (res.stat != stat) {
                flag = true;
            }

            stmap_update__(node_id, stat);

            if (flag == true) {
                cb_(fct_, node_id, stat);
            }
        }

        return -1;
    }

    friend class CheckStatWork;

private:
    TimerWorker* tw_;
    utime_t wait_time_;
    utime_t durable_time_;
    std::map<uint64_t, csd_stat_attr_t> stmap_;
    SpinLock stmap_lock_;

    bool stmap_get__(csd_stat_attr_t& res, uint64_t node_id) {
        SpinLocker(stmap_lock_);
        auto it = stmap_.find(node_id);
        if (it != stmap_.end()) {
            res = it->second;
            return true;
        }
        return false;
    }

    void stmap_update__(uint64_t node_id, uint32_t stat) {
        SpinLocker(stmap_lock_);
        csd_stat_attr_t item;
        item.stat = stat;
        item.latime = utime_t::now();
        stmap_[node_id] = item;
    }
}; // class MyClusterMgmt

} // namespace flame

#endif