#ifndef FLAME_LAOUT_EASDL_H
#define FLAME_LAOUT_EASDL_H

#include "layout.h"
#include <unordered_map>

namespace flame  {
namespace layout {

class EasdLayout;

class GetCsdInfoWork : public WorkEntry {
public:
    GetCsdInfoWork(EasdLayout* elayout)
    : m_elayout(elayout) {}

    virtual void entry() override;

private:
    EasdLayout* m_elayout;
}; // class GetCsdInfoWork

class EasdLayout final : public StaticLayoutBase {
public:
    EasdLayout(CsdManager* csd_mgmt, TimerWorker* tw, utime_t cycle)
    :m_csd_mgmt(csd_mgmt), m_tw(tw), m_cycle(cycle), k1(0.6), k2(0.2), k3(0.2) {}

    // 设置定时更新csd信息
    void init();

    virtual void cal_csd_list() override;
    
    virtual int static_layout(std::map<uint64_t, std::list<uint64_t>>& res, const volume_meta_t& vol) override;

    friend class GetCsdInfoWork;
private:
    std::multimap<double, std::pair<uint64_t, uint64_t>> m_csd_list; // load, csd_id, left
    std::multimap<double, std::pair<uint64_t, uint64_t>>::iterator m_it;
    RWLock m_csd_list_lock;

    CsdManager* m_csd_mgmt;
    TimerWorker* m_tw;
    utime_t m_cycle;
    double k1;// 每个负载所占的比重
    double k2;
    double k3;
}; // class Easdl

} //namespace layout
} //namespace flame

#endif