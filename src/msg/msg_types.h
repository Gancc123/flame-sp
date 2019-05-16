#ifndef FLAME_MSG_MSG_TYPES_H
#define FLAME_MSG_MSG_TYPES_H

#include "internal/msg_buffer_list.h"
#include <vector>

namespace flame{
namespace msg{

/**
 * interface
 */
class ListenPort;
class Connection;
class NodeAddr;
class Msg;
class Session;
class RdmaRecvWr;

class ListenPortListener{
public:
    virtual void on_listen_accept(ListenPort *, Connection *conn) = 0;
    virtual void on_listen_error(NodeAddr *listen_addr) = 0;
};

class ConnectionListener{
public:
    virtual void on_conn_recv(Connection *, Msg *) = 0;
    virtual void on_conn_error(Connection *conn) = 0;
};

struct MsgData{
    virtual int encode(MsgBufferList &bl) = 0;
    virtual int decode(MsgBufferList::iterator &it) = 0;
    virtual size_t size() { return 0; }
    virtual ~MsgData() {};
};

class MsgerCallback{
public:
    virtual ~MsgerCallback() {};
    virtual void on_listen_accept(ListenPort *lp, Connection *conn) {};
    virtual void on_listen_error(NodeAddr *listen_addr) {};
    virtual void on_conn_error(Connection *conn) {};
    virtual void on_conn_declared(Connection *conn, Session *s) {};
    
    virtual void on_conn_recv(Connection *, Msg *) {};

    //for RdmaConnection V2
    virtual int get_recv_wrs(int n, std::vector<RdmaRecvWr *> &wrs) { return 0;}
};

} //namespace msg
} //namespace flame

#endif 