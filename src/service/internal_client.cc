#include "internal_client.h"
#include "proto/internal.pb.h"

#include "log_service.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace flame {

int InternalClientImpl::register_csd(reg_attr_t& reg_attr, uint64_t csd_id, uint64_t ctime)
{
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
        csd_id = reply.csd_id();
        ctime = reply.ctime();
        return reply.retcode.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::unregister_csd(uint64_t csd_id)
{
    UnregisterRequest req;
    req.set_csd_id(csd_id);

    InternalReply reply;
    ClientContext ctx;
    Status stat = stub_->unregisterCsd(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::sign_up(uint64_t csd_id, uint32_t stat, uint64_t io_addr, uint64_t admin_addr)
{
    SignUpRequest req;
    req.set_csd_id(csd_id);
    req.set_stat(stat);
    req->set_io_addr(io_addr);
    req->set_admin_addr(admin_addr);

    InternalReply reply;
    ClientContext ctx;
    Status stat = stub_->signUp(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::sign_out(uint64_t csd_id)
{
    SignOutRequest req;
    req.set_csd_id(csd_id);

    InternalReply reply;
    ClientContext ctx;
    Status stat = stub_->signOut(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::push_heart_beat(uint64_t csd_id)
{
    HeartBeatRequest req;
    req.set_csd_id(csd_id);

    InternalReply reply;
    ClientContext ctx;
    Status stat = stub_->pushHeartBeat(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::push_status(uint64_t csd_id, uint32_t stat)
{
    StatusRequest req;
    req.set_csd_id(csd_id);
    req.set_stat(stat);

    InternalReply reply;
    ClientContext ctx;
    Status stat = stub_->pushStatus(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::push_health(csd_hlt_attr_t& csd_hlt_attr)
{
    HealthRequest req;
    
    req.set_csd_id(csd_hlt_attr.csd_id);
    req.set_size(csd_hlt_attr.size);
    req.set_alloced(csd_hlt_attr.alloced);
    req.set_used(csd_hlt_attr.used);
    req.set_last_time(csd_hlt_attr.last_time);
    req.set_last_write(csd_hlt_attr.last_write);
    req.set_last_read(csd_hlt_attr.last_read);
    req.set_last_latency(csd_hlt_attr.last_latency);
    req.set_last_alloc(csd_hlt_attr.last_alloc);
    for(auto it = csd_hlt_attr.chk_hlt_list.begin(); it != csd_hlt_attr.chk_hlt_list.end(); ++it)
    {
        ChunkHealthItem* item = req.add_chk_id_list();
        item->set_chk_id(it->chk_id);
        item->set_size(it->size);
        item->set_stat(it->stat);
        item->set_used(it->used);
        item->set_csd_used(csd_used);
        item->set_dst_used(dst_used);
        item->set_last_time(it->last_time);
        item->set_last_write(it->last_write);
        item->set_last_read(it->last_read);
        item->set_last_latency(it->last_latency);
        item->set_last_alloc(it->last->alloc);
    }

    InternalReply reply;
    ClientContext ctx;
    Status stat = stub_->pushHealth(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::pull_related_chunk(list<uint64_t>& chk_list, list<related_chk_attr_t>& res)
{
    ChunkPullRequest req;
    for(auto it = chk_list.begin(); it != chk_list.end(); ++it)
    {
        req.add_chkid_list()->set_from_uint64(*it);
    }

    ChunkPullReply reply;
    ClientContext ctx;
    Status stat = stub_->pullRelatedChunk(&ctx, req, &reply);

    if (stat.ok()) {
        for(uint64_t i = 0; i < reply.rchk_list_size(); ++i)
        {
            related_chk_attr_t item;
            item.org_id = reply.rchk_list(i).org_id();
            item.chk_id = reply.rchk_list(i).chk_id();
            item.csd_id = reply.rchk_list(i).csd_id();
            item.dst_id = reply.rchk_list(i).dst_id();
            res.push_back(item);
        }
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int InternalClientImpl::push_chunk_status(list<chk_push_attr_t>& chk_list)
{
    ChunkPushRequest req;
    for(auto it = chk_list.begin(); it != chk_list.end(); ++it)
    {
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
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

} // namespace 