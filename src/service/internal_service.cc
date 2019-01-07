#include "internal_service.h"
#include "include/retcode.h"

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
    csd_reg_attr_t mt;
    mt.csd_name = request->csd_name();
    mt.size = request->size();
    mt.io_addr = request->io_addr();
    mt.admin_addr = request->admin_addr();
    mt.stat = request->stat();

    mct_->log()->ldebug("internal_service", "csd register: csd_name(%s), size(%llu)",
        mt.csd_name.c_str(), mt.size
    );

    int r;
    CsdHandle* hdl;
    if ((r = mct_->csdm()->csd_register(mt, &hdl)) == RC_SUCCESS) {
        CsdObject* obj = hdl->read_and_lock();
        response->set_csd_id(obj->get_csd_id());
        response->set_ctime(obj->get_ctime());
        hdl->unlock();
    }
    
    response->mutable_retcode()->set_code(r);
    mct_->log()->ldebug("internal_service", "csd register: %d", r);
    return Status::OK;
}

// CSD注销
Status InternalServiceImpl::unregisterCsd(ServerContext* context,
const UnregisterRequest* request, InternalReply* response)
{
    int r = mct_->csdm()->csd_unregister(request->csd_id());
    response->set_code(r);
    mct_->log()->ldebug("internal_service", "csd (%llu) register: %d", request->csd_id(), r);
    return Status::OK;
}
    
// CSD上线
Status InternalServiceImpl::signUp(ServerContext* context,
const SignUpRequest* request, InternalReply* response)
{
    csd_sgup_attr_t at;
    at.csd_id = request->csd_id();
    at.stat = request->stat();
    at.io_addr = request->io_addr();
    at.admin_addr = request->admin_addr();
    int r = mct_->csdm()->csd_sign_up(at); 
    response->set_code(r);
    mct_->log()->ldebug("internal_service", "csd (%llu) sign up: %d", at.csd_id, r);
    return Status::OK;
}
    
// CSD下线
Status InternalServiceImpl::signOut(ServerContext* context,
const SignOutRequest* request, InternalReply* response)
{
    int r = mct_->csdm()->csd_sign_out(request->csd_id());
    response->set_code(r);
    mct_->log()->ldebug("internal_service", "csd (%llu) sign out: %d", request->csd_id(), r);
    return Status::OK;
}
    
// CSD心跳汇报
Status InternalServiceImpl::pushHeartBeat(ServerContext* context,
const HeartBeatRequest* request, InternalReply* response)
{
    // 心跳汇报改用pushStatus接口
    // uint64_t latime = utime_t::now().to_usec();
    // int r = csd_ms->update_at(request->csd_id(), latime);
    // response->set_code(r);
    return Status::OK;
}
    
// CSD状态汇报
Status InternalServiceImpl::pushStatus(ServerContext* context,
const StatusRequest* request, InternalReply* response)
{
    int r = mct_->cltm()->update_stat(request->csd_id(), request->stat());
    response->set_code(r);
    mct_->log()->ldebug("internal_service", "csd (%llu) push status (%u): %d", request->csd_id(), request->stat(), r);
    return Status::OK;
}
    
// CSD健康信息汇报 todo
Status InternalServiceImpl::pushHealth(ServerContext* context,
const HealthRequest* request, InternalReply* response)
{
    int r;
    // 提取出csd的健康信息
    csd_hlt_sub_t csd_hlt;
    csd_hlt.csd_id = request->csd_id();
    csd_hlt.size = request->size();
    csd_hlt.alloced = request->alloced();
    csd_hlt.used = request->used();
    csd_hlt.period.ctime = request->last_time();
    csd_hlt.period.wr_cnt = request->last_write();
    csd_hlt.period.rd_cnt = request->last_read();
    csd_hlt.period.lat = request->last_latency();
    csd_hlt.period.alloc = request->last_alloc();

    // 更新csd的健康信息
    if ((r = mct_->csdm()->csd_health_update(csd_hlt.csd_id, csd_hlt)) != RC_SUCCESS) {
        mct_->log()->lerror("internal_service", "update csd health faild: %llu", csd_hlt.csd_id);
        response->set_code(r);
        return Status::OK;
    }

    // 提取出chunk的健康信息
    // std::list<chk_hlt_attr_t> chk_hlt_list;
    // for (uint64_t i = 0; i < request->chunk_health_list_size(); ++i) {
    //     const ChunkHealthItem& item = request->chunk_health_list(i);
    //     chk_hlt_attr_t hmt;
    //     hmt.chk_id = item.chk_id();
    //     hmt.size = item.size();
    //     hmt.stat = item.stat();
    //     hmt.used = item.used();
    //     hmt.csd_used = item.csd_used();
    //     hmt.dst_used = item.dst_used();
    //     hmt.hlt_meta.last_time = item.last_time();
    //     hmt.hlt_meta.last_write = item.last_write();
    //     hmt.hlt_meta.last_read = item.last_read();
    //     hmt.hlt_meta.last_latency = item.last_latency();
    //     hmt.hlt_meta.last_alloc = item.last_alloc();

    //     chk_hlt_list.push_back(hmt);
    // }

    

    // // 更新chunk的健康信息
    // int r = mct_->chkm()->chunk_push_health(chk_hlt_list);

    response->set_code(r);

    return Status::OK;
}

// 拉取关联Chunk信息
Status InternalServiceImpl::pullRelatedChunk(ServerContext* context,
const ChunkPullRequest* request, ChunkPullReply* response)
{
    // for (uint64_t i = 0; i < request->chkid_list_size(); ++i) {
    //     uint64_t org_id = request->chkid_list(i);
    //     list<chunk_meta_t> res_list;
    //     int r = mct_->chkm()->chunk_get_related(res_list, org_id);

    //     for (auto it = res_list.begin(); it != res_list.end(); ++it) {
    //         RelatedChunkItem* item = response->add_rchk_list();
    //         item->set_org_id(org_id);
    //         item->set_chk_id(it->chk_id);
    //         item->set_csd_id(it->csd_id);
    //         item->set_dst_id(it->dst_id);
    //     }

    // }
    return Status::OK;
}
    
// 推送Chunk相关信息
Status InternalServiceImpl::pushChunkStatus(ServerContext* context,
const ChunkPushRequest* request, InternalReply* response)
{
    // std::list<chk_push_attr_t> chk_list;
    // for (uint64_t i = 0; i < request->chk_list_size(); ++i) {
    //     const ChunkPushItem& item = request->chk_list(i);
    //     chunk_meta_t mt;
    //     mt.chk_id = item.chk_id();
    //     mt.stat = item.stat();
    //     mt.csd_id = item.csd_id();
    //     mt.dst_id = item.dst_id();
    //     mt.dst_ctime = item.dst_mtime();

    //     chk_list.push_back(mt);
    // }

    // int r = mct_->chkm()->chunk_push_status(chk_list);

    return Status::OK;
}

} // namespace service
} // namespace flame