#ifndef FLAME_SERVICE_CSDS_H
#define FLAME_SERVICE_CSDS_H

#include <grpcpp/grpcpp.h>

#include "proto/csds.pb.h"
#include "proto/csds.grpc.pb.h"

#include "csd/csd_service.h"

namespace flame {
namespace service {

class CsdsServiceImpl final : public CsdsService::Service, public CsdService {
public:
    CsdsServiceImpl(CsdContext* cct) : CsdsService::Service(), CsdService(cct) {}

    // 拉取Chunk版本信息
    virtual ::grpc::Status fetchChunk(::grpc::ServerContext* context, const ::ChunkFetchRequest* request, ::ChunkFetchReply* response);
    // 推送Chunk信号：告知Chunk的行为或者状态
    virtual ::grpc::Status pushChunkSignal(::grpc::ServerContext* context, const ::ChunkSignalRequest* request, ::CsdsReply* response);

    // MGR Ctrl
    // 关闭CSD
    virtual ::grpc::Status shutdown(::grpc::ServerContext* context, const ::ShutdownRequest* request, ::CsdsReply* response);
    // 清理CSD
    virtual ::grpc::Status clean(::grpc::ServerContext* context, const ::CleanRequest* request, ::CsdsReply* response);
    // 创建Chunk
    virtual ::grpc::Status createChunk(::grpc::ServerContext* context, const ::ChunkCreateRequest* request, ::CsdsReply* response);
    // 删除Chunk
    virtual ::grpc::Status removeChunk(::grpc::ServerContext* context, const ::ChunkRemoveRequest* request, ::CsdsReply* response);
    // 告知选主信息
    virtual ::grpc::Status chooseChunk(::grpc::ServerContext* context, const ::ChunkChooseRequest* request, ::CsdsReply* response);
    // 迁移Chunk
    virtual ::grpc::Status moveChunk(::grpc::ServerContext* context, const ::ChunkMoveRequest* request, ::CsdsReply* response);
 
}; // class CsdsServiceImpl

} // namespace service
} // namespace flame

#endif // FLAME_SERVICE_CSDS_H