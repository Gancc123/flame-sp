#include "spolicy/spolicy.h"
#include "spolicy/sp_types.h"

#include "spolicy/sp_easy.h"

namespace flame {
namespace spolicy {

StorePolicy* create_spolicy(int sp_type) {
    switch (sp_type) {
    case SP_TYPE_EASY_SM:
        return new EasyStorePolicy(1ULL << 30);
    case SP_TYPE_EASY_MD:
        return new EasyStorePolicy(4ULL << 30);
    case SP_TYPE_EASY_BG:
        return new EasyStorePolicy(16ULL << 30);
    default:
        break;
    }
    return nullptr;
}

void create_spolicy_bulk(std::vector<StorePolicy*>& sp_tlb_) {
    // 0: SP_TYPE_EASY_SM
    sp_tlb_.push_back(new EasyStorePolicy(1ULL << 30));

    // 1: SP_TYPE_EASY_MD
    sp_tlb_.push_back(new EasyStorePolicy(4ULL << 30));

    // 2: SP_TYPE_EASY_BG
    sp_tlb_.push_back(new EasyStorePolicy(16ULL << 30));

    sp_tlb_.resize(3);
}

} // namespace spolicy
} // namespace flame