#ifndef FLAME_MEMZONE_STDMZ_H
#define FLAME_MEMZONE_STDMZ_H

#include "include/buffer.h"
#include "memzone/mz_types.h"

#include <cstdlib>

namespace flame {

class StdAllocator;

class StdBufferPtr : public BufferPtr {
public:
    virtual ~StdBufferPtr() { free(addr_); }

    virtual uint8_t* addr() const override { return addr_; }

    virtual size_t size() const override { return len_; }

    virtual int type() const { return MZ_TYPE_STD; }

    virtual bool resize(size_t sz) override {
        addr_ = realloc(addr_, sz);
        len_ = sz;
        return true;
    }

    friend class StdAllocator;

private:
    StdBufferPtr(uint8_t* addr, size_t sz)
    : addr_(addr), len_(sz) {}

    uint8_t* addr_;
    size_t len_;
}; // class StdBufferPtr

class StdAllocator : public BufferAllocator {
public:
    StdAllocator() {}

    virtual size_t max_size() const override { return UINT64_MAX; }

    virtual Buffer allocate(size_t sz) override {
        void* addr = malloc(sz);
        return addr == nullptr ? Buffer() 
            : Buffer(std::shared_ptr<BufferPtr>(new StdBufferPtr(addr, sz)));
    }

}; // class StdAllocator

} // namespace flame

#endif // FLAME_MEMZONE_STDMZ_H