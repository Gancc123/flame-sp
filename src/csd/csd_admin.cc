#include "csd/csd_admin.h"

namespace flame {

// bool MgrServer::register_service(MgrService* service) {
//     service_list_.push_back(service)
// }

int CsdAdminServer::run() {
    server_ = builder_.BuildAndStart();
    server_->Wait();
    return 0;
}

void CsdAdminServer::__init() {
    builder_.AddListeningPort(addr_, grpc::InsecureServerCredentials());

    builder_.RegisterService(&csds_service_);
}

} // namespace flame