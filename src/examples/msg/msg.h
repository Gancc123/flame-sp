#ifndef FLAME_EXAMPLES_MSG_MSG_H
#define FLAME_EXAMPLES_MSG_MSG_H

#include "msg/msg_core.h"
#include "common/context.h"
#include "util/clog.h"

#include <map>
#include <memory>

#define LOGP "example_msg"

namespace flame{
namespace msg{

class FakeCsdAddrResolver : public CsdAddrResolver{
    FlameContext *fct;
    std::map<uint64_t, csd_addr_t> addr_map;
public:
    FakeCsdAddrResolver(FlameContext *c) : fct(c) {};
    void set_addr(uint64_t csd_id, csd_addr_t &addr){
        addr_map[csd_id] = addr;
    }

    virtual int pull_csd_addr(std::list<csd_addr_t>& res,
                              const std::list<uint64_t>& csd_id_list) override;
};

int FakeCsdAddrResolver::pull_csd_addr(std::list<csd_addr_t>& res,
                                       const std::list<uint64_t>& csd_id_list){
    for(auto csd_id : csd_id_list){
        auto it = addr_map.find(csd_id);
        if(it != addr_map.end()){
            res.push_back(it->second);
        }
    }
    return res.size();
}

class MsgClient : public MsgChannel{
    FlameContext *fct;
    MsgContext *mct;
    bool is_mgr;
public:
    MsgClient(FlameContext *c, MsgContext *mc, bool mgr) 
    : fct(c), mct(mc), is_mgr(mgr){};
    //接收本地消息和远端消息
    virtual int on_conn_recv(Connection *conn, MessagePtr msg) override;
    virtual int on_local_recv(MessagePtr msg) override;
};

int MsgClient::on_conn_recv(Connection *conn, MessagePtr msg){
    fct->log()->linfo(LOGP, "recv %s", msg->to_string().c_str());
    if(is_mgr){
        uint64_t src, dst;
        src = msg->src();
        dst = msg->dst();
        msg->src(0, dst);
        msg->dst(0, src);
        msg->flags(MessageFlags::REQ_RES, true);
        //回复消息时，可附加conn参数，避免通过id查找连接。
        mct->dispatcher->deliver_msg(msg, conn);
    }
    return 0;
}

int MsgClient::on_local_recv(MessagePtr msg){
    fct->log()->linfo(LOGP, "local recv %s", msg->to_string().c_str());
    return 0;
}

MessagePtr gen_msg(uint64_t src, uint64_t dst){
    MessagePtr msg = std::make_shared<Message>();
    msg->req_id(gen_rand_seq());
    msg->src(0, src);
    msg->dst(0, dst);
    msg->req_typ(MessageType::IO);
    return msg;
}

}// namespace msg
}// namespace flame


#endif 