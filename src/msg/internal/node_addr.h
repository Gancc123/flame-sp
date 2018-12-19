#ifndef FLAME_MSG_INTERNAL_NODE_ADDR_H
#define FLAME_MSG_INTERNAL_NODE_ADDR_H

#include "common/context.h"
#include "types.h"
#include "util.h"
#include "ref_counted_obj.h"

#include <netinet/in.h>
#include <cstring>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cassert>

namespace flame{

template<class T>
struct compare_in_ptr{
    bool operator()( T* p1, T* p2 ){
        if(p1 == p2) return false;
        if(p1 == nullptr) return true;
        if(p2 == nullptr) return false;
        return *p1 < *p2;
    }
};

class NodeAddr : public RefCountedObject{
    uint8_t ttype = NODE_ADDR_TTYPE_UNKNOWN;
    union {
        sockaddr sa;
        sockaddr_in sin;
        sockaddr_in6 sin6;
    } u;
public:
    NodeAddr(FlameContext *fct) 
    : RefCountedObject(fct){
        std::memset(&u, 0, sizeof(u)); // !important for compare
    }
    explicit NodeAddr(FlameContext *fct, struct node_addr_t &addr)
    : RefCountedObject(fct){
        // will clean this->u.
        decode(&addr, sizeof(addr));
    }

    explicit NodeAddr(FlameContext *fct, NodeAddr &addr)
    : RefCountedObject(fct){
        this->u = addr.u;
    }

    ~NodeAddr(){};

    static std::string ttype_str(int ttype){
        switch(ttype){
        case NODE_ADDR_TTYPE_TCP:
            return "tcp";
        case NODE_ADDR_TTYPE_RDMA:
            return "rdma";
        case NODE_ADDR_TTYPE_UNKNOWN:
        default:
            return "unknown";
        }
    }

    void set_ttype(const std::string &t) {
        auto lt = str2lower(t);
        if(lt == "tcp"){
            this->ttype = NODE_ADDR_TTYPE_TCP;
        }else if(lt == "rdma"){
            this->ttype = NODE_ADDR_TTYPE_RDMA;
        }else{
            this->ttype = NODE_ADDR_TTYPE_UNKNOWN;
        }
    }
    void set_ttype(int ttype) {this->ttype = ttype;}
    int get_ttype() const {return this->ttype;}

    int get_addr_type() const {
        switch (u.sa.sa_family) {
        case AF_INET:
            return NODE_ADDR_ADDRTYPE_IPV4;
        case AF_INET6:
            return NODE_ADDR_ADDRTYPE_IPV6;
        }
        return NODE_ADDR_ADDRTYPE_UNKNOWN;
    }

    int get_family() const {
        return u.sa.sa_family;
    }
    void set_family(int f) {
        u.sa.sa_family = f;
    }

    sockaddr_in &in4_addr() {
        return u.sin;
    }
    const sockaddr_in &in4_addr() const{
        return u.sin;
    }
    sockaddr_in6 &in6_addr(){
        return u.sin6;
    }
    const sockaddr_in6 &in6_addr() const{
        return u.sin6;
    }
    const sockaddr *get_sockaddr() const {
        return &u.sa;
    }
    size_t get_sockaddr_len() const {
        switch (u.sa.sa_family) {
        case AF_INET:
            return sizeof(u.sin);
        case AF_INET6:
            return sizeof(u.sin6);
        }
        return sizeof(u);
    }
    bool set_sockaddr(const struct sockaddr &sa){
        switch (sa.sa_family) {
        case AF_INET:
            std::memcpy(&u.sin, &sa, sizeof(u.sin));
            break;
        case AF_INET6:
            std::memcpy(&u.sin6, &sa, sizeof(u.sin6));
            break;
        default:
            return false;
        }
        return true;
    }

    sockaddr_storage get_sockaddr_storage() const {
        sockaddr_storage ss;
        std::memcpy(&ss, &u, sizeof(u));
        std::memset((char*)&ss + sizeof(u), 0, sizeof(ss) - sizeof(u));
        return ss;
    }

    void set_port(int port) {
        switch (u.sa.sa_family) {
        case AF_INET:
            u.sin.sin_port = htons(port);
            break;
        case AF_INET6:
            u.sin6.sin6_port = htons(port);
            break;
        }
    }

    int get_port() const {
        switch (u.sa.sa_family) {
        case AF_INET:
            return ntohs(u.sin.sin_port);
            break;
        case AF_INET6:
            return ntohs(u.sin6.sin6_port);
            break;
        }
        return 0;
    }

    bool probably_equals(NodeAddr &o) const {
        if (is_blank_ip() || o.is_blank_ip())
            return true;
        if (std::memcmp(&u, &o.u, sizeof(u)) == 0)
            return true;
        return false;
    }
    
    bool is_same_host(NodeAddr &o) const {
        if (u.sa.sa_family != o.u.sa.sa_family)
            return false;
        if (u.sa.sa_family == AF_INET)
            return u.sin.sin_addr.s_addr == o.u.sin.sin_addr.s_addr;
        if (u.sa.sa_family == AF_INET6)
            return std::memcmp(u.sin6.sin6_addr.s6_addr,
                    o.u.sin6.sin6_addr.s6_addr,
                    sizeof(u.sin6.sin6_addr.s6_addr)) == 0;
        return false;
    }

    bool is_blank_ip() const {
        switch (u.sa.sa_family) {
        case AF_INET:
            return u.sin.sin_addr.s_addr == INADDR_ANY;
        case AF_INET6:
            return std::memcmp(&u.sin6.sin6_addr, &in6addr_any,
                         sizeof(in6addr_any)) == 0;
        default:
            return true;
        }
    }

    bool is_ip() const {
        switch (u.sa.sa_family) {
        case AF_INET:
        case AF_INET6:
            return true;
        default:
            return false;
        }
    }

    std::string ip_to_string() const;
    bool ip_from_string(const std::string);

    std::string to_string() const;

    ssize_t decode(const void *buf, size_t len);

    ssize_t encode(void *buffer, size_t len) const;

    bool operator<(NodeAddr& o){
        // compare sockaddr
        if (u.sa.sa_family != o.u.sa.sa_family)
            return u.sa.sa_family < o.u.sa.sa_family;
        if (u.sa.sa_family == AF_INET)
            if(u.sin.sin_addr.s_addr != o.u.sin.sin_addr.s_addr)
                return u.sin.sin_addr.s_addr < o.u.sin.sin_addr.s_addr;
        if (u.sa.sa_family == AF_INET6){
            int cmp_r = std::memcmp(u.sin6.sin6_addr.s6_addr,
                                        o.u.sin6.sin6_addr.s6_addr,
                                        sizeof(u.sin6.sin6_addr.s6_addr));
            if(cmp_r != 0)
                return cmp_r < 0;
        }
        if (get_port() != o.get_port())
            return get_port() < o.get_port();
        
        // same
        return false;
    }

};



}

#endif