#include "layout/calculator.h"

namespace flame {
namespace layout {

void CsdHealthCaculator::cal_health(csd_health_meta_t& hlt) {
    hlt.grand.wr_cnt += hlt.period.wr_cnt;
    hlt.grand.rd_cnt += hlt.period.rd_cnt;
    hlt.weight.w_load = cal_load_weight(hlt);
    hlt.weight.w_wear = cal_wear_weight(hlt);
    hlt.weight.w_total = cal_total_weight(hlt);
}

void ChunkHealthCaculator::cal_health(chunk_health_meta_t& hlt) {
    hlt.grand.wr_cnt += hlt.period.wr_cnt;
    hlt.grand.rd_cnt += hlt.period.rd_cnt;
    hlt.weight.w_load = cal_load_weight(hlt);
    hlt.weight.w_wear = cal_wear_weight(hlt);
    hlt.weight.w_total = cal_total_weight(hlt);
}

} // namespace layout
} // namespace flame