#include "internal_service.h"

using grpc::ServerContext;
using grpc::Status;

#include <list>

using namespace std;
namespace flame {
namespace service {

// CSD注册
Status InternalServiceImpl::registerCsd(ServerContext* context,
const RegisterRequest* request, RegisterReply* response)
{
    csd_meta_t mt;
    mt.name = request->csd_name();
    mt.size = request->size();
    mt.io_addr = request->io_addr();
    mt.admin_addr = request->admin_addr();
    mt.stat = request->stat();
    mt.ctime = utime_t::now().to_msec();
    mt.latime = utime_t::now().to_msec();

    int r = csd_ms->create_and_get(mt);
    
    response->mutable_retcode()->set_code(r);
    response->set_csd_id(mt.csd_id);
    response->set_ctime(mt.ctime);

    return Status::OK;
}

// CSD注销
Status InternalServiceImpl::unregisterCsd(ServerContext* context,
const UnregisterRequest* request, InternalReply* response)
{
    int r = csd_ms->remove(request->csd_id());
    response->set_code(r);
    return Status::OK;
}
    
// CSD上线
Status InternalServiceImpl::signUp(ServerContext* context,
const SignUpRequest* request, InternalReply* response)
{
    int r = csd_ms->update_sm(request->csd_id(), request->stat(), request->io_addr(), request->admin_addr());
    response->set_code(r);
    return Status::OK;
}
    
// CSD下线
Status InternalServiceImpl::signOut(ServerContext* context,
const SignOutRequest* request, InternalReply* response)
{
    int r = csd_ms->update_sm(request->csd_id(), 0, 0, 0);
    response->set_code(r);
    return Status::OK;
}
    
// CSD心跳汇报
Status InternalServiceImpl::pushHeartBeat(ServerContext* context,
const HeartBeatRequest* request, InternalReply* response)
{
    uint64_t latime = utime_t::now().to_msec();
    int r = csd_ms->update_at(request->csd_id(), latime);
    response->set_code(r);
    return Status::OK;
}
    
// CSD状态汇报
Status InternalServiceImpl::pushStatus(ServerContext* context,
const StatusRequest* request, InternalReply* response)
{
    int r = csd_ms->update_sm(request->csd_id(), request->stat(), 0xffff, 0xffff);
    response->set_code(r);
    return Status::OK;
}
    
// CSD健康信息汇报
Status InternalServiceImpl::pushHealth(ServerContext* context,
const HealthRequest* request, InternalReply* response)
{
    csd_health_meta_t csd_hlt;
    csd_hlt_ms->get(csd_hlt, request->csd_id());

    // update csd's write_count read_count wear_weight
    csd_hlt.write_count = csd_hlt.write_count + request->last_write();
    csd_hlt.read_count = csd_hlt.read_count + request->last_read();
    double u = request->used() / request->size();//存储空间利用率
    csd_hlt.wear_weight = csd_hlt.wear_weight + (request->last_write() / (1 - u));

    // update csd's others info
    csd_hlt.size = request->size();
    csd_hlt.alloced = request->alloced();
    csd_hlt.used = request->used();
    csd_hlt.last_time = request->last_time();
    csd_hlt.last_write = request->last_write();
    csd_hlt.last_read = request->last_read();
    csd_hlt.last_latency = request->last_latency();
    csd_hlt.last_alloc = request->last_alloc();

    int r = csd_hlt_ms->update(csd_hlt);

    // update chunk's health info
    for (uint64_t i = 0; i < request->chunk_health_list_size(); ++i) {
        const ChunkHealthItem& item = request->chunk_health_list(i);
        chunk_health_meta_t hmt;
        chk_hlt_ms->get(hmt, item.chk_id());
        //update chunk's write_count read_count
        hmt.write_count = hmt.write_count / 2 + item.last_write();
        hmt.read_count = hmt.read_count / 2 + item.last_read();
        //update chunk's others info
        hmt.size = item.size();
        hmt.stat = item.stat();
        hmt.used = item.used();
        hmt.csd_used = item.csd_used();
        hmt.dst_used = item.dst_used();
        hmt.last_time = item.last_time();
        hmt.last_write = item.last_write();
        hmt.last_read = item.last_read();
        hmt.last_latency = item.last_latency();
        hmt.last_alloc = item.last_alloc();

        r = chk_hlt_ms->update(hmt);
    }

    response->set_code(r);

    return Status::OK;
}

// 拉取关联Chunk信息
Status InternalServiceImpl::pullRelatedChunk(ServerContext* context,
const ChunkPullRequest* request, ChunkPullReply* response)
{
    for (uint64_t i = 0; i < request->chkid_list_size(); ++i) {
        uint64_t org_id = request->chkid_list(i);
        chunk_meta_t mt;
        chk_ms->get(mt, org_id);
        uint64_t vol_id = mt.vol_id;
        uint32_t index = mt.index;
        list<chunk_meta_t> res_list;
        chk_ms->list_cg(res_list, vol_id, index);
        for (auto it = res_list.begin(); it != res_list.end(); ++it) {
            RelatedChunkItem* item = response->add_rchk_list();
            item->set_org_id(org_id);
            item->set_chk_id(it->chk_id);
            item->set_csd_id(it->csd_id);
            item->set_dst_id(it->dst_id);
        }
    }
    return Status::OK;
}
    
// 推送Chunk相关信息
Status InternalServiceImpl::pushChunkStatus(ServerContext* context,
const ChunkPushRequest* request, InternalReply* response)
{
    for (uint64_t i = 0; i < request->chk_list_size(); ++i) {
        const ChunkPushItem& item = request->chk_list(i);
        chunk_meta_t mt;
        chk_ms->get(mt, item.chk_id());

        mt.stat = item.stat();
        mt.csd_id = item.csd_id();
        mt.dst_id = item.dst_id();
        mt.dst_ctime = item.dst_mtime();

        chk_ms->update(mt);
    }
    return Status::OK;
}

} // namespace service
} // namespace flame