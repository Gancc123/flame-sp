#include "chunkstore/nvmestore/nvmechunkmap.h"

using namespace flame;

void NvmeChunkMap::insert_chunk(NvmeChunk *chunk) {
    std::map<uint64_t, NvmeChunk*>::iterator iter;
    chunk_map.emplace(chunk->get_chunk_id(), chunk);
}

void NvmeChunkMap::remove_chunk(const uint64_t chk_id) {
    std::map<uint64_t, NvmeChunk*>::iterator iter;
    iter = chunk_map.find(chk_id);
    if(iter != chunk_map.end()) {
        chunk_map.erase(iter, chunk_map.end());
    }
}

bool NvmeChunkMap::chunk_exist(const uint64_t chk_id) {
    std::map<uint64_t, NvmeChunk*>::iterator iter;
    for(iter = chunk_map.begin(); iter != chunk_map.end(); iter++) {
        if(iter->first == chk_id) {
            return true;
        }
    }
    
    return false;
}

NvmeChunk* NvmeChunkMap::get_chunk(const uint64_t chk_id) {
    std::map<uint64_t, NvmeChunk*>::iterator iter;
    iter = chunk_map.find(chk_id);
    if(iter != chunk_map.end()) {
        return iter->second;
    }

    return nullptr;
}

bool NvmeChunkMap::is_empty() {
    return chunk_map.empty();
}

uint64_t NvmeChunkMap::get_active_nums() const {
    return chunk_map.size();
}

//该函数目前被弃用
int NvmeChunkMap::store() {

}

//该函数目前被弃用
int NvmeChunkMap::load() {

}

void NvmeChunkMap::cleanup() {
    std::map<uint64_t, NvmeChunk*>::iterator iter;
    for(iter = chunk_map.begin(); iter != chunk_map.end(); iter++) {
        NvmeChunk *chunk = iter->second;
        if(chunk != nullptr) {
            delete chunk;
        }
    }
    
    chunk_map.erase(chunk_map.begin(), chunk_map.end());
}