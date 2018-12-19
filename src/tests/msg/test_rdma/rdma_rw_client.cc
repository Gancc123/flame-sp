#include "msg/msg_core.h"
#include "common/context.h"
#include "util/clog.h"

#include "rdma_msger.h"

#include <unistd.h>
#include <cstdio>

using namespace flame;

void send_first_incre_msg(FlameContext *fct){
    NodeAddr *addr = new NodeAddr(fct);
    addr->ip_from_string("127.0.0.1");
    addr->set_port(6666);
    msger_id_t msger_id = msger_id_from_node_addr(addr);
    auto session = fct->msg()->manager->get_session(msger_id);
    NodeAddr *rdma_addr = new NodeAddr(fct);
    rdma_addr->set_ttype(NODE_ADDR_TTYPE_RDMA);
    rdma_addr->ip_from_string("127.0.0.1");
    rdma_addr->set_port(7777);
    session->set_listen_addr(addr);
    session->set_listen_addr(rdma_addr, msg_ttype_t::RDMA);
    auto conn = session->get_conn(msg_ttype_t::RDMA);

    auto allocator = Stack::get_rdma_stack()->get_rdma_allocator();
    
    auto buf = allocator->alloc(1 << 22); //4MB
    msg_rdma_header_d rdma_header;
    rdma_header.rdma_bufs.push_back(buf);
    buf->buffer()[0] = 'A';
    buf->buffer()[buf->size() - 1] = 'Z';
    buf->data_len = buf->size();

    auto req_msg = Msg::alloc_msg(fct, msg_ttype_t::RDMA);
    req_msg->flag |= FLAME_MSG_FLAG_RDMA;
    req_msg->set_rdma_cnt(1);
    req_msg->append_data(rdma_header);
    

    conn->send_msg(req_msg);
    req_msg->put();
    rdma_addr->put();
    addr->put();
}

#define CFG_PATH "flame_client.cfg"

int main(){
    FlameContext *fct = FlameContext::get_context();
    if(!fct->init_config(CFG_PATH)){
        clog("init config failed.");
        return -1;
    }
    if(!fct->init_log("log", "TRACE", "client")){
         clog("init log failed.");
        return -1;
    }

    ML(fct, info, "init complete.");
    ML(fct, info, "load cfg: " CFG_PATH);
    
    ML(fct, info, "before msg module init");
    msg_module_init(fct, nullptr);
    ML(fct, info, "after msg module init");

    ML(fct, info, "msger_id {:x} {:x} ", fct->msg()->config->msger_id.ip,
                                         fct->msg()->config->msger_id.port);

    send_first_incre_msg(fct);

    std::getchar();

    ML(fct, info, "before msg module fin");
    msg_module_finilize(fct);
    ML(fct, info, "after msg module fin");

    return 0;
}