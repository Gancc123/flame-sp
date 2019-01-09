#ifndef FLAME_INCLUDE_SPOLICY_H
#define FLAME_INCLUDE_SPOLICY_H

#include "include/meta.h"

namespace flame {

class StorePolicy {
public:
    virtual ~StorePolicy() {}

    /**
     * @brief 返回Chunk Group 大小
     * 返回的范围在1 ~ 15之间（包含）
     * @return int 
     */
    virtual int chunk_group_size() const = 0;

    

protected:
    StorePolicy() {}
}; // class StorePolicy

} // namespace flame 

#endif // !FLAME_INCLUDE_SPOLICY_H