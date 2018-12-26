#ifndef FLAME_MSG_RDMA_PREP_CONN_H
#define FLAME_MSG_RDMA_PREP_CONN_H

#include "msg/msg_common.h"
#include "msg/MsgWorker.h"
#include "msg/event/event.h"
#include "Infiniband.h"
#include "RdmaStack.h"

#include <string>

namespace flame{

class RdmaConnection;
class RdmaListenPort;

class RdmaPrepConn : public EventCallBack{
public:
    enum class PrepStatus{
        CLOSED,
        INIT,
        SYNED_MY_MSG,
        SYNED_PEER_MSG,
        ACKED
    };
private:
    RdmaListenPort *lp = nullptr;
    MsgWorker *worker;
    PrepStatus status;
    RdmaConnection *real_conn;
    MsgBuffer my_msg_buffer;
    int my_msg_buffer_offset = 0;
    MsgBuffer peer_msg_buffer;
    int peer_msg_buffer_offset = 0;
    static int connect(MsgContext *mct, NodeAddr *addr);
    void close_rdma_conn_if_need();
    RdmaPrepConn(MsgContext *mct);
public:
    static RdmaPrepConn *create(MsgContext *mct, int cfd);
    static RdmaPrepConn *create(MsgContext *mct, NodeAddr *addr, 
                                                                uint8_t sl=0);
    ~RdmaPrepConn();

    void set_rdma_listen_port(RdmaListenPort *lp);
    void set_owner(MsgWorker *worker) { this->worker = worker; }
    MsgWorker *get_owner() const { return this->worker; }
    RdmaConnection *get_rdma_conn() { return real_conn; }
    bool is_server() { return !!lp; }
    bool is_connected() { return status != PrepStatus::CLOSED; }

    int send_my_msg();
    int recv_peer_msg();

    virtual void read_cb() override;
    virtual void write_cb() override;
    virtual void error_cb() override;

    void close();
    static std::string prep_status_str(PrepStatus s);
};



}

#endif