#ifndef FLAME_LAYOUT_EAMOU_H
#define FLAME_LAYOUT_EAMOU_H

#include "layout.h"
#include "mgr/chunkm/chk_mgmt.h"
#include "include/csds.h"
#include <math.h>
#include <algorithm>


namespace flame  {
namespace layout {

class EamouLayout;

class MigWork : public WorkEntry {
public:
    MigWork(EamouLayout* dlayout)
    : m_dlayout(dlayout) {};

    virtual void entry() override;

private:
    EamouLayout* m_dlayout;
};

class EamouLayout final : public DynamicLayoutBase {
public:
    EamouLayout(CsdManager* csd_mgmt, ChkManager* chk_mgmt, TimerWorker* tw, utime_t cycle, uint32_t spolicy_num)
    : m_csd_mgmt(csd_mgmt), m_chk_mgmt(chk_mgmt), m_tw(tw), m_cycle(cycle), m_spolicy_num(spolicy_num), m_ratio(0.8), m_sd(123.12), m_mig_cnt(10) {}

    // 对上次计算出的迁移三元组信息发送强制迁移的命令
    void force_mig();

    // 判断磨损是否均衡,计算集群磨损标准差,false不需要迁移
    virtual bool judge_wear() override;

    // 计算迁移涉及到的源设备和目标设备需要迁入、迁出的写次数
    int  migration_cal_wear();

    // 获取源csd的k个最热chunk信息
    virtual int  get_hot_chunk() override;

    // 计算迁移的三元组
    virtual int  migration_cal_triple() override;

    // 对这次计算出的迁移三元组发通知迁移的命令
    void notify_mig();

    void init();

    friend class MigWork;

private:
    CsdManager* m_csd_mgmt;
    ChkManager* m_chk_mgmt;
    TimerWorker* m_tw;
    utime_t m_cycle;
    uint32_t m_spolicy_num; // 单副本 or 三副本

    std::list<mig_csd_attr_t> m_csd_info;
    //保存src、dst需要迁入、迁出的写次数,key为写次数,从低到高排序
    std::map<uint64_t, uint64_t> m_mig_src;
    std::map<uint64_t, uint64_t> m_mig_dst;
    //保存csd的k个最热的chunk的信息,排除主副本，记录同副本的chunk位置
    std::map<uint64_t, std::list<mig_chk_attr_t>> m_chk_info;
    //保存迁移的三元组信息
    std::map<uint64_t, std::list<chunk_move_attr_t>> m_mig_chk;
    double m_wear_avg;
    double m_ratio;// 平均值的上下安全范围,可在构造函数中设置默认值
    double m_sd;// 集群磨损标准差的阈值
    int m_mig_cnt;// 控制迁移的chunk个数
    
}; //class EAMOU

} // namespace layout
} // namespace flame

#endif