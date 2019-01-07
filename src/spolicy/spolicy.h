#ifndef FLAME_SPOLICY_H
#define FLAME_SPOLICY_H

#include "include/meta.h"
#include "include/buffer.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace flame {
namespace spolicy {

class VolumeDispatcher {
public:
    virtual ~VolumeDispatcher() {}

protected:
    VolumeDispatcher(uint64_t vol_id, uint64_t vol_sz)
    : vol_id_(vol_id), vol_sz_(vol_sz) {}

    uint64_t vol_id_;
    uint64_t vol_sz_;
}; // class VolumeDispatcher

class ChunkDispatcher {
public:
    virtual ~ChunkDispatcher() {}

protected:
    ChunkDispatcher(chunk_id_t chk_id)
    : chk_id_(chk_id) {}

    chunk_id_t chk_id_;
}; // class ChunkDispatcher

/**
 * StorePolicy
 * @brief 存储策略主接口
 * 
 */
class StorePolicy {
public:
    virtual ~StorePolicy() {}

    /**
     * Layout Part
     */
    virtual int type() const = 0;
    
    /**
     * @brief Chunk Size
     * Chunk 大小
     * @return uint64_t 
     */
    virtual uint64_t chk_size() const = 0;

    /**
     * @brief Chunk Group Number
     * 每个CG包含的Chunk个数
     * @return uint8_t [1, 16]
     */
    virtual uint8_t cgn() const = 0;

    /**
     * @brief 每组CG的有效容量
     * 
     * @return uint64_t 
     */
    virtual uint64_t cg_size() const = 0;

    /**
     * @brief Create a volume dispatcher object
     * 
     * @param vol_id 
     * @param vol_sz 
     * @param chk_sz 
     * @return VolumeDispatcher* 
     */
    virtual VolumeDispatcher* create_volume_dispatcher(uint64_t vol_id, uint64_t vol_sz) const = 0;

    /**
     * @brief Create a chunk dispatcher object
     * 
     * @param chk_id 
     * @param chk_sz 
     * @return ChunkDispatcher* 
     */
    virtual ChunkDispatcher* create_chunk_dispatcher(chunk_id_t chk_id) const = 0;

protected:
    StorePolicy() {}
}; // class StorePolicy

StorePolicy* create_spolicy(int sp_type);
void create_spolicy_bulk(std::vector<StorePolicy*>& sp_tlb_);

} // namespace spolicy
} // namespace flame

#endif // !FLAME_SPOLICY_H