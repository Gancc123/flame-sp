#include "common/context.h"
#include "msg/msg_core.h"
#include "util/clog.h"

#include "incre_msger.h"

#include <unistd.h>
#include <cstdio>

using namespace flame;

void send_first_incre_msg(FlameContext *fct){
    msg_incre_d data;
    data.num = 0;
    auto req_msg = Msg::alloc_msg(fct);
    req_msg->append_data(data);
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
    conn->send_msg(req_msg);
    req_msg->put();
    rdma_addr->put();
    addr->put();
}

#define CFG_PATH "flame_clinet.cfg"

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

    auto incre_msger = new IncreMsger(fct);
    
    ML(fct, info, "before msg module init");
    msg_module_init(fct, incre_msger);
    ML(fct, info, "after msg module init");

    ML(fct, info, "msger_id {:x} {:x} ", fct->msg()->config->msger_id.ip,
                                         fct->msg()->config->msger_id.port);

    send_first_incre_msg(fct);

    std::getchar();

    ML(fct, info, "before msg module fin");
    msg_module_finilize(fct);
    ML(fct, info, "after msg module fin");

    delete incre_msger;

    return 0;
}