#ifndef FLAME_TESTS_MSG_TEST_MEM_CPY_MEM_CPY_H
#define FLAME_TESTS_MSG_TEST_MEM_CPY_MEM_CPY_H

#include "msg/msg_core.h"
#include "util/clog.h"
#include "util/option_parser.h"

#include "get_clock.h"

#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <regex>
#include <atomic>

#define M_ENCODE(bl, data) (bl).append(&(data), sizeof(data))
#define M_DECODE(it, data) (it).copy(&(data), sizeof(data))

namespace flame{
namespace msg{

void print_time(cycles_t start, cycles_t end, const std::string &info){
    double cycle_to_unit = get_cpu_mhz(0);
    double t = (end - start) / cycle_to_unit;
    clog(fmt::format(info, t));
}

enum class perf_type_t {
    NONE,
    MEM_PUSH,  // send req + rdma read + send resp
    SEND,      // send req + send resp
    SEND_DATA, // send req with data + send resp
    MEM_FETCH, // send req + rdma write + send resp
    MEM_FETCH_WITH_IMM, //send req + rdma write_with_imm
};

std::string str_from_perf_type(perf_type_t t){
    switch(t){
    case perf_type_t::SEND:
        return "send";
    case perf_type_t::SEND_DATA:
        return "senddata";
    case perf_type_t::MEM_PUSH:
        return "mempush";
    case perf_type_t::MEM_FETCH:
        return "memfetch";
    case perf_type_t::MEM_FETCH_WITH_IMM:
        return "memfetchimm";
    }
    return "unknown";
}

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
    if(lower == "mem_fetch"){
        return perf_type_t::MEM_FETCH;
    }
    if(lower == "mem_fetch_with_imm"){
        return perf_type_t::MEM_FETCH_WITH_IMM;
    }
    return perf_type_t::NONE;
}

struct mem_cpy_config_t{
    bool is_tgt;
    bool is_optimized;
    uint64_t size;
    uint32_t num;
    cycles_t *tposted = nullptr;
    perf_type_t perf_type;
    std::string result_file;
    std::vector<Session *> sessions;
};

void dump_result(mem_cpy_config_t &cfg){
    double cycle_to_unit = get_cpu_mhz(0); // no warn
    std::vector<cycles_t> deltas;
    deltas.resize(cfg.num, 0);
    for(int i = 0;i < cfg.num;++i){
        deltas[i] = cfg.tposted[i+1] - cfg.tposted[i];
    }

    if(cfg.result_file == "result.txt"){
        cfg.result_file = fmt::format("result_rn{}_{}_{}{}.txt",
                                cfg.sessions.size(), 
                                size_str_from_uint64(cfg.size),
                                str_from_perf_type(cfg.perf_type),
                                cfg.is_optimized?"_O":"");
    }

    //std::sort(deltas.begin(), deltas.end());
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

    std::sort(deltas.begin(), deltas.end());
    //ignore the largest deltas(it's for conn establish.)
    double median = 0;
    size_t dsize = deltas.size() - 1;
    if(dsize % 2 == 0){
        median = (deltas[dsize / 2 - 1] + deltas[dsize / 2]) 
                    / cycle_to_unit / 2;
    }else{
        median = deltas[dsize / 2] / cycle_to_unit;
    }
    f << "Median: " << median << " us\n";

    f.close();
    clog(fmt::format("Avg: {} us, Median: {} us", 
                        (average_sum / cfg.num), median));
    clog(fmt::format("dump result to {}", cfg.result_file));
}

optparse::OptionParser init_parser(){
    using namespace optparse;
    const std::string usage = "usage: %prog [OPTION]... ";
    auto parser = OptionParser().usage(usage);
    
    parser.add_option("-c", "--config").help("set config file");
    parser.add_option("-i", "--index").type("int").set_default(0)
        .help("set node index");
    parser.add_option("-n", "--num").type("long")
        .set_default(1000)
        .help("iter count for perf");
    parser.add_option("-t", "--type")
        .choices({"mem_push", "mem_fetch_with_imm"})
        .set_default("mem_push")
        .help("perf type: mem_push, mem_fetch_with_imm");
    parser.add_option("--log_level").set_default("info");
    parser.add_option("--result_file").set_default("result.txt")
        .help("result file path");

    parser.add_option("-s", "--size").set_default("64K")
        .help("size for mem_push, mem_fetch_with_imm ");

    parser.add_option("-d", "--dst").help("target ip:port[/ip:port]...");

    parser.add_option("-O", "--optimize")
          .action("store_true")
          .set_default("false")
          .help("enable optimization");

    return parser;
}

void parse_target_ips(const std::string &ips_str, MsgContext *mct,
                        mem_cpy_config_t &cfg){
    std::regex ip_regex("([0-9.]+):([0-9]+)");
    auto ip_begin = std::sregex_iterator(ips_str.begin(), ips_str.end(),
                                        ip_regex);
    auto ip_end = std::sregex_iterator();
    for(auto i = ip_begin;i != ip_end;++i){
        auto match = *i;
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
        cfg.sessions.push_back(session);
        addr->put();
    }
}

void init_resource(mem_cpy_config_t &config, optparse::Values &options){
    assert(config.num > 0);
    config.tposted = new cycles_t[config.num + 1];
    assert(config.tposted);
    std::memset(config.tposted, 0, sizeof(cycles_t)*(config.num + 1));
    clog(fmt::format("size: {} B", config.size));
    clog(fmt::format("num: {}", config.num));
    clog(fmt::format("type: {}", std::string(options.get("type"))));
}

void fin_resource(mem_cpy_config_t &config){
    delete [] config.tposted;
}

struct msg_incre_d : public MsgData{
    uint32_t num;
    virtual int encode(MsgBufferList &bl) override{
        return M_ENCODE(bl, num);
    }
    virtual int decode(MsgBufferList::iterator &it) override{
        return M_DECODE(it, num);
    }
};

class MemCpyMsger : public MsgerCallback{
    MsgContext *mct;
    mem_cpy_config_t *cfg;
    std::atomic<int> recv_cnt;
    std::vector<ib::RdmaBuffer *> rw_buffers;
    void on_mem_push_req(Connection *conn, Msg *msg);
    void on_mem_push_resp(Connection *conn, Msg *msg);
    void on_mem_fetch_req(Connection *conn, Msg *msg);
    void on_mem_fetch_resp(Connection *conn, Msg *msg);
public:
    explicit MemCpyMsger(MsgContext *c, mem_cpy_config_t *config)
    : mct(c), cfg(config), recv_cnt(0) {}
    virtual void on_conn_recv(Connection *conn, Msg *msg) override;
    int prep_rw_bufs();
    void free_rw_bufs();
    void send_req_msg(int num);
};

void MemCpyMsger::send_req_msg(int num){
    cycles_t start, end;
    auto src_buf = rw_buffers[0];
    
    if(cfg->perf_type == perf_type_t::MEM_PUSH){
        src_buf->buffer()[0] = 'A' + (num % 26);
        src_buf->buffer()[src_buf->size() - 1] = 'Z' - (num % 26);
        // std::memset(src_buf->buffer() + 1, 'a' + (num % 26), 
        //             src_buf->size() - 2);
        for(int i = 0;i < cfg->sessions.size();++i){
            ML(mct, info, "send to {}", cfg->sessions[i]->to_string());
            auto buf = src_buf;

            // if(num < 10) start = get_cycles();
            if(!cfg->is_optimized){
                buf = rw_buffers[i+1];
                std::memcpy(buf->buffer(), src_buf->buffer(), buf->size());
            }
            // if(num < 10) {
            //     end = get_cycles();
            //     print_time(start, end, "memcpy overhead: {}us");
            // }

            msg_rdma_header_d rdma_header(1, false);
            rdma_header.rdma_bufs.push_back(buf);
            msg_incre_d incre_data;
            incre_data.num = num;

            auto req_msg = Msg::alloc_msg(mct, msg_ttype_t::RDMA);
            req_msg->flag |= FLAME_MSG_FLAG_RDMA;
            req_msg->set_rdma_cnt(1);
            req_msg->append_data(rdma_header);
            req_msg->append_data(incre_data);

            auto tgt_conn = cfg->sessions[i]->get_conn(msg_ttype_t::RDMA);
            tgt_conn->send_msg(req_msg);
            req_msg->put();
        }
    }else if(cfg->perf_type == perf_type_t::MEM_FETCH_WITH_IMM){
        for(int i = 0;i < cfg->sessions.size();++i){
            ML(mct, info, "send to {}", cfg->sessions[i]->to_string());
            auto buf = rw_buffers[i + 1];

            msg_rdma_header_d rdma_header(1, false);
            rdma_header.rdma_bufs.push_back(buf);
            msg_incre_d incre_data;
            incre_data.num = num;

            auto req_msg = Msg::alloc_msg(mct, msg_ttype_t::RDMA);
            req_msg->flag |= FLAME_MSG_FLAG_RDMA;
            req_msg->set_rdma_cnt(1);
            req_msg->append_data(rdma_header);
            req_msg->append_data(incre_data);

            auto tgt_conn = cfg->sessions[i]->get_conn(msg_ttype_t::RDMA);
            tgt_conn->send_msg(req_msg);

            req_msg->put();
        }
    }
}

int MemCpyMsger::prep_rw_bufs(){
    auto allocator = Stack::get_rdma_stack()->get_rdma_allocator();
    int r = 0, buf_num = 1;
    if(!cfg->is_tgt){
        buf_num += cfg->sessions.size();
    }

    r = allocator->alloc_buffers(cfg->size, buf_num, rw_buffers);
    assert(r == buf_num);

    for(auto buf : rw_buffers){
        buf->data_len = buf->size();
    }

    return r;
}

void MemCpyMsger::free_rw_bufs(){
    auto allocator = Stack::get_rdma_stack()->get_rdma_allocator();
    allocator->free_buffers(rw_buffers);
}

void MemCpyMsger::on_conn_recv(Connection *conn, Msg *msg){
    switch(cfg->perf_type){
    case perf_type_t::MEM_PUSH:
        if(msg->has_rdma() && msg->is_req()){
            on_mem_push_req(conn, msg);
        }else if(msg->is_resp()){
            on_mem_push_resp(conn, msg);
        }
        break;
    case perf_type_t::MEM_FETCH_WITH_IMM:
        if(msg->is_req()){
            on_mem_fetch_req(conn, msg);
        }else if(msg->is_resp()){
            on_mem_fetch_resp(conn, msg);
        }
        break;
    default:
        break;
    }
    return;
}

void MemCpyMsger::on_mem_push_req(Connection *conn, Msg *msg){
    assert(msg->has_rdma());
    assert(cfg->is_tgt);

    msg_rdma_header_d rdma_header(msg->get_rdma_cnt(), msg->with_imm());
    msg_incre_d incre_data;

    auto it = msg->data_iter();
    rdma_header.decode(it);
    incre_data.decode(it);
    
    auto msger_id = conn->get_session()->peer_msger_id;
    ML(mct, trace, "{}=>  {}", msger_id_to_str(msger_id), msg->to_string());

    auto allocator = Stack::get_rdma_stack()->get_rdma_allocator();

    auto lbuf = rw_buffers[0];

    auto rdma_cb = new RdmaRwWork();
    assert(rdma_cb);
    rdma_cb->target_func = 
            [this, allocator, incre_data](RdmaRwWork *w, RdmaConnection *conn){
        auto lbuf = w->lbufs[0];
        ML(mct, info, "rdma read done. buf: {}...{} {}B",
            lbuf->buffer()[0], lbuf->buffer()[lbuf->data_len - 1],
            lbuf->data_len);
        
        auto resp_msg = Msg::alloc_msg(mct, msg_ttype_t::RDMA);
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

void MemCpyMsger::on_mem_push_resp(Connection *conn, Msg *msg){
    msg_incre_d incre_data;
    auto it = msg->data_iter();
    incre_data.decode(it);

    assert(this->cfg);
    assert(!this->cfg->is_tgt);

    recv_cnt++;
    if(recv_cnt % cfg->sessions.size() == 0){
        this->cfg->tposted[incre_data.num] = get_cycles();
        assert(recv_cnt/cfg->sessions.size() == incre_data.num);

        if(incre_data.num >= this->cfg->num){
            dump_result(*(this->cfg));
            ML(mct, info, "iter {} times, done. recv_cnt: {}", incre_data.num,
                                                                recv_cnt);
            return;
        }

        send_req_msg(incre_data.num);
    }
}

void MemCpyMsger::on_mem_fetch_req(Connection *conn, Msg *msg){
    assert(msg->has_rdma());
    assert(cfg->is_tgt);

    msg_rdma_header_d rdma_header(msg->get_rdma_cnt(), msg->with_imm());
    msg_incre_d incre_data;

    auto it = msg->data_iter();
    rdma_header.decode(it);
    incre_data.decode(it);
    
    auto msger_id = conn->get_session()->peer_msger_id;
    ML(mct, trace, "{}=>  {}", msger_id_to_str(msger_id), msg->to_string());

    auto allocator = Stack::get_rdma_stack()->get_rdma_allocator();

    auto lbuf = rw_buffers[0];
    lbuf->buffer()[0] = 'A' + (incre_data.num % 26);
    lbuf->buffer()[lbuf->size() - 1] = 'Z' - (incre_data.num % 26);
    lbuf->data_len = lbuf->size(); //make sure that data_len is not zero.

    auto rdma_cb = new RdmaRwWork();
    assert(rdma_cb);
    rdma_cb->is_write = true;
    rdma_cb->rbufs = rdma_header.rdma_bufs;
    rdma_cb->lbufs.push_back(lbuf);
    rdma_cb->cnt = 1;

    assert(cfg->perf_type == perf_type_t::MEM_FETCH_WITH_IMM);

    rdma_cb->imm_data = incre_data.num + 1;
    rdma_cb->target_func = 
        [this, allocator](RdmaRwWork *w, RdmaConnection *conn){
        allocator->free_buffers(w->rbufs);
        delete w;
    };

    auto rdma_conn = RdmaStack::rdma_conn_cast(conn);
    assert(rdma_conn);
    rdma_conn->post_rdma_rw(rdma_cb);
}

void MemCpyMsger::on_mem_fetch_resp(Connection *conn, Msg *msg){
    msg_incre_d incre_data;
    assert(cfg);
    assert(cfg->perf_type == perf_type_t::MEM_FETCH_WITH_IMM);
    assert(msg->is_imm_data());

    recv_cnt++;
    if(recv_cnt % cfg->sessions.size() != 0){
        return;
    }

    incre_data.num = msg->imm_data;

    if(!cfg->is_optimized){
        std::memcpy(rw_buffers[0]->buffer(),
                    rw_buffers[1]->buffer(),
                    rw_buffers[0]->size());
        rw_buffers[0]->data_len = rw_buffers[0]->size();
    }

    this->cfg->tposted[incre_data.num] = get_cycles();

    auto buf = rw_buffers[0];
    ML(mct, info, "rdma write done. buf: {}...{} {}B",
            buf->buffer()[0], buf->buffer()[buf->data_len - 1],
            buf->data_len);

    if(incre_data.num >= this->cfg->num){
        dump_result(*(this->cfg));
        ML(mct, info, "iter {} times, done. recv_cnt: {}", incre_data.num, 
                                                            recv_cnt);
        return;
    }

    //next iter
    send_req_msg(incre_data.num);
    
}


}// namespace msg
}// namespace flame

#undef M_ENCODE
#undef M_DECODE

#endif //FLAME_TESTS_MSG_TEST_MEM_CPY_MEM_CPY_H