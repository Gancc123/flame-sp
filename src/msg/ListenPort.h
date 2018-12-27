#ifndef FLAME_MSG_LISTEN_PORT_H
#define FLAME_MSG_LISTEN_PORT_H

#include "msg_common.h"
#include "Connection.h"
#include "MsgWorker.h"
#include "msg/event/event.h"

namespace flame{

class ListenPort : public EventCallBack{
    ListenPortListener *m_listener;
    NodeAddr *m_listen_addr;

public:
    explicit ListenPort(MsgContext *mct, NodeAddr *listen_addr)
    :EventCallBack(mct, FLAME_EVENT_READABLE){
        listen_addr->get();
        m_listen_addr = listen_addr;
    }

    virtual ~ListenPort(){
        m_listener = nullptr;
        if(m_listen_addr){
            m_listen_addr->put();
            m_listen_addr = nullptr;
        }
    }

    virtual msg_ttype_t get_ttype() = 0;

    virtual void set_listener(ListenPortListener *listener){
        this->m_listener = listener;
    }

    virtual ListenPortListener* get_listener() const{
        return this->m_listener;
    }

    virtual void set_listen_addr(NodeAddr *addr){
        if(!addr) return;
        if(m_listen_addr){
            m_listen_addr->put();
        }
        addr->get();
        m_listen_addr = addr;
    }

    virtual NodeAddr* get_listen_addr() const{
        return this->m_listen_addr;
    }

    virtual void set_listen_fd(int fd){
        this->fd = fd;
    }

    virtual int get_listen_fd() const {
        return this->fd;
    }

    virtual void read_cb() = 0;
    virtual void write_cb() = 0;
    virtual void error_cb() = 0;

    std::string to_string() const{
        auto s = fmt::format("[LP {}]({:p})", 
                    m_listen_addr?m_listen_addr->to_string():"", (void *)this);
        return s;
    }
};

}

#endif