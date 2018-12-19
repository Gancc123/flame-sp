#ifndef FLAME_MSG_INTERNAL_BUFFER_LIST_H
#define FLAME_MSG_INTERNAL_BUFFER_LIST_H

#include "msg_buffer.h"
#include <cstring>
#include <cassert>
#include <list>
#include <utility>
#include <algorithm>
#include <iterator>

#define FLAME_BUFFER_LIST_UNIT_SIZE 1024

namespace flame{

class BufferList{
    std::list<MsgBuffer> m_buffer_list;
    uint64_t len;
public:
    explicit BufferList() 
    :len(0){}

    explicit BufferList(BufferList &&o) noexcept
    :m_buffer_list(std::move(o.m_buffer_list)) {
        len = o.len;
        o.len = 0;
    }

    void swap(BufferList &other) noexcept{
        m_buffer_list.swap(other.m_buffer_list);
        std::swap(len, other.len);
    }

    uint64_t splice(BufferList &other){
        this->len += other.len;
        m_buffer_list.splice(m_buffer_list.end(), other.m_buffer_list);
        auto tmp_len = other.len;
        other.len = 0;
        return tmp_len;
    }

    int append_nocp(MsgBuffer &&buf){
        this->len += buf.offset();
        m_buffer_list.push_back(std::move(buf));
        return buf.offset();
    }

    int append_nocp(void *buf, int l){
        if(l < 0) return 0;
        char *buffer = (char *)buf;
        MsgBuffer b(buffer, l);
        b.set_offset(l);
        m_buffer_list.push_back(std::move(b));
        this->len += l;
        return l;
    }

    int append(void *b, int l){
        char *buf = (char *)b;
        int total_len = 0, cb_len = 0;
        auto data_buf = cur_data_buffer(cb_len, l - total_len);
        while(total_len < l && data_buf != nullptr){
            if(cb_len > l - total_len){
                cb_len = l - total_len;
            }
            std::memcpy(data_buf, buf + total_len, cb_len);
            total_len += cb_len;
            cur_data_buffer_extend(cb_len);
            if(total_len >= l) break;
            data_buf = cur_data_buffer(cb_len, l - total_len);
        }
        return total_len;
    }

    int append(MsgBuffer &buf){
        return append(buf.data(), buf.offset());
    }

    void clear(){
        m_buffer_list.clear();
        len = 0;
    }

    uint64_t length() const{
        return len;
    }

    int buffer_num() const{
        return m_buffer_list.size();
    }

    std::_List_iterator<MsgBuffer> list_begin(){
        return m_buffer_list.begin();
    }

    std::_List_iterator<MsgBuffer> list_end(){
        return m_buffer_list.end();
    }

    char *cur_data_buffer(int &cb_len, size_t new_buffer_len=1024){
        if(m_buffer_list.empty()){
            if(new_buffer_len <= 0){
                return nullptr;
            }
            m_buffer_list.push_back(MsgBuffer(new_buffer_len));
        }
        MsgBuffer *buf = &(m_buffer_list.back());
        if(buf->offset() == buf->length()){
            if(new_buffer_len <= 0){
                return nullptr;
            }
            m_buffer_list.push_back(MsgBuffer(new_buffer_len));
            buf = &(m_buffer_list.back());
        }
        cb_len = buf->length() - buf->offset();
        return buf->data() + buf->offset();
    }

    int cur_data_buffer_extend(int ex_len){
        if(m_buffer_list.empty()){
            return 0;
        }
        MsgBuffer &buf = m_buffer_list.back();
        int r = buf.advance(ex_len);
        this->len += r;
        return r;
    }

    class iterator : public std::iterator< std::forward_iterator_tag, char>{
        BufferList *m_bl;
        std::_List_iterator<MsgBuffer> m_it;
        size_t m_cur_off;
    public:
        explicit iterator()
        :m_bl(nullptr), m_cur_off(0){

        }
        explicit iterator(BufferList *bl, bool is_end)
        :m_bl(bl){
            m_cur_off = 0;
            if(!is_end){
                m_it = m_bl->m_buffer_list.begin();
            }else{
                m_it = m_bl->m_buffer_list.end();
            }
        }

        void swap(iterator& other) noexcept{
            using std::swap;
            swap(m_bl, other.m_bl);
            swap(m_it, other.m_it);
            swap(m_cur_off, other.m_cur_off);
        }

        int64_t advance(int64_t len){
            int64_t cur_len = 0;
            if(len > 0){
                while(cur_len < len && m_it != m_bl->m_buffer_list.end()){
                    MsgBuffer &cur_buf = *m_it;
                    if(cur_buf.offset() - m_cur_off <= len - cur_len){
                        cur_len += cur_buf.offset() - m_cur_off;
                        ++m_it;
                        m_cur_off = 0;
                    }else{
                        m_cur_off += (len - cur_len);
                        cur_len = len;
                    }
                }
            }else if(len < 0){
                while(cur_len > len && m_it != m_bl->m_buffer_list.begin()){
                    MsgBuffer &cur_buf = *m_it;
                    if(m_cur_off < cur_len - len){
                        cur_len -= (m_cur_off + 1);
                        --m_it;
                        m_cur_off = (*m_it).offset() - 1;
                    }else{
                        m_cur_off -= (cur_len - len);
                        cur_len = len;
                    }
                }
                if(cur_len > len){
                    // now, m_it is at the first buffer.
                    cur_len -= m_cur_off;
                    m_cur_off = 0;
                }
            }
            return cur_len;
        }

        char *cur_data_buffer(int &cd_len){
            if(m_it == m_bl->m_buffer_list.end()){
                return nullptr;
            }
            MsgBuffer &cur_buf = *m_it;
            cd_len = cur_buf.offset() - m_cur_off;
            if(cd_len == 0){
                ++m_it;
                m_cur_off = 0;
                return nullptr;
            }
            return cur_buf.data() + m_cur_off;
        }

        int cur_data_buffer_extend(int ex_len){
            if(m_it == m_bl->m_buffer_list.end() || ex_len <= 0){
                return 0;
            }
            MsgBuffer &cur_buf = *m_it;
            if(ex_len >= cur_buf.offset() - m_cur_off){
                ex_len = cur_buf.offset() - m_cur_off;
                ++m_it;
                m_cur_off = 0;
            }else{
                m_cur_off += ex_len;
            }
            return ex_len;
        }

        iterator& operator++ (){
            if(m_it == m_bl->m_buffer_list.end()) return *this;
            this->advance(1);
            return *this;
        }

        iterator operator++ (int){
            if(m_it == m_bl->m_buffer_list.end()) return *this;
            iterator tmp = *this;
            this->advance(1);
            return tmp;
        }

        bool operator == (const iterator &o) const{
            if(m_it == o.m_it){
                if(m_cur_off == o.m_cur_off
                    || m_it == m_bl->m_buffer_list.end()){
                    return true;
                }
            }
            return false;
        }

        bool operator != (const iterator &o) const{
            return !(*this == o);
        }

        char& operator* () {
            if(m_it == m_bl->m_buffer_list.end()){
                throw "Out of range!";
            } 
            return (*m_it).data()[m_cur_off];
        }

        char& operator-> () {
            if(m_it == m_bl->m_buffer_list.end()){
                throw "Out of range!";
            } 
            return (*m_it).data()[m_cur_off];
        }

        int copy(void *b, int64_t len){
            char *buf = (char *)b;
            int64_t total_len = 0;
            int cb_len = 0;
            auto data_buf = cur_data_buffer(cb_len);
            while(total_len < len && data_buf != nullptr){
                if(cb_len > len - total_len){
                    cb_len = len - total_len;
                }
                std::memcpy(buf + total_len, data_buf, cb_len);
                cur_data_buffer_extend(cb_len);
                total_len += cb_len;

                data_buf = cur_data_buffer(cb_len);
            }
            return total_len;
        }

        int copy(MsgBuffer &buf){
            return copy(buf.data() + buf.offset(),
                        buf.length() - buf.offset());
        }

    };

    iterator begin() noexcept{
        return iterator(this, false);
    }

    iterator end() noexcept{
        return iterator(this, true);
    }

};

class FixedBufferList{
    std::list<MsgBuffer> m_buffer_list;
    std::_List_iterator<MsgBuffer> end_it;
    int off;
    int len;
    int unit_size;

    void append_data_buffer(){
        MsgBuffer buffer(unit_size);
        buffer.set_offset(buffer.length());
        m_buffer_list.push_back(std::move(buffer));
    }

    void prepend_data_buffer(){
        MsgBuffer buffer(unit_size);
        buffer.set_offset(buffer.length());
        m_buffer_list.push_front(std::move(buffer));
    }

    inline void append_byte(char byte){
        int end_off = (off + len) % unit_size;
        if(end_off == 0){
            ++end_it;
            if(end_it == m_buffer_list.end()){
                append_data_buffer();
                --end_it;
            }
        }
        (*end_it)[end_off] = byte;
        ++len;
    }

    inline void prepend_byte(char byte){
        if(off == 0){
            prepend_data_buffer();
            off = unit_size - 1;
        }else{
            --off;
        }
        m_buffer_list.front()[off] = byte;
        ++len;
    }

public:
    explicit FixedBufferList(int unit_size) 
    :unit_size(unit_size), off(0), len(0){
        end_it = m_buffer_list.begin();
    };

    explicit FixedBufferList() : FixedBufferList(FLAME_BUFFER_LIST_UNIT_SIZE){};

    explicit FixedBufferList(FixedBufferList &&o) noexcept{
        m_buffer_list.swap(o.m_buffer_list);
        o.m_buffer_list.clear();
        off = o.off;
        o.off = 0;
        len = o.len;
        o.len = 0;
        unit_size = o.unit_size;
        end_it = o.end_it;
        o.end_it = o.m_buffer_list.begin();
    }

    void swap(FixedBufferList &other) noexcept{
        m_buffer_list.swap(other.m_buffer_list);
        std::swap(end_it, other.end_it);
        std::swap(off, other.off);
        std::swap(len, other.len);
        std::swap(unit_size, other.unit_size);
        std::swap(end_it, other.end_it);
    }

    int prepend(const void *buf, int len){
        char *buffer = (char *)buf;
        for(int i = len - 1;i >= 0;--i){
            prepend_byte(buffer[i]);
        }
        return len;
    }

    int prepend(MsgBuffer &buffer){
        for(int i = buffer.offset() - 1;i >= 0;--i){
            prepend_byte(buffer[i]);
        }
        return buffer.offset();
    }

    int append(const void *buf, int len){
        char *buffer = (char *)buf;
        for(int i = 0;i < len;++i){
            append_byte(buffer[i]);
        }
        return len;
    }

    int append(MsgBuffer &buffer){
        for(int i = 0;i < buffer.offset();++i){
            append_byte(buffer[i]);
        }
        return buffer.offset();
    }

    int clear(){
        m_buffer_list.clear();
        off = 0;
        len = 0;
    }

    int length() const{
        return len;
    }

    char *cur_data_buffer(int &cb_len){
        int end_off = (off + len) % unit_size;
        char *buffer = nullptr;
        if(end_off == 0){
            ++end_it;
            if(end_it == m_buffer_list.end()){
                append_data_buffer();
                --end_it;
            }
            buffer = end_it->data();
            cb_len = end_it->length();
            --end_it;
            return buffer;
        }
        buffer = end_it->data()+end_off;
        cb_len = unit_size - end_off;
        return buffer;
    }

    int cur_data_buffer_extend(int ex_len){
        int end_off = (off + len) % unit_size;
        if(ex_len > unit_size - end_off || ex_len < 0){
            return -1;
        }
        if(end_off == 0 && ex_len > 0){
            ++end_it;
            assert(end_it != m_buffer_list.end());
        }
        len += ex_len;
        return ex_len;
    }

    class iterator : public std::iterator< std::forward_iterator_tag, char>{
        FixedBufferList *m_bl;
        std::_List_iterator<MsgBuffer> m_it;
        int m_cur_off;
        bool m_is_end;
        char& cur_byte(){
            if(m_is_end) throw "Offset invalid!";
            return (*m_it)[(m_cur_off + m_bl->off) % m_bl->unit_size];
        }

    public:
        explicit iterator(){
            m_bl = nullptr;
            m_cur_off = 0;
            m_is_end = true;
        }

        explicit iterator(FixedBufferList *bl, bool is_end){
            m_bl = bl;
            m_it = m_bl->m_buffer_list.begin();
            m_cur_off = 0;
            m_is_end = is_end;
        }

        ~iterator(){
            m_bl = nullptr;
        }

        void swap(iterator& other) noexcept
        {
            using std::swap;
            swap(m_bl, other.m_bl);
            swap(m_it, other.m_it);
            swap(m_cur_off, other.m_cur_off);
            swap(m_is_end, other.m_is_end);
        }


        int seek(int off){
            if(off > m_bl->len || off < 0){
                return -1;
            }
            if(off == m_bl->len) {
                m_is_end = true;
                return 0;
            }
            if(m_is_end){
                --off;
                m_it = m_bl->end_it;
                --m_it;
                m_cur_off = m_bl->len - 1;
                m_is_end = false;
            }
            int step = (off+m_bl->off)/m_bl->unit_size  
                            - (m_cur_off+m_bl->off)/m_bl->unit_size;
            if(step > 0){
                while(step){
                    ++m_it;
                    --step;
                }
            }else{
                while(step){
                    --m_it;
                    ++step;
                }
            }
            m_cur_off = off;
            return 0;
        }

        inline int advance(int len){
            return this->seek(m_cur_off + len);
        }

        char *cur_data_buffer(int &cd_len){
            if(m_is_end) {
                cd_len = 0;
                return nullptr;
            }
            int cd_off = (m_cur_off + m_bl->off) % m_bl->unit_size;
            cd_len = m_bl->unit_size - cd_off;
            if(cd_len > m_bl->len - m_cur_off){
                cd_len = m_bl->len - m_cur_off;
            }
            if(cd_len == 0){
                return nullptr;
            }
            return m_it->data() + cd_off;
        }

        int cur_data_buffer_extend(int ex_len){
            if(m_is_end || ex_len < 0) return -1;
            int cd_off = (m_cur_off + m_bl->off) % m_bl->unit_size;
            if(ex_len > m_bl->unit_size - cd_off
                    || ex_len > m_bl->len - m_cur_off) 
                return -1;
            if(this->advance(ex_len) >= 0) return ex_len;
            return -1;
        }

        iterator& operator++ (){
            if(m_is_end) return *this;
            if(m_cur_off+1 == m_bl->len){
                m_is_end = true;
                return *this;
            }
            this->seek(m_cur_off+1);
            return *this;
        }

        iterator operator++ (int){
            if(m_is_end) return *this;
            iterator tmp = *this;
            if(m_cur_off+1 == m_bl->len){
                m_is_end = true;
                return tmp;
            }
            this->seek(m_cur_off+1);
            return tmp;
        }

        bool operator == (const iterator &o) const{
            if(m_is_end != o.m_is_end) return false;
            if(m_bl != o.m_bl) return false;
            if(m_is_end) return true;
            return (m_cur_off == o.m_cur_off) && (m_it == o.m_it);
        }

        bool operator != (const iterator &o) const{
            return !(*this == o);
        }

        char& operator* () {
            return this->cur_byte();
        }

        char& operator-> () {
            return this->cur_byte();
        }

        int copy(void *buf, int len){
            char *buffer = (char *)buf;
            int total = 0;
            if(m_is_end) return total;
            while((m_cur_off < m_bl->len) && (total < len)){
                int off_in_buffer = (m_cur_off + m_bl->off)%m_bl->unit_size;
                int cp_len = std::min(len - total, 
                                            m_bl->unit_size - off_in_buffer);
                std::memcpy(buffer+total, (*m_it).data()+off_in_buffer, cp_len);
                total += cp_len;
                m_cur_off += cp_len;
                if(off_in_buffer + cp_len == m_bl->unit_size){
                    ++m_it;
                }
            }
            if(m_cur_off == m_bl->len){
                m_is_end = true;
            }
            return total;
        }

        int copy(MsgBuffer &buffer){
            return copy(buffer.data(), buffer.length());
        }

    };

    iterator begin() noexcept{
        return iterator(this, false);
    }

    iterator end() noexcept{
        return iterator(this, true);
    }

};

} // namespace flame

#endif