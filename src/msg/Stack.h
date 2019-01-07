#ifndef FLAME_MSG_STACK_H
#define FLAME_MSG_STACK_H

#include "acconfig.h"
#include "msg_context.h"
#include "msg_types.h"
#include "Connection.h"
#include "ListenPort.h"

namespace flame{
namespace msg{

class TcpStack;
class RdmaStack;

class Stack{
    static MsgContext *mct;
public:
    static int init_all_stack(MsgContext *mct);
    static int clear_all_before_stop();
    static int fin_all_stack();

    static TcpStack* get_tcp_stack();
#ifdef HAVE_RDMA
    static RdmaStack* get_rdma_stack();
#endif

    virtual int init() { return 0; };
    // clear before worker threads stop
    virtual int clear_before_stop() { return 0; } 
    virtual int fin()  { return 0; }
    virtual ~Stack() {};

    virtual ListenPort* create_listen_port(NodeAddr *addr) = 0;

    virtual Connection * connect(NodeAddr *addr) = 0;
};

} //namespace msg
} //namespace flame

#endif