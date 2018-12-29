#ifndef FLAME_MSG_RDMA_MEM_POOL_H
#define FLAME_MSG_RDMA_MEM_POOL_H

#include <infiniband/verbs.h>
#include <boost/pool/pool.hpp>
#include <map>

#include "msg/msg_context.h"
#include "common/thread/mutex.h"
#include "Infiniband.h"
#include "msg/msg_common.h"

namespace flame{
namespace ib{


class MemPool;
class MemoryManager;
class ProtectionDomain;

class Chunk {
public:
    Chunk(uint32_t len);
    ~Chunk();

    void set_offset(uint32_t o);
    uint32_t get_offset();
    void set_bound(uint32_t b);
    void prepare_read(uint32_t b);
    uint32_t get_bound();
    uint32_t read(char* buf, uint32_t len);
    uint32_t write(char* buf, uint32_t len);
    bool full();
    bool over();
    void clear();

public:
    // !!! the lkey and bytes are setted when the mem_pool expand
    // But Boost::mem_pool (Simple Segregated Storage ) will use the first 
    // 64 bit memory for the next pointer in free list.
    // So, if lkey and bytes are at the start of the Chunk,
    // they will be overwrite by the pointer, which causes losing lkey.
    uint32_t bound = 0;
    uint32_t offset = 0;

    //!!!the order between above and below can't be changed.
    uint32_t lkey = 0;
    uint32_t bytes;
    char  data[0];
};

struct mem_info {
    ibv_mr   *mr;
    MemoryManager *mmgr;
    uint64_t nbufs;   
    // make payload_of_boost_pool(16B) + sizeof(mem_info)(24B + __pad)
    //  == sizeof(chunk) + sizeof(chunk.data) 
    //  == 4K
    //char __pad[4096 - 24 - 16]; 
    // when use  uint32_t nbufs, Chunks will start with 0x14 instead of 0x18
    Chunk    chunks[0]; 
};

class PoolAllocator{
public:
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    static char * malloc(const size_type bytes);
    static void free(char * const block);

    static Mutex m_lock;
    static MemoryManager *mmgr;
};

/**
 * modify boost pool so that it is possible to
 * have a thread safe 'context' when allocating/freeing
 * the memory. It is needed to allow a different pool
 * configurations and bookkeeping per MsgContext and
 * also to be able to use same allocator to deal with
 * RX and TX pool.
 */
class MemPool : public boost::pool<PoolAllocator>{
    MemoryManager *mmgr;
    void *slow_malloc();

public:
    explicit MemPool(MemoryManager *m, const size_type nrequested_size,
        const size_type nnext_size = 32,
        const size_type nmax_size = 0) 
    :pool(nrequested_size, nnext_size, nmax_size), mmgr(m) { }

    void *malloc() {
        if (!store().empty())
            return (store().malloc)();
        // need to alloc more memory...
        // slow path code
        return slow_malloc();
    }

};

class RdmaBufferAllocator;

class MemoryManager{
public:
    MemoryManager(MsgContext *c, ProtectionDomain *p);
    ~MemoryManager();

    void* malloc(size_t size);
    void  free(void *ptr);

    int get_buffers(size_t bytes, std::vector<Chunk*> &c);
    void release_buffers(std::vector<Chunk*> &c);

    bool is_from(Chunk *chunk){
        MutexLocker l(lock);
        return mem_pool.is_from(chunk);
    }

    Chunk *get_buffer() {
        MutexLocker l(lock);
        auto ch = reinterpret_cast<Chunk *>(mem_pool.malloc());
        if(ch){
            ch->clear(); // !!!clear the mem_pool's pointer.
        }
        return ch;
    }

    void release_buffer(Chunk *chunk) {
        if(!chunk) return;
        MutexLocker l(lock);
        mem_pool.free(chunk);
    }

    uint32_t get_buffer_size() const{
        return buffer_size;
    }

    ProtectionDomain *get_pd() const{
        return pd;
    }

    void update_stats(int nbufs){
        n_bufs_allocated += nbufs;
    }

    bool can_alloc(unsigned nbufs){
        auto msg_cfg = mct->config;
        if(msg_cfg->rdma_buffer_num <= 0){
            return true;
        } 
        if(msg_cfg->rdma_buffer_num < (n_bufs_allocated + nbufs)){
            ML(mct, error, 
                "rdma buffer number reach limit: {}, need: {}, allocted: {}", 
                msg_cfg->rdma_buffer_num, nbufs, n_bufs_allocated);
            return false;
        }
        return true;
    }

    RdmaBufferAllocator *get_rdma_allocator() const { 
        return rdma_buffer_allocator; 
    }

    MsgContext  *mct;
private:
    Mutex lock;
    ProtectionDomain *pd;
    MemPool mem_pool;
    unsigned n_bufs_allocated;
    std::map<void *, size_t> huge_page_map;
    const uint32_t buffer_size;
    /**
     * Not thread safe.
     * When used in slow_malloc, already take lock.
     */
    void* huge_pages_malloc(size_t size);
    void  huge_pages_free(void *ptr);

    static uint64_t calcu_init_space(MsgContext *c);

    RdmaBufferAllocator *rdma_buffer_allocator;
};

}
}


#endif 