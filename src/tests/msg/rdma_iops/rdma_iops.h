#ifndef FLAME_TESTS_MSG_RDMA_IOPS_H
#define FLAME_TESTS_MSG_RDMA_IOPS_H

#include "msg/msg_core.h"
#include "util/clog.h"
#include "util/option_parser.h"
#include "util/fmt.h"

#include "get_clock.h"

#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <regex>
#include <atomic>

namespace flame{
namespace msg{

struct rdma_iops_config_t {
    bool is_mgr;
    bool use_imm_data;
    uint64_t num;
    uint32_t depth;
    uint32_t batch_size;
    std::vector<bool> send_vec;
    std::vector<Msg *> msg_vec;
    cycles_t cycles[2];
    Session *session;
};

#define M_ENCODE(bl, data) (bl).append(&(data), sizeof(data))
#define M_DECODE(it, data) (it).copy(&(data), sizeof(data))
struct msg_slot_d : public MsgData{
    uint32_t num;
    virtual int encode(MsgBufferList &bl) override{
        return M_ENCODE(bl, num);
    }
    virtual int decode(MsgBufferList::iterator &it) override{
        return M_DECODE(it, num);
    }
};
#undef M_ENCODE
#undef M_DECODE

void print_result(rdma_iops_config_t *cfg){
    double cycle_to_unit = get_cpu_mhz(0); // no warn
    double m_iops = (cfg->num * cycle_to_unit 
                        / (cfg->cycles[1] - cfg->cycles[0]));
    double t = (cfg->cycles[1] - cfg->cycles[0]) / cycle_to_unit / 1000000;
    clog(fmt::format("iter {} times in {} s, iops: {} x10^6", 
                        cfg->num, t, m_iops));
}

optparse::OptionParser init_parser(){
    using namespace optparse;
    const std::string usage = "usage: %prog [OPTION]... ";
    auto parser = OptionParser().usage(usage);
    
    parser.add_option("-c", "--config").help("set config file");
    parser.add_option("-i", "--index").type("int").set_default(0)
        .help("set node index");
    parser.add_option("-n", "--num").set_default("1000000")
        .help("iter count for measure iops");
    parser.add_option("-D", "--depth").type("unsigned long")
        .set_default(32)
        .help("set inflight work requests's num");
    parser.add_option("-b", "--batch_size").type("unsigned long")
        .set_default(16)
        .help("set work request's num in one post_send() (max:32)");
    parser.add_option("--log_level").set_default("info");

    parser.add_option("-d", "--dst").help("target ip:port");

    parser.add_option("--imm_data")
          .action("store_true")
          .set_default("false")
          .help("enable imm_data");

    return parser;
}

void parse_target_ip(const std::string &ips_str, MsgContext *mct,
                        rdma_iops_config_t &cfg){
    std::regex ip_regex("([0-9.]+):([0-9]+)");
    auto ip_begin = std::sregex_iterator(ips_str.begin(), ips_str.end(),
                                        ip_regex);
    auto ip_end = std::sregex_iterator();
    if(ip_begin != ip_end){
        auto match = *ip_begin;
        NodeAddr *addr = new NodeAddr(mct);
        addr->set_ttype(NODE_ADDR_TTYPE_RDMA);
        addr->ip_from_string(match[1].str());
        int port = std::stoi(match[2].str());
        assert(port >= 0 && port <= 65535);
        addr->set_port(port);
        ML(mct, info, "target: {}", addr->to_string());
        msger_id_t msger_id =  msger_id_from_msg_node_addr(addr);
        auto session = mct->manager->get_session(msger_id);
        session->set_listen_addr(addr, msg_ttype_t::RDMA);
        auto conn = session->get_conn(msg_ttype_t::RDMA);
        cfg.session = session;
        addr->put();
    }
}

void init_resource(MsgContext *mct, rdma_iops_config_t *cfg){
    if(cfg->is_mgr){
        cfg->send_vec.resize(cfg->depth, false);
    }else{
        cfg->send_vec.resize(cfg->depth, true);
    }
    
    cfg->msg_vec.reserve(cfg->depth);
    msg_slot_d slot_data;
    for(uint32_t i = 0;i < cfg->depth;++i){
        slot_data.num = i;
        auto msg = Msg::alloc_msg(mct, msg_ttype_t::RDMA);
        msg->append_data(slot_data);
        if(cfg->is_mgr){
            msg->set_flags(FLAME_MSG_FLAG_RESP);
        }
        cfg->msg_vec.push_back(msg);
    }
}

void fin_resource(rdma_iops_config_t *cfg){
    for(auto msg : cfg->msg_vec){
        msg->put();
    }
}

class IopsMsger : public MsgerCallback{
    MsgContext *mct;
    rdma_iops_config_t *cfg;
    // ensure MsgWorkers' num <= 2
    // means that the proc will run in the same thread.(for no thread-safe)
    uint64_t cnt;
    uint32_t inflight_cnt;
    void on_req(Connection *conn, Msg *msg);
    void on_resp(Connection *conn, Msg *msg);
    void send_resp_batch();
public:
    explicit IopsMsger(MsgContext *c, rdma_iops_config_t *config)
    : mct(c), cfg(config), cnt(0), inflight_cnt(0) {}
    virtual void on_conn_error(Connection *conn) override;
    virtual void on_conn_declared(Connection *conn, Session *s) override;
    virtual void on_conn_recv(Connection *conn, Msg *msg) override;
    void send_req_batch();
};

void IopsMsger::on_conn_declared(Connection *conn, Session *s){
    clog(fmt::format("Conn {} declared! ",  conn->to_string()));
    if(!cfg->is_mgr){
        return;
    }
    cfg->session = s;
    assert(cfg->session != nullptr);
}

void IopsMsger::on_conn_error(Connection* conn){
    if(!cfg->is_mgr){
        return;
    }
    clog(fmt::format("Conn {} disconnected! reset cnt and infight_cnt."
                    " {} => 0, {} => 0", conn->to_string(), cnt, inflight_cnt));
    cnt = 0;
    inflight_cnt = 0;
    cfg->session = nullptr;
}

void IopsMsger::on_conn_recv(Connection *conn, Msg *msg){
    if(msg->is_req()){
        on_req(conn, msg);
    }else if(msg->is_resp()){
        on_resp(conn, msg);
    }
}

void IopsMsger::on_req(Connection *conn, Msg *msg){
    assert(cfg->is_mgr);
    
    msg_slot_d slot_data;
    auto it = msg->data_iter();
    slot_data.decode(it);

    uint32_t index = slot_data.num;
    cfg->send_vec[index] = true;
    ++cnt;
    ++inflight_cnt;

    auto send_cnt = inflight_cnt;

    if(send_cnt % cfg->batch_size == 0){
        send_resp_batch();
    }

    while(cnt >= cfg->num && inflight_cnt > 0){
        ML(mct, trace, "cnt: {}, inflight_cnt: {}, in end loop" , 
                    cnt, inflight_cnt);
        send_resp_batch();
    }

}

void IopsMsger::on_resp(Connection *conn, Msg *msg){
    assert(!cfg->is_mgr);
    uint32_t index = 0;
    if(cfg->use_imm_data){
        assert(msg->is_imm_data());
        index = msg->imm_data;
    }else{
        msg_slot_d slot_data;
        auto it = msg->data_iter();
        slot_data.decode(it);
        index = slot_data.num;
    }

    cfg->send_vec[index] = true;
    ++cnt;
    --inflight_cnt;

    if(cnt == cfg->num){
        cfg->cycles[1] = get_cycles();
        ML(mct, info, "iter {} times. done!", cnt);
        print_result(cfg);
        return;
    }
    if(cnt + inflight_cnt >= cfg->num){
        return;
    }

    auto send_cnt = cfg->depth - inflight_cnt;
    if(send_cnt == 0){
        return;
    }
    if(send_cnt % cfg->batch_size == 0 
        || send_cnt + inflight_cnt + cnt == cfg->num){
        send_req_batch();
    }

}

void IopsMsger::send_req_batch(){
    uint32_t send_cnt = 0;
    std::list<Msg *> msgs;
    auto conn = cfg->session->get_conn(msg_ttype_t::RDMA);
    auto max_send = std::min((uint64_t)(cfg->depth - inflight_cnt), 
                            cfg->num - cnt - inflight_cnt);
    max_send = std::min(max_send, (uint64_t)cfg->batch_size);
    for(uint32_t i = 0;i < cfg->depth && send_cnt < max_send;++i){
        if(cfg->send_vec[i]){
            msgs.push_back(cfg->msg_vec[i]);
            ++send_cnt;
            cfg->send_vec[i] = false;
        }
    }
    inflight_cnt += send_cnt;
    conn->send_msgs(msgs);
    ML(mct, info, "batch send req: {}", send_cnt);
}

void IopsMsger::send_resp_batch(){
    uint32_t send_cnt = 0;
    int r = 0;
    auto conn = cfg->session->get_conn(msg_ttype_t::RDMA);
    auto max_send = std::min((uint32_t)cfg->batch_size, (uint32_t)inflight_cnt);
    if(cfg->use_imm_data){
        auto rdma_conn = RdmaStack::rdma_conn_cast(conn);
        std::vector<uint32_t> imm_data_vec;
        imm_data_vec.reserve(max_send);
        for(uint32_t i = 0;i < cfg->depth && send_cnt < max_send;++i){
            if(cfg->send_vec[i]){
                imm_data_vec.push_back(i);
                ++send_cnt;
                cfg->send_vec[i] = false;
            }
        }
        r = rdma_conn->post_imm_data(&imm_data_vec);
        ML(mct, info, "batch send resp: {} by imm_data", r);
    }else{
        std::list<Msg *> msgs;
        for(uint32_t i = 0;i < cfg->depth && send_cnt < max_send;++i){
            if(cfg->send_vec[i]){
                msgs.push_back(cfg->msg_vec[i]);
                ++send_cnt;
                cfg->send_vec[i] = false;
            }
        }
        conn->send_msgs(msgs);
        ML(mct, info, "batch send resp: {}", send_cnt);
    }
    inflight_cnt -= send_cnt;
   
}

}// namespace msg
}// namespace flame

#endif //FLAME_TESTS_MSG_RDMA_IOPS_H