#ifndef FLAME_LAYOUT_H
#define FLAME_LAYOUT_H

#include <set>
#include <list>
#include <map>
#include "metastore/metastore.h"
#include "mgr/csdm/csd_mgmt.h"
#include "work/timer_work.h"

namespace flame  {
namespace layout {

#define MIN_LEFT 4ull*1024*1024*1024*10

struct stc_csd_attr_t {
    uint64_t  csd_id;
    uint64_t  left; // 用size - alloced判断还能分配多少数据
    uint64_t  alloced;
    uint64_t  last_latency;
    double    wear_weight;
}; //struct stc_csd_attr_t

struct mig_chk_attr_t {
    uint64_t              chk_id       {0};
    uint64_t              write_count  {0};
    std::set<uint64_t>    related_chk_pos;
}; //struct mig_chk_attr_t

struct mig_csd_attr_t {
    uint64_t    csd_id       {0};
    double      u            {0.0};
    uint64_t    write_count  {0};
    uint64_t    wear_weight  {0};
}; //struct mig_csd_attr_t

struct mig_csd_wr_attr_t {
    uint64_t    csd_id    {0};
    uint64_t    wr_cnt    {0};
}; //struct mig_csd_wr_attr_t

class StaticLayoutBase {
public:
    virtual ~StaticLayoutBase() {}
    virtual void cal_csd_list();
    virtual int static_layout(std::map<uint64_t, std::list<uint64_t>> &res, const volume_meta_t& vol) = 0;

protected:
    StaticLayoutBase() {}
}; //class StaticLayoutBase

class DynamicLayoutBase {
public:
    virtual ~DynamicLayoutBase() {}
    // 判断磨损是否均衡,计算集群磨损标准差
    virtual bool judge_wear() = 0;
    // 获取源csd的k个最热chunk信息
    virtual int  get_hot_chunk() = 0;
    // 计算迁移的三元组
    virtual int  migration_cal_triple() = 0;

protected:
    DynamicLayoutBase() {}
}; //class DynamicLayoutBase

} //namespace layout
} //namespace flame

#endif