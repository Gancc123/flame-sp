#ifndef FLAME_TESTS_MSG_TEST_RDMA_H
#define FLAME_TESTS_MSG_TEST_RDMA_H

#include "msg/msg_core.h"
#define M_ENCODE(bl, data) (bl).append(&(data), sizeof(data))
#define M_DECODE(it, data) (it).copy(&(data), sizeof(data))

namespace flame {

struct msg_incre_d : public MsgData{
    int num;
    virtual int encode(BufferList &bl) override;
    virtual int decode(BufferList::iterator &it) override;
};


class IncreMsger : public MsgerCallback{
    FlameContext *fct;
public:
    explicit IncreMsger(FlameContext *c) : fct(c) {};
    virtual void on_conn_recv(Connection *conn, Msg *msg) override;
};

int msg_incre_d::encode(BufferList &bl){
    return M_ENCODE(bl, num);
}

int msg_incre_d::decode(BufferList::iterator &it){
    return M_DECODE(it, num);
}

void IncreMsger::on_conn_recv(Connection *conn, Msg *msg){
    msg_incre_d data;
    auto it = msg->data_buffer_list().begin();
    data.decode(it);
    ++data.num;
    auto msger_id = conn->get_session()->peer_msger_id;
    ML(fct, trace, "{}=>  incre num {} -> {}",
                            msger_id_to_str(msger_id), data.num - 1, data.num);
    if(data.num >= 200){
        return;
    }

    auto req_msg = Msg::alloc_msg(fct, msg_ttype_t::RDMA);
    req_msg->append_data(data);
    conn->send_msg(req_msg);
    req_msg->put();
    return;
}

} //namespace flame

#undef M_ENCODE
#undef M_DECODE

#endif 