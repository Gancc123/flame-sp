#ifndef FLAME_MSG_DISPATCHER_CHANNEL_H
#define FLAME_MSG_DISPATCHER_CHANNEL_H

#include "msg/Message.h"

namespace flame{
namespace msg{

class NodeAddr;
class ListenPort;
class Connection;
class Session;
class RdmaRwWork;
class MsgDispatcher;

class MsgChannel{
public:
    virtual int on_listen_accept(ListenPort *lp, Connection *conn) { return 0; }
    virtual int on_listen_error(NodeAddr *listen_addr) { return 0; }
    virtual int on_conn_error(Connection *conn) { return 0; }
    virtual int on_conn_declared(Connection *conn, Session *s) { return 0; }

    virtual int on_conn_recv(Connection *, uint32_t imm_data) { return 0; }
    virtual int on_conn_recv(Connection *, MessagePtr msg) = 0;

    virtual int on_local_recv(MessagePtr msg) = 0;

};

} //namespace msg
} //namespace flame

#endif // FLAME_MSG_DISPATCHER_CHANNEL_H