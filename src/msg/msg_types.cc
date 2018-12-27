#include "msg_types.h"
#include "msg_def.h"
#include "util/fmt.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <cstdlib>

namespace flame{

std::string msger_id_to_str(msger_id_t msger_id){
    auto s = fmt::format("[msger_id {} {}]", msger_id.ip, msger_id.port);
    return s;
}

NodeAddr *node_addr_from_msger_id(MsgContext *mct, msger_id_t id){
    NodeAddr *node_addr = new NodeAddr(mct);
    node_addr->set_ttype(NODE_ADDR_TTYPE_TCP);
    node_addr->set_family(AF_INET);
    std::memcpy(&node_addr->in4_addr().sin_addr, &id.ip, 
                    sizeof(node_addr->in4_addr().sin_addr));
    node_addr->set_port(id.port);
    return node_addr;
}

msger_id_t msger_id_from_node_addr(NodeAddr *addr){
    msger_id_t msger_id;
    std::memcpy(&msger_id.ip, &addr->in4_addr().sin_addr,
                                             sizeof(msger_id.ip));
    msger_id.port = addr->get_port();
    return msger_id;
}

msg_ttype_t msg_ttype_from_str(const std::string &v){
    auto tmp = str2upper(v);
    if(tmp == "TCP") return msg_ttype_t::TCP;
    if(tmp == "RDMA") return msg_ttype_t::RDMA;
    return msg_ttype_t::UNKNOWN;
}

std::string msg_ttype_to_str(msg_ttype_t ttype){
    switch(ttype){
    case msg_ttype_t::TCP:
        return "TCP";
    case msg_ttype_t::RDMA:
        return "RDMA";
    default:
        return "UNKNOWN";
    }
}

std::string conn_id_t::to_string() const{
    auto s = fmt::format("[{} {}]", msg_ttype_to_str(type), id);
    return s;
}

}