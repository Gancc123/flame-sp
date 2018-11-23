#include "gw_service.h"

using grpc::ServerContext;
using grpc::Status;

namespace flame {

/**
 * GW Part
 */

Status GWServiceImpl::MgrGwConnect(ServerContext* context, 
const MgrGwConnectReq* request, MgrGwConnectRes* response) 
{
    return Status::OK;
}

Status GWServiceImpl::MgrGwDisconnect(ServerContext* context, 
const MgrGwDisconnectReq* request, MgrGwDisconnectRes* response) 
{
    return Status::OK;
}

Status GWServiceImpl::MgrGwClusterInfo(ServerContext* context, 
const ::MgrGwClusterInfoReq* request, MgrGwClusterInfoRes* response) 
{
    return Status::OK;
}

Status GWServiceImpl::MgrGwClusterShutdown(ServerContext* context, 
const MgrGwClusterShutdownReq* request, MgrGwClusterShutdownRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrGwClusterClean(ServerContext* context, 
const MgrGwClusterCleanReq* request, MgrGwClusterCleanRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrGwNodeRoute(ServerContext* context, 
const MgrGwNodeRouteReq* request, MgrGwNodeRouteRes* response)
{
    return Status::OK;
}

/**
 * Group Part
 */
    
Status GWServiceImpl::MgrGroupList(ServerContext* context, 
const MgrGroupListReq* request, MgrGroupListRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrGroupCreate(ServerContext* context, 
const MgrGroupCreateReq* request, MgrGroupCreateRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrGroupDelete(ServerContext* context, 
const MgrGroupDeleteReq* request, MgrGroupDeleteRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrGroupRename(ServerContext* context, 
const MgrGroupRenameReq* request, MgrGroupRenameRes* response)
{
    return Status::OK;
}
    
/**
 * Volume Part
 */

Status GWServiceImpl::MgrVolumeList(ServerContext* context, 
const MgrVolumeListReq* request, MgrVolumeListRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrVolumeCreate(ServerContext* context, 
const MgrVolumeCreateReq* request, MgrVolumeCreateRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrVolumeDelete(ServerContext* context, 
const MgrVolumeDeleteReq* request, MgrVolumeDeleteRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrVolumeRename(ServerContext* context, 
const MgrVolumeRenameReq* request, MgrVolumeRenameRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrVolumeInfo(ServerContext* context, 
const MgrVolumeInfoReq* request, MgrVolumeInfoRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrVolumeResize(ServerContext* context, 
const MgrVolumeResizeReq* request, MgrVolumeResizeRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrVolumeOpen(ServerContext* context, 
const MgrVolumeOpenReq* request, MgrVolumeOpenRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrVolumeClose(ServerContext* context, 
const MgrVolumeCloseReq* request, MgrVolumeCloseRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrVolumeLock(ServerContext* context, 
const MgrVolumeLockReq* request, MgrVolumeLockRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrVolumeUnlock(ServerContext* context, 
const MgrVolumeUnlockReq* request, MgrVolumeUnlockRes* response)
{
    return Status::OK;
}

Status GWServiceImpl::MgrVolumeMaps(ServerContext* context, 
const MgrVolumeMapsReq* request, MgrVolumeMapsRes* response)
{
    return Status::OK;
}
    
/**
 * Chunk Part
 */

Status GWServiceImpl::MgrChunkMaps(ServerContext* context, 
const MgrChunkMapsReq* request, MgrChunkMapsRes* response)
{
    return Status::OK;
}

} // namespace flame