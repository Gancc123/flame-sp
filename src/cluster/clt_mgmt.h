#ifndef FLAME_CLUSTER_MGMT_H
#define FLAME_CLUSTER_MGMT_H

#include "common/context.h"
#include "work/work_base.h"

#include <cstdint>

namespace flame {

class CsdManager;

/**
 * @brief 状态变更回调函数类型
 * 当有节点状态发生改变时调用该回调函数
 */
typedef void (*clt_cb_t)(FlameContext* fct, uint64_t node_id, uint32_t stat);

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
    ClusterMgmt(FlameContext* fct, clt_cb_t cb) : fct_(fct), cb_(cb) {}

    FlameContext* fct_;
    clt_cb_t cb_;
}; // class ClusterMgmt

class ClusterEvent : public WorkEntry {
public:
    ClusterEvent(FlameContext* fct, clt_cb_t cb, uint64_t node_id, uint32_t stat)
    : fct_(fct), cb_(cb), node_id_(node_id), stat_(stat) {}

    virtual void entry() override {
        cb_(fct_, node_id_, stat_);
    }

private:
    FlameContext* fct_;
    clt_cb_t cb_;
    uint64_t node_id_;
    uint32_t stat_;
}; // class ClusterEvent

} // namespace flame

#endif // !FLAME_CLUSTER_MGMT_H