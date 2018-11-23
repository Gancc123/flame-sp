#include <grpcpp/grpcpp.h>

#include "proto/gw.pb.h"
#include "proto/gw.grpc.pb.h"

namespace flame {

class GWServiceImpl final : public MgrGwService::Service {
public:
    // GW
    virtual ::grpc::Status MgrGwConnect(::grpc::ServerContext* context, const ::MgrGwConnectReq* request, ::MgrGwConnectRes* response) override;
    virtual ::grpc::Status MgrGwDisconnect(::grpc::ServerContext* context, const ::MgrGwDisconnectReq* request, ::MgrGwDisconnectRes* response) override;
    virtual ::grpc::Status MgrGwClusterInfo(::grpc::ServerContext* context, const ::MgrGwClusterInfoReq* request, ::MgrGwClusterInfoRes* response) override;
    virtual ::grpc::Status MgrGwClusterShutdown(::grpc::ServerContext* context, const ::MgrGwClusterShutdownReq* request, ::MgrGwClusterShutdownRes* response) override;
    virtual ::grpc::Status MgrGwClusterClean(::grpc::ServerContext* context, const ::MgrGwClusterCleanReq* request, ::MgrGwClusterCleanRes* response) override;
    virtual ::grpc::Status MgrGwNodeRoute(::grpc::ServerContext* context, const ::MgrGwNodeRouteReq* request, ::MgrGwNodeRouteRes* response) override;
    
    // Group
    virtual ::grpc::Status MgrGroupList(::grpc::ServerContext* context, const ::MgrGroupListReq* request, ::MgrGroupListRes* response) override;
    virtual ::grpc::Status MgrGroupCreate(::grpc::ServerContext* context, const ::MgrGroupCreateReq* request, ::MgrGroupCreateRes* response) override;
    virtual ::grpc::Status MgrGroupDelete(::grpc::ServerContext* context, const ::MgrGroupDeleteReq* request, ::MgrGroupDeleteRes* response) override;
    virtual ::grpc::Status MgrGroupRename(::grpc::ServerContext* context, const ::MgrGroupRenameReq* request, ::MgrGroupRenameRes* response) override;
    
    // Volume
    virtual ::grpc::Status MgrVolumeList(::grpc::ServerContext* context, const ::MgrVolumeListReq* request, ::MgrVolumeListRes* response) override;
    virtual ::grpc::Status MgrVolumeCreate(::grpc::ServerContext* context, const ::MgrVolumeCreateReq* request, ::MgrVolumeCreateRes* response) override;
    virtual ::grpc::Status MgrVolumeDelete(::grpc::ServerContext* context, const ::MgrVolumeDeleteReq* request, ::MgrVolumeDeleteRes* response) override;
    virtual ::grpc::Status MgrVolumeRename(::grpc::ServerContext* context, const ::MgrVolumeRenameReq* request, ::MgrVolumeRenameRes* response) override;
    virtual ::grpc::Status MgrVolumeInfo(::grpc::ServerContext* context, const ::MgrVolumeInfoReq* request, ::MgrVolumeInfoRes* response) override;
    virtual ::grpc::Status MgrVolumeResize(::grpc::ServerContext* context, const ::MgrVolumeResizeReq* request, ::MgrVolumeResizeRes* response) override;
    virtual ::grpc::Status MgrVolumeOpen(::grpc::ServerContext* context, const ::MgrVolumeOpenReq* request, ::MgrVolumeOpenRes* response) override;
    virtual ::grpc::Status MgrVolumeClose(::grpc::ServerContext* context, const ::MgrVolumeCloseReq* request, ::MgrVolumeCloseRes* response) override;
    virtual ::grpc::Status MgrVolumeLock(::grpc::ServerContext* context, const ::MgrVolumeLockReq* request, ::MgrVolumeLockRes* response) override;
    virtual ::grpc::Status MgrVolumeUnlock(::grpc::ServerContext* context, const ::MgrVolumeUnlockReq* request, ::MgrVolumeUnlockRes* response) override;
    virtual ::grpc::Status MgrVolumeMaps(::grpc::ServerContext* context, const ::MgrVolumeMapsReq* request, ::MgrVolumeMapsRes* response) override;
    
    // chunk
    virtual ::grpc::Status MgrChunkMaps(::grpc::ServerContext* context, const ::MgrChunkMapsReq* request, ::MgrChunkMapsRes* response) override;
}; // class 

} // namespace flame