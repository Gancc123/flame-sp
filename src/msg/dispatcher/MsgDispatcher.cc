#include "MsgDispatcher.h"

#include "msg/msg_data.h"
#include "msg/Msg.h"
#include "msg/MsgManager.h"
#include "msg/rdma/RdmaStack.h"

namespace flame{
namespace msg{

Msg *MsgDispatcher::msg_from_message(MessagePtr& msg){
    Msg *m = Msg::alloc_msg(mct, msg_ttype_t::RDMA);
    assert(m);
    switch(msg->req_typ()){
    case MessageType::ADMIN :
        m->type = FLAME_MSG_TYPE_CTL;
        break;
    case MessageType::IO :
        m->type = FLAME_MSG_TYPE_IO;
        break;
    default:
        ML(mct, warn, "Unrecongnized req_type: {}", msg->req_typ());
    }

    if(msg->is_res()){
        m->set_flags(FLAME_MSG_FLAG_RESP);
    }

    auto &message_header = msg->get_header();
    m->append_data(&message_header, sizeof(message_header));

    return m;
}

MessagePtr MsgDispatcher::message_from_msg(Msg *m){
    MessagePtr msg = std::make_shared<Message>();

    auto it = m->data_iter();
    auto &message_header = msg->get_header();
    it.copy(&message_header, sizeof(message_header));

    return msg;
}

int MsgDispatcher::resolve_addr(uint64_t csd_id, csd_addr_t &res){
    if(mct->csd_addr_resolver == nullptr){
        ML(mct, error, "can't resolve dst addr, csd_addr_resolver is null.");
        return -1;
    }
    std::list<csd_addr_t> csd_addr_list;
    std::list<uint64_t> csd_id_list;
    csd_id_list.push_back(csd_id);
    mct->csd_addr_resolver->pull_csd_addr(csd_addr_list, csd_id_list);
    if(!csd_addr_list.empty()){
        res = csd_addr_list.front();
        return 0;
    }
    return -1;
}

Session *MsgDispatcher::get_session(uint64_t dst_id){
    csd_addr_t dst_addr_attr;
    if(resolve_addr(dst_id, dst_addr_attr)){
        return nullptr;
    }
    msger_id_t msger_id = MsgDispatcher::msger_id_from_node_addr(
                                                                 dst_addr_attr
                                                                .admin_addr);
    auto session = mct->manager->get_session(msger_id);
    if(!session->ready()){
        session->set_listen_addr(dst_addr_attr.admin_addr, msg_ttype_t::TCP);
        session->set_listen_addr(dst_addr_attr.io_addr, msg_ttype_t::RDMA);
    }
    return session;
}

int MsgDispatcher::deliver_to_remote(MessagePtr msg, Connection *conn){

    if(conn == nullptr){
        auto session = get_session(msg->dst());

        conn = session->get_conn(msg_ttype_t::RDMA);
        if(conn == nullptr){
            ML(mct, error, "get conn failed. {}", session->to_string());
            return -1;
        }
    }

    Msg *m = msg_from_message(msg);

    conn->send_msg(m);

    m->put();
    return 0;
}


int MsgDispatcher::deliver_msg(MessagePtr msg, Connection *conn){
    auto channel = get_channel(msg->dst());
    if(channel){
        channel->on_local_recv(msg);
    }else{
        deliver_to_remote(msg, conn);
    }
    return 0;
}

int MsgDispatcher::post_rdma_work(uint64_t dst_id, RdmaRwWork *work){
    auto channel = get_channel(dst_id);
    if(channel){
        ML(mct, error, "can't post rdma_work to local channel! {:x}", dst_id);
        return -1;
    }

    auto session = get_session(dst_id);
    if(session == nullptr) {
        ML(mct, error, "get session failed! {:x}", dst_id);
        return -1;
    }

    auto conn = session->get_conn(msg_ttype_t::RDMA);
    if(conn == nullptr){
        ML(mct, error, "get conn failed. {}", session->to_string());
        return -1;
    }

    auto rdma_conn = RdmaStack::rdma_conn_cast(conn);

    return rdma_conn->post_rdma_rw(work);
}

int MsgDispatcher::reg_channel(uint64_t comp_id, MsgChannel *ch){
    WriteLocker l(channel_map_rwlock);
    channel_map[comp_id] = ch;
    return 0;
}

int MsgDispatcher::unreg_channel(uint64_t comp_id){
    WriteLocker l(channel_map_rwlock);
    channel_map.erase(comp_id);
    return 0;
}

MsgChannel *MsgDispatcher::get_channel(uint64_t comp_id){
    ReadLocker l(channel_map_rwlock);
    auto it = channel_map.find(comp_id);
    if(it != channel_map.end()){
        return it->second;
    }
    return nullptr;
}


//derived from MsgerCallback
void MsgDispatcher::on_listen_accept(ListenPort *lp, Connection *conn){
    ReadLocker l(channel_map_rwlock);
    for(auto const &channel_pair : channel_map){
        if(channel_pair.second->on_listen_accept(lp, conn) > 0){
            break;
        }
    }
}

void MsgDispatcher::on_listen_error(NodeAddr *listen_addr){
    ReadLocker l(channel_map_rwlock);
    for(auto const &channel_pair : channel_map){
        if(channel_pair.second->on_listen_error(listen_addr) > 0){
            break;
        }
    }
}

void MsgDispatcher::on_conn_error(Connection *conn){
    ReadLocker l(channel_map_rwlock);
    for(auto const &channel_pair : channel_map){
        if(channel_pair.second->on_conn_error(conn) > 0){
            break;
        }
    }
}

void MsgDispatcher::on_conn_declared(Connection *conn, Session *s){
    ReadLocker l(channel_map_rwlock);
    for(auto const &channel_pair : channel_map){
        if(channel_pair.second->on_conn_declared(conn, s) > 0){
            break;
        }
    }
}

void MsgDispatcher::on_conn_recv(Connection *conn, Msg *m){
    if(m->is_imm_data()){
        ReadLocker l(channel_map_rwlock);
        for(auto const &channel_pair : channel_map){
            if(channel_pair.second->on_conn_recv(conn, m->imm_data) > 0){
                return;
            }
        }
        ML(mct, warn, "no channel process imm_data, drop it. {} {:x}", 
                                            conn->to_string(), m->imm_data);
        return;
    }

    MessagePtr msg = message_from_msg(m);

    auto channel = get_channel(msg->dst());
    if(!channel){
        ML(mct, warn, "unknown dst, ignore the message. {}",
                                                         msg->to_string());
        return;
    }

    channel->on_conn_recv(conn, msg); 
    
}

} //namespace msg
} //namespace flame