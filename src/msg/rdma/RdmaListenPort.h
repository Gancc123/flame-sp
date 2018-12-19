#ifndef FLAME_MSG_RDMA_RDMA_LISTEN_PORT_H
#define FLAME_MSG_RDMA_RDMA_LISTEN_PORT_H

#include "msg/msg_common.h"
#include "msg/ListenPort.h"

namespace flame{

class RdmaListenPort : public ListenPort{
    RdmaListenPort(FlameContext *fct, NodeAddr *listen_addr);
public:
    static RdmaListenPort *create(FlameContext *fct, NodeAddr *listen_addr);
    ~RdmaListenPort();

    virtual msg_ttype_t get_ttype() override { 
        return msg_ttype_t::RDMA;
    }

    virtual void read_cb() override;
    virtual void write_cb() override;
    virtual void error_cb() override;
};

}

#endif