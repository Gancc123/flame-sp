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
namespace msg{

typedef std::function<void(void)> work_fn_t;

//Return non-zero means that poller has processed some event.
typedef std::function<int(void)> poller_fn_t;

typedef void(* work_fn_p)(void *arg1, void *arg2);
typedef int(* poller_fn_p)(void *ctx);

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

enum class msg_worker_type_t{
    UNKNOWN = 0,
    THREAD = 1,
    SPDK = 2
};

class MsgWorker{
protected:
    int index;
    MsgContext *mct;
public:
    explicit MsgWorker(MsgContext *c, int i)
    : mct(c), index(i) {};
    virtual ~MsgWorker() {};

    virtual int set_affinity(int id) = 0;

    virtual msg_worker_type_t type() const { 
        return msg_worker_type_t::UNKNOWN; 
    }

    virtual int get_job_num() = 0;
    virtual void update_job_num(int v) = 0;

    virtual int get_event_num() = 0;
    virtual int add_event(EventCallBack *ecb) = 0;
    virtual int del_event(int fd) = 0;

    virtual void wakeup() = 0;

    virtual bool am_self() const = 0;

    int get_id() const{ return index; }
    virtual std::string get_name() const = 0; 

    virtual uint64_t post_time_work(uint64_t microseconds, 
                                    work_fn_t work_fn) = 0;
    virtual void cancel_time_work(uint64_t time_work_id) = 0;

    virtual void post_work(work_fn_t work) = 0;
    virtual void post_work(work_fn_p work_fn, void *arg1, void *arg2){
        post_work([work_fn, arg1, arg2](){
            work_fn(arg1, arg2);
        });
    }

    virtual uint64_t reg_poller(poller_fn_t poller_fn) = 0;
    virtual uint64_t reg_poller(poller_fn_p poller_fn, void *arg){
        return reg_poller([poller_fn, arg]()->int{
            return poller_fn(arg);
        });
    }
    virtual void unreg_poller(uint64_t poller_id) = 0;

    virtual void start() = 0;
    virtual void stop() = 0;
    
    virtual bool running() const = 0;

    virtual void process() = 0;
    virtual void drain() = 0;
};


class ThrMsgWorker : public MsgWorker{
    using time_point = std::chrono::steady_clock::time_point;
    using clock_type = std::chrono::steady_clock;
    std::string name;

    EventPoller event_poller;

    std::atomic<uint64_t> next_id;
    std::multimap<time_point, std::pair<uint64_t, work_fn_t>> time_works;
    std::map<uint64_t, 
            std::multimap<time_point, std::pair<uint64_t, work_fn_t>>::iterator
            > tw_map;

    std::list<std::pair<uint64_t, poller_fn_t>> poller_list;

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
    void add_poller(poller_fn_t poller_fn, uint64_t poller_id);
    void del_poller(uint64_t poller_id);
    int  process_time_works();
    int  iter_poller();

    int set_nonblock(int sd);
public:
    friend class MsgWorkerThread;

    explicit ThrMsgWorker(MsgContext *c, int i);
    ~ThrMsgWorker();

    virtual int set_affinity(int id) override;

    virtual msg_worker_type_t type() const override{
        return msg_worker_type_t::THREAD;
    }

    virtual int get_job_num() override;
    virtual void update_job_num(int v) override;

    virtual int get_event_num() override;
    virtual int add_event(EventCallBack *ecb) override;
    virtual int del_event(int fd) override;

    virtual void wakeup() override;
    virtual bool am_self() const override{
        return worker_thread.am_self();
    }

    virtual std::string get_name() const override{
        return name;
    }

    virtual uint64_t post_time_work(uint64_t microseconds, 
                                    work_fn_t work_fn) override;
    
    /***
     * 不保证一定能及时取消掉time_work
     */
    virtual void cancel_time_work(uint64_t time_work_id) override;

    virtual uint64_t reg_poller(poller_fn_t poller_fn) override;
    virtual void unreg_poller(uint64_t poller_id) override;

    virtual void post_work(work_fn_t work) override;

    virtual void start() override;
    virtual void stop() override;
    virtual bool running() const override{
        return is_running.load();
    }

    virtual void process() override;
    virtual void drain() override;

};

} //namespace msg
} //namespace flame


#endif