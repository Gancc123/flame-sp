#ifndef FLAME_SERVICE_FLAME_H
#define FLAME_SERVICE_FLAME_H

#include <grpcpp/grpcpp.h>

#include "proto/flame.pb.h"
#include "proto/flame.grpc.pb.h"

#include "mgr/mgr_service.h"

namespace flame {
namespace service {

class FlameServiceImpl final : public FlameService::Service, public MgrService {
public:
    FlameServiceImpl(MgrContext* mct) : FlameService::Service(), MgrService(mct) {}

    /**
     * Interface for Service 
     */

    //* Gateway Set
    // GW注册：建立一个Gw连接
    virtual ::grpc::Status connect(::grpc::ServerContext* context, const ::ConnectRequest* request, ::FlameReply* response);
    // GW注销：关闭一个GW连接
    virtual ::grpc::Status disconnect(::grpc::ServerContext* context, const ::DisconnectRequest* request, ::FlameReply* response);
    // 获取Flame集群信息
    virtual ::grpc::Status getClusterInfo(::grpc::ServerContext* context, const ::NoneRequest* request, ::ClusterInfoReply* response);
    // 自动关闭Flame集群
    virtual ::grpc::Status shutdownCluster(::grpc::ServerContext* context, const ::NoneRequest* request, ::FlameReply* response);
    // 自动清理Flame集群
    virtual ::grpc::Status cleanCluster(::grpc::ServerContext* context, const ::NoneRequest* request, ::FlameReply* response);
    // 获取CSD地址信息，需要指定一系列CSD ID
    virtual ::grpc::Status pullCsdAddr(::grpc::ServerContext* context, const ::CsdIDListRequest* request, ::CsdAddrListReply* response);
    
    //* Group Set
    // 获取所有VG信息，支持分页（需要提供<offset, limit>，以vg_name字典顺序排序）
    virtual ::grpc::Status getVolGroupList(::grpc::ServerContext* context, const ::VGListRequest* request, ::VGListReply* response);
    // 创建VG
    virtual ::grpc::Status createVolGroup(::grpc::ServerContext* context, const ::VGCreateRequest* request, ::FlameReply* response);
    // 删除VG
    virtual ::grpc::Status removeVolGroup(::grpc::ServerContext* context, const ::VGRemoveRequest* request, ::FlameReply* response);
    // 重命名VG
    virtual ::grpc::Status renameVolGroup(::grpc::ServerContext* context, const ::VGRenameRequest* request, ::FlameReply* response);

    //* Volume Set
    // 获取指定VG内的所有Volume信息
    virtual ::grpc::Status getVolumeList(::grpc::ServerContext* context, const ::VolListRequest* request, ::VolListReply* response);
    // 创建Volume
    virtual ::grpc::Status createVolume(::grpc::ServerContext* context, const ::VolCreateRequest* request, ::FlameReply* response);
    // 删除Volume
    virtual ::grpc::Status removeVolume(::grpc::ServerContext* context, const ::VolRemoveRequest* request, ::FlameReply* response);
    // 重命名Volume
    virtual ::grpc::Status renameVolume(::grpc::ServerContext* context, const ::VolRenameRequest* request, ::FlameReply* response);
    // 获取Volume信息
    virtual ::grpc::Status getVolumeInfo(::grpc::ServerContext* context, const ::VolInfoRequest* request, ::VolInfoReply* response);
    // 更改Volume大小
    virtual ::grpc::Status resizeVolume(::grpc::ServerContext* context, const ::VolResizeRequest* request, ::FlameReply* response);
    // 打开Volume：在MGR登记Volume访问信息（没有加载元数据信息）
    virtual ::grpc::Status openVolume(::grpc::ServerContext* context, const ::VolOpenRequest* request, ::FlameReply* response);
    // 关闭Volume：在MGR消除Volume访问信息
    virtual ::grpc::Status closeVolume(::grpc::ServerContext* context, const ::VolCloseRequest* request, ::FlameReply* response);
    // 锁定Volume：在MGR登记Volume锁定信息，防止其他GW打开Volume
    virtual ::grpc::Status lockVolume(::grpc::ServerContext* context, const ::VolLockRequest* request, ::FlameReply* response);
    // 解锁Volume
    virtual ::grpc::Status unlockVolume(::grpc::ServerContext* context, const ::VolUnlockRequest* request, ::FlameReply* response);
    // 获取Volume的Chunk信息
    virtual ::grpc::Status getVolumeMaps(::grpc::ServerContext* context, const ::VolMapsRequest* request, ::VolMapsReply* response);

    //* Chunk Set
    // 获取指定Chunk信息
    virtual ::grpc::Status getChunkMaps(::grpc::ServerContext* context, const ::ChunkMapsRequest* request, ::ChunkMapsReply* response);

}; // class FlameServiceImpl

} // namespace service
} // namespace flame

#endif // FLAME_SERVICE_FLAME_H 
