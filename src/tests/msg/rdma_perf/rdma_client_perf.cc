#include "msg/msg_core.h"
#include "common/context.h"
#include "util/fmt.h"
#include "util/clog.h"

#include "rdma_msger.h"

#include <unistd.h>
#include <cstdio>
#include <string>

using namespace flame;

perf_config_t global_config;

void send_first_msg(MsgContext *mct){
    NodeAddr *addr = new NodeAddr(mct);
    addr->ip_from_string("127.0.0.1");
    addr->set_port(6666);
    msger_id_t msger_id = msger_id_from_node_addr(addr);
    auto session = mct->manager->get_session(msger_id);
    NodeAddr *rdma_addr = new NodeAddr(mct);
    rdma_addr->set_ttype(NODE_ADDR_TTYPE_RDMA);
    rdma_addr->ip_from_string(global_config.target_rdma_ip);
    rdma_addr->set_port(global_config.target_rdma_port);
    session->set_listen_addr(addr);
    session->set_listen_addr(rdma_addr, msg_ttype_t::RDMA);
    auto conn = session->get_conn(msg_ttype_t::RDMA);
    if(global_config.perf_type == perf_type_t::MEM_PUSH){
        ML(mct, info, "send first msg (mem_push)");
        auto allocator = Stack::get_rdma_stack()->get_rdma_allocator();
        
        auto buf = allocator->alloc(global_config.size); //4MB
        assert(buf);
        msg_rdma_header_d rdma_header(1, false);
        rdma_header.rdma_bufs.push_back(buf);
        buf->buffer()[0] = 'A';
        buf->buffer()[buf->size() - 1] = 'Z';
        buf->data_len = buf->size();
        global_config.tx_buffer = buf;

        global_config.tposted[0] = get_cycles();

        auto req_msg = Msg::alloc_msg(mct, msg_ttype_t::RDMA);
        req_msg->flag |= FLAME_MSG_FLAG_RDMA;
        req_msg->set_rdma_cnt(1);
        req_msg->append_data(rdma_header);

        msg_incre_d incre_data;
        incre_data.num = 0;
        req_msg->append_data(incre_data);

        conn->send_msg(req_msg);
        req_msg->put();
    }else if(global_config.perf_type == perf_type_t::SEND){
        ML(mct, info, "send first msg (send)");
        global_config.tposted[0] = get_cycles();
        auto req_msg = Msg::alloc_msg(mct, msg_ttype_t::RDMA);

        msg_incre_d incre_data;
        incre_data.num = 0;
        req_msg->append_data(incre_data);

        conn->send_msg(req_msg);
        req_msg->put();
    }else if(global_config.perf_type == perf_type_t::SEND_DATA){
        ML(mct, info, "send first msg (send)");

        auto buf = new MsgBuffer(global_config.size);
        assert(buf);
        buf->set_offset(global_config.size);
        (*buf)[0] = 'A';
        (*buf)[buf->offset() - 1] = 'Z';
        global_config.data_buffer = buf;

        global_config.tposted[0] = get_cycles();
        auto req_msg = Msg::alloc_msg(mct, msg_ttype_t::RDMA);

        msg_incre_d incre_data;
        incre_data.num = 0;
        req_msg->append_data(incre_data);
        req_msg->append_data(*buf);

        conn->send_msg(req_msg);
        req_msg->put();
    }
    rdma_addr->put();
    addr->put();
}

#define CFG_PATH "flame_client.cfg"

int main(int argc, char *argv[]){
    auto parser = init_parser();
    optparse::Values options = parser.parse_args(argc, argv);
    auto config_path = options.get("config");

    FlameContext *fct = FlameContext::get_context();
    if(!fct->init_config(
            std::string(config_path).empty()?CFG_PATH:config_path)){
        clog("init config failed.");
        return -1;
    }
    if(!fct->init_log("", str2upper(std::string(options.get("log_level"))), 
                fmt::format("client{}",  std::string(options.get("index"))))){
         clog("init log failed.");
        return -1;
    }

    global_config.num = (int)options.get("num");
    global_config.result_file = std::string(options.get("result_file"));
    global_config.perf_type = perf_type_from_str(
                                            std::string(options.get("type")));
    global_config.size = size_str_to_uint64(std::string(options.get("size")));
    
    global_config.target_rdma_ip = std::string(options.get("address"));
    global_config.target_rdma_port = (int)options.get("port");

    auto mct = new MsgContext(fct);

    ML(mct, info, "init complete.");
    ML(mct, info, "load cfg: " CFG_PATH);

    init_resource(global_config);

    auto msger = new RdmaMsger(mct, &global_config);

    mct->load_config();
    mct->config->set_msg_log_level(std::string(options.get("log_level")));

    ML(mct, info, "before msg module init");
    mct->init(msger);
    ML(mct, info, "after msg module init");

    ML(mct, info, "msger_id {:x} {:x} ", mct->config->msger_id.ip,
                                         mct->config->msger_id.port);

    send_first_msg(mct);

    std::getchar();

    ML(mct, info, "before msg module fin");
    mct->fin();
    ML(mct, info, "after msg module fin");

    delete msger;

    fin_resource(global_config);

    delete mct;

    return 0;
}