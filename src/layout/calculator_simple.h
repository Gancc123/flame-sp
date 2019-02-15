#ifndef FLAME_LAYOUT_CALCULATOR_SIMPLE_H
#define FLAME_LAYOUT_CALCULATOR_SIMPLE_H

#include "calculator.h"

namespace flame  {
namespace layout {

class SimpleCsdHealthCalculator : public CsdHealthCaculator {
public:
    virtual double cal_load_weight(const csd_health_meta_t& hlt) override {
        return  hlt.weight.w_load + hlt.period.wr_cnt;
    }

    virtual double cal_wear_weight(const csd_health_meta_t& hlt) override {
        return hlt.weight.w_wear + hlt.period.wr_cnt;
    }

    virtual double cal_total_weight(const csd_health_meta_t& hlt) override {
        return hlt.weight.w_total + hlt.period.wr_cnt;
    }

};

class SimpleChunkHealthCalulator : public ChunkHealthCaculator {
public:
    virtual double cal_load_weight(const chunk_health_meta_t& hlt) override {
        return hlt.weight.w_load / 2 + hlt.period.wr_cnt;
    }

    virtual double cal_wear_weight(const chunk_health_meta_t& hlt) override {
        return hlt.weight.w_wear / 2 + hlt.period.wr_cnt;
    }

    virtual double cal_total_weight(const chunk_health_meta_t& hlt) override {
        return hlt.weight.w_total / 2 + hlt.period.wr_cnt;
    }
};

} // namespace layout
} // namespace flame
 
#endif // FLAME_LAYOUT_CALCULATOR_SIMPLE_H