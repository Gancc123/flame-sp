#include "msg_module.h"
#include "msg_common.h"
#include "msg/internal/msg_config.h"
#include "Msg.h"
#include "Message.h"
#include "Stack.h"
#include "MsgManager.h"

#ifdef HAVE_RDMA
    #include "msg/rdma/RdmaStack.h"
#endif

#include <functional>
#include <tuple>
#include <string>

namespace flame{

int msg_module_init(FlameContext *fct, MsgerCallback *msger_cb,
                                                            MsgConfig *config){
    rand_init_helper::init();
    MsgModule *msg_module = new MsgModule();
    fct->set_msg(msg_module);

    if(config == nullptr){
        config = new MsgConfig(fct);
        if(config->load()){
            return -1;
        }
    }
    msg_module->config = config;

    MsgManager *msg_manager = new MsgManager(fct);
    msg_manager->set_msger_cb(msger_cb);

    msg_module->manager = msg_manager;

    ML(fct, trace, "before init all stack.");
    Stack::init_all_stack();
    ML(fct, trace, "after init all stack.");

    std::string transport;
    std::string address;
    int min_port, max_port, i;
    NodeAddr *addr = nullptr;
    for(const auto &listen_port : config->node_listen_ports){
        std::tie(transport, address, min_port, max_port) = listen_port;
        if(!addr){
            addr = new NodeAddr(fct);
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
            ML(fct, warn, "add listen port failed : {}@{}/{}-{}", 
                                transport, address, min_port, max_port);
        }
    }
    if(addr){
        addr->put();
    }

    ML(fct, trace, "before msg manager start.");
    msg_manager->start();
    ML(fct, trace, "after msg manager start.");

    return 0;
}


int msg_module_finilize(FlameContext *fct){
    auto msg_module = fct->msg();
    auto msg_manager = msg_module->manager;

    msg_manager->clear_before_stop();

    ML(fct, trace, "before stack clear_all_before_stop.");
    Stack::clear_all_before_stop();
    ML(fct, trace, "after stack clear_all_before_stop.");

    ML(fct, trace, "before msg manager stop.");
    msg_manager->stop();
    ML(fct, trace, "after msg manager stop.");

    ML(fct, trace, "before fin all stack.");
    Stack::fin_all_stack();
    ML(fct, trace, "after fin all stack.");

    fct->set_msg(nullptr);

    delete msg_module->manager;
    delete msg_module->config;
    delete msg_module;

    return 0;
}


}