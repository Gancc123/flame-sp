#include "msg_context.h"
#include "msg_common.h"
#include "msg/internal/msg_config.h"
#include "Msg.h"
#include "Message.h"
#include "Stack.h"
#include "MsgManager.h"
#include "msg/dispatcher/MsgDispatcher.h"

#ifdef HAVE_RDMA
    #include "msg/rdma/RdmaStack.h"
#endif

#include <functional>
#include <tuple>
#include <string>

namespace flame{
namespace msg{

msg_log_level_t g_msg_log_level = msg_log_level_t::print;

int MsgContext::load_config(){
    if(config == nullptr){
        config = MsgConfig::load_config(fct);
    }
    if(config == nullptr){
        return -1;
    }
    return 0;
}

int MsgContext::init(MsgerCallback *msger_cb, CsdAddrResolver *r){
    //fct can't be null.
    if(fct == nullptr) return -1;

    rand_init_helper::init();

    if(load_config()){
        return -1;
    }

    g_msg_log_level = config->msg_log_level;

    if(msger_cb == nullptr){
        ML(this, info, "init MsgDispatcher");
        dispatcher = new MsgDispatcher(this);
        msger_cb = dispatcher;
    }else{
        ML(this, info, "use custome MsgerCallback, not init MsgDispatcher.");
    }

    csd_addr_resolver = r;
    if(csd_addr_resolver == nullptr){
        ML(this, warn, "CsdAddrResolver is null, can't resolve csd addrs!");
    }

    MsgManager *msg_manager = new MsgManager(this);
    msg_manager->set_msger_cb(msger_cb);

    this->manager = msg_manager;

    ML(this, trace, "before init all stack.");
    Stack::init_all_stack(this);
    ML(this, trace, "after init all stack.");

    std::string transport;
    std::string address;
    int min_port, max_port, i;
    NodeAddr *addr = nullptr;
    for(const auto &listen_port : config->node_listen_ports){
        std::tie(transport, address, min_port, max_port) = listen_port;
        if(!addr){
            addr = new NodeAddr(this);
        }
        addr->set_ttype(transport);
        addr->ip_from_string(address);
        auto ttype  = msg_ttype_from_str(transport);

        for(i = min_port; i <= max_port; ++i){
            addr->set_port(i);
            if(msg_manager->add_listen_port(addr, ttype)){
                addr->put();
                addr = nullptr;
                break;
            }
        }

        if(i > max_port){
            ML(this, warn, "add listen port failed : {}@{}/{}-{}", 
                                transport, address, min_port, max_port);
        }
    }
    if(addr){
        addr->put();
    }

    ML(this, trace, "before msg manager start.");
    msg_manager->start();
    ML(this, trace, "after msg manager start.");

    return 0;
}


int MsgContext::fin(){
    if(manager == nullptr) return 0;

    auto msg_manager = manager;

    msg_manager->clear_before_stop();

    ML(this, trace, "before stack clear_all_before_stop.");
    Stack::clear_all_before_stop();
    ML(this, trace, "after stack clear_all_before_stop.");

    ML(this, trace, "before msg manager stop.");
    msg_manager->stop();
    ML(this, trace, "after msg manager stop.");

    ML(this, trace, "before fin all stack.");
    Stack::fin_all_stack();
    ML(this, trace, "after fin all stack.");

    if(manager){
        delete manager;
        manager = nullptr;
    }

    if(dispatcher){
        delete dispatcher;
        dispatcher = nullptr;
    }
    
    if(config){
        delete config;
        config = nullptr;
    }

    return 0;
}


} //namespace msg
} //namespace flame