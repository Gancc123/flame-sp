#include "Dispatcher.h"

#include "msg/msg_data.h"
#include "msg/Msg.h"

namespace flame{

void MemPushWork::callback(RdmaConnection *conn){
    auto channel = dispatcher->get_channel(message->src());
    channel->on_rdma_done(conn, message, this);
    if(this->failed_indexes.emtpy()){ // only when all ok.
        channel->on_conn_recv(message);
    }
    delete this;
}

void MemFetchWork::callback(RdmaConnection *conn){
    auto channel = dispatcher->get_channel(message->src());
    channel->on_rdma_done(conn, message, this);
    if(this->failed_indexes.emtpy()){ // only when all ok
        conn->send_msg(msg);
    }
    delete message;
    delete this;
}

int Dispatcher::deliver_to_remote(Message *msg){

    auto msg_manager = fct->msg()->manager;
    //auto session = msg_manager->get_session(xxxx);
    //auto conn = session->get_conn(msg_ttype_t::RDMA);


    Msg *m = Msg::alloc_msg(fct, msg_ttype_t::RDMA);
    assert(m);
    m->type = FLAME_MSG_TYPE_IO;
    if(msg->is_res()){
        m->set_flags(FLAME_MSG_FLAG_RESP);
    }

    if(msg->has_rdma()){
        m->set_flags(FLAME_MSG_FLAG_RDMA);
        if(!msg->is_rdma_fetch()){ // rdma mem push
            m->set_rdma_type(FLAME_MSG_RDMA_MEM_PUSH);
        }else{ // rdma mem fetch
            m->set_rdma_type(FLAME_MSG_RDMA_MEM_FETCH);
        }

        if(msg->is_req()){
            m->set_rdma_cnt(msg->lbufs.size());
            msg_rdma_header_d rdma_header();
            rdma_header.lbufs = msg->rdma_lbufs;
            m->append_data(rdma_header);
        }

    }

    auto &message_header = msg->get_header();
    m->append_data(&message_header, sizeof(message_header));
    

    if(msg->has_rdma() && msg->is_res() && msg->is_rdma_fetch()){
        MemFetchWork *fetch_work = new MemFetchWork(this, m);
        fetch_work->message = msg;
        fetch_work->rbufs = msg->rdma_rbufs;
        fetch_work->lbufs = msg->rdma_lbufs;
        fetch_work->cnt = msg->rdma_rbufs.size();
        conn->post_rdma_rw(fetch_work);
        return 0;
    }

    conn->send_msg(m);

    delete msg; // delete message 
    return 0;
}

int Dispatcher::deliver_msg(Message *msg){
    auto channel = get_channel(msg->dst());
    if(channel){
        channel->on_local_recv(msg);
    }else{
        deliver_to_remote(msg);
    }
    return 0;
}

int Dispatcher::reg_channel(uint64_t comp_id, Channel *ch){
    MutexLocker l(channel_map_mutex);
    channel_map[comp_id] = ch;
    return 0;
}

int Dispatcher::unreg_channel(uint64_t comp_id){
    MutexLocker l(channel_map_mutex);
    channel_map.erase(comp_id);
    return 0;
}

Channel *get_channel(uin64_t comp_id){
    MutexLocker l(channel_map_mutex);
    auto it = channel_map.find(comp_id);
    if(it != channel_map.end()){
        return it.second;
    }
    return nullptr;
}


//derived from MsgerCallback
void Dispatcher::on_listen_accept(ListenPort *lp, Connection *conn){
    MutexLocker l(channel_map_mutex);
    for(auto const &channel_pair : channel_map){
        if(channel_pair->second->on_listen_accept(lp, conn) > 0){
            break;
        }
    }
}

void Dispatcher::on_listen_error(NodeAddr *listen_addr){
    MutexLocker l(channel_map_mutex);
    for(auto const &channel_pair : channel_map){
        if(channel_pair->second->on_listen_error(listen_addr) > 0){
            break;
        }
    }
}

void Dispatcher::on_conn_error(Connection *conn){
    MutexLocker l(channel_map_mutex);
    for(auto const &channel_pair : channel_map){
        if(channel_pair->second->on_conn_error(conn) > 0){
            break;
        }
    }
}

void Dispatcher::on_conn_declared(Connection *conn, Session *s){
    MutexLocker l(channel_map_mutex);
    for(auto const &channel_pair : channel_map){
        if(channel_pair->second->on_conn_declared(conn, s) > 0){
            break;
        }
    }
}

void Dispatcher::on_conn_recv(Connection *conn, Msg *m){
    Message *message = new Message();
    assert(message);

    auto it = m->data_iter();
    if(m->has_rdma()){
        msg_rdma_header_d rdma_header(m->get_rdma_cnt());
        rdma_header.decode(it);
    }

    it.copy(&(message.get_header()), sizeof(message.get_header());

    auto channel = get_channel(message->dst());
    if(!channel){
        ML(fct, warn, "unknown dst, ignore the message. {}",
                                                         message->to_string());
        delete message;
        return;
    }

    // mem push req
    if(m->has_rdma() && message->is_req() && !message->is_rdma_fetch()){ 
        MemPushWork *push_work = new MemPushWork(this, m);
        assert(push_work);
        push_work->rbufs.swap(rdma_header.rdma_bufs);
        push_work->cnt = rdma_header.cnt;
        push_work->message = message;
        auto &ib = Stack::get_rdma_stack()->get_manager()->get_ib();
        auto allocator = ib.get_memory_manager()->get_rdma_allocator();
        for(auto rbuf : push_work->rbufs){
            auto lbuf = allocator->alloc(rbuf->data_len);
            assert(lbuf);
            push_work->lbufs.push_back(lbuf);
        }
        conn->post_rdma_rw(push_work);
        //call channel->on_conn_recv() after MemPushWork done.
        return;
    }

    channel->on_conn_recv(message); 
    
}

}