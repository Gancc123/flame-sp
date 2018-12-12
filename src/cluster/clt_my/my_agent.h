#ifndef FLAME_CLUSTER_MY_AGENT_H
#define FLAME_CLUSTER_MY_AGENT_H

#include "work/timer_work.h"
#include "cluster/clt_agent.h"

#include "include/internal.h"

namespace flame {

class PushStatWork : public WorkEntry {
public:
    PushStatusWork(InternalClient* icli, uint64_t node_id)
    : icli_(icli), node_id_(node_id) {}

    virtual void entry() override {
        icli_->push_status(node_id_, 0);
    }

private:
    InternalClient* icli_;
    uint64_t node_id_;
}; // class PushStatusWork

class MyClusterAgent : public ClusterAgent {
public:
    MyClusterAgent(FlameContext* fct, uint64_t node_id, TimerWorker* tw, InternalClient* icli, utime_t cycle)
    : ClusterAgent(fct, node_id), tw_(tw), icli_(icli), cycle_(cycle) {}

    virtual int init() override {
        tw_->push_cycle(std::shared_ptr<WorkEntry>(new PushStatWork(icli_, node_id_)), cycle_);
    }

private:
    TimerWorker* tw_;
    InternalClient* icli_;
    utime_t cycle_;

}; // class MyClusterAgent

} // namespace flame

#endif