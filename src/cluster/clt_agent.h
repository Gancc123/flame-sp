#ifndef FLAME_CLUSTER_AGENT_H
#define FLAME_CLUSTER_AGENT_H

#include "common/context.h"
#include "csd/csd_context.h"

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
    ClusterAgent(CsdContext* cct) : cct_(cct) {}

    CsdContext* cct_;
}; // class ClusterAgent

} // namespace flame

#endif // !FLAME_CLUSTER_AGENT_H