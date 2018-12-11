#ifndef FLAME_CLUSTER_AGENT_H
#define FLAME_CLUSTER_AGENT_H

#include "common/context.h"

namespace flame {

class ClusterAgent {
public:
    virtual ~ClusterAgent() {}

    /**
     * @brief 初始化集群管理代理
     * 
     * @return int 0 iff success
     */
    virtual int init() = 0;

protected:
    ClusterAgent(FlameContext* fct, uint64_t node_id) 
    : fct_(fct), node_id_(node_id) {}

    FlameContext* fct_;
    uint64_t node_id_;
}; // class ClusterAgent

} // namespace flame

#endif // !FLAME_CLUSTER_AGENT_H