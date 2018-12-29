#ifndef FLAME_MSG_MSG_WORKER_H
#define FLAME_MSG_MSG_WORKER_H

#include "msg_common.h"
#include "common/thread/mutex.h"
#include "common/thread/thread.h"
#include "msg/event/EventPoller.h"

#include <deque>
#include <map>
#include <atomic>
#include <functional>
#include <chrono>


namespace flame{

typedef std::function<void(void)> work_fn_t;

class MsgWorker;

class MsgWorkerThread : public Thread{
    MsgWorker *worker;
public:
    explicit MsgWorkerThread(MsgWorker *w)
    : worker(w) {};
    ~MsgWorkerThread() override{};
protected:
    virtual void entry() override;

};

struct HandleNotifyCallBack : public EventCallBack{
    HandleNotifyCallBack(MsgContext *c)
    : EventCallBack(c, FLAME_EVENT_READABLE){}
    virtual void read_cb() override;
};


class MsgWorker{
    using time_point = std::chrono::steady_clock::time_point;
    using clock_type = std::chrono::steady_clock;
    MsgContext *mct;
    int index;
    std::string name;

    EventPoller event_poller;

    std::atomic<uint64_t> time_work_next_id;
    std::multimap<time_point, std::pair<uint64_t, work_fn_t>> time_works;
    std::map<uint64_t, 
            std::multimap<time_point, std::pair<uint64_t, work_fn_t>>::iterator
            > tw_map;

    Mutex external_mutex;
    std::deque<work_fn_t> external_queue;
    std::atomic<int> external_num;
    std::atomic<bool> is_running; 

    std::atomic<int> extra_job_num;

    int notify_receive_fd;
    int notify_send_fd;

    MsgWorkerThread worker_thread;

    void add_time_work(time_point expire, work_fn_t work_fn, uint64_t id);
    void del_time_work(uint64_t time_work_id);
    int  process_time_works();

    int set_nonblock(int sd);
public:
    friend class MsgWorkerThread;

    explicit MsgWorker(MsgContext *c, int i);
    ~MsgWorker();

    int get_job_num();
    void update_job_num(int v);

    int get_event_num();
    int add_event(EventCallBack *ecb);
    int del_event(int fd);

    void wakeup();
    bool am_self() const{
        return worker_thread.am_self();
    }
    int get_id() const{
        return index;
    }

    std::string get_name() const{
        return name;
    }

    uint64_t post_time_work(uint64_t microseconds, work_fn_t work_fn);
    
    /***
     * 不保证一定能及时取消掉time_work
     */
    void cancel_time_work(uint64_t time_work_id);

    void post_work(work_fn_t work);

    void start();
    void stop();
    bool running() const{
        return is_running.load();
    }

    void process();
    void drain();

};


}


#endif