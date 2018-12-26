#include "NetHandler.h"
#include "msg/msg_context.h"
#include "internal/errno.h"
#include "msg/msg_def.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <memory>

namespace flame{

int NetHandler::create_socket(int domain, bool reuse){
    int sfd, r;
    sfd = ::socket(domain, SOCK_STREAM, 0);
    if(sfd < 0){
        r = errno;
        ML(mct, error, "create socket failed. {}", cpp_strerror(r));
        return -r;
    }
    int on = 1;
    if (::setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        r = errno;
        ML(mct, error, "setsockopt SO_REUSEADDR failed: {}", cpp_strerror(r));
        close(sfd);
        return -r;
    }
    return sfd;
}

int NetHandler::set_nonblock(int fd){
    int flags;
    int r = 0;

    /* Set the socket nonblocking.
    * Note that fcntl(2) for F_GETFL and F_SETFL can't be
    * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) < 0 ) {
        r = errno;
        ML(mct, error, "fcntl(F_GETFL) failed: {}", cpp_strerror(r));
        return -r;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        r = errno;
        ML(mct, error, "fcntl(F_SETFL,O_NONBLOCK) failed: {}", cpp_strerror(r));
        return -r;
    }
    return 0;
}

void NetHandler::set_close_on_exec(int fd){
    int flags = fcntl(fd, F_GETFD, 0);
    if (flags < 0) {
        int r = errno;
        ML(mct, error, "fcntl(F_GETFD) failed: {}", cpp_strerror(r));
        return;
    }
    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC)) {
        int r = errno;
        ML(mct, error, "fcntl(F_SETFD) failed: {}", cpp_strerror(r));
    }
}

int NetHandler::set_socket_options(int fd, bool nodelay, int size){
    int r = 0;
    // disable Nagle algorithm?
    if (nodelay) {
        int flag = 1;
        r = ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                                     (char*)&flag, sizeof(flag));
        if (r < 0) {
            r = errno;
            ML(mct, error, "couldn't set TCP_NODELAY: {}", cpp_strerror(r));
        }
    }
    if (size) {
        r = ::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void*)&size, sizeof(size));
        if (r < 0)  {
            r = errno;
            ML(mct, error, "couldn't set SO_RCVBUF to {}:{}", 
                                                    size , cpp_strerror(r));
        }
    }

    return -r;
}

int NetHandler::accept(int sfd, NodeAddr &caddr){
    int cfd, r;
    struct sockaddr sock_addr;
    socklen_t sock_len = sizeof(sock_addr);
    cfd = ::accept(sfd, &sock_addr, &sock_len);
    if(cfd < 0){
        r = errno;
        //ML(mct, error, "accept failed: {}", cpp_strerror(r));
        return -r;
    }
    caddr.set_sockaddr(sock_addr);
    return cfd;
}

int NetHandler::get_sockname(int fd, NodeAddr &addr){
    int r;
    struct sockaddr sock_addr;
    socklen_t sock_len = sizeof(sock_addr);
    r = ::getsockname(fd, &sock_addr, &sock_len);
    if(r < 0){
        r = errno;
        ML(mct, error, "getsockname failed: {}", cpp_strerror(r));
        return -r;
    }
    addr.set_sockaddr(sock_addr);
    return r;
}

}