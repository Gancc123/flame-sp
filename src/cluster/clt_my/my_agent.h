#ifndef FLAME_CLUSTER_MY_AGENT_H
#define FLAME_CLUSTER_MY_AGENT_H

#include "include/meta.h"
#include "util/utime.h"
#include "work/timer_work.h"
#include "cluster/clt_agent.h"
#include "csd/csd_context.h"
#include "include/retcode.h"

#include "include/internal.h"

#include <memory>

namespace flame {

class PushStatWork : public WorkEntry {
public:
    PushStatWork(CsdContext* cct)
    : cct_(cct), stub_(cct->mgr_stub()) {}

    virtual void entry() override {
        stub_->push_status(cct_->csd_id(), cct_->csd_stat());
    }

private:
    CsdContext* cct_;
    std::shared_ptr<InternalClient> stub_;
}; // class PushStatWork

class MyClusterAgent : public ClusterAgent {
public:
    MyClusterAgent(CsdContext* cct, utime_t cycle) 
    : ClusterAgent(cct), cycle_(cycle) {}

    virtual int init() override {
        cct_->timer()->push_cycle(
            std::shared_ptr<WorkEntry>(new PushStatWork(cct_)), 
            cycle_
        );
        return RC_SUCCESS;
    }

private:
    utime_t cycle_;
}; // class MyClusterAgent

} // namespace flame

#endif