#include "easdl.h"

using namespace std;
using namespace flame::layout;

namespace flame  {
namespace layout {

void GetCsdInfoWork::entry() {
    m_elayout->cal_csd_list();
}

void EasdLayout::init() {
    m_tw->push_cycle(std::shared_ptr<WorkEntry>(new GetCsdInfoWork(this)), m_cycle);
}

void EasdLayout::cal_csd_list() {
    // 保存所有csd的负载信息
    std::list<stc_csd_attr_t> csd_info;
    // 从csd_mgmt获得csd的负载信息
    m_csd_mgmt->read_lock();
    for (auto it = m_csd_mgmt->csd_hdl_begin(); it != m_csd_mgmt->csd_hdl_end(); ++it) {
        stc_csd_attr_t at;
        CsdObject* obj = it->second->read_and_lock();

        at.csd_id = obj->get_csd_id();
        at.alloced = obj->get_alloced();
        at.left = obj->get_size() - at.alloced;
        at.last_latency = obj->get_last_latency();
        at.wear_weight = obj->get_wear_weight();

        it->second->unlock();
        csd_info.push_back(at);
    }
    m_csd_mgmt->unlock();

    //对用于分配数据的csd_list加写锁，重新计算用于分配数据的csd列表
    WriteLocker locker(m_csd_list_lock);
    m_csd_list.clear();

    //计算所有csd单个负载的总和
    double wear_total = 0.0;
    double alloc_total = 0.0;
    double latency_total = 0.0;
    double load_avg = 0.0;
    double load_max = 0.0;
    int len = csd_info.size();

    
    for (auto it = csd_info.begin(); it != csd_info.end(); ++it) {
        wear_total += it->wear_weight;
        alloc_total += it->alloced;
        latency_total += it->last_latency;
    }
    
    //用于临时记录csd的总负载权值,csd_id, load, left
    unordered_map<uint64_t, pair<double, uint64_t>> t;

    for (auto it = csd_info.begin(); it != csd_info.end(); ++it) {
        double load_weight = 0.0;
        load_weight = (it->wear_weight / wear_total) * k1 + (it->alloced / alloc_total) * k2
                      + (it->last_latency / latency_total) * k3;
        
        t.insert(make_pair(it->csd_id, make_pair(load_weight, it->left)));
        load_avg += load_weight / len;
        if (load_weight > load_max) load_max = load_weight;
    }

    // 高低负载的阈值，低于这个值的视为低负载
    load_avg = load_avg + (load_max - load_avg) / 2;

    for(auto it = t.begin(); it != t.end(); ++it){
        // 只有综合负载小于阈值和剩余可分配空间>=MIN_LEFT的csd才可以加入用于分配数据的csd列表
        if(it->second.first <= load_avg && it->second.second >= MIN_LEFT){
            m_csd_list.insert(make_pair(it->second.first, make_pair(it->first, it->second.second)));
        }
    }

    // 计算出csd_list以后初始化迭代器
    m_it = m_csd_list.begin();
}

int EasdLayout::static_layout(std::map<uint64_t, std::list<uint64_t>>& res, const volume_meta_t& vol){
    //静态数据布局需要对csd列表加读锁
    ReadLocker locker(m_csd_list_lock);

    if (m_csd_list.empty()) {
        return -1;
    }
    
    uint64_t chk_nums = vol.size / vol.chk_sz + (vol.size % vol.chk_sz == 0 ? 0 : 1);
    uint32_t spolicy_num = 3; // 根据存储策略决定chunk组的chunk个数
    
    for (uint64_t index = 0; index < chk_nums; ++index) {
        auto t_it = m_it;
        for (uint32_t gid = 0; gid < spolicy_num; ++gid) {
            if (t_it == m_csd_list.end()) {
                t_it = m_csd_list.begin();
            }
            
            // todo: 判断csd的剩余可分配空间和状态是否为active
            while (t_it->second.second < vol.chk_sz || !m_csd_mgmt->find(t_it->second.first)->is_active()) {
                ++t_it;
                if (t_it == m_csd_list.end()) t_it = m_csd_list.begin();
                
            }
            
            uint64_t chk_id = vol.vol_id << 20 | index << 4 | gid;
            res[t_it->second.first].push_back(chk_id);
            t_it->second.second -= vol.chk_sz;

            ++t_it;
        }

        ++m_it;
        if (m_it == m_csd_list.end())
            m_it = m_csd_list.begin();
    }

    return 0;
}

} // namespace layout
} // namespace flame