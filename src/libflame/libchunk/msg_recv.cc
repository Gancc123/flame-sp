#include "libflame/libchunk/msg_recv.h"


#define M_ENCODE(bl, data) (bl).append(&(data), sizeof(data))
#define M_DECODE(it, data) (it).copy(&(data), sizeof(data))

namespace flame {

//--------------------------MsgCmd------------------------------------------------------
int MsgCmd::encode(msg::MsgBufferList &bl){
    return M_ENCODE(bl, cmd);
}

int MsgCmd::decode(msg::MsgBufferList::iterator &it){
    return M_DECODE(it, cmd);
}

//--------------------------Client-----------------------------------------------------
void MsgerClientCallback::on_conn_recv(msg::Connection *conn, msg::Msg *msg){
    FlameContext *flame_context = mct_->fct;
    flame_context->log()->ltrace("recv response");

    MsgCmd msg_data;
    auto it = msg->data_buffer_list().begin();
    msg_data.decode(it);    //**这里接收到的实际上是response
    Response *response;
    // if(msg_data.cmd.hdr.cn.seq == CMD_CHK_IO_READ) //**现在只关注了数据平面，所以没有判断cn.cls
    uint16_t rc = *(uint16_t *)msg_data.cmd.cont;
    response = new CommonRes(msg_data.cmd, rc);
    flame_context->log()->ltrace("get_num() = %lu",response->get_num());
    flame_context->log()->ltrace("get_cqg() = %lu",response->get_cqg());
    flame_context->log()->ltrace("get_cqn() = %lu",response->get_cqn());
    flame_context->log()->ltrace("get_len() = %lu",response->get_len());
    flame_context->log()->ltrace("get_rc()  = %lu",response->get_rc());
    return;
}

//--------------------------Server-----------------------------------------------------//
void MsgerServerCallback::on_conn_recv(msg::Connection *conn, msg::Msg *msg){
    MsgCmd data;
    auto it = msg->data_buffer_list().begin();
    data.decode(it);
    msg::msger_id_t msger_id = conn->get_session()->peer_msger_id;
    FlameContext *flame_context = mct_->fct;
    flame_context->log()->ltrace("on_conn_recieved");
    flame_context->log()->ltrace("peer_msger_id.ip = %llu, peer_msger_id.port = %llu",msger_id.ip,msger_id.port);
    CmdServiceMapper *cmd_service_mapper = CmdServiceMapper::get_cmd_service_mapper();
    uint8_t cls = data.cmd.hdr.cn.cls;
    uint8_t num = data.cmd.hdr.cn.seq;
    CmdService *cmd_service = cmd_service_mapper->get_service(cls, num);
    cmd_service->call(conn, data.cmd);

    return;
}

} //namespace flame

#undef M_ENCODE
#undef M_DECODE