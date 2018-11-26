#include "internal_service.h"

using grpc::ServerContext;
using grpc::Status;

namespace flame {
namespace service {

// CSD注册
Status InternalServiceImpl::registerCsd(ServerContext* context,
const RegisterRequest* request, RegisterReply* response)
{
    return Status::OK;
}

// CSD注销
Status InternalServiceImpl::unregisterCsd(ServerContext* context,
const UnregisterRequest* request, InternalReply* response)
{
    return Status::OK;
}
    
// CSD上线
Status InternalServiceImpl::signUp(ServerContext* context,
const SignUpRequest* request, InternalReply* response)
{
    return Status::OK;
}
    
// CSD下线
Status InternalServiceImpl::signOut(ServerContext* context,
const SignOutRequest* request, InternalReply* response)
{
    return Status::OK;
}
    
// CSD心跳汇报
Status InternalServiceImpl::pushHeartBeat(ServerContext* context,
const HeartBeatRequest* request, InternalReply* response)
{
    return Status::OK;
}
    
// CSD状态汇报
Status InternalServiceImpl::pushStatus(ServerContext* context,
const StatusRequest* request, InternalReply* response)
{
    return Status::OK;
}
    
// CSD健康信息汇报
Status InternalServiceImpl::pushHealth(ServerContext* context,
const HealthRequest* request, InternalReply* response)
{
    return Status::OK;
}

// 拉取关联Chunk信息
Status InternalServiceImpl::pullRelatedChunk(ServerContext* context,
const ChunkPullRequest* request, ChunkPullReply* response)
{
    return Status::OK;
}
    
// 推送Chunk相关信息
Status InternalServiceImpl::pushChunkStatus(ServerContext* context,
const ChunkPushRequest* request, InternalReply* response)
{
    return Status::OK;
}

} // namespace service
} // namespace flame