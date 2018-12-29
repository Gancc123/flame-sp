#ifndef FLAME_MSG_NET_HANDLER_H
#define FLAME_MSG_NET_HANDLER_H

#include "msg/msg_context.h"
#include "internal/node_addr.h"

namespace flame{

class NetHandler{
    MsgContext *mct;
public:
    explicit NetHandler(MsgContext *c)
    :mct(c) {}

    int create_socket(int domain, bool reuse);
    int set_nonblock(int fd);
    void set_close_on_exec(int fd);
    int set_socket_options(int fd, bool nodelay, int size);
    int accept(int sfd, NodeAddr &caddr);
    int get_sockname(int fd, NodeAddr &addr);
};

}

#endif