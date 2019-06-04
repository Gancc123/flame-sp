#ifndef FLAME_MEMORY_RDMA_RDMA_MEM_H
#define FLAME_MEMORY_RDMA_RDMA_MEM_H

#include "memzone/BuddyAllocator.h"
#include "memzone/LockFreeList.h"
#include "common/thread/mutex.h"
#include "msg/msg_common.h"

#include <cassert>
#include <map>
#include <infiniband/verbs.h>

namespace flame{
namespace memory{
namespace ib{

class MemoryManager;

struct rdma_mem_header_t{
    uint32_t lkey;
    uint32_t rkey;
    size_t   size;
};

class RdmaBuffer{
    uint32_t m_lkey;
    uint32_t m_rkey;
    size_t   m_size;
    char * m_buffer;
    BuddyAllocator *m_allocator;
public:
    explicit RdmaBuffer(uint32_t rkey, uint64_t addr, size_t len)
    :m_rkey(rkey), m_buffer(reinterpret_cast<char *>(addr)), data_len(len),
      m_allocator(nullptr) {};
    explicit RdmaBuffer(void *ptr, BuddyAllocator *a);
    uint32_t lkey() const { return m_lkey; }
    uint32_t rkey() const { return m_rkey; }
    size_t   size() const { return m_size; }
    char *   buffer() const { return m_buffer; }
    uint64_t addr() const { return reinterpret_cast<uint64_t>(m_buffer); }
    BuddyAllocator *allocator() const { return m_allocator; }

    void   set_rkey(uint32_t rkey) { m_rkey = rkey; }
    void   set_addr(uint64_t addr) { m_buffer = reinterpret_cast<char *>(addr);}
    size_t data_len = 0;
};

class RdmaMemSrc : public MemSrc{
    MemoryManager *mmgr;
    ibv_mr *mr;
public:
    explicit RdmaMemSrc(MemoryManager *m) : mmgr(m) { assert(mmgr); }
    virtual void *alloc(size_t s) override;
    virtual void free(void *p) override;
    virtual void *prep_mem_before_return(void *p, void *base, size_t size) 
                                                                    override;
};

class RdmaBufferAllocator{
    MemoryManager *mmgr;
    LockFreeList lfl_allocators;
    uint8_t min_level;
    uint8_t max_level;
    static Mutex &mutex_of_allocator(BuddyAllocator *a){
        return *(reinterpret_cast<Mutex *>(a->extra_data));
    }
    static void delete_cb(void *p){
        BuddyAllocator *a = reinterpret_cast<BuddyAllocator *>(p);
        auto mem_src = a->get_mem_src();
        delete a;
        delete mem_src;
    }
    int expand();
    // size_t shrink();
public:
    explicit RdmaBufferAllocator(MemoryManager *m) 
    : mmgr(m), lfl_allocators(RdmaBufferAllocator::delete_cb) { 
        assert(mmgr); 
    }

    int init();
    int fin();

    RdmaBuffer *alloc(size_t s);
    void        free(RdmaBuffer *buf);

    int  alloc_buffers(size_t s, int cnt, std::vector<RdmaBuffer*> &b);
    void free_buffers(std::vector<RdmaBuffer*> &b);

    size_t get_mem_used() const;
    size_t get_mem_reged() const;
    int get_mr_num() const;
};

} //namespace ib
} //namespace memory
} //namespace flame

#endif //FLAME_MSG_RDMA_RDMA_MEM_H