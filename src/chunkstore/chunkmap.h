#ifndef CHUNKSTORE_CHUNKMAP_H
#define CHUNKSTORE_CHUNKMAP_H

#include <string>
#include <iostream>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <pthread.h>

#include "common/context.h"

#define CHUNKMAP_OP_SUCCESS 0x00

namespace flame{   
class Chunk;
class ChunkStore;

class ChunkMap {
public:
    ChunkMap() {

    }
    
    Chunk *get_chunk(const uint64_t chk_id);
    void insert_chunk(Chunk *chunk);
    void remove_chunk(const uint64_t chk_id);
    bool chunk_exist(const uint64_t chk_id);

    bool is_empty();
    int store();
    void load();
    void cleanup();

    virtual ~ChunkMap() {

    }
};
}


#endif