#include "eamou.h"

using namespace std;
using namespace flame::layout;

bool EamouLayout::judge_wear() {
     
    //保存csd的磨损信息
    m_csd_info.clear();
    // 调用csd_mgmt的接口获取csd的信息
    m_csd_mgmt->read_lock();
    for (auto it = m_csd_mgmt->csd_hdl_begin(); it != m_csd_mgmt->csd_hdl_end(); ++it) {
        CsdObject* obj = it->second->read_and_lock();
        mig_csd_attr_t at;
        at.csd_id = obj->get_csd_id();
        at.write_count = obj->get_write_count();
        at.wear_weight = obj->get_wear_weight();
        at.u = (double)obj->get_used() / obj->get_size();
        m_csd_info.push_back(at);
        it->second->unlock();
    }
    m_csd_mgmt->unlock();
    
    int len = m_csd_info.size();
    m_wear_avg = 0.0;
    for(auto it = m_csd_info.begin(); it != m_csd_info.end(); ++it) {
        m_wear_avg += it->wear_weight / len;
    }

    double dev = 0.0;
    for(auto it = m_csd_info.begin(); it != m_csd_info.end(); ++it) {
        dev = (it->wear_weight - m_wear_avg) * (it->wear_weight - m_wear_avg);
    }
    dev = sqrt(dev);

    if(dev < m_sd) {
        return false; //磨损均衡，不需要进行数据迁移
    }
    else {
        return true;
    }
}

int EamouLayout::migration_cal_wear() {
    if(m_csd_info.empty()) {
        return -1;
    }

    m_mig_src.clear();
    m_mig_dst.clear();

    for(auto it = m_csd_info.begin(); it != m_csd_info.end(); ++it) {
        // 判断csd是否为active状态
        if (m_csd_mgmt->find(it->csd_id)->is_active()) {
            if(it->wear_weight >= m_wear_avg) {
                double diff_wear = it->wear_weight - m_wear_avg;
                // 如果磨损权值超过平均值的一定比例，则为迁移的源设备
                if(diff_wear > m_wear_avg * m_ratio) {
                    uint64_t wr_cnt = (uint64_t)((diff_wear - m_wear_avg * m_ratio) * (1 - it->u));
                    m_mig_src.insert(make_pair(wr_cnt, it->csd_id));
                }
            }
            else {
                double diff_wear = m_wear_avg - it->wear_weight;
                // 如果磨损权值低于平均值的一定比例，则为迁移的目标设备
                if(diff_wear > m_wear_avg * m_ratio) {
                    uint64_t wr_cnt = (uint64_t)((diff_wear - m_wear_avg * m_ratio) * (1 - it->u));
                    m_mig_dst.insert(make_pair(wr_cnt, it->csd_id));
                }
            }
        }
    }

    return 0;
}

int EamouLayout::get_hot_chunk() {
    // 调用chunk_mgmt的接口从数据库中获取迁移的源设备的k个写热点chunk，包括chunk同组的其他chunk的csd_id
    if (m_mig_src.empty()) return -1;

    m_chk_info.clear();
    int r;

    for (auto it = m_mig_src.begin(); it != m_mig_src.end(); ++it) {
        std::map<uint64_t, uint64_t> chk_map; // 保存chunk_id和write_cnt;
        r = m_chk_mgmt->chunk_get_hot(chk_map, it->first, 10, m_spolicy_num);

        for (auto cit = chk_map.begin(); cit != chk_map.end(); ++cit) {
            mig_chk_attr_t at;
            at.chk_id = cit->first;
            at.write_count = cit->second;
            if (m_spolicy_num == 3) { // 找出候选迁移chunk的同组chunk的csd_id
                std::list<chunk_meta_t> res_list;
                m_chk_mgmt->chunk_get_related(res_list, cit->first);
                for (auto lit = res_list.begin(); lit != res_list.end(); ++lit) {
                    at.related_chk_pos.insert(lit->dst_id);
                }
            }
            m_chk_info[it->first].push_back(at);
        }
    }

    return 0;
}

// 计算迁移的三元组信息
int EamouLayout::migration_cal_triple() {
    if(m_mig_src.empty() || m_mig_dst.empty() || m_chk_info.empty()) {
        return -1;
    }

    m_mig_chk.clear();

    // 保存迁出、迁入的写次数，从高到低排列
    list<mig_csd_wr_attr_t> src_wr;
    list<mig_csd_wr_attr_t> dst_wr;

    for(auto it = m_mig_src.rbegin(); it != m_mig_src.rend(); ++it) {
        mig_csd_wr_attr_t item;
        item.csd_id = it->second;
        item.wr_cnt = it->first;
        src_wr.push_back(item);
    }
    
    for(auto it = m_mig_dst.rbegin(); it != m_mig_dst.rend(); ++it) {
        mig_csd_wr_attr_t item;
        item.csd_id = it->second;
        item.wr_cnt = it->first;
        dst_wr.push_back(item);
    }

    int cur_mig_cnt = 0;

    auto it = src_wr.begin();
    for( ; cur_mig_cnt < m_mig_cnt && it != src_wr.end(); ++it) {
        if (dst_wr.empty()) break;
        // 获取迁移的源设备的写热点chunk信息
        list<mig_chk_attr_t> hot_chk = m_chk_info[it->csd_id];
        auto hit = hot_chk.begin();
        // 迁移的chunk个数不超过阈值  && 源设备仍需要迁移数据 && hot_chk还有chunk
        for( ; cur_mig_cnt < m_mig_cnt && it->wr_cnt > 0 && hit != hot_chk.end(); ++hit) {
            auto dit = dst_wr.begin();
            for( ; dit != dst_wr.end(); ++dit) {
                if(dit->wr_cnt <= 0 || hit->related_chk_pos.find(dit->csd_id) != hit->related_chk_pos.end())
                    continue;
                
                //记录迁移的三元组信息
                chunk_move_attr_t item;
                item.chk_id = hit->chk_id;
                item.src_id = it->csd_id;
                item.dst_id = dit->csd_id;
                item.signal = 1; // 通知迁移
                m_mig_chk[it->csd_id].push_back(item);

                //修改src和dst对应的写次数
                it->wr_cnt -= hit->write_count;
                dit->wr_cnt -= hit->write_count;
                if(dit->wr_cnt <= 0) {
                    dst_wr.erase(dit);
                }
                
                //增加迁移的chunk计数
                ++cur_mig_cnt;

                break;
            }
        }
    }

    return 0; 
}

void EamouLayout::force_mig() {
    if (!m_mig_chk.empty()) {
        // 调用csd_mgmt的grpc接口向csd发送强制迁移数据的命令
        for (auto it = m_mig_chk.begin(); it != m_mig_chk.end(); ++it) {
            std::map<uint64_t, chunk_move_attr_t> chk_move_map;

            for (auto lit = it->second.begin(); lit != it->second.end(); ++lit) {
                lit->signal = 2; // 强制迁移
                chk_move_map[lit->chk_id] = *lit;
            }
            std::list<chunk_bulk_res_t> res;
            m_csd_mgmt->find(it->first)->get_client()->chunk_move(res, it->second);

            // 检查返回的结果，调用数据库的接口修改相应的迁移chunk的src_id = dst_id
            for (auto rit = res.begin(); rit != res.end(); ++rit) {
                if (rit->res == 0) {
                    auto mit = chk_move_map.find(rit->chk_id);
                    if (mit != chk_move_map.end()) {
                        m_chk_mgmt->chunk_record_move(mit->second);
                    }
                }
            }

        }

    }
}

void EamouLayout::notify_mig() {
    if (!m_mig_chk.empty()) {
        // 调用csd_mgmt的grpc接口向csd发送通知迁移数据的命令
        for (auto it = m_mig_chk.begin(); it != m_mig_chk.end(); ++it) {
            std::map<uint64_t, chunk_move_attr_t> chk_move_map;

            for (auto lit = it->second.begin(); lit != it->second.end(); ++lit) {
                chk_move_map[lit->chk_id] = *lit;
            }
            std::list<chunk_bulk_res_t> res;
            m_csd_mgmt->find(it->first)->get_client()->chunk_move(res, it->second);

            // 检查返回的结果，调用数据库的接口修改相应的迁移chunk的dst_id
            for (auto rit = res.begin(); rit != res.end(); ++rit) {
                if (rit->res == 0) {
                    auto mit = chk_move_map.find(rit->chk_id);
                    if (mit != chk_move_map.end()) {
                        m_chk_mgmt->chunk_record_move(mit->second);
                    }
                }
            }            
        }

    }
}

void EamouLayout::init() {
    m_tw->push_cycle(std::shared_ptr<WorkEntry>(new MigWork(this)), m_cycle);
}

void MigWork::entry() {
    // 强制迁移上次计算出的迁移三元组信息
    m_dlayout->force_mig();

    // 判断当前是否磨损均衡,返回true说明磨损不均衡，需要进行数据迁移
    if (m_dlayout->judge_wear()) {
        int r;
        // 计算迁移的源设备、目标设备需要改变的写次数
        r = m_dlayout->migration_cal_wear();
        if (r != 0) return;

        // 获取迁移的源设备的k个写热点chunk信息
        r = m_dlayout->get_hot_chunk();
        if (r != 0) return;

        // 计算迁移的三元组信息
        r = m_dlayout->migration_cal_triple();
        if (r != 0) return;

        // 通知迁移
        m_dlayout->notify_mig();
    }
    
}