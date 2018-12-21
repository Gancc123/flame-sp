#include "msg/msg_core.h"
#include "common/context.h"
#include "util/fmt.h"
#include "util/clog.h"

#include "rdma_msger.h"

#include <unistd.h>
#include <cstdio>
#include <string>

using namespace flame;

perf_config_t global_config;

#define CFG_PATH "flame_mgr.cfg"

int main(int argc, char *argv[]){
    auto parser = init_parser();
    optparse::Values options = parser.parse_args(argc, argv);
    auto config_path = options.get("config");

    FlameContext *fct = FlameContext::get_context();
    if(!fct->init_config(
            std::string(config_path).empty()?CFG_PATH:config_path)){
        clog("init config failed.");
        return -1;
    }
    if(!fct->init_log("log", str2upper(std::string(options.get("log_level"))), 
                    fmt::format("mgr{}",  std::string(options.get("index"))))){
         clog("init log failed.");
        return -1;
    }

    ML(fct, info, "init complete.");
    ML(fct, info, "load cfg: " CFG_PATH);


    global_config.num = (int)options.get("num");
    global_config.perf_type = perf_type_from_str(
                                            std::string(options.get("type")));

    init_resource(global_config);

    auto rdma_msger = new RdmaMsger(fct, &global_config);

    auto msg_config = MsgConfig::load_config(fct);
    msg_config->set_msg_log_level(std::string(options.get("log_level")));

    ML(fct, info, "before msg module init");
    msg_module_init(fct, rdma_msger, msg_config);
    ML(fct, info, "after msg module init");

    ML(fct, info, "msger_id {:x} {:x} ", fct->msg()->config->msger_id.ip,
                                         fct->msg()->config->msger_id.port);

    std::getchar();

    ML(fct, info, "before msg module fin");
    msg_module_finilize(fct);
    ML(fct, info, "after msg module fin");

    delete rdma_msger;

    fin_resource(global_config);

    return 0;
}