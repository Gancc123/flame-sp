#ifndef FLAME_LAYOUT_CALCULATOR_H
#define FLAME_LAYOUT_CALCULATOR_H

#include "include/meta.h"

namespace flame {
namespace layout {

class CsdHealthCaculator {
public:
    virtual ~CsdHealthCaculator() {}

    /**
     * @brief 计算健康参数
     * 需要在更新完Period数据后调用
     * @param hlt 
     */
    virtual void cal_health(csd_health_meta_t& hlt);

    /**
     * @brief 计算负载权值
     * 
     * @param hlt 
     * @return double 
     */
    virtual double cal_load_weight(const csd_health_meta_t& hlt) = 0;

    /**
     * @brief 计算磨损权值
     * 
     * @param hlt 
     * @return double 
     */
    virtual double cal_wear_weight(const csd_health_meta_t& hlt) = 0;

    /**
     * @brief 计算总权值
     * 
     * @param hlt 
     * @return double 
     */
    virtual double cal_total_weight(const csd_health_meta_t& hlt) = 0;

protected:
    CsdHealthCaculator() {}
};

class ChunkHealthCaculator {
public:
    virtual ~ChunkHealthCaculator() {}

    /**
     * @brief 计算健康参数
     * 需要在更新完Period数据后调用
     * @param hlt 
     */
    virtual void cal_health(chunk_health_meta_t& hlt);

    /**
     * @brief 计算负载权值
     * 
     * @param hlt 
     * @return double 
     */
    virtual double cal_load_weight(const chunk_health_meta_t& hlt) = 0;

    /**
     * @brief 计算磨损权值
     * 
     * @param hlt 
     * @return double 
     */
    virtual double cal_wear_weight(const chunk_health_meta_t& hlt) = 0;

    /**
     * @brief 计算总权值
     * 
     * @param hlt 
     * @return double 
     */
    virtual double cal_total_weight(const chunk_health_meta_t& hlt) = 0;

protected:
    ChunkHealthCaculator() {}
};

} // namespace layout
} // namespace flame

#endif // FLAME_LAYOUT_CALCULATOR_H