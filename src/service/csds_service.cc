#include "csds_service.h"

#include "include/meta.h"

using grpc::ServerContext;
using grpc::Status;

namespace flame {
namespace service {

// 拉取Chunk版本信息
Status CsdsServiceImpl::fetchChunk(ServerContext* context, 
const ChunkFetchRequest* request, ChunkFetchReply* response)
{
    return Status::OK;
}

// 推送Chunk信号：告知Chunk的行为或者状态
Status CsdsServiceImpl::pushChunkSignal(ServerContext* context,
const ChunkSignalRequest* request, CsdsReply* response)
{
    return Status::OK;
}

// MGR Ctrl
// 关闭CSD
Status CsdsServiceImpl::shutdown(ServerContext* context,
const ShutdownRequest* request, CsdsReply* response)
{
    return Status::OK;
}

// 清理CSD
Status CsdsServiceImpl::clean(ServerContext* context,
const CleanRequest* request, CsdsReply* response)
{
    return Status::OK;
}

// 创建Chunk
Status CsdsServiceImpl::createChunk(ServerContext* context,
const ChunkCreateRequest* request, ChunkBulkReply* response)
{
    int r;
    for (int i = 0; i < request->chk_id_list_size(); i++) {
        chunk_id_t chk_id = request->chk_id_list(i);
        chunk_create_opts_t opts;
        opts.vol_id = chk_id.get_vol_id();
        opts.index = chk_id.get_index();
        opts.spolicy = request->spolicy();
        opts.flags = request->flags();
        opts.size = request->size();
        r = cs_->chunk_create(chk_id, opts);
        auto chkr = response->add_res_list();
        chkr->set_chk_id(chk_id);
        chkr->set_res(r);
    }
    return Status::OK;
}

// 删除Chunk
Status CsdsServiceImpl::removeChunk(ServerContext* context,
const ChunkRemoveRequest* request, ChunkBulkReply* response)
{
    int r;
    for (int i = 0; i < request->chk_id_list_size(); i++) {
        uint64_t chk_id = request->chk_id_list(i);
        r = cs_->chunk_remove(chk_id);
        auto chkr = response->add_res_list();
        chkr->set_chk_id(chk_id);
        chkr->set_res(r);
    }
    return Status::OK;
}

// 告知选主信息
Status CsdsServiceImpl::chooseChunk(ServerContext* context,
const ChunkChooseRequest* request, ChunkBulkReply* response)
{
    return Status::OK;
}

// 迁移Chunk
Status CsdsServiceImpl::moveChunk(ServerContext* context,
const ChunkMoveRequest* request, ChunkBulkReply* response)
{
    return Status::OK;
}

} // namespace service
} // namespace flame