#include "RdmaStack.h"
#include "Infiniband.h"
#include "RdmaListenPort.h"
#include "RdmaConnection.h"
#include "RdmaPrepConn.h"
#include "msg/msg_def.h"
#include "msg/MsgManager.h"
#include "msg/internal/errno.h"

#include <cassert>

namespace flame{
namespace msg{

void RdmaTxCqNotifier::read_cb() {
    worker->process_cq_dry_run();
}

void RdmaRxCqNotifier::read_cb() {
    worker->process_cq_dry_run();
}

void RdmaAsyncEventHandler::read_cb() {
    manager->handle_async_event();
}

RdmaWorker::~RdmaWorker(){
    //ack events, otherwise the ibv_destroy_cq will failed.
    if(tx_cc){
        tx_cc->ack_events();
    }
    if(rx_cc){
        rx_cc->ack_events();
    }
    if(tx_cq){
        delete tx_cq;
        tx_cq = nullptr;
    }
    if(rx_cq){
        delete rx_cq;
        rx_cq = nullptr;
    }
    if(tx_cc){
        delete tx_cc;
        tx_cc = nullptr;
    }
    if(rx_cc){
        delete rx_cc;
        rx_cc = nullptr;
    }

    if(srq){
        int r = ibv_destroy_srq(srq);
        assert(r == 0);
        srq = nullptr;
    }
}

int RdmaWorker::init(){
    ib::Infiniband &ib = manager->get_ib();
    memory_manager = ib.get_memory_manager();

    tx_cc = ib.create_comp_channel(mct);
    assert(tx_cc);
    rx_cc = ib.create_comp_channel(mct);
    assert(rx_cc);
    tx_cq = ib.create_comp_queue(mct, tx_cc);
    assert(tx_cq);
    rx_cq = ib.create_comp_queue(mct, rx_cc);
    assert(rx_cq);

    if(mct->config->rdma_enable_srq){
        srq = ib.create_shared_receive_queue(ib.get_rx_queue_len());
        if(!srq){
            return 1;
        }
        ib.post_chunks_to_srq(ib.get_rx_queue_len(), srq);
    }

    return 0;
}

int RdmaWorker::clear_before_stop(){
    for(auto &it : qp_conns){
        it.second->close();
    }

    is_fin = true;

    if(!qp_conns.empty()){
        ML(mct, info, "wait for all rdma conn cleaned... ({:p})", (void *)this);
        fin_clean_wait();
    }
}

void RdmaWorker::clear_qp(uint32_t qpn){
    //remove conn from qp_conns map, and dec its refcount.
    reg_rdma_conn(qpn, nullptr);
    //if no qp_conn and  is_fin, signal RdmaWorker.
    if(is_fin && qp_conns.empty()){
        remove_notify();
        fin_clean_signal();
    }
}

void RdmaWorker::fin_clean_wait(){
    MutexLocker l(fin_clean_mutex);
    while(!is_fin_clean){
        fin_clean_cond.wait();
    }
}

void RdmaWorker::fin_clean_signal(){
    MutexLocker l(fin_clean_mutex);
    is_fin_clean = true;
    fin_clean_cond.signal();
}

int RdmaWorker::process_cq_dry_run(){
    static int MAX_COMPLETIONS = 32;
    ibv_wc wc[MAX_COMPLETIONS];
    int total_proc = 0;
    bool has_cq_event = false;
    int tx_ret = 0;
    int rx_ret = 0;

    has_cq_event = false;
    while(true){
        //drain channel fd
        if(!rx_cc->get_cq_event()){
            break;
        }
        has_cq_event = true;
    }
    if(has_cq_event){
        rx_cq->rearm_notify();
    }
    
    has_cq_event = false;
    while(true){
        //drain channel fd
        if(!tx_cc->get_cq_event()){
            break;
        }
        has_cq_event = true;
    }
    if(has_cq_event){
        tx_cq->rearm_notify();
    }

    do{
        rx_ret = process_rx_cq(wc, MAX_COMPLETIONS);
        tx_ret = process_tx_cq(wc, MAX_COMPLETIONS);
        total_proc += (rx_ret + tx_ret);
    }while(rx_ret + tx_ret > 0);

    reap_dead_conns();

    return total_proc;
}

void RdmaWorker::handle_tx_cqe(ibv_wc *cqe, int n){
    std::vector<Chunk*> tx_chunks;
    std::set<RdmaConnection *> to_wake_conns;

    for (int i = 0; i < n; ++i) {
        ibv_wc* response = &cqe[i];

        auto conn = get_rdma_conn(response->qp_num);
        assert(conn);
        ib::QueuePair *qp = conn->get_qp();
        if(qp){
            qp->dec_tx_wr(1);
            //wakeup conn after dec_tx_wr;
            to_wake_conns.insert(conn);
        }

        if(response->opcode == IBV_WC_RDMA_READ
            || response->opcode == IBV_WC_RDMA_WRITE){
            handle_rdma_rw_cqe(*response, conn);
            continue;
        }
        
        Chunk* chunk = reinterpret_cast<Chunk *>(response->wr_id);
        ML(mct, info, "QP: {} len: {}, addr: {:p} {}", response->qp_num,
                    response->byte_len, (void *)chunk,
                    manager->get_ib().wc_status_to_string(response->status));

        if (response->status != IBV_WC_SUCCESS) {
            if (response->status == IBV_WC_RETRY_EXC_ERR) {
                ML(mct, error, "Connection between server and client not"
                                " working. Disconnect this now");
            } else if (response->status == IBV_WC_WR_FLUSH_ERR) {
                ML(mct, error, "Work Request Flushed Error: this connection's "
                    "qp={} should be down while this WR={:x} still in flight.",
                    response->qp_num, response->wr_id);
            } else {
                ML(mct, error, "send work request returned error for "
                    "buffer({}) status({}): {}",
                    (void *)response->wr_id, response->status,
                    manager->get_ib().wc_status_to_string(response->status));
            }

            ML(mct, info, "qp state is : {}", conn->get_qp_state());
            conn->fault();
        }

        //TX completion may come either from regular send message or from 'fin'
        // message. In the case of 'fin' wr_id points to the QueuePair.
        if(reinterpret_cast<ib::QueuePair*>(response->wr_id) == qp){
            ML(mct, debug, "sending of the disconnect msg completed");
        } else if(chunk != nullptr) {
            // MemoryManager doesn't check whether the chunk is valid.
            // if you worry about that, may use MemoryManager.is_from().
            tx_chunks.push_back(chunk);
        }
    }

    owner->post_work([to_wake_conns](){
        for(RdmaConnection *conn : to_wake_conns){
            conn->send_msg(nullptr);
        }
    });

    memory_manager->release_buffers(tx_chunks);
}

void RdmaWorker::handle_rdma_rw_cqe(ibv_wc &wc, RdmaConnection *conn){

    ML(mct, info, "QP: {} len: {}, wr_id: {:x} {} {}", wc.qp_num,
                    wc.byte_len, wc.wr_id,
                    ib::Infiniband::wc_opcode_string(wc.opcode), 
                    ib::Infiniband::wc_status_to_string(wc.status));
    
    RdmaRwWork *work = reinterpret_cast<RdmaRwWork *>(wc.wr_id);

    if (wc.status != IBV_WC_SUCCESS) {
        if (wc.status == IBV_WC_RETRY_EXC_ERR) {
            ML(mct, error, "Connection between server and client not"
                            " working. Disconnect this now");
        } else if (wc.status == IBV_WC_WR_FLUSH_ERR) {
            ML(mct, error, "Work Request Flushed Error: this connection's "
                "qp={} should be down while this WR={:x} still in flight.",
                wc.qp_num, wc.wr_id);
        } else if (wc.status == IBV_WC_REM_ACCESS_ERR) {
            ML(mct, error, "Remote Access Error: qp: {}, wr_id: {:x}", 
                wc.qp_num, wc.wr_id);
        } else {
            ML(mct, error, "send work request returned error for "
                "buffer({}) status({}): {}",
                wc.wr_id, wc.status,
                ib::Infiniband::wc_status_to_string(wc.status));
        }

        ML(mct, info, "qp state is : {}", conn->get_qp_state());
        conn->fault();
        work->failed_indexes.push_back(work->lbufs.size() - work->cnt);
    }

    --work->cnt;

    if(work->cnt == 0){
        work->callback(conn);
    }

}

int RdmaWorker::process_tx_cq_dry_run(){
    static int MAX_COMPLETIONS = 32;
    ibv_wc wc[MAX_COMPLETIONS];
    int total_proc = 0;
    int tx_ret = 0;

    while(true){
        //drain channel fd
        if(!tx_cc->get_cq_event()){
            break;
        }
    }
    tx_cq->rearm_notify();

    do{
        tx_ret = process_tx_cq(wc, MAX_COMPLETIONS);
        total_proc += tx_ret;
    }while(tx_ret > 0);

    reap_dead_conns();

    return total_proc;
}

int RdmaWorker::process_tx_cq(ibv_wc *wc, int max_cqes){
    int tx_ret = tx_cq->poll_cq(max_cqes, wc);
    if(tx_ret > 0){
        ML(mct, info, "tx completion queue got {} responses.", tx_ret);
        handle_tx_cqe(wc, tx_ret);
        return tx_ret;
    }
    //here may be error or poll no cqe.
    return 0;
}

void RdmaWorker::reap_dead_conns(){
    auto dead_it = dead_conns.begin();
    while(dead_it != dead_conns.end()){
        auto dead_conn = *dead_it;
        if(dead_conn->get_tx_wr() && !dead_conn->is_error()){
            ML(mct, debug, "bypass conn={:p} tx_wr={}", (void *)dead_conn,
                                                        dead_conn->get_tx_wr());
            ++dead_it;
        }else{
            ML(mct, debug, "finally release conn={:p}", (void *)dead_conn);
            dead_conn->get_qp()->to_dead();
            dead_it = dead_conns.erase(dead_it);
        }
    }
}

void RdmaWorker::handle_rx_cqe(ibv_wc *cqe, int n){
    std::map<RdmaConnection *, std::list<ibv_wc>> polled;
    for (int i = 0; i < n; ++i) {
        ibv_wc* response = &cqe[i];
        Chunk* chunk = reinterpret_cast<Chunk *>(response->wr_id);
        ML(mct, info, "QP: {} len: {}, opcode: {}, wc_flags:{}, addr: {:p} {}", 
                    response->qp_num, response->byte_len, 
                    ib::Infiniband::wc_opcode_string(response->opcode), 
                    response->wc_flags, (void *)chunk,
                    ib::Infiniband::wc_status_to_string(response->status));

        assert(response->opcode & IBV_WC_RECV);

        auto conn = get_rdma_conn(response->qp_num);

        if (response->status == IBV_WC_SUCCESS) {
            if (!conn) {
                ML(mct, info, "RdmaConnection with qpn {} may be dead. "
                                "Return rx buffer {:p} to MemoryManager.",
                                    response->qp_num, (void *)chunk);
                memory_manager->release_buffer(chunk);
            } else {
                polled[conn].push_back(*response);
            }
        } else {
            ML(mct, error, "work request returned error for buffer({:p}) "
                            "status({}:{})", (void *)chunk, response->status,
                    manager->get_ib().wc_status_to_string(response->status));
            if (conn)
                conn->fault();

            memory_manager->release_buffer(chunk);
        }
    }
    for (auto &i : polled){
        i.first->pass_wc(i.second);
        i.first->recv_data();
        // i.first->read_cb();
        // i.first->recv_notify();
    }

}

int RdmaWorker::process_rx_cq_dry_run(){
    static int MAX_COMPLETIONS = 32;
    ibv_wc wc[MAX_COMPLETIONS];
    int total_proc = 0;
    int rx_ret = 0;

    while(true){
        //drain channel fd
        if(!rx_cc->get_cq_event()){
            break;
        }
    }

    rx_cq->rearm_notify();

    do{
        rx_ret = process_rx_cq(wc, MAX_COMPLETIONS);
        total_proc += rx_ret;
    }while(rx_ret > 0);

    return total_proc;
}

int RdmaWorker::process_rx_cq(ibv_wc *wc, int max_cqes){
    int rx_ret = rx_cq->poll_cq(max_cqes, wc);
    if(rx_ret > 0){
        ML(mct, info, "rx completion queue got {} responses.", rx_ret);
        if(srq){
            srq_buffer_backlog = srq_buffer_backlog + rx_ret - 
                        post_chunks_to_rq(srq_buffer_backlog + rx_ret);
        }
        //if don't have srq, reuse rx buffers after RdmaConn recv_data().
        handle_rx_cqe(wc, rx_ret);
        return rx_ret;
    }

    //here may be error or poll no cqe.
    return 0;
}

int RdmaWorker::reg_rdma_conn(uint32_t qpn, RdmaConnection *conn){
    if(conn){
        conn->get();
        conn->set_owner(owner);
    }
    auto it = qp_conns.find(qpn);
    if(it != qp_conns.end()){
        it->second->put();
        if(!conn){
            qp_conns.erase(it);
            return 0;
        }else{
            it->second = conn;
        }
    }else{
        if(conn){
            qp_conns[qpn] = conn;
        }else{
            return 0;
        }
    }
    return 1;

}

RdmaConnection *RdmaWorker::get_rdma_conn(uint32_t qpn){
    auto it = qp_conns.find(qpn);
    if(it != qp_conns.end()){
        return it->second;
    }
    return nullptr;
}

void RdmaWorker::make_conn_dead(RdmaConnection *conn){
    if(!conn) return;
    if(!get_owner()->am_self()){
        get_owner()->post_work([this, conn](){
            this->make_conn_dead(conn);
        });
        return;
    }
    if(conn->get_tx_wr() == 0 || conn->is_error()){
        conn->get_qp()->to_dead();
    }else{
        dead_conns.push_back(conn);
    }
    
}

int RdmaWorker::post_chunks_to_rq(std::vector<Chunk *> &chunks, ibv_qp *qp){
    if(srq){
        return manager->get_ib().post_chunks_to_srq(chunks, srq);
    }else{
        assert(qp);
        return manager->get_ib().post_chunks_to_rq(chunks, qp);
    }
}

int RdmaWorker::post_chunks_to_rq(int num, ibv_qp *qp){
    if(srq){
        return manager->get_ib().post_chunks_to_srq(num, srq);
    }else{
        assert(qp);
        return manager->get_ib().post_chunks_to_rq(num, qp);
    }
}


int RdmaWorker::on_buffer_reclaimed(){
    return 0;
}

int RdmaWorker::arm_notify(MsgWorker *worker){
    if(!worker) return 1;
    tx_notifier = new RdmaTxCqNotifier(mct, this);
    tx_notifier->fd = tx_cc->get_fd();
    rx_notifier = new RdmaRxCqNotifier(mct, this);
    rx_notifier->fd = rx_cc->get_fd();

    int r = worker->add_event(tx_notifier);
    assert(r == 0);
    r = worker->add_event(rx_notifier);
    assert(r == 0);

    owner = worker;
    return 0;
}

int RdmaWorker::remove_notify(){
    if(tx_notifier){
        owner->del_event(tx_notifier->fd);
        tx_notifier->put();;
        tx_notifier = nullptr;
    }

    if(rx_notifier){
        owner->del_event(rx_notifier->fd);
        rx_notifier->put();
        rx_notifier = nullptr;
    }

    return 0;
}


int RdmaManager::handle_async_event(){
    auto d = get_ib().get_device();
    assert(d);
    int count = 0;
    ibv_async_event async_event;
    while(1){
        if(ibv_get_async_event(d->ctxt, &async_event)){
            if(errno != EAGAIN){
                ML(mct, error, "ibv_get_async_event failed. (errno= {})",
                                     cpp_strerror(errno));
            }
            return -1;
        }
        if(async_event.event_type == IBV_EVENT_QP_LAST_WQE_REACHED){
            ib::QueuePair *qp = ib::QueuePair::get_by_ibv_qp(
                                                        async_event.element.qp);
            uint32_t qpn = qp->get_local_qp_number();
            ML(mct, info, "event associated qp={} evt: {}, destroy it.", qpn,
                                    ibv_event_type_str(async_event.event_type));
            
            for(auto &worker : workers){
                //Only one worker has the conn.
                //The others will do nothing.
                worker->get_owner()->post_work([qpn, worker](){
                    worker->clear_qp(qpn);
                });
            }

        }else{
            ML(mct, info, "ibv_get_async_event: dev={} evt: {}", 
                (void *)(d->ctxt), ibv_event_type_str(async_event.event_type));
        }
        ++count;
        ibv_ack_async_event(&async_event);
    }
    return count;
}

RdmaManager::~RdmaManager(){
    if(async_event_handler){
        owner->del_event(async_event_handler->fd);
        async_event_handler->put();
        async_event_handler = nullptr;
    }
    for(auto it : workers){
        delete it;
    }
    workers.clear();
}

int RdmaManager::init(){
    int res = (m_ib.init()?0:1);
    if(res) return res;
    auto worker = mct->manager->get_lightest_load_worker();
    assert(worker);
    res = arm_async_event_handler(worker);
    if(res) return res;

    int cqp_num = mct->config->rdma_cq_pair_num;
    int msg_worker_num = mct->manager->get_worker_num();
    if(cqp_num >= msg_worker_num){
        cqp_num = msg_worker_num - 1;
        ML(mct, warn, "config->rdma_cq_pair_num too large, "
                        "set equal to msg_worker_num - 1: {}", 
                        cqp_num);
    }
    workers.reserve(cqp_num);

    
    int arm_step = (msg_worker_num - 1) / cqp_num;
    ML(mct, info, "rdma_worker_num(rdma_cq_pair_num): {}, msg_worker_num: {},"
                        " arm_step:{}", cqp_num, msg_worker_num, arm_step);

    for(int i = 0;i < cqp_num; ++i){
        auto worker = new RdmaWorker(mct, this);
        worker->init();
        int msg_worker_index = arm_step * i;
        res = worker->arm_notify(mct->manager->get_worker(msg_worker_index));
        assert(res == 0);
        workers.push_back(worker);
    }
    return res;
}

int RdmaManager::clear_before_stop(){
    for(auto it : workers){
        it->clear_before_stop();
    }
}

RdmaWorker *RdmaManager::get_rdma_worker(int index){
    if(index < 0 || index >= workers.size()){
        return nullptr;
    }
    return workers[index];
}

RdmaWorker *RdmaManager::get_lightest_load_rdma_worker(){
    if(workers.size() <= 0) return nullptr;
    int load = workers[0]->get_qp_size(), index = 0;
    for(int i = 1;i < workers.size(); ++i){
        if(load > workers[i]->get_qp_size()){
            load = workers[i]->get_qp_size();
            index = i;
        }
    }
    return workers[index];
}

int RdmaManager::arm_async_event_handler(MsgWorker *worker){
    if(!worker) return 1;
    auto d = m_ib.get_device();
    assert(d);
    int res = d->set_async_fd_nonblock();
    if(res) return res;
    int fd = d->get_async_fd();

    async_event_handler = new RdmaAsyncEventHandler(mct, this);
    async_event_handler->fd = fd;

    res = worker->add_event(async_event_handler);
    if(!res){
        this->owner = worker;
    }
    return res;
}

RdmaStack::RdmaStack(MsgContext *c)
:mct(c), manager(nullptr) {

}

int RdmaStack::init(){
    manager = new RdmaManager(mct, mct->manager->get_worker_num());
    if(!manager) return -1;
    return manager->init();
}

int RdmaStack::clear_before_stop(){
    if(!manager) return 0;
    return manager->clear_before_stop();
}

int RdmaStack::fin(){
    delete manager;
    return 0;
}

ListenPort* RdmaStack::create_listen_port(NodeAddr *addr){
    return RdmaListenPort::create(mct, addr);
}

Connection* RdmaStack::connect(NodeAddr *addr){
    auto prep_conn = RdmaPrepConn::create(mct, addr);

    auto worker = mct->manager->get_lightest_load_worker();
    assert(worker);
    worker->add_event(prep_conn);
    prep_conn->set_owner(worker);

    RdmaConnection *conn = prep_conn->get_rdma_conn();
    prep_conn->put();
    return conn;
}

ib::RdmaBufferAllocator *RdmaStack::get_rdma_allocator() {
    return manager->get_ib().get_memory_manager()->get_rdma_allocator();
}

} //namespace msg
} //namespace flame