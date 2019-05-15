#ifndef FLAME_MSG_SESSION_H
#define FLAME_MSG_SESSION_H

#include "msg/msg_context.h"
#include "common/thread/mutex.h"

#include "internal/types.h"
#include "msg_types.h"
#include "Connection.h"

#include <vector>

namespace flame{
namespace msg{

class Session : public RefCountedObject{

    NodeAddr *tcp_listen_addr = nullptr;
    NodeAddr *rdma_listen_addr = nullptr;

    struct conn_entry_t{
        msg_ttype_t ttype; // transport type
        uint8_t sl; // service level
        Connection *conn;
    };

    std::vector<conn_entry_t> conns;

    Mutex conns_mutex;
    Mutex lp_mutex;

    MsgContext *msg_context_;

public:
    explicit Session(MsgContext *c, msger_id_t peer)
    : RefCountedObject(c), msg_context_(c), peer_msger_id(peer), 
     conns_mutex(MUTEX_TYPE_ADAPTIVE_NP),
     lp_mutex(MUTEX_TYPE_ADAPTIVE_NP){
        conns.reserve(4);
    }

    ~Session(){
        {
            MutexLocker l(lp_mutex);
            if(tcp_listen_addr){
                tcp_listen_addr->put();
                tcp_listen_addr = nullptr;
            }
            if(rdma_listen_addr){
                rdma_listen_addr->put();
                rdma_listen_addr = nullptr;
            }
        }
        
        MutexLocker l(conns_mutex);
        for(auto entry : conns){
            entry.conn->set_session(nullptr);
            entry.conn->put();
        }
        conns.clear();
    }

    MsgContext* get_msg_context(){
        return msg_context_;
    }

    bool ready() const {
        return (tcp_listen_addr != nullptr) && (rdma_listen_addr != nullptr);
    }

    NodeAddr *get_listen_addr(msg_ttype_t ttype=msg_ttype_t::TCP){
        MutexLocker l(lp_mutex);
        switch(ttype){
        case msg_ttype_t::TCP:
            return tcp_listen_addr;
        case msg_ttype_t::RDMA:
            return rdma_listen_addr;
        default:
            return nullptr;
        }
    }

    void set_listen_addr(uint64_t na, msg_ttype_t ttype=msg_ttype_t::TCP){
        node_addr_t addr(na);
        NodeAddr *nap = new NodeAddr(mct, addr);
        nap->set_ttype((uint8_t)ttype);
        set_listen_addr(nap, ttype);
        nap->put();
    }

    void set_listen_addr(NodeAddr *addr, msg_ttype_t ttype=msg_ttype_t::TCP){
        if(!addr) return;
        addr->get();
        MutexLocker l(lp_mutex);
        switch(ttype){
        case msg_ttype_t::TCP:
            if(tcp_listen_addr){
                tcp_listen_addr->put();
            }
            tcp_listen_addr = addr;
            break;
        case msg_ttype_t::RDMA:
            if(rdma_listen_addr){
                rdma_listen_addr->put();
            }
            rdma_listen_addr = addr;
            break;
        default:
            addr->put();
        }
    }

    Connection *get_conn(msg_ttype_t ttype=msg_ttype_t::TCP, uint8_t sl=0);
    int add_conn(Connection *conn, uint8_t sl=0);
    int del_conn(Connection *conn);

    const msger_id_t peer_msger_id;

    std::string to_string() const;
};

} //namespace msg
} //namespace flame

#endif