#ifndef FLAME_MSG_INTERNAL_BITMAP_H
#define FLAME_MSG_INTERNAL_BITMAP_H

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace flame{
namespace msg{

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

} //namespace msg
} //namespace flame

#endif //FLAME_MSG_INTERNAL_BITMAP_H