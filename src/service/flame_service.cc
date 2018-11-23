#include "flame_service.h"

using grpc::ServerContext;
using grpc::Status;

namespace flame {
namespace service {

/**
 * Interface for Service 
 */

//* Gateway Set
// GW注册：建立一个Gw连接
Status FlameServiceImpl::connect(ServerContext* context, 
const ConnectRequest* request, FlameReply* response) 
{
    return Status::OK;
}
    
// GW注销：关闭一个GW连接
Status FlameServiceImpl::disconnect(ServerContext* context, 
const DisconnectRequest* request, FlameReply* response)
{
    return Status::OK;
}
    
// 获取Flame集群信息
Status FlameServiceImpl::getClusterInfo(ServerContext* context, 
const NoneRequest* request, ClusterInfoReply* response)
{
    return Status::OK;
}

// 自动关闭Flame集群
Status FlameServiceImpl::shutdownCluster(ServerContext* context, 
const NoneRequest* request, FlameReply* response)
{
    return Status::OK;
}

// 自动清理Flame集群
Status FlameServiceImpl::cleanCluster(ServerContext* context, 
const NoneRequest* request, FlameReply* response)
{
    return Status::OK;
}
    
// 获取CSD地址信息，需要指定一系列CSD ID
Status FlameServiceImpl::pullCsdAddr(ServerContext* context,
const CsdIDListRequest* request, CsdAddrListReply* response)
{
    return Status::OK;
}
    
//* Group Set
// 获取所有VG信息，支持分页（需要提供<offset, limit>，以vg_name字典顺序排序）
Status FlameServiceImpl::getVolGroupList(ServerContext* context,
const VGListRequest* request, VGListReply* response)
{
    return Status::OK;
}

// 创建VG
Status FlameServiceImpl::createVolGroup(ServerContext* context,
const VGCreateRequest* request, FlameReply* response)
{
    return Status::OK;
}

// 删除VG
Status FlameServiceImpl::removeVolGroup(ServerContext* context,
const VGRemoveRequest* request, FlameReply* response)
{
    return Status::OK;
}
    
// 重命名VG
Status FlameServiceImpl::renameVolGroup(ServerContext* context,
const VGRenameRequest* request, FlameReply* response)
{
    return Status::OK;
}

//* Volume Set
// 获取指定VG内的所有Volume信息
Status FlameServiceImpl::getVolumeList(ServerContext* context,
const VolListRequest* request, VolListReply* response)
{
    return Status::OK;
}
    
// 创建Volume
Status FlameServiceImpl::createVolume(ServerContext* context,
const VolCreateRequest* request, FlameReply* response)
{
    return Status::OK;
}

// 删除Volume
Status FlameServiceImpl::removeVolume(ServerContext* context,
const VolRemoveRequest* request, FlameReply* response)
{
    return Status::OK;
}
    
// 重命名Volume
Status FlameServiceImpl::renameVolume(ServerContext* context,
const VolRenameRequest* request, FlameReply* response)
{
    return Status::OK;
}
    
// 获取Volume信息
Status FlameServiceImpl::getVolumeInfo(ServerContext* context,
const VolInfoRequest* request, VolInfoReply* response)
{
    return Status::OK;
}

// 更改Volume大小
Status FlameServiceImpl::resizeVolume(ServerContext* context,
const VolResizeRequest* request, FlameReply* response)
{
    return Status::OK;
}
    
// 打开Volume：在MGR登记Volume访问信息（没有加载元数据信息）
Status FlameServiceImpl::openVolume(ServerContext* context,
const VolOpenRequest* request, FlameReply* response)
{
    return Status::OK;
}

// 关闭Volume：在MGR消除Volume访问信息
Status FlameServiceImpl::closeVolume(ServerContext* context,
const VolCloseRequest* request, FlameReply* response)
{
    return Status::OK;
}
    
// 锁定Volume：在MGR登记Volume锁定信息，防止其他GW打开Volume
Status FlameServiceImpl::lockVolume(ServerContext* context,
const VolLockRequest* request, FlameReply* response)
{
    return Status::OK;
}
    
// 解锁Volume
Status FlameServiceImpl::unlockVolume(ServerContext* context,
const VolUnlockRequest* request, FlameReply* response)
{
    return Status::OK;
}
    
// 获取Volume的Chunk信息
Status FlameServiceImpl::getVolumeMaps(ServerContext* context,
const VolMapsRequest* request, VolMapsReply* response)
{
    return Status::OK;
}

//* Chunk Set
// 获取指定Chunk信息
Status FlameServiceImpl::getChunkMaps(ServerContext* context,
const ChunkMapsRequest* request, ChunkMapsReply* response)
{
    return Status::OK;
}

} // namespace service
} // namespace flame