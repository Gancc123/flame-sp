#ifndef FLAME_MSG_SOCKET_TCP_STACK_H
#define FLAME_MSG_SOCKET_TCP_STACK_H

#include "msg/Stack.h"
#include "msg/msg_types.h"
#include "msg/event/event.h"
#include "msg/Connection.h"
#include "msg/ListenPort.h"

#include <memory>
#include <cassert>
#include <pthread.h>
#include <signal.h>
#include <time.h>

namespace flame{

struct sigpipe_stopper {
    bool blocked;
    sigset_t existing_mask;
    sigset_t pipe_mask;
    sigpipe_stopper() {
        sigemptyset(&pipe_mask);
        sigaddset(&pipe_mask, SIGPIPE);
        sigset_t signals;
        sigemptyset(&signals);
        sigpending(&signals);
        if (sigismember(&signals, SIGPIPE)) {
            blocked = false;
        } else {
            blocked = true;
            int r = pthread_sigmask(SIG_BLOCK, &pipe_mask, &existing_mask);
            assert(r == 0);
        }
    }
    ~sigpipe_stopper() {
        if (blocked) {
            struct timespec nowait{0};
            int r = sigtimedwait(&pipe_mask, 0, &nowait);
            assert(r == EAGAIN || r == 0);
            r = pthread_sigmask(SIG_SETMASK, &existing_mask, 0);
            assert(r == 0);
        }
    }
};

class TcpStack: public Stack{
    FlameContext *fct;
public:
    explicit TcpStack(FlameContext *c):fct(c){};
    virtual ListenPort* create_listen_port(NodeAddr *addr) override;

    virtual Connection* connect(NodeAddr *addr) override;
};

}

#endif