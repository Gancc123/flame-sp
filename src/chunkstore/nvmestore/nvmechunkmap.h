#ifndef CHUNKSTORE_NVMECHUNKMAP_H
#define CHUNKSTORE_NVMECHUNKMAP_H

#include "chunkstore/chunkmap.h"
#include "chunkstore/nvmestore/nvmestore.h"

namespace flame {
class NvmeStore;
class NvmeChunk;

class NvmeChunkMap : public ChunkMap {
private:
    /**NvmeChunkMap的主要工作是保存打开Nvmechunk的句柄信息，而对于NVMeChunk的句柄信息的操作基本上就是我们常说的元数据操作，
     * 而对于元数据操作，在NvmeStore中，必定只会被一个admin线程处理，所以不会出现数据结构争用的情况
     * 所以所有的锁操作，基本上都是不必要的。
    */ 
    pthread_mutex_t mutex;      //在NvmeChunkMap中不需要
    pthread_cond_t cond;        //在NvmeChunkMap中不需要
    pthread_rwlock_t rwlock;    //在NvmeChunkMap中不需要

    NvmeStore *nvmestore;
    //map中存储的是NvmeChunk的指针，最好是让NvmeChunk可以分配到特定的socket上
    std::map<uint64_t, NvmeChunk*> chunk_map;
    
public:
    NvmeChunkMap(NvmeStore *_store):ChunkMap(), nvmestore(_store) {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
        pthread_rwlock_init(&rwlock, NULL);
    }

    NvmeChunk *get_chunk(const uint64_t chk_id);
    uint64_t get_active_nums() const;
    void insert_chunk(NvmeChunk* chunk);
    void remove_chunk(const uint64_t chk_id);
    bool chunk_exist(const uint64_t chk_id);
    bool is_empty();

    int store();
    int load();

    void cleanup();

    virtual ~NvmeChunkMap() {
        cleanup();
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        pthread_rwlock_destroy(&rwlock);
    }
};

}

#endif