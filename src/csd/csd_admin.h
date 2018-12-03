#ifndef FLAME_CSD_ADMIN_H
#define FLAME_CSD_ADMIN_H

#include "csd/csd_context.h"

#include <grpcpp/grpcpp.h>
#include <memory>
#include <list>

#include "service/csds_service.h"

namespace flame {

class CsdAdminServer {
public:
    CsdAdminServer(CsdContext* cct, const std::string& addr) 
    : cct_(cct), addr_(addr), csds_service_(cct) { 
        __init(); 
    }

    ~CsdAdminServer() {}

    std::string get_addr() const { return addr_; }
    void set_addr(const std::string& addr) { addr_ = addr; }
    
    int run();

private:
    void __init();

    CsdContext* cct_;
    std::string addr_;

    service::CsdsServiceImpl csds_service_;

    grpc::ServerBuilder builder_;
    std::unique_ptr<grpc::Server> server_;
}; // class MgrServer

} // namespace flame

#endif // FLAME_CSD_ADMIN_H