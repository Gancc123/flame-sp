#ifndef CHUNKSTORE_FILECHUNKMAP_H
#define CHUNKSTORE_FILECHUNKMAP_H

#include "chunkstore/chunkmap.h"
#include "chunkstore/filestore/filestore.h"

namespace flame {
class FileStore;
class FileChunk;

class FileChunkMap : public ChunkMap {
private:
    //对于FileChunkMap来说，各类操作可能在不同的线程中完成，所以需要使用读写锁来同步，
    //之后如果实现线程绑定，使用一个线程来处理admin命令，则可以去掉锁
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_rwlock_t rwlock;
    FileStore *filestore;
    std::map<uint64_t, FileChunk*> chunk_map;

public:
    FileChunkMap(FileStore *_store):ChunkMap(), filestore(_store) {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
        pthread_rwlock_init(&rwlock, NULL);
    }

    FileChunk *get_chunk(const uint64_t chk_id);
    void insert_chunk(FileChunk* chunk);
    void remove_chunk(const uint64_t chk_id);
    bool chunk_exist(const uint64_t chk_id);

    size_t get_chunk_nums();
    FileChunk *get_chunk_by_efd(const int efd);
    FileChunk *get_active_chunk();
    

    bool is_empty();
    int  store(const char *store_path);
    int  load(const char *load_path);
    void cleanup();

    virtual ~FileChunkMap() {
        cleanup();
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        pthread_rwlock_destroy(&rwlock);
    }
};

}

#endif