#include "BuddyAllocator.h"

#include "common/context.h"
#include "msg/msg_def.h"

#include <sstream>

#define FLAME_BUDDY_ALLOCAOTR_MIN_LEVEL     (4)
#define FLAME_BUDDY_ALLOCAOTR_MAX_LEVELS    (32)

namespace flame{
namespace ib{

BuddyAllocator *BuddyAllocator::create(FlameContext *c,  
                            uint8_t max_level, uint8_t min_level, MemSrc *src){
    if(!src) return nullptr;

    size_t levels = max_level - min_level + 1;

    if(max_level < min_level 
        || min_level < FLAME_BUDDY_ALLOCAOTR_MIN_LEVEL
        || levels > FLAME_BUDDY_ALLOCAOTR_MAX_LEVELS){
        ML(c, error, "min_level is too small or max_level is too large");
        return nullptr;
    }

    char *base = reinterpret_cast<char *>(src->alloc(1 << max_level));
    if(!base){
        ML(c, error, "mem_src->alloc( {}B ) failed! ", 1 << max_level);
        return nullptr;
    }

    size_t bm_size = 1 << levels;

    BitMap *bm = BitMap::create(bm_size);
    if(!bm){
        src->free(base);
        return nullptr;
    }

    BuddyAllocator *ap = new BuddyAllocator();
    if(!ap){
        src->free(base);
        delete bm;
        return nullptr;
    }

    ap->fct = c;
    ap->min_level = min_level;
    ap->max_level = max_level;
    ap->bit_map = bm;

    
    ap->free_list.resize(levels, nullptr);

    ap->base = base;
    ap->mem_free = 1 << max_level;

    ap->mem_src = src;

    //add top chunk
    ap->push_chunk_of_level(base, max_level);

    return ap;
}

BuddyAllocator::~BuddyAllocator(){
    delete bit_map;
    mem_src->free(base);
}

void *BuddyAllocator::alloc(size_t s){
    uint8_t level = find_level(s);
    if(level == 0) return nullptr;

    char *p = nullptr;
    uint8_t index = level - min_level;

    p = pop_chunk_of_level(level);
    if(p){
        bit_map->set(index_of_p(p, level), true);
    }else{
        //find the parent chunk.
        auto level_it = level + 1;
        char *par_p = nullptr;
        while(level_it <= max_level){
            par_p = pop_chunk_of_level(level_it);
            if(par_p) break;
            ++level_it;
        }

        if(!par_p) return nullptr; //no mem can alloc.

        // set the parent chunk used.
        bit_map->set(index_of_p(par_p, level_it), true);

        --level_it;
        while(level_it >= level){
            //use the left chunk 
            bit_map->set(index_of_p(par_p, level_it), true);
            //the right chunk will be pushed to freelist
            push_chunk_of_level(par_p + (1 << level_it) , level_it);
            --level_it;
        }
        p = par_p;
    }
    mem_free -= (1 << level);
    return mem_src->prep_mem_before_return(p, base, (1 << level));
}

void BuddyAllocator::free(void *pvoid, size_t s){
    if(!is_from(pvoid)) return;
    char *p = reinterpret_cast<char *>(pvoid);
    uint8_t level = 0;
    if(s){
        level = find_level(s);
    }else{
        level = find_level_for_free(p);
    }
    mem_free += (1 << level);

    while(level < max_level){
        size_t index = index_of_p(p, level);
        bit_map->set(index, false);
        if(index & 1){ // cur buddy is at left
            if(bit_map->at(index + 1)){
                break;
            }
            remove_chunk_of_level(p + (1 << level), level);
        }else{ // cur buddy is at right
            if(bit_map->at(index - 1)){
                break;
            }
            // cur buddy expands to its parent.
            p -= (1 << level);
            remove_chunk_of_level(p, level);
        }
        ++level;
    }
    
    push_chunk_of_level(p, level);
    bit_map->set(index_of_p(p, level), false);
}

std::string BuddyAllocator::get_stat() const{
    std::stringstream ss;
    size_t total = get_mem_total();
    size_t used = total - get_mem_free();
    ss << used << "/" << total << " " << used*100/total << "%: "; 
    for(uint8_t l = max_level;l >= min_level;--l){
        ss << "L" << (uint32_t)l << "(" << (1 << l) << "B)=" 
                                                << chunks_of_level(l) << " ";
    }
    return ss.str();
}

} // namespace ib
} // namespace flame