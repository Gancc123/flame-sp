#include "msg/msg_core.h"
#include "common/context.h"
#include "util/clog.h"

#include "incre_msger.h"

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

    auto incre_msger = new IncreMsger(fct);

    msg_module_init(fct, incre_msger);

    ML(fct, info, "msger_id {:x} {:x} ", fct->msg()->config->msger_id.ip,
                                         fct->msg()->config->msger_id.port);

    //test time work
    auto worker = fct->msg()->manager->get_worker(0);
    worker->post_time_work(100, [fct](){
        ML(fct, info, "test time work1: 100us");
    });

    worker->post_time_work(1000000, [fct](){
        ML(fct, info, "test time work2: 1000000us");
    });

    worker->post_time_work(5000000, [fct](){
        ML(fct, info, "test time work3: 5000000us");
    });

    std::getchar();

    msg_module_finilize(fct);

    delete incre_msger;

    return 0;
}