#ifndef FLAME_SPOLICY_EASY_H
#define FLAME_SPOLICY_EASY_H

#include "spolicy/spolicy.h"
#include "spolicy/sp_types.h"

namespace flame {
namespace spolicy {

class EasyVolumeDispatcher : public VolumeDispatcher {
public:
    EasyVolumeDispatcher(uint64_t vol_id, uint64_t vol_sz, uint64_t chk_sz)
    : VolumeDispatcher(vol_id, vol_sz) {}

}; // class EasyVolumeDispatcher

class EasyChunkDispatcher : public ChunkDispatcher {
public:
    EasyChunkDispatcher(chunk_id_t chk_id, uint64_t chk_sz)
    : ChunkDispatcher(chk_id) {}

}; // class EasyChunkDispatcher

class EasyStorePolicy : public StorePolicy {
public:
    EasyStorePolicy(uint64_t chk_sz)
    : chk_sz_(chk_sz) {}

    virtual int type() const override { return SP_BASE_EASY; }

    virtual uint64_t chk_size() const override { return chk_sz_; }

    virtual uint8_t cgn() const override { return 1; }

    virtual uint64_t cg_size() const override { return chk_sz_; }

    virtual VolumeDispatcher* create_volume_dispatcher(uint64_t vol_id, uint64_t vol_sz) const override {
        return new EasyVolumeDispatcher(vol_id, vol_sz, chk_sz_);
    }

    virtual ChunkDispatcher* create_chunk_dispatcher(chunk_id_t chk_id) const override {
        return new EasyChunkDispatcher(chk_id, chk_sz_);
    }

private:
    uint64_t chk_sz_;
}; // class EasyStorePolicy

} // namespace spolicy
} // namespace flame

#endif // FLAME_SPOLICY_EASY_H