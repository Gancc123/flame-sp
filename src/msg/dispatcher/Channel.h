#ifndef FLAME_MSG_DISPATCHER_CHANNEL_H
#define FLAME_MSG_DISPATCHER_CHANNEL_H

namespace flame{

class NodeAddr;
class ListenPort;
class Connection;
class Session;
class RdmaRwWork;
class Message;
class Dispatcher;

class Channel{
public:
    virtual int on_listen_accept(ListenPort *lp, Connection *conn) { return 0; }
    virtual int on_listen_error(NodeAddr *listen_addr) { return 0; }
    virtual int on_conn_error(Connection *conn) { return 0; }
    virtual int on_conn_declared(Connection *conn, Session *s) { return 0; }

    virtual int on_rdma_done(Connection *, Message *, RdmaRwWork *) { return 0;}
    
    //must delete Message by User.
    virtual int on_conn_recv(Connection *, Message *) = 0;

    //must delete Message by User.
    virtual int on_local_recv(Message *) = 0;

};

}

#endif // FLAME_MSG_DISPATCHER_CHANNEL_H