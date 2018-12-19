#include "msg/msg_core.h"
#include "common/context.h"
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
    session->set_listen_addr(addr);
    auto conn = session->get_conn();
    conn->send_msg(req_msg);
    req_msg->put();
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

    IncreMsger *msger = new IncreMsger(fct);

    msg_module_init(fct, msger);

    ML(fct, info, "msger_id {:x} {:x} ", fct->msg()->config->msger_id.ip,
                                         fct->msg()->config->msger_id.port);

    send_first_incre_msg(fct);

    std::getchar();

    msg_module_finilize(fct);

    delete msger;


    return 0;
}