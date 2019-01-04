#include "internal_client.h"
#include "proto/internal.pb.h"

#include <sstream>

#include "log_service.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace std;

namespace flame {

int InternalClientImpl::register_csd(reg_res_t& res, const reg_attr_t& reg_attr) {
    RegisterRequest req;
    req.set_csd_name(reg_attr.csd_name);
    req.set_size(reg_attr.size);
    req.set_io_addr(reg_attr.io_addr);
    req.set_admin_addr(reg_attr.admin_addr);
    req.set_stat(reg_attr.stat);

    RegisterReply reply;
    ClientContext ctx;
    Status stat = stub_->registerCsd(&ctx, req, &reply);

    if (stat.ok()) {
        res.csd_id = reply.csd_id();
        res.ctime = reply.ctime();
        return reply.retcode().code();
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::unregister_csd(uint64_t csd_id) {
    UnregisterRequest req;
    req.set_csd_id(csd_id);

    InternalReply reply;
    ClientContext ctx;
    Status stat = stub_->unregisterCsd(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::sign_up(uint64_t csd_id, uint32_t stat, uint64_t io_addr, uint64_t admin_addr) {
    SignUpRequest req;
    req.set_csd_id(csd_id);
    req.set_stat(stat);
    req.set_io_addr(io_addr);
    req.set_admin_addr(admin_addr);

    InternalReply reply;
    ClientContext ctx;
    Status rets = stub_->signUp(&ctx, req, &reply);

    if (rets.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", rets.error_code(), rets.error_message().c_str());
        return -rets.error_code();
    }
}

int InternalClientImpl::sign_out(uint64_t csd_id) {
    SignOutRequest req;
    req.set_csd_id(csd_id);

    InternalReply reply;
    ClientContext ctx;
    Status stat = stub_->signOut(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::push_heart_beat(uint64_t csd_id) {
    HeartBeatRequest req;
    req.set_csd_id(csd_id);

    InternalReply reply;
    ClientContext ctx;
    Status stat = stub_->pushHeartBeat(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::push_status(uint64_t csd_id, uint32_t stat) {
    StatusRequest req;
    req.set_csd_id(csd_id);
    req.set_stat(stat);

    InternalReply reply;
    ClientContext ctx;
    Status rets = stub_->pushStatus(&ctx, req, &reply);

    if (rets.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", rets.error_code(), rets.error_message().c_str());
        return -rets.error_code();
    }
}

int InternalClientImpl::push_health(const csd_hlt_attr_t& csd_hlt_attr) {
    HealthRequest req;
    
    req.set_csd_id(csd_hlt_attr.csd_hlt_sub.csd_id);
    req.set_size(csd_hlt_attr.csd_hlt_sub.size);
    req.set_alloced(csd_hlt_attr.csd_hlt_sub.alloced);
    req.set_used(csd_hlt_attr.csd_hlt_sub.used);
    req.set_last_time(csd_hlt_attr.csd_hlt_sub.period.ctime);
    req.set_last_write(csd_hlt_attr.csd_hlt_sub.period.wr_cnt);
    req.set_last_read(csd_hlt_attr.csd_hlt_sub.period.rd_cnt);
    req.set_last_latency(csd_hlt_attr.csd_hlt_sub.period.lat);
    req.set_last_alloc(csd_hlt_attr.csd_hlt_sub.period.alloc);
    for (auto it = csd_hlt_attr.chk_hlt_list.begin(); it != csd_hlt_attr.chk_hlt_list.end(); ++it) {
        ChunkHealthItem* item = req.add_chunk_health_list();
        item->set_chk_id(it->chk_id);
        item->set_size(it->size);
        item->set_stat(it->stat);
        item->set_used(it->used);
        item->set_csd_used(it->csd_used);
        item->set_dst_used(it->dst_used);
        item->set_last_time(it->period.ctime);
        item->set_last_write(it->period.wr_cnt);
        item->set_last_read(it->period.rd_cnt);
        item->set_last_latency(it->period.lat);
        item->set_last_alloc(it->period.alloc);
    }

    InternalReply reply;
    ClientContext ctx;
    Status stat = stub_->pushHealth(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::pull_related_chunk(std::list<related_chk_attr_t>& res, const std::list<uint64_t>& chk_list) {
    ChunkPullRequest req;
    for (auto it = chk_list.begin(); it != chk_list.end(); ++it) {
        req.add_chkid_list(*it);
    }

    ChunkPullReply reply;
    ClientContext ctx;
    Status stat = stub_->pullRelatedChunk(&ctx, req, &reply);

    if (stat.ok()) {
        for (uint64_t i = 0; i < reply.rchk_list_size(); ++i) {
            related_chk_attr_t item;
            item.org_id = reply.rchk_list(i).org_id();
            item.chk_id = reply.rchk_list(i).chk_id();
            item.csd_id = reply.rchk_list(i).csd_id();
            item.dst_id = reply.rchk_list(i).dst_id();
            res.push_back(item);
        }
        return 0;
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::push_chunk_status(const std::list<chk_push_attr_t>& chk_list) {
    ChunkPushRequest req;
    for (auto it = chk_list.begin(); it != chk_list.end(); ++it) {
        ChunkPushItem* item = req.add_chk_list();
        item->set_chk_id(it->chk_id);
        item->set_stat(it->stat);
        item->set_csd_id(it->csd_id);
        item->set_dst_id(it->dst_id);
        item->set_dst_mtime(it->dst_mtime);
    }

    InternalReply reply;
    ClientContext ctx;
    Status stat = stub_->pushChunkStatus(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

std::shared_ptr<InternalClient> InternalClientFoctoryImpl::make_internal_client(node_addr_t addr) {
    uint32_t ip = addr.get_ip();
    uint8_t* part = (uint8_t*)&ip;

    ostringstream oss;
    oss << *part << "." << *(part + 1) << "." << *(part + 2) << "." << *(part + 3);
    oss << ":" << addr.get_port();

    InternalClient* client = new InternalClientImpl(fct_, grpc::CreateChannel(
        oss.str(), grpc::InsecureChannelCredentials()
    ));

    return std::shared_ptr<InternalClient>(client);
}

std::shared_ptr<InternalClient> InternalClientFoctoryImpl::make_internal_client(const std::string& addr) {
    InternalClient* client = new InternalClientImpl(fct_, grpc::CreateChannel(
        addr, grpc::InsecureChannelCredentials()
    ));

    return std::shared_ptr<InternalClient>(client);
}

} // namespace flame