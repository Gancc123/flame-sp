#include "node_addr.h"
#include "msg/msg_def.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <sstream>


namespace flame{

std::string NodeAddr::to_string() const{
    std::stringstream ss;
    ss << "[NodeAddr " 
        << ip_to_string()  << "/" << get_port() 
        << "](" << (void *)this << ")";
    return ss.str();
}

std::string NodeAddr::ip_to_string() const{
    const char *host_ip = nullptr;
    char addr_buf[INET6_ADDRSTRLEN];
    switch (get_family()) {
    case AF_INET:
        host_ip = inet_ntop(AF_INET, &in4_addr().sin_addr, 
                            addr_buf, INET_ADDRSTRLEN);
        break;
    case AF_INET6:
        host_ip = inet_ntop(AF_INET6, &in6_addr().sin6_addr, 
                            addr_buf, INET6_ADDRSTRLEN);
        break;
    default:
        break;
    }
    return host_ip ? host_ip : "";
}

bool NodeAddr::ip_from_string(const std::string str){
    // inet_pton() requires a null terminated input, so let's fill two
    // buffers, one with ipv4 allowed characters, and one with ipv6, and
    // then see which parses.
    char buf4[39];
    char *o = buf4;
    const char *p = str.c_str();
    while (o < buf4 + sizeof(buf4) &&
        *p && ((*p == '.') ||
            (*p >= '0' && *p <= '9'))) {
        *o++ = *p++;
    }
    *o = 0;

    char buf6[64];  // actually 39 + null is sufficient.
    o = buf6;
    p = str.c_str();
    while (o < buf6 + sizeof(buf6) &&
        *p && ((*p == ':') ||
            (*p >= '0' && *p <= '9') ||
            (*p >= 'a' && *p <= 'f') ||
            (*p >= 'A' && *p <= 'F'))) {
        *o++ = *p++;
    }
    *o = 0;
    

    // ipv4?
    struct in_addr a4;
    struct in6_addr a6;
    if (inet_pton(AF_INET, buf4, &a4)) {
        u.sin.sin_addr.s_addr = a4.s_addr;
        u.sa.sa_family = AF_INET;
    } else if (inet_pton(AF_INET6, buf6, &a6)) {
        u.sa.sa_family = AF_INET6;
        memcpy(&u.sin6.sin6_addr, &a6, sizeof(a6));
    } else {
        return false;
    }
    return true;
}

ssize_t NodeAddr::decode(const void *buf, size_t len){
    if(len < sizeof(struct node_addr_t)){
        ML(mct, error, "buffer length must > sizeof struct node_addr_t");
        return -1;
    }
    char *buffer = (char *)buf;
    struct node_addr_t *addr = (struct node_addr_t *)buffer;
    std::memset(&u, 0, sizeof(u));
    this->ttype = addr->ttype;
    switch(addr->addr_type){
    case NODE_ADDR_ADDRTYPE_IPV4:
        u.sa.sa_family = AF_INET;
        u.sin.sin_port = htons(addr->port);
        std::memcpy(&u.sin.sin_addr, &addr->addr.ipv4,
                        sizeof(addr->addr.ipv4));
        break;
    case NODE_ADDR_ADDRTYPE_IPV6:
        u.sa.sa_family = AF_INET6;
        u.sin6.sin6_port = htons(addr->port);
        std::memcpy(&u.sin6.sin6_addr, &addr->addr.ipv6,
                        sizeof(addr->addr.ipv6));
        break;
    default:
        ML(mct, error, "Invalid addr_type.");
        return -1;
    }
    return sizeof(struct node_addr_t);
}

ssize_t NodeAddr::encode(void *buf, size_t len) const {
    if(len < sizeof(struct node_addr_t)){
        ML(mct, error, "buffer length must > sizeof struct node_addr_t");
        return -1;
    }
    char *buffer = (char *)buf;
    struct node_addr_t *addr = (struct node_addr_t *)buffer;
    std::memset(buffer, 0, sizeof(struct node_addr_t));
    addr->ttype = get_ttype();
    addr->addr_type = get_addr_type();
    addr->port = get_port();
    switch(addr->addr_type){
    case NODE_ADDR_ADDRTYPE_IPV4:
        std::memcpy(&addr->addr.ipv4, &u.sin.sin_addr,
                        sizeof(addr->addr.ipv4));
        break;
    case NODE_ADDR_ADDRTYPE_IPV6:
        std::memcpy(&addr->addr.ipv6, &u.sin6.sin6_addr,
                        sizeof(addr->addr.ipv6));
        break;
    default:
        ML(mct, error, "Invalid addr_type.");
        return -1;
    }
    return sizeof(struct node_addr_t);
}


}