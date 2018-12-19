#ifndef FLAME_MSG_INTERNAL_BUFFER_H
#define FLAME_MSG_INTERNAL_BUFFER_H

#include <stdlib.h>
#include <functional>

namespace flame{

typedef void (*msg_buffer_release_cb_t)(void *, size_t);

class MsgBuffer{
    char *m_data;
    size_t m_off;
    size_t m_len;
    msg_buffer_release_cb_t m_release_cb;
public:
    explicit MsgBuffer(size_t len) 
    :m_data(new char[len]), m_len(len), m_off(0), m_release_cb(nullptr){};

    explicit MsgBuffer(char *buffer, size_t len, 
                                            msg_buffer_release_cb_t cb=nullptr) 
    :m_data(buffer), m_len(len), m_off(0), m_release_cb(cb){};

    explicit MsgBuffer(MsgBuffer &&o) noexcept
    : m_data(o.m_data), m_len(o.m_len), m_off(o.m_off),
      m_release_cb(nullptr){
        o.m_data = nullptr;
        o.m_len = 0;
        o.m_off = 0;
    }

    void clear() {
        m_off = 0;
    }

    char *data() const{
        return this->m_data;
    }

    /**
     * the number of bytes that buffer has stored.
     */
    size_t offset() const{
        return this->m_off;
    }

    bool full() const{
        return (m_off == m_len);
    }

    /**
     * move data offset by step.
     * @param step: positive or negative
     * @return: actually moved step.
     */
    int advance(int step){
        int new_off = m_off + step;
        if(new_off < 0){
            new_off = 0;
        }else if(new_off > m_len){
            new_off = m_len;
        }
        step = new_off - m_off;
        m_off = new_off;
        return step;
    }


    /**
     * set data offset 
     */
    void set_offset(size_t off) {
        if(off > m_len){
            off = m_len;
        }
        this->m_off = off;
    }

    /**
     * the real size of this buffer.
     */
    size_t length() const{
        return this->m_len;
    }


    char& operator[] (int x) {
        return m_data[x];
    }

    ~MsgBuffer(){
        if(m_data){
            if(m_release_cb){
                m_release_cb(m_data, m_len);
                m_release_cb = nullptr;
            }else{
                delete []m_data;
            }
            m_data = nullptr;
            m_off = 0;
            m_len = 0;
        }
    }
};

} //namespace flame

#endif //FLAME_MSG_INTERNAL_BUFFER_H