#include "mgr_server.h"

namespace flame {

bool MgrServer::register_service(MgrService* service) {
    service_list_.push_back(service)
}

void MgrServer::run() {
    server_ = builder_.BuildAndStart();
    server_->Wait();
}

void MgrServer::__init() {
    builder_.AddListeningPort(addr_, grpc::InsecureServerCredentials());

    builder_.RegisterService(&flame_service_);
    builder_.RegisterService(&internal_service_);
}

} // namespace flame