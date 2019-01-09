#ifndef FLAME_MSG_INTERNAL_TYPES_HELPER_H
#define FLAME_MSG_INTERNAL_TYPES_HELPER_H

#include "node_addr.h"

#include <string>
#include <tuple>

namespace flame{
namespace msg{

/**
 * msger_id
 */
struct msger_id_comparator{
    bool operator()(const msger_id_t& lhs, const msger_id_t& rhs){
        return std::tie(lhs.ip, lhs.port) < std::tie(rhs.ip, rhs.port);
    }
};

std::string msger_id_to_str(msger_id_t msger_id);

NodeAddr *node_addr_from_msger_id(MsgContext *mct, msger_id_t id);
msger_id_t msger_id_from_msg_node_addr(NodeAddr *addr);

/**
 * msger_ttype
 */
enum class msg_ttype_t : uint8_t{
    UNKNOWN,
    TCP,
    RDMA
};

msg_ttype_t msg_ttype_from_str(const std::string &v);
std::string msg_ttype_to_str(msg_ttype_t ttype);

/**
 * conn_id_t 
 */
struct conn_id_t{
    msg_ttype_t type;
    uint32_t id; //fd for tcp, qpn for rdma
    std::string to_string() const;
};

struct conn_id_comparator{
    bool operator()(const conn_id_t& lhs, const conn_id_t& rhs){
        return std::tie(lhs.type, lhs.id) < std::tie(rhs.type, rhs.id);
    }
};

} //namespace msg
} //namespace flame

#endif //FLAME_MSG_INTERNAL_TYPES_HELPER_H