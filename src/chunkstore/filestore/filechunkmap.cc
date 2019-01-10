#include "chunkstore/filestore/filechunkmap.h"
#include <inttypes.h>

using namespace flame;

bool FileChunkMap::is_empty() {
    return chunk_map.empty();
}

FileChunk* FileChunkMap::get_chunk(const uint64_t chk_id) {
    FileChunk *chunk_ptr = nullptr;
    pthread_rwlock_rdlock(&rwlock);
    std::map<uint64_t, FileChunk*>::iterator iter;
    iter = chunk_map.find(chk_id);
    if(iter != chunk_map.end()) {
        chunk_ptr = iter->second;
    }
    pthread_rwlock_unlock(&rwlock);

    return chunk_ptr;
}

void FileChunkMap::insert_chunk(FileChunk* chunk) {
    FileChunk *chunk_ptr = nullptr;
    pthread_rwlock_wrlock(&rwlock);
    chunk_map.emplace(chunk->get_chunk_id(), chunk);
    pthread_rwlock_unlock(&rwlock);
}

void FileChunkMap::remove_chunk(const uint64_t chk_id) {
    pthread_rwlock_wrlock(&rwlock);
    std::map<uint64_t, FileChunk*>::iterator iter;
    iter = chunk_map.find(chk_id);
    if(iter != chunk_map.end()) {
        chunk_map.erase(iter);
    }
    pthread_rwlock_unlock(&rwlock);
}

bool FileChunkMap::chunk_exist(const uint64_t chk_id) {
    bool ret = false;
    pthread_rwlock_rdlock(&rwlock);
    std::map<uint64_t, FileChunk*>::iterator iter;
    iter = chunk_map.find(chk_id);
    if(iter != chunk_map.end()) {
        ret = true;
    }
    pthread_rwlock_unlock(&rwlock);

    return ret;
}

size_t FileChunkMap::get_chunk_nums() {
    size_t nums;
    pthread_rwlock_rdlock(&rwlock);
    nums = chunk_map.size();
    pthread_rwlock_unlock(&rwlock);

    return nums;
}

FileChunk* FileChunkMap::get_chunk_by_efd(const int efd) {
    FileChunk* chunk = nullptr;
    pthread_rwlock_rdlock(&rwlock);
    std::map<uint64_t, FileChunk*>::iterator iter;
    for(iter = chunk_map.begin(); iter != chunk_map.end(); iter++) {
        if(efd == iter->second->get_efd()) {
            chunk = (iter->second);
        }
    }
    pthread_rwlock_unlock(&rwlock);

    return chunk;
}

FileChunk* FileChunkMap::get_active_chunk() {
    FileChunk *chunk = nullptr;
    pthread_rwlock_rdlock(&rwlock);
    std::map<uint64_t, FileChunk*>::iterator iter;
    iter = chunk_map.begin();
    if(iter != chunk_map.end()) {
        chunk = iter->second;
    }
    pthread_rwlock_unlock(&rwlock);

    return chunk;
}

void FileChunkMap::cleanup() {
    pthread_rwlock_wrlock(&rwlock);
    std::map<uint64_t, FileChunk*>::iterator iter;
    for(iter = chunk_map.begin(); iter != chunk_map.end(); iter++) {
        FileChunk *chunk = iter->second;
        if(chunk != nullptr) {
            delete chunk;
        }
    }

    chunk_map.erase(chunk_map.begin(), chunk_map.end());
    pthread_rwlock_unlock(&rwlock);
}

/*
*load & store已经被弃用了，没有任何操作会调用这两个函数
*/

int FileChunkMap::store(const char *store_path) {
    int ret = 0;
    char full_path[256];
    pthread_rwlock_wrlock(&rwlock);
    std::map<uint64_t, FileChunk*>::iterator iter;
    for(iter = chunk_map.begin(); iter != chunk_map.end(); iter++) {
        snprintf(full_path, sizeof(full_path), "%s/%" PRIx64, store_path, iter->first);
        ret = iter->second->store(full_path);
        if(ret != CHUNK_OP_SUCCESS)
            break;
    }
    pthread_rwlock_unlock(&rwlock);

    return ret;
}

int FileChunkMap::load(const char *load_path) {
    int ret = 0;
    DIR *dir;
    struct dirent *entry;
    char file_name[512];
    struct stat st;
    dir = opendir(load_path);

    pthread_rwlock_wrlock(&rwlock);
    while((entry = readdir(dir)) != NULL) {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        snprintf(file_name, sizeof(file_name), "%s/%s", load_path, entry->d_name);
        stat(file_name, &st);

        if(S_ISDIR(st.st_mode))
            continue;
        else {
            FileChunk *chunk = new FileChunk(filestore, filestore->get_flame_context());
            ret = chunk->load(load_path);
            if(ret != CHUNK_OP_SUCCESS) {
                break;
            }
            insert_chunk(chunk);
        }
    }
    pthread_rwlock_unlock(&rwlock);

    return ret;
}