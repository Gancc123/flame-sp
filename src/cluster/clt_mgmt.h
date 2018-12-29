#ifndef FLAME_CLUSTER_MGMT_H
#define FLAME_CLUSTER_MGMT_H

#include "common/context.h"
#include "work/work_base.h"
#include "mgr/csdm/csd_mgmt.h"

#include <cstdint>
#include <memory>

namespace flame {

class CsdManager;

class ClusterMgmt {
public:
    virtual ~ClusterMgmt() {}

    /**
     * @brief 初始化集群管理代理
     * 
     * @return int 0 iff success
     */
    virtual int init() = 0;

    /**
     * @brief 节点状态更新
     * 
     * @param node_id 节点ID
     * @param stat 
     * @return int 0 iff success
     */
    virtual int update_stat(uint64_t node_id, uint32_t stat) = 0;

protected:
    ClusterMgmt(FlameContext* fct, const std::shared_ptr<CsdManager>& csdm)
    : fct_(fct), csdm_(csdm) {}

    FlameContext* fct_;
    std::shared_ptr<CsdManager> csdm_;
}; // class ClusterMgmt

} // namespace flame

#endif // !FLAME_CLUSTER_MGMT_H