#ifndef FLAME_TESTS_MSG_TEST_RDMA_H
#define FLAME_TESTS_MSG_TEST_RDMA_H

#include "msg/msg_core.h"
#include "util/clog.h"

namespace flame{

class RdmaMsger : public MsgerCallback{
    MsgContext *mct;
public:
    explicit RdmaMsger(MsgContext *c) : mct(c) {};
    virtual void on_conn_recv(Connection *conn, Msg *msg) override;
};

void RdmaMsger::on_conn_recv(Connection *conn, Msg *msg){
    assert(msg->has_rdma());

    msg_rdma_header_d rdma_header(msg->get_rdma_cnt(), msg->with_imm());

    auto it = msg->data_buffer_list().begin();
    rdma_header.decode(it);
    
    auto msger_id = conn->get_session()->peer_msger_id;
    ML(mct, trace, "{}=>  {}", msger_id_to_str(msger_id), msg->to_string());

    auto allocator = Stack::get_rdma_stack()->get_rdma_allocator();

    auto lbuf = allocator->alloc(rdma_header.rdma_bufs[0]->data_len);
    assert(lbuf);

    auto rdma_cb = new RdmaRwWork();
    assert(rdma_cb);
    rdma_cb->target_func = 
                        [this, allocator](RdmaRwWork *w, RdmaConnection *conn){
        auto lbuf = w->lbufs[0];
        ML(mct, info, "rdma read done. buf: {}...{} {}B",
            lbuf->buffer()[0], lbuf->buffer()[lbuf->data_len - 1],
            lbuf->data_len);

        allocator->free_buffers(w->lbufs);
        allocator->free_buffers(w->rbufs);
        delete w;
    };

    rdma_cb->is_write = false;
    rdma_cb->rbufs = rdma_header.rdma_bufs;
    rdma_cb->lbufs.push_back(lbuf);
    rdma_cb->cnt = 1;

    auto rdma_conn = RdmaStack::rdma_conn_cast(conn);
    assert(rdma_conn);
    rdma_conn->post_rdma_rw(rdma_cb);

    return;
}

}

#endif 