#include "TcpListenPort.h"
#include "TcpStack.h"
#include "msg/internal/errno.h"
#include "msg/msg_def.h"
#include "TcpConnection.h"
#include "msg/NetHandler.h"

#include <unistd.h>

namespace flame{
namespace msg{

TcpListenPort::TcpListenPort(MsgContext *mct, NodeAddr *listen_addr)
:ListenPort(mct, listen_addr){

}

TcpListenPort* TcpListenPort::create(MsgContext *mct, NodeAddr *listen_addr){
    int sfd, r;
    NetHandler net_handler(mct);
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
        ML(mct, error, "unable to bind to [{} {}] : ", 
                        addr->ip_to_string(), addr->get_port(),
                        cpp_strerror(r));
        ::close(sfd);
        return nullptr;
    }

    r = ::listen(sfd, 512);
    if (r < 0) {
        r = -errno;
        ML(mct, error, "unable to listen on [{} {}]: {}",
                        addr->ip_to_string(), addr->get_port(),
                        cpp_strerror(r));
        ::close(sfd);
        return nullptr;
    }
    
    auto lp = new TcpListenPort(mct, addr);
    lp->set_listen_fd(sfd);

    ML(mct, debug, "add listen port {}:{}", listen_addr->ip_to_string(),
                                                    listen_addr->get_port());
    return lp;
}

TcpListenPort::~TcpListenPort(){
    ::close(get_listen_fd());
}

void TcpListenPort::read_cb(){
    int sfd = this->fd, r;
    NetHandler net_handler(mct);
    NodeAddr caddr(mct);
    for(;;){
        r = net_handler.accept(sfd, caddr);
        if(r >= 0){  // success
            int cfd = r;
            auto conn = TcpConnection::create(mct, cfd);
            if(this->get_listener()){
                this->get_listener()->on_listen_accept(this, conn);
            }
            conn->put();
            continue;
        } else if (r == -EINTR) {
            continue;
        } else if (r == -EAGAIN) {
            break;
        } else if (r == -EMFILE || r == -ENFILE) {
            ML(mct, error, "open file descriptions limit reached sd = {}"
                            " errno {} {}", sfd, r, cpp_strerror(r));
            break;
        } else if (r == -ECONNABORTED) {
            ML(mct, error, "it was closed because of rst arrived sd =  {}"
                            " errno {} {}", sfd, r, cpp_strerror(r));
            continue;
        } else {
            ML(mct, error, "no incoming connection? errno {} {}", 
                                                            r, cpp_strerror(r));
            break;
        }
    }
}

void TcpListenPort::write_cb(){

}

void TcpListenPort::error_cb(){
    if(this->get_listener()){
        this->get_listener()->on_listen_error(get_listen_addr());
    }
}

} //namespace msg
} //namespace flame