#ifndef FLAME_MSG_EVENT_EVENT_H
#define FLAME_MSG_EVENT_EVENT_H

#include "msg/internal/ref_counted_obj.h"

#include <cstdint>
#include <sys/epoll.h>

#define FLAME_EVENT_NONE       0x0
#define FLAME_EVENT_READABLE   0x1
#define FLAME_EVENT_WRITABLE   0x2
#define FLAME_EVENT_ERROR      0x4

namespace flame{

struct EventCallBack : public RefCountedObject{
    uint64_t fd;
    int mask;
    virtual void read_cb() {};
    virtual void write_cb() {};
    virtual void error_cb() {};
    explicit EventCallBack(FlameContext *fct) 
    : RefCountedObject(fct), mask(FLAME_EVENT_NONE) {};
    explicit EventCallBack(FlameContext *fct, int m) 
    : RefCountedObject(fct), mask(m) {}; 
    virtual ~EventCallBack() {};
    
};

struct FiredEvent{
    int mask;
    struct EventCallBack *ecb;
};

}

#endif //FLAME_MSG_EVENT_EVENT_H