#ifndef FLAME_MSG_SOCKET_TCP_LISTEN_PORT_H
#define FLAME_MSG_SOCKET_TCP_LISTEN_PORT_H

#include "msg/msg_common.h"
#include "msg/ListenPort.h"

namespace flame{
namespace msg{

class TcpListenPort : public ListenPort{
    TcpListenPort(MsgContext *mct, NodeAddr *listen_addr);
public:
    static TcpListenPort *create(MsgContext *mct, NodeAddr *listen_addr);
    ~TcpListenPort();

    virtual msg_ttype_t get_ttype() override { 
        return msg_ttype_t::TCP;
    }

    virtual void read_cb() override;
    virtual void write_cb() override;
    virtual void error_cb() override;
};

} //namespace msg
} //namespace flame

#endif