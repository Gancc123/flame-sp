#include "mgr.h"
#include "common/context.h"
#include "common/cmdline.h"

#include <grpcpp/grpcpp.h>
#include "service/flame_service.h"
#include "service/internal_service.h"

#include "mgr/log_mgr.h"

#include <memory>
#include <iostream>
#include <string>

using grpc::Server;
using grpc::ServerBuilder;

namespace flame {

int MGR::run_server() {
    if (!server_) {
        return 1;
    }
    server_->run();
    return 0;
}

//     std::string addr("0.0.0.0:6666");
    
//     fct->log()->info("GWService Cons");
//     service::FlameServiceImpl flame_service;
//     service::InternalServiceImpl internal_service;

//     fct->log()->info("ServerBuilder");
//     ServerBuilder builder;

//     fct->log()->info("Add Listening Port");
//     builder.AddListeningPort(addr, grpc::InsecureServerCredentials());

//     fct->log()->info("Register Service");
//     builder.RegisterService(&flame_service);
//     builder.RegisterService(&internal_service);

//     fct->log()->info("server builded");
//     std::unique_ptr<Server> server(builder.BuildAndStart());

//     fct->log()->info("start");
//     server->Wait();

//     fct->log()->info("exit");
//     return 0;
// }

} // namespace flame

using namespace std;
using namespace flame;
using namespace flame::cli;

class MgrCli final : public Cmdline {
public:
    MgrCli() : Cmdline("mgr", "Flame Manager Node") {}

    // Argument
    Argument<string>    config_path {this, 'c', "config_file", "config file path", "/etc/flame/mgr.conf"};
    Argument<int>       port        {this, 'p', "port", "MGR listen port", 6666};
    Argument<string>    metastore   {this, "metastore", "MetaStore url"};

    HelpAction help {this};
}; // class MgrCli

// 重载日志
bool reopen_log__(FlameContext* fct) {
    FlameConfig* config = fct->config();
    if (!config->has_key(CFG_LOG_DIR)) {
        fct->log()->error("missing config item '%s'", CFG_LOG_DIR);
        return false;
    }
    if (fct->log()->reopen(config->get(CFG_LOG_DIR, "."))) {
        fct->log()->error("reopen logger to file faild");
        return false;
    }
    return true;
}

int create_metastore__(MgrContext* mct) {

}

int main(int agrc, char** argv) {
    MgrCli* mgr_cli = new MgrCli();
    int r = mgr_cli->parser(argc, argv);
    if (r != CmdRetCode::SUCCESS) {
        mgr_cli->print_help();
        return r;
    }

    // 获取全局上下文
    FlameContext* fct = FlameContext::get_context();

    // 创建命令行日志并设置
    Logger* cmdlog = new Logger(LogLevel::PRINT);
    fct->set_log(cmdlog);

    // 加载配置文件
    string path = mgr_cli->config_path;
    FlameConfig* config = FlameConfig::create_config(fct, path);
    if (config == nullptr) {
        fct->log()->error("read config file(%s) faild.", path.c_str());
        exit(-1);
    }
    fct->set_config(config);

    // 重载日志
    if (!reopen_log__(fct))
        exit(-1);

    // 创建MGR上下文
    MgrContext* mct = new MgrContext(fct);

    // 创建Metastore



    logger->info("set_log");
    fct->set_log(logger);

    fct->log()->info("MGR Cons");
    MGR mgr(fct);

    fct->log()->info("run_server");
    return mgr.run_server();
}