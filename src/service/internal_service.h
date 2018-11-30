#ifndef FLAME_SERVICE_INTERNAL_H
#define FLAME_SERVICE_INTERNAL_H

#include <grpcpp/grpcpp.h>

#include "proto/internal.pb.h"
#include "proto/internal.grpc.pb.h"

#include "mgr/mgr_service.h"

namespace flame {
namespace service {

class InternalServiceImpl final : public InternalService::Service, public MgrService {
public:
    InternalServiceImpl(MgrContext* mct) : InternalService::Service(), MgrService(mct) {}

    // CSD注册
    virtual ::grpc::Status registerCsd(::grpc::ServerContext* context, const ::RegisterRequest* request, ::RegisterReply* response);
    // CSD注销
    virtual ::grpc::Status unregisterCsd(::grpc::ServerContext* context, const ::UnregisterRequest* request, ::InternalReply* response);
    // CSD上线
    virtual ::grpc::Status signUp(::grpc::ServerContext* context, const ::SignUpRequest* request, ::InternalReply* response);
    // CSD下线
    virtual ::grpc::Status signOut(::grpc::ServerContext* context, const ::SignOutRequest* request, ::InternalReply* response);
    // CSD心跳汇报
    virtual ::grpc::Status pushHeartBeat(::grpc::ServerContext* context, const ::HeartBeatRequest* request, ::InternalReply* response);
    // CSD状态汇报
    virtual ::grpc::Status pushStatus(::grpc::ServerContext* context, const ::StatusRequest* request, ::InternalReply* response);
    // CSD健康信息汇报
    virtual ::grpc::Status pushHealth(::grpc::ServerContext* context, const ::HealthRequest* request, ::InternalReply* response);
    // 拉取关联Chunk信息
    virtual ::grpc::Status pullRelatedChunk(::grpc::ServerContext* context, const ::ChunkPullRequest* request, ::ChunkPullReply* response);
    // 推送Chunk相关信息
    virtual ::grpc::Status pushChunkStatus(::grpc::ServerContext* context, const ::ChunkPushRequest* request, ::InternalReply* response);
protected:
    ClusterMS* cluster_ms {mct_->ms()->get_cluster_ms()};
    VolumeGroupMS* vg_ms {mct_->ms()->get_vg_ms()};
    VolumeMS* vol_ms {mct_->ms()->get_volume_ms()};
    ChunkMS* chk_ms {mct_->ms()->get_chunk_ms()};
    ChunkHealthMS* chk_hlt_ms {mct_->ms()->get_chunk_health_ms()};
    CsdMS* csd_ms {mct_->ms()->get_csd_ms()};
    CsdHealthMS* csd_hlt_ms {mct_->ms()->get_csd_health_ms()};
    GatewayMS* gw_ms {mct_->ms()->get_gw_ms()};
}; // class InternalServiceImpl

} // namespace service 
} // namespace flame

#endif // FLAME_SERVICE_INTERNAL_H