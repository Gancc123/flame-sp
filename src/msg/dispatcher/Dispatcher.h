#ifndef FLAME_MSG_DISPATCHER_DISPATCHER_H
#define FLAME_MSG_DISPATCHER_DISPATCHER_H

#include "msg/msg_context.h"
#include "msg/msg_data.h"
#include "msg/Message.h"

#include "Channel.h"

#include <map>

namespace flame{

class Dispatcher;

struct MemPushWork : public RdmaRwWork{
    Msg *msg;
    Message *message;
    Dispatcher *dispatcher;
    explicit MemPushWork(Dispatcher *d, Msg *m) 
    : is_write(false), dispatcher(d), msg(m) {
        msg->get();
    }
    ~MemPushWork() { msg->put(); }
    virtual void callback(RdmaConnection *conn) override;
}

struct MemFetchWork : public RdmaRwWork{
    Msg *msg;
    Message *message;
    Dispatcher *dispatcher;
    explicit MemFetchWork(Dispatcher *d, Msg *m) 
    : is_write(true), dispatcher(d), msg(m) {
        msg->get();
    }
    ~MemFetchWork() { msg->put(); }
    virtual void callback(RdmaConnection *conn) override;
}

class Dispatcher : public MsgerCallback{
    MsgContext *mct;

    Mutex channel_map_mutex;
    std::map<uint64_t, Channel *> channel_map;

    int deliver_to_remote(Message *);

public:
    explicit Dispatcher(MsgContext *c) 
    : mct(c), channel_map_mutex(MUTEX_TYPE_ADAPTIVE_NP) {}

    int deliver_msg(Message *);

    int reg_channel(uint64_t comp_id, Channel *);

    int unreg_channel(Channel *);

    Channel *get_channel(uint64_t comp_id);

    //derived from MsgerCallback
    virtual void on_listen_accept(ListenPort *lp, Connection *conn) override;
    virtual void on_listen_error(NodeAddr *listen_addr) override;
    virtual void on_conn_error(Connection *conn) override;
    virtual void on_conn_declared(Connection *conn, Session *s) override;
    
    virtual void on_conn_recv(Connection *, Msg *) override;

};

}

#endif //FLAME_MSG_DISPATCHER_DISPATCHER_H