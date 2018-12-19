#include "Stack.h"
#include "msg/socket/TcpStack.h"
#include "common/context.h"
#include "msg_def.h"

#ifdef HAVE_RDMA
    #include "msg/rdma/RdmaStack.h"
#endif

namespace flame{

TcpStack* Stack::get_tcp_stack(){
    static TcpStack instance(FlameContext::get_context());
    return &instance;
}

int Stack::init_all_stack(){
    int r = 0;
    r = Stack::get_tcp_stack()->init();
#ifdef HAVE_RDMA
    if(r) return r;
    auto fct = FlameContext::get_context();
    if(fct->msg()->config->rdma_enable){
        r = Stack::get_rdma_stack()->init();
    }
#endif
    return r;
}

int Stack::clear_all_before_stop(){
    int r = 0;
#ifdef HAVE_RDMA
    auto fct = FlameContext::get_context();
    if(fct->msg()->config->rdma_enable){
        r = Stack::get_rdma_stack()->clear_before_stop();
    }
    if(r) return r;
#endif
    r = Stack::get_tcp_stack()->clear_before_stop();
    return r;
}

int Stack::fin_all_stack(){
    int r = 0;
#ifdef HAVE_RDMA
    auto fct = FlameContext::get_context();
    if(fct->msg()->config->rdma_enable){
        r = Stack::get_rdma_stack()->fin();
    }
    if(r) return r;
#endif
    r = Stack::get_tcp_stack()->fin();
    return r;
}

#ifdef HAVE_RDMA
RdmaStack* Stack::get_rdma_stack(){
    auto fct = FlameContext::get_context();
    if(!fct->msg()->config->rdma_enable){
        ML(fct, error, "RDMA disabled!");
        return nullptr;
    }
    static RdmaStack instance(FlameContext::get_context());
    return &instance;
}
#endif

}