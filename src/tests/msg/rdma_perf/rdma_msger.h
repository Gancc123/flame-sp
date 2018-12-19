#ifndef FLAME_TESTS_MSG_RDMA_PERF_H
#define FLAME_TESTS_MSG_RDMA_PERF_H

#include "msg/msg_core.h"
#include "util/clog.h"
#include "util/option_parser.h"

#include "get_clock.h"

#include <cstring>
#include <vector>
#include <algorithm>
#include <fstream>

#define M_ENCODE(bl, data) (bl).append(&(data), sizeof(data))
#define M_DECODE(it, data) (it).copy(&(data), sizeof(data))

namespace flame{

enum class perf_type_t {
    NONE,
    MEM_PUSH,  // send req + rdma read + send resp
    SEND,      // send req + send resp
    SEND_DATA, // send req with data + send resp
};

perf_type_t perf_type_from_str(const std::string &s){
    auto lower = str2lower(s);
    if(lower == "mem_push"){
        return perf_type_t::MEM_PUSH;
    }
    if(lower == "send"){
        return perf_type_t::SEND;
    }
    if(lower == "send_data"){
        return perf_type_t::SEND_DATA;
    }
    return perf_type_t::NONE;
}

struct perf_config_t{
    std::string target_rdma_ip;
    int target_rdma_port;
    perf_type_t perf_type;
    int64_t size;
    int num;
    ib::RdmaBuffer *tx_buffer = nullptr;
    MsgBuffer *data_buffer = nullptr;
    cycles_t *tposted = nullptr;
    std::string result_file;
};

void dump_result(perf_config_t &cfg){
    double cycle_to_unit = get_cpu_mhz(0); // no warn
    std::vector<cycles_t> deltas;
    deltas.reserve(cfg.num);
    for(int i = 0;i < cfg.num;++i){
        deltas[i] = cfg.tposted[i+1] - cfg.tposted[i];
    }

    std::sort(deltas.begin(), deltas.end());
    std::ofstream f;
    f.open(cfg.result_file);
    f << "Cnt: " << cfg.num << '\n';
    double average = 0, average_sum = 0;
    for(int i = 0;i < cfg.num;++i){
        double t = deltas[i] / cycle_to_unit;
        f << t << '\n';
        average_sum += t;
    }
    f << "Avg: " << (average_sum / cfg.num) << " us\n";
    f.close();
}

int init_resource(perf_config_t &config){
    assert(config.num > 0);
    config.tposted = new cycles_t[config.num + 1];
    assert(config.tposted);
    std::memset(config.tposted, 0, sizeof(cycles_t)*(config.num + 1));
}

int fin_resource(perf_config_t &config){
    delete config.data_buffer;
    delete [] config.tposted;
}

optparse::OptionParser init_parser(){
    using namespace optparse;
    const std::string usage = "usage: %prog [OPTION]... ";
    auto parser = OptionParser()
        .usage(usage);
    
    parser.add_option("-c", "--config").help("set config file");
    parser.add_option("-i", "--index").type("int").set_default(0)
        .help("set node index");
    parser.add_option("-n", "--num").type("int").set_default(1000)
        .help("iter count for perf");
    parser.add_option("-t", "--type").choices({"mem_push", "send", "send_data"})
        .set_default("send").help("perf type: mem_push, send, send_data");
    parser.add_option("--log_level").set_default("info");
    parser.add_option("--result_file").set_default("result.txt")
        .help("result file path");

    parser.add_option("-s", "--size").set_default("4M")
        .help("size for mem_push and send_data");

    parser.add_option("-a", "--address").set_default("127.0.0.1")
        .help("target ip for rdma");
    parser.add_option("-p", "--port").type("int").set_default(7777)
        .help("target port for rdma");

    return parser;
}

struct msg_incre_d : public MsgData{
    int num;
    virtual int encode(BufferList &bl) override{
        return M_ENCODE(bl, num);
    }
    virtual int decode(BufferList::iterator &it) override{
        return M_DECODE(it, num);
    }
};

class RdmaMsger : public MsgerCallback{
    FlameContext *fct;
    perf_config_t *config;
    ib::RdmaBuffer *rx_buffer = nullptr;
    void on_mem_push_req(Connection *conn, Msg *msg);
    void on_mem_push_resp(Connection *conn, Msg *msg);
    void on_send_req(Connection *conn, Msg *msg);
    void on_send_resp(Connection *conn, Msg *msg);
public:
    explicit RdmaMsger(FlameContext *c, perf_config_t *cfg=nullptr) 
    : fct(c), config(cfg) {};
    virtual void on_conn_recv(Connection *conn, Msg *msg) override;
};

void RdmaMsger::on_mem_push_req(Connection *conn, Msg *msg){
    assert(msg->has_rdma());

    msg_rdma_header_d rdma_header(msg->get_rdma_cnt());
    msg_incre_d incre_data;

    auto it = msg->data_buffer_list().begin();
    rdma_header.decode(it);
    incre_data.decode(it);
    
    auto msger_id = conn->get_session()->peer_msger_id;
    ML(fct, trace, "{}=>  {}", msger_id_to_str(msger_id), msg->to_string());

    auto allocator = Stack::get_rdma_stack()->get_rdma_allocator();

    auto lbuf = rx_buffer;
    if(!lbuf){
        lbuf = allocator->alloc(rdma_header.rdma_bufs[0]->data_len);
        assert(lbuf);
        rx_buffer = lbuf;
    }

    auto rdma_cb = new RdmaRwWork();
    assert(rdma_cb);
    rdma_cb->target_func = 
            [this, allocator, incre_data](RdmaRwWork *w, RdmaConnection *conn){
        auto lbuf = w->lbufs[0];
        ML(fct, info, "rdma read done. buf: {}...{} {}B",
            lbuf->buffer()[0], lbuf->buffer()[lbuf->data_len - 1],
            lbuf->data_len);
        
        auto resp_msg = Msg::alloc_msg(fct, msg_ttype_t::RDMA);
        resp_msg->flag |= FLAME_MSG_FLAG_RESP;

        msg_incre_d new_incre_data;
        new_incre_data.num = incre_data.num + 1;
        resp_msg->append_data(new_incre_data);
        conn->send_msg(resp_msg);

        resp_msg->put();
        allocator->free_buffers(w->rbufs);
        delete w;
    };

    rdma_cb->is_write = false;
    rdma_cb->rbufs = rdma_header.rdma_bufs;
    rdma_cb->lbufs.push_back(lbuf);
    rdma_cb->cnt = 1;

    auto rdma_conn = RdmaStack::rdma_conn_cast(conn);
    assert(rdma_conn);
    rdma_conn->post_rdma_rw(rdma_cb);

    return;
}

void RdmaMsger::on_mem_push_resp(Connection *conn, Msg *msg){
    msg_incre_d incre_data;
    auto it = msg->data_buffer_list().begin();
    incre_data.decode(it);

    assert(this->config);
    this->config->tposted[incre_data.num] = get_cycles();

    if(incre_data.num >= this->config->num){
        dump_result(*(this->config));
        ML(fct, info, "iter {} times, done.", incre_data.num);
        return;
    }

    auto buf = this->config->tx_buffer;
    msg_rdma_header_d rdma_header;
    rdma_header.rdma_bufs.push_back(buf);
    buf->buffer()[0] = 'A' + (incre_data.num % 26);
    buf->buffer()[buf->size() - 1] = 'Z' - (incre_data.num % 26);

    auto req_msg = Msg::alloc_msg(fct, msg_ttype_t::RDMA);
    req_msg->flag |= FLAME_MSG_FLAG_RDMA;
    req_msg->set_rdma_cnt(1);
    req_msg->append_data(rdma_header);
    req_msg->append_data(incre_data);

    conn->send_msg(req_msg);

    req_msg->put();
}

void RdmaMsger::on_send_req(Connection *conn, Msg *msg){
    msg_incre_d incre_data;

    auto it = msg->data_buffer_list().begin();
    incre_data.decode(it);
    
    auto msger_id = conn->get_session()->peer_msger_id;
    ML(fct, trace, "{}=>  {}", msger_id_to_str(msger_id), msg->to_string());

    auto resp_msg = Msg::alloc_msg(fct, msg_ttype_t::RDMA);
    resp_msg->flag |= FLAME_MSG_FLAG_RESP;

    ++incre_data.num;
    resp_msg->append_data(incre_data);
    conn->send_msg(resp_msg);

    resp_msg->put();
}

void RdmaMsger::on_send_resp(Connection *conn, Msg *msg){
    msg_incre_d incre_data;
    auto it = msg->data_buffer_list().begin();
    incre_data.decode(it);

    assert(this->config);
    this->config->tposted[incre_data.num] = get_cycles();

    if(incre_data.num >= this->config->num){
        dump_result(*(this->config));
        ML(fct, info, "iter {} times, done.", incre_data.num);
        return;
    }

    auto req_msg = Msg::alloc_msg(fct, msg_ttype_t::RDMA);
    req_msg->append_data(incre_data);

    if(config->perf_type == perf_type_t::SEND_DATA){
        (*config->data_buffer)[0] = 'A' + (incre_data.num % 26);
        (*config->data_buffer)[config->data_buffer->offset() - 1] =
                                                    'Z' - (incre_data.num % 26);
        req_msg->append_data(*config->data_buffer);
    }
    
    conn->send_msg(req_msg);

    req_msg->put();
}

void RdmaMsger::on_conn_recv(Connection *conn, Msg *msg){
    switch(config->perf_type){
    case perf_type_t::MEM_PUSH:
        if(msg->has_rdma() && msg->is_req()){
            on_mem_push_req(conn, msg);
        }else if(msg->is_resp()){
            on_mem_push_resp(conn, msg);
        }
        break;
    case perf_type_t::SEND:
    case perf_type_t::SEND_DATA:
        if(msg->is_req()){
            on_send_req(conn, msg);
        }else if(msg->is_resp()){
            on_send_resp(conn, msg);
        }
        break;
    }
    
    return;
}

}

#undef M_ENCODE
#undef M_DECODE

#endif 