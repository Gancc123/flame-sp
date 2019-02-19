#include "common/context.h"
#include "util/clog.h"

#include "msg.h"

#define CFG_PATH "flame_mgr.cfg"

using namespace flame;

/**
 * 上层调用主要围绕Message, MsgDispatcher, MsgChannel几个类:
 * 1. 初始化fct
 * 2. 创建CsdAddrResolver实例
 * 3. 创建并初始化mct
 * 4. 创建MsgChannel实例，并注册给mct->dispatcher
 * 
 * -- 消息模块处于工作状态 --
 * 
 * 5. mct->fin() 结束消息模块，释放消息模块内部资源
 * 6. 释放相关资源
 * 
 * 待定：
 * 目前Message的定义还有待完善。
 * 一些id的关系还有点乱。
 */

int main(){
    //初始化fct
    auto fct = FlameContext::get_context();
    if(!fct->init_config(CFG_PATH)){
        clog("init config failed.");
        return -1;
    }
    if(!fct->init_log("", "TRACE", "mgr")){
        clog("init log failed.");
        return -1;
    }

     //初始化addr_resolver
    auto addr_resolver = new msg::FakeCsdAddrResolver(fct);
    csd_addr_t addr;
    node_addr_t na(0, 0, 0, 0, 5555);
    node_addr_t na_rdma(0, 0, 0, 0, 6666);
    addr.csd_id = 0x22;  //target id
    addr.admin_addr = na;
    addr.io_addr = na_rdma;
    addr_resolver->set_addr(0x22, addr); 

    //创建MsgContext
    auto mct = new msg::MsgContext(fct);

    ML(mct, info, "init complete.");
    ML(mct, info, "load cfg: " CFG_PATH);
    
    //初始化MsgContext
    //将addr_resolver注册给mct
    ML(mct, info, "before msg module init");
    mct->init(nullptr, addr_resolver); //第一个参数保持为null即可
    ML(mct, info, "after msg module init");

    ML(mct, info, "msger_id {:x} {:x} ", mct->config->msger_id.ip,
                                         mct->config->msger_id.port);

    //创建一个MsgChannel实例，以便接收信息
    auto  client = new msg::MsgClient(fct, mct, true);
    //将MsgChannel实列注册给mct->dispatcher
    mct->dispatcher->reg_channel(0x11, client);

    std::getchar();

    //结束消息模块，释放相关资源
    ML(mct, info, "before msg module fin");
    mct->fin();
    ML(mct, info, "after msg module fin");

    delete client;

    delete mct;

    delete addr_resolver;

    return 0;
}