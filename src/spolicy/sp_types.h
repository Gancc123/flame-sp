#ifndef FLAME_SPOLICY_TYPES_H
#define FLAME_SPOLICY_TYPES_H

namespace flame {
namespace spolicy {

enum SPolicyBaseTypes {
    /**
     * @brief easy 基础策略
     * 单副本
     */
    SP_BASE_EASY = 0
}; // enum SPolicyBaseTypes

enum SPolicyTypes {
    /**
     * @brief easy存储策略
     * 单副本
     * suffix:  sm  md  bg
     * chk_sz:  1G  4G  16G
     */
    SP_TYPE_EASY_SM = 0,
    SP_TYPE_EASY_MD = 1,
    SP_TYPE_EASY_BG = 2
}; // enum SPolicyTypes

} // namespace spolicy
} // namespace flame

#endif // FLAME_SPOLICY_TYPES_H