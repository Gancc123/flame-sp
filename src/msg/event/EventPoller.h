#ifndef FLAME_MSG_EVENT_EVENT_POLLER_H
#define FLAME_MSG_EVENT_EVENT_POLLER_H

#include <sys/epoll.h>
#include <sys/time.h>
#include <vector>
#include <map>
#include <atomic>

#include "event.h"
#include "common/context.h"
#include "common/thread/mutex.h"

namespace flame{

class EventPoller{
    FlameContext *fct;
    int epfd;
    std::atomic<int> event_num;
    Mutex ecb_mutex;
    std::map<int,  EventCallBack *> ecb_map;
    struct epoll_event *events;
    int size;
public:
    int get_event_num() const { return event_num.load(); };
    int set_event(int fd, EventCallBack *ecb);
    int del_event(int fd);
    int process_events(std::vector<FiredEvent> &, struct timeval *);

    explicit EventPoller(FlameContext *fct, int nevent);
    ~EventPoller();
};

}

#endif //FLAME_MSG_EVENT_EVENT_POLLER_H