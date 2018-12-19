#include "msg/msg_core.h"
#include "common/context.h"
#include "util/clog.h"

#include "rdma_msger.h"

#include <unistd.h>
#include <cstdio>

using namespace flame;

#define CFG_PATH "flame_mgr.cfg"

int main(){
    FlameContext *fct = FlameContext::get_context();
    if(!fct->init_config(CFG_PATH)){
        clog("init config failed.");
        return -1;
    }
    if(!fct->init_log("log", "TRACE", "mgr")){
         clog("init log failed.");
        return -1;
    }

    ML(fct, info, "init complete.");
    ML(fct, info, "load cfg: " CFG_PATH);

    auto rdma_msger = new RdmaMsger(fct);

    ML(fct, info, "before msg module init");
    msg_module_init(fct, rdma_msger);
    ML(fct, info, "after msg module init");

    ML(fct, info, "msger_id {:x} {:x} ", fct->msg()->config->msger_id.ip,
                                         fct->msg()->config->msger_id.port);

    std::getchar();

    ML(fct, info, "before msg module fin");
    msg_module_finilize(fct);
    ML(fct, info, "after msg module fin");

    delete rdma_msger;

    return 0;
}