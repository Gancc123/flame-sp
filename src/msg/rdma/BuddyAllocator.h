#ifndef FLAME_MSG_RDMA_BUDDY_ALLOCATOR_H
#define FLAME_MSG_RDMA_BUDDY_ALLOCATOR_H

#include <vector>
#include <string>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#define PA(p) (reinterpret_cast<char *>(p))

namespace flame{

class MsgContext;

namespace ib{

struct chunk_header_t{
    struct chunk_header_t *prev;
    struct chunk_header_t *next;
};

class BitMap{
    char *data;
    size_t data_len;
    size_t m_size;
    BitMap() {}
public:
    static BitMap *create(size_t s){
        size_t data_len = (s + 7) >> 3;
        char *data = new char[data_len];
        if(!data) {
            return nullptr;
        }

        memset(data, 0, sizeof(char)*data_len);

        BitMap *bm = new BitMap();
        if(!bm){
            delete [] data;
            return nullptr;
        }

        bm->data = data;
        bm->data_len = data_len;
        bm->m_size = s;
        return bm;
    }

    ~BitMap(){
        if(data){
            delete [] data;
            data = nullptr;
        }
    }

    bool at(size_t index) const {
        size_t byte_index = index >> 3;
        if(byte_index >= bytes()) return false;
        return (bool)((data[byte_index] >> (index & 0x7)) & 1);
    }

    void set(size_t index, bool val){
        size_t byte_index = index >> 3;
        if(byte_index >= bytes()) return;
        if(val){
            data[byte_index] |= (1 << (index & 0x7));
        }else{
            data[byte_index] &= ~(1 << (index & 0x7));
        }
    }

    size_t size() const {
        return m_size;
    }

    size_t bytes() const{
        return data_len;
    }

};

class MemSrc{
public:
    virtual void *alloc(size_t s) = 0;
    virtual void free(void *p) = 0;
    virtual void *prep_mem_before_return(void *p, void *base, size_t size) = 0;
};

class BuddyAllocator{
    MsgContext *mct;
    uint8_t min_level;
    uint8_t max_level;
    size_t mem_free;
    BitMap *bit_map = nullptr;
    std::vector<struct chunk_header_t *> free_list;
    char *base = nullptr;
    MemSrc *mem_src = nullptr;
    BuddyAllocator() {}
public:
    static BuddyAllocator *create(MsgContext *c, uint8_t max_level, 
                                                uint8_t min_level, MemSrc *src);
    ~BuddyAllocator();

    void *alloc(size_t s);

    void free(void *p, size_t s=0);

    bool is_from(void *p){
        return ((PA(p) >= base) && (PA(p) < base + (1 << max_level)));
    }

    size_t get_mem_used() const { return (1 << max_level) - mem_free; }
    size_t get_mem_free() const { return mem_free; }
    size_t get_mem_total() const { return (1 << max_level); }
    std::string get_stat() const;

    uint8_t get_min_level() const { return min_level; }
    uint8_t get_max_level() const { return max_level; }
    MemSrc *get_mem_src() const { return mem_src; }

    void *extra_data = nullptr;

private:

    uint8_t find_level(size_t s){
        uint8_t level = min_level;
        while(level <= max_level){
            if((1 << level) >= s){
                return level;
            }
            ++level;
        }
        return 0;
    }

    size_t index_in_level_of(void *p, uint8_t level){
        return (PA(p) - base) / (1 << level);
    }

    size_t index_of_p(void *p, uint8_t level){
        return (1 << (max_level - level)) - 1 + index_in_level_of(p, level);
    }

    char *pop_chunk_of_level(uint8_t level){
        uint8_t index = level - min_level;
        auto p_chunk = free_list[index];
        if(!p_chunk) return nullptr;
        if(nullptr == p_chunk->next){
            free_list[index] = nullptr;
        }else{
            free_list[index] = p_chunk->next;
            free_list[index]->prev = nullptr;
        }
        return reinterpret_cast<char *>(p_chunk);
    }

    void remove_chunk_of_level(void *p, uint8_t level){
        assert(p != nullptr);
        uint8_t index = level - min_level;
        auto p_chunk = free_list[index];
        chunk_header_t *tgt_chunk = reinterpret_cast<chunk_header_t *>(p);
        if(p_chunk == tgt_chunk){
            free_list[index] = tgt_chunk->next;
            if(free_list[index]){
                free_list[index]->prev = nullptr;
            }
        }else{
            if(tgt_chunk->prev){
                tgt_chunk->prev->next = tgt_chunk->next;
            }
            if(tgt_chunk->next){
                tgt_chunk->next->prev = tgt_chunk->prev;
            }
        }
    }

    void push_chunk_of_level(void *p, uint8_t level){
        assert(p != nullptr);
        uint8_t index = level - min_level;
        auto p_chunk = free_list[index];
        chunk_header_t *new_chunk = reinterpret_cast<chunk_header_t *>(p);
        if(p_chunk){
            new_chunk->next = p_chunk;
            new_chunk->prev = p_chunk->prev;
            p_chunk->prev = new_chunk;
        }else{
            new_chunk->next = nullptr;
            new_chunk->prev = nullptr;
        }
        free_list[index] = new_chunk;
    }

    size_t chunks_of_level(uint8_t level) const {
        uint8_t index = level - min_level;
        size_t r = 0;
        chunk_header_t *it = free_list[index];
        while(it){
            ++r;
            it = it->next;
        }
        return r;
    }

    uint8_t find_level_for_free(void *p){
        uint8_t level = min_level;
        while(level <= max_level){
            if(bit_map->at(index_of_p(p, level))){
                return level;
            }
            ++level;
        }
        assert(level <= max_level); // invalid p!
    }

};

} //namespace ib
} //namespace flame

#undef PA

#endif //FLAME_MSG_RDMA_BUDDY_ALLOCATOR_H