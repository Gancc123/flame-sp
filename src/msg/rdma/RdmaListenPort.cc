#include "RdmaListenPort.h"
#include "msg/internal/errno.h"
#include "msg/msg_def.h"
#include "msg/MsgManager.h"
#include "msg/NetHandler.h"
#include "RdmaPrepConn.h"

#include <unistd.h>

namespace flame{

RdmaListenPort::RdmaListenPort(FlameContext *fct, NodeAddr *listen_addr)
:ListenPort(fct, listen_addr){

}

RdmaListenPort* RdmaListenPort::create(FlameContext *fct, NodeAddr *listen_addr){
    int sfd, r;
    NetHandler net_handler(fct);
    NodeAddr *addr = listen_addr;
    auto family = addr->get_family();
    sfd = net_handler.create_socket(family, true);
    if(sfd < 0){
        return nullptr;
    }

    r = net_handler.set_nonblock(sfd);
    if (r < 0) {
        ::close(sfd);
        return nullptr;
    }

    net_handler.set_close_on_exec(sfd);
    r = net_handler.set_socket_options(sfd, false, 0);
    if (r < 0) {
        ::close(sfd);
        return nullptr;
    }

    r = ::bind(sfd, addr->get_sockaddr(), addr->get_sockaddr_len());
    if (r < 0) {
        r = -errno;
        ML(fct, error, "unable to bind to [{} {}] : {}", 
                        addr->ip_to_string(), addr->get_port(),
                        cpp_strerror(r));
        ::close(sfd);
        return nullptr;
    }

    r = ::listen(sfd, 512);
    if (r < 0) {
        r = -errno;
        ML(fct, error, "unable to listen on [{} {}]: {}",
                        addr->ip_to_string(), addr->get_port(),
                        cpp_strerror(r));
        ::close(sfd);
        return nullptr;
    }
    
    auto lp = new RdmaListenPort(fct, addr);
    lp->set_listen_fd(sfd);

    ML(fct, debug, "add listen port {}:{}",  listen_addr->ip_to_string(),
                                                    listen_addr->get_port());
    return lp;
}

RdmaListenPort::~RdmaListenPort(){
    ::close(get_listen_fd());
}

void RdmaListenPort::read_cb(){
    int sfd = this->fd, r;
    NetHandler net_handler(fct);
    NodeAddr *caddr =  nullptr;
    for(;;){
        if(!caddr){
            caddr = new NodeAddr(fct);
        }
        r = net_handler.accept(sfd, *caddr);
        if(r >= 0){  // success
            int cfd = r;
            auto conn = RdmaPrepConn::create(fct, cfd);
            assert(conn);
            conn->set_rdma_listen_port(this);

            auto worker = fct->msg()->manager->get_lightest_load_worker();
            assert(worker);
            worker->add_event(conn);
            conn->set_owner(worker);

            if(get_listener()){
                get_listener()->on_listen_accept(this, conn->get_rdma_conn());
            }

            caddr->put();
            caddr = nullptr;
            conn->put();
            continue;
        } else if (r == -EINTR) {
            continue;
        } else if (r == -EAGAIN) {
            break;
        } else if (r == -EMFILE || r == -ENFILE) {
            ML(fct, error, "open file descriptions limit reached sd = {}"
                            " errno {} {}", sfd, r, cpp_strerror(r));
            break;
        } else if (r == -ECONNABORTED) {
            ML(fct, error, "it was closed because of rst arrived sd =  {}"
                            " errno {} {}", sfd, r, cpp_strerror(r));
            continue;
        } else {
            ML(fct, error, "no incoming connection? errno {} {}", 
                                                            r, cpp_strerror(r));
            break;
        }
    }
    if(caddr){
        caddr->put();
        caddr = nullptr;
    }
}

void RdmaListenPort::write_cb(){

}

void RdmaListenPort::error_cb(){
    if(this->get_listener()){
        this->get_listener()->on_listen_error(get_listen_addr());
    }
}

}