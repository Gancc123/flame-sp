#ifndef FLAME_SERVICE_MGR_SERVER_H
#define FLAME_SERVICE_MGR_SERVER_H

#include "mgr_context.h"
#include "service/flame_service.h"
#include "service/internal_service.h"

#include <grpcpp/grpcpp.h>
#include <memory>
#include <list>

namespace flame {

class MgrService {
public:
    virtual ~MgrService() {}

protected:
    MgrService(MgrContext* mct) : mct_(mct), fct_(mct->fct()) {}

    MgrContext* mct_;
    FlameContext* fct_;
}; // class MgrService 

class MgrServer {
public:
    MgrServer(MgrContext* mct, const std::string& addr) 
    : mct_(mct), addr_(addr), flame_service_(mct), internal_service_(mct) { 
        __init(); 
    }

    ~MgrServer() {}

    std::string get_addr() const { return addr_; }
    void run();

private:
    void __init();

    MgrContext* mct_;
    std::string addr_;

    FlameServiceImpl flame_service_;
    InternalServiceImpl internal_service_;

    grcp::ServerBuilder builder_;
    std::unique_ptr<grcp::Server> server_;
}; // class MgrServer

} // namespace flame

#endif // FLAME_SERVICE_MGR_SERVER_H