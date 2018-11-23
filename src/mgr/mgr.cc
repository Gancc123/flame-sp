#include "mgr.h"
#include "common/context.h"
#include "mgr/log_mgr.h"

#include <grpcpp/grpcpp.h>
#include "service/flame_service.h"

#include <memory>
#include <iostream>

using grpc::Server;
using grpc::ServerBuilder;

namespace flame {

int MGR::run_server() {
    std::string addr("0.0.0.0:6666");
    
    fct->log()->info("GWService Cons");
    service::FlameServiceImpl flame_service;

    fct->log()->info("ServerBuilder");
    ServerBuilder builder;

    fct->log()->info("Add Listening Port");
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());

    fct->log()->info("Register Service");
    builder.RegisterService(&flame_service);

    fct->log()->info("server builded");
    std::unique_ptr<Server> server(builder.BuildAndStart());

    fct->log()->info("start");
    server->Wait();

    fct->log()->info("exit");
    return 0;
}

} // namespace flame


using namespace flame;

int main() {
    FlameContext* fct = FlameContext::get_context();
    
    Logger* logger = new Logger(LogLevel::DEBUG);

    logger->info("set_log");
    fct->set_log(logger);

    fct->log()->info("MGR Cons");
    MGR mgr(fct);

    fct->log()->info("run_server");
    return mgr.run_server();
}