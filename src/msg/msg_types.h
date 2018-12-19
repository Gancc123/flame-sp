#ifndef FLAME_MSG_MSG_TYPES_H
#define FLAME_MSG_MSG_TYPES_H

#include "internal/types.h"
#include "internal/buffer_list.h"
#include "internal/ref_counted_obj.h"
#include "internal/node_addr.h"

#include <netinet/in.h>
#include <cstring>
#include <string>
#include <cstdlib>
#include <mutex>
#include <tuple>
#include <sstream>

namespace flame{

/**
 * msger_id
 */
struct msger_id_comparator{
    bool operator()(const msger_id_t& lhs, const msger_id_t& rhs){
        return std::tie(lhs.ip, lhs.port) < std::tie(rhs.ip, rhs.port);
    }
};

std::string msger_id_to_str(msger_id_t msger_id);

NodeAddr *node_addr_from_msger_id(FlameContext *fct, msger_id_t id);
msger_id_t msger_id_from_node_addr(NodeAddr *addr);


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

/**
 * interface
 */
class ListenPort;
class Connection;
class NodeAddr;
class Msg;
class Session;

class ListenPortListener{
public:
    virtual void on_listen_accept(ListenPort *, Connection *conn) = 0;
    virtual void on_listen_error(NodeAddr *listen_addr) = 0;
};

class ConnectionListener{
public:
    virtual void on_conn_recv(Connection *, Msg *) = 0;
    virtual void on_conn_error(Connection *conn) = 0;
};

struct MsgData{
    virtual int encode(BufferList &bl) = 0;
    virtual int decode(BufferList::iterator &it) = 0;
    virtual size_t size() { return 0; }
    virtual ~MsgData() {};
};

class MsgerCallback{
public:
    virtual void on_listen_accept(ListenPort *lp, Connection *conn) {};
    virtual void on_listen_error(NodeAddr *listen_addr) {};
    virtual void on_conn_error(Connection *conn) {};
    virtual void on_conn_declared(Connection *conn, Session *s) {};
    
    virtual void on_conn_recv(Connection *, Msg *) = 0;
};

}

#endif 