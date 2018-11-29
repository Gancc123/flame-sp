#include "csds_service.h"

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
const ChunkCreateRequest* request, CsdsReply* response)
{
    return Status::OK;
}

// 删除Chunk
Status CsdsServiceImpl::removeChunk(ServerContext* context,
const ChunkRemoveRequest* request, CsdsReply* response)
{
    return Status::OK;
}

// 告知选主信息
Status CsdsServiceImpl::chooseChunk(ServerContext* context,
const ChunkChooseRequest* request, CsdsReply* response)
{
    return Status::OK;
}

// 迁移Chunk
Status CsdsServiceImpl::moveChunk(ServerContext* context,
const ChunkMoveRequest* request, CsdsReply* response)
{
    return Status::OK;
}

} // namespace service
} // namespace flame