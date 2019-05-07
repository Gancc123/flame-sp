#include "SpdkMsgWorker.h"

#include <string>
#include <cassert>
#include <stdlib.h>
#include <unistd.h>

namespace flame{
namespace msg{

static const int EVENT_POLLER_MAX_BATCH = 16;

SpdkMsgWorker::SpdkMsgWorker(MsgContext *c, int i)
: MsgWorker(c, i), event_poller(c, EVENT_POLLER_MAX_BATCH), is_running(false),
  extra_job_num(0), next_poller_id(1), event_poller_id(0){
    int r;
    name = "SpdkMsgWorker" + std::to_string(i);
    //pick a lcore.
    set_affinity(-1);
    ML(c, info, "{} default bind to core {}", name, lcore);

    fevents.reserve(EVENT_POLLER_MAX_BATCH);
    event_poller_timeout_zero.tv_sec = 0;
    event_poller_timeout_zero.tv_usec = 0;
}

SpdkMsgWorker::~SpdkMsgWorker(){
    while(!poller_list.empty()){
        std::tuple<uint64_t, struct spdk_poller *, poller_fn_p, void *> &tuple 
                                                        = poller_list.front();
        destroy_poller(&std::get<1>(tuple), std::get<2>(tuple), 
                                                            std::get<3>(tuple));
        poller_list.pop_front();
    }
}

inline int SpdkMsgWorker::set_affinity(int id){
    uint32_t core_id, core_it;
    //Choose a proper lcore by msg_worker id.
    if(id < 0) {
        uint32_t worker_id = (uint32_t)get_id();
        core_id = worker_id % spdk_env_get_core_count();
        SPDK_ENV_FOREACH_CORE(core_it){
            if(core_id == 0){
                lcore = core_it;
                break;
            }
            core_id--;
        }
        return 0;
    }
    core_id = (uint32_t)id;
    SPDK_ENV_FOREACH_CORE(core_it){
        if(core_it == core_id){
            this->lcore = core_id;
            return 0;
        }
    }
    return -1;
}

int SpdkMsgWorker::get_job_num(){
    return event_poller.get_event_num() + extra_job_num;
}

void SpdkMsgWorker::update_job_num(int v){
    int extra_old = extra_job_num.load(std::memory_order_relaxed);
    int extra_new;
    do{
        extra_new = extra_old + v;
        if(extra_new < 0){
            extra_new = 0;
        }
    }while(!extra_job_num.compare_exchange_weak(extra_old, extra_new,
                                                std::memory_order_release,
                                                std::memory_order_relaxed));
}

int SpdkMsgWorker::get_event_num(){
    return event_poller.get_event_num();
}

int SpdkMsgWorker::add_event(EventCallBack *ecb){
    return event_poller.set_event(ecb->fd, ecb);
}

int SpdkMsgWorker::del_event(int fd){
    return event_poller.del_event(fd);
}

void SpdkMsgWorker::add_poller(poller_fn_p fn_p, void *arg, uint64_t poller_id){
    assert(am_self());
    struct spdk_poller *poller = spdk_poller_register(fn_p, arg, 0);
    assert(poller);
    auto tuple = std::make_tuple(poller_id, poller, fn_p, arg);
    poller_list.push_front(std::move(tuple));
}

void SpdkMsgWorker::del_poller(uint64_t poller_id){
    assert(am_self());
    poller_list.remove_if(
        [poller_id, this](std::tuple<uint64_t, struct spdk_poller *, 
                                        poller_fn_p, void *> tuple)->bool{
            if(std::get<0>(tuple) == poller_id){
                this->destroy_poller(&std::get<1>(tuple),
                                        std::get<2>(tuple),
                                        std::get<3>(tuple));
                return true;
            }
            return false;
        }
    );
}

inline void SpdkMsgWorker::destroy_poller(struct spdk_poller ** ppoller,
                                            poller_fn_p fn_p, void *arg){
    auto poller = *ppoller;
    spdk_poller_unregister(ppoller);
    //Arg is poller_fn_container_t, need to be released.
    if(fn_p == SpdkMsgWorker::poller_call_fn){
        poller_fn_container_t *container = (poller_fn_container_t *)arg;
        delete container;
    }
}

uint64_t SpdkMsgWorker::reg_poller(poller_fn_t poller_fn){
    uint64_t id = next_poller_id++;
    poller_fn_container_t *fn = new poller_fn_container_t();
    assert(fn);
    fn->fn = poller_fn;

    if(am_self()){
        add_poller(SpdkMsgWorker::poller_call_fn, fn, id);
    }else{
        post_work([fn, id, this](){
            ML(this->mct, trace, "in reg_poller(poller_fn_t)");
            this->add_poller(SpdkMsgWorker::poller_call_fn, fn, id);
        });
    }
    return id;
}

uint64_t SpdkMsgWorker::reg_poller(poller_fn_p poller_fn, void *arg){
    uint64_t id = next_poller_id++;
    if(am_self()){
        add_poller(poller_fn, arg, id);
    }else{
        post_work([poller_fn, arg, id, this](){
            ML(this->mct, trace, "in reg_poller(poller_fn_p, void *)");
            this->add_poller(poller_fn, arg, id);
        });
    }
    return id;
}

void SpdkMsgWorker::unreg_poller(uint64_t poller_id){
    if(am_self()){
        del_poller(poller_id);
    }else{
        post_work([poller_id, this]{
            ML(this->mct, trace, "in unreg_poller()");
            this->del_poller(poller_id);
        });
    }
}

inline void SpdkMsgWorker::post_work(work_fn_t work){
    work_fn_container_t *fn_container = new work_fn_container_t();
    assert(fn_container);
    fn_container->fn = work;
    post_work(SpdkMsgWorker::event_call_fn, fn_container, nullptr);
}

inline void SpdkMsgWorker::post_work(work_fn_p work_fn, void *arg1, void *arg2){
    struct spdk_event *event = spdk_event_allocate(lcore, work_fn, arg1, arg2);
    assert(event);
    spdk_event_call(event);
}

void SpdkMsgWorker::start(){
    is_running = true;
    event_poller_id = reg_poller(SpdkMsgWorker::event_poller_function, this);
    ML(mct, debug, "{} start", this->name);
}

void SpdkMsgWorker::stop(){
    is_running = false;
    unreg_poller(event_poller_id);
    ML(mct, debug, "{} stop", this->name);
}

void SpdkMsgWorker::event_call_fn(void *arg1, void *arg2){
    work_fn_container_t *fn_container = (work_fn_container_t *)arg1;
    fn_container->fn();
    delete fn_container;
}

int SpdkMsgWorker::poller_call_fn(void *ctx){
    poller_fn_container_t *fn_container = (poller_fn_container_t *)ctx;
    return fn_container->fn();
}

int SpdkMsgWorker::event_poller_function(void *arg){
    SpdkMsgWorker *mw = (SpdkMsgWorker *)arg;
    if(!mw->is_running){
        return 0;
    }
   
    int numevents;
    numevents = mw->event_poller.process_events(mw->fevents, 
                                                &mw->event_poller_timeout_zero);
    
    for(int i = 0;i < numevents; ++i){
        //* 确保ecb不会被释放，直到该ecb处理完成
        ML(mw->mct, trace, "{} process fd:{}, mask:{}", mw->name,
                                mw->fevents[i].ecb->fd, mw->fevents[i].mask);
        mw->fevents[i].ecb->get();
        if(FLAME_EVENT_ERROR & mw->fevents[i].mask){
            mw->fevents[i].ecb->error_cb();
            mw->fevents[i].ecb->put();
            continue;
        }
        if(FLAME_EVENT_READABLE 
            & mw->fevents[i].mask 
            & mw->fevents[i].ecb->mask){
            mw->fevents[i].ecb->read_cb();
        }
        if(FLAME_EVENT_WRITABLE 
            & mw->fevents[i].mask 
            & mw->fevents[i].ecb->mask){
            mw->fevents[i].ecb->write_cb();
        }
        mw->fevents[i].ecb->put();
    }

    return numevents;
}

} // namespace msg
} // namespace flame