#ifndef FLAME_MSG_RDMA_RDMA_STACK_H
#define FLAME_MSG_RDMA_RDMA_STACK_H

#include "msg/Stack.h"
#include "msg/event/event.h"
#include "msg/MsgWorker.h"
#include "common/thread/mutex.h"
#include "common/thread/cond.h"
#include "common/context.h"
#include "RdmaConnection.h"
#include "Infiniband.h"
#include "RdmaMem.h"

#include <map>
#include <vector>
#include <deque>
#include <functional>

namespace flame{

class RdmaWorker;
class RdmaManager;

struct RdmaRwWork;
typedef std::function<void(RdmaRwWork *, RdmaConnection*)> rdma_rw_work_func_t;

struct RdmaRwWork{
    using RdmaBuffer = ib::RdmaBuffer;
    std::vector<RdmaBuffer *> rbufs;
    std::vector<RdmaBuffer *> lbufs; //must be same num as rbufs.
    bool is_write;
    int cnt;
    std::vector<int> failed_indexes;
    rdma_rw_work_func_t target_func = nullptr;

    virtual ~RdmaRwWork() {}

    virtual void callback(RdmaConnection *conn){
        if(target_func){
            target_func(this, conn);
        }
    }
};

class RdmaTxCqNotifier : public EventCallBack{
    RdmaWorker *worker;
public:
    explicit RdmaTxCqNotifier(FlameContext *fct, RdmaWorker *w)
    :EventCallBack(fct, FLAME_EVENT_READABLE), worker(w){}
    virtual void read_cb() override;
};

class RdmaRxCqNotifier : public EventCallBack{
    RdmaWorker *worker;
public:
    explicit RdmaRxCqNotifier(FlameContext *fct, RdmaWorker *w)
    :EventCallBack(fct, FLAME_EVENT_READABLE), worker(w){}
    virtual void read_cb() override;
};

class RdmaAsyncEventHandler : public EventCallBack{
    RdmaManager *manager;
public:
    explicit RdmaAsyncEventHandler(FlameContext *fct, RdmaManager *m)
    :EventCallBack(fct, FLAME_EVENT_READABLE), manager(m){}
    virtual void read_cb() override;
};


class RdmaWorker{
    using Chunk = ib::Chunk; 
    FlameContext *fct;
    MsgWorker *owner = nullptr;
    RdmaManager *manager;
    ib::MemoryManager *memory_manager = nullptr;
    ib::CompletionChannel *tx_cc = nullptr, *rx_cc = nullptr;
    ib::CompletionQueue   *tx_cq = nullptr, *rx_cq = nullptr;
    RdmaTxCqNotifier *tx_notifier = nullptr;
    RdmaRxCqNotifier *rx_notifier = nullptr;
    ibv_srq *srq = nullptr; // if srq enabled, one worker has one srq.
    uint32_t srq_buffer_backlog = 0;

    std::map<uint32_t , RdmaConnection *> qp_conns; // qpn, conn

    //Waitting for dead connection that has no tx_cq to poll.
    //This will ensure that all tx_buffer will be returned to memory manager.
    std::list<RdmaConnection *> dead_conns;

    Mutex fin_clean_mutex;
    Cond  fin_clean_cond;
    bool is_fin_clean = false;

    std::atomic<bool> is_fin;

    void handle_tx_cqe(ibv_wc *cqe, int n);
    void handle_rdma_rw_cqe(ibv_wc &wc, RdmaConnection *conn);
    void handle_rx_cqe(ibv_wc *cqe, int n);
public:
    explicit RdmaWorker(FlameContext *c, RdmaManager *m)
    :fct(c), manager(m), fin_clean_mutex(MUTEX_TYPE_ADAPTIVE_NP), 
     fin_clean_cond(fin_clean_mutex), is_fin(false) {}
    ~RdmaWorker();
    int init();
    int clear_before_stop();
    void clear_qp(uint32_t qpn);
    void fin_clean_wait();
    void fin_clean_signal();
    int on_buffer_reclaimed();
    int arm_notify(MsgWorker *worker);
    int remove_notify();
    int process_tx_cq();
    int process_rx_cq();
    int reg_rdma_conn(uint32_t qpn, RdmaConnection *conn);
    RdmaConnection *get_rdma_conn(uint32_t qpn);
    void make_conn_dead(RdmaConnection *conn);
    int post_chunks_to_rq(std::vector<Chunk *> &chunks, ibv_qp *qp=nullptr);
    int post_chunks_to_rq(int num, ibv_qp *qp=nullptr);
    ibv_srq *get_srq() const { return this->srq; }
    ib::CompletionQueue *get_tx_cq() const { return tx_cq; }
    ib::CompletionQueue *get_rx_cq() const { return rx_cq; }
    int get_qp_size() const { return qp_conns.size(); }
    MsgWorker *get_owner() const { return this->owner; }
    RdmaManager *get_manager() const { return this->manager; }
    ib::MemoryManager *get_memory_manager() const { 
        return this->memory_manager; 
    }
};

class RdmaManager{
    using Chunk = ib::Chunk; 
    FlameContext *fct;
    ib::Infiniband m_ib;
    MsgWorker *owner = nullptr;
    RdmaAsyncEventHandler *async_event_handler = nullptr;

    std::vector<RdmaWorker *> workers;
public:
    explicit RdmaManager(FlameContext *c, uint32_t max_cc_num)
    :fct(c), m_ib(c), max_concurrent_num(max_cc_num) {};
    ~RdmaManager();
    int init();
    int clear_before_stop();
    void post_buffers_to_worker();
    int arm_async_event_handler(MsgWorker *worker);
    int handle_async_event();
    int get_rdma_worker_num() { return workers.size(); }
    RdmaWorker *get_rdma_worker(int index);
    RdmaWorker *get_lightest_load_rdma_worker();
    MsgWorker *get_owner() const { return this->owner; }
    ib::Infiniband &get_ib() { return m_ib; }
    const uint32_t max_concurrent_num;
};


class RdmaStack : public Stack{
    FlameContext *fct;
    RdmaManager *manager;
public:
    explicit RdmaStack(FlameContext *c);
    virtual int init() override;
    virtual int clear_before_stop() override;
    virtual int fin() override;
    virtual ListenPort* create_listen_port(NodeAddr *addr) override;
    virtual Connection* connect(NodeAddr *addr) override;
    RdmaManager *get_manager() { return manager; }
    ib::RdmaBufferAllocator *get_rdma_allocator();
    static RdmaConnection *rdma_conn_cast(Connection *conn){
        auto ttype = conn->get_ttype();
        if(ttype == msg_ttype_t::RDMA){
            return static_cast<RdmaConnection *>(conn);
        }
        return nullptr;
    }
};



}

#endif