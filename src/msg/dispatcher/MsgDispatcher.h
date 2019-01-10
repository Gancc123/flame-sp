#ifndef FLAME_MSG_DISPATCHER_DISPATCHER_H
#define FLAME_MSG_DISPATCHER_DISPATCHER_H

#include "common/thread/rw_lock.h"
#include "msg/msg_context.h"
#include "msg/msg_types.h"
#include "msg/msg_data.h"
#include "msg/Message.h"
#include "include/meta.h"
#include "msg/internal/types.h"

#include "MsgChannel.h"

#include <map>

namespace flame{
namespace msg{

class MsgDispatcher : public MsgerCallback{
    MsgContext *mct;

    RWLock channel_map_rwlock;
    std::map<uint64_t, MsgChannel *> channel_map;

    Session *get_session(uint64_t dst_id);

    int deliver_to_remote(MessagePtr msg);

    int resolve_addr(uint64_t dst_id, csd_addr_attr_t &res);

    Msg *msg_from_message(MessagePtr& msg);

    MessagePtr message_from_msg(Msg *m);

    static msger_id_t msger_id_from_node_addr(uint64_t na){
        msger_id_t mid;
        node_addr_t addr(na);
        mid.ip = addr.get_ip();
        mid.port = addr.get_port();
        return mid;
    }

public:
    explicit MsgDispatcher(MsgContext *c) 
    : mct(c), channel_map_rwlock() {};

    int deliver_msg(MessagePtr m);

    int post_rdma_work(uint64_t dst_id, RdmaRwWork *work);

    int reg_channel(uint64_t comp_id, MsgChannel *);

    int unreg_channel(uint64_t comp_id);

    MsgChannel *get_channel(uint64_t comp_id);

    //derived from MsgerCallback
    virtual void on_listen_accept(ListenPort *lp, Connection *conn) override;
    virtual void on_listen_error(NodeAddr *listen_addr) override;
    virtual void on_conn_error(Connection *conn) override;
    virtual void on_conn_declared(Connection *conn, Session *s) override;
    
    virtual void on_conn_recv(Connection *, Msg *) override;

};

} //namespace msg
} //namespace flame

#endif //FLAME_MSG_DISPATCHER_DISPATCHER_H