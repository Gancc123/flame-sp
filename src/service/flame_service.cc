#include "flame_service.h"
#include "include/retcode.h"
#include "util/utime.h"

#include <string>
#include <list>

using grpc::ServerContext;
using grpc::Status;

using namespace std;

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
    gateway_meta_t gw;
    gw.gw_id = request->gw_id();
    gw.admin_addr = request->admin_addr();
    gw.ltime = utime_t::now().to_usec();
    gw.atime = utime_t::now().to_usec();
    int r = gw_ms->create(gw);
    response->set_code(r);
    return Status::OK;
}
    
// GW注销：关闭一个GW连接
Status FlameServiceImpl::disconnect(ServerContext* context, 
const DisconnectRequest* request, FlameReply* response)
{
    int gw_id = request->gw_id();
    int r = gw_ms->remove(gw_id);
    response->set_code(r);
    return Status::OK;
}
    
// 获取Flame集群信息
Status FlameServiceImpl::getClusterInfo(ServerContext* context, 
const NoneRequest* request, ClusterInfoReply* response)
{
    cluster_meta_t cluster;
    cluster_ms->get(cluster);
    response->set_name(cluster.name);
    response->set_mgrs(cluster.mgrs);
    response->set_csds(cluster.csds);
    response->set_size(cluster.size);
    response->set_alloced(cluster.alloced);
    response->set_used(cluster.used);
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
    list<uint64_t> csd_ids;
    for(uint64_t i = 0; i < request->csd_id_list_size(); ++i)
    {
        csd_ids.push_back(request->csd_id_list(i));
    }
    list<csd_meta_t> res_list;
    csd_ms->list(res_list, csd_ids);

    for(auto it = res_list.begin(); it != res_list.end(); ++it)
    {
        CsdAddrItem* item = response->add_csd_list();
        item->set_csd_id(it->csd_id);
        item->set_io_addr(it->io_addr);
        item->set_stat(it->stat);
    }
    return Status::OK;
}
    
//* Group Set
// 获取所有VG信息，支持分页（需要提供<offset, limit>，以vg_name字典顺序排序）
Status FlameServiceImpl::getVolGroupList(ServerContext* context,
const VGListRequest* request, VGListReply* response)
{
    // list<volume_group_meta_t> res_list;

    // int r = mct_->volm()->volume_group_list(res_list, request->offset(), request->limit());
    // if (r) return r;

    // for (auto it = res_list.begin(); it != res_list.end(); it++) {
    //     VGItem* item = response->add_vg_list();
    //     item->set_vg_id(it->vg_id);
    //     item->set_name(it->name);
    //     item->set_ctime(it->ctime);
    //     item->set_volumes(it->volumes);
    //     item->set_size(it->size);
    //     item->set_alloced(it->alloced);
    //     item->set_used(it->used);
    // }
    return Status::OK;
}

// 创建VG
Status FlameServiceImpl::createVolGroup(ServerContext* context,
const VGCreateRequest* request, FlameReply* response)
{
    // int r = mct_->volm()->volume_group_create(request->vg_name());

    // response->set_code(r);
    return Status::OK;
}

// 删除VG
Status FlameServiceImpl::removeVolGroup(ServerContext* context,
const VGRemoveRequest* request, FlameReply* response)
{
    // int r = mct_->volm()->volume_group_remove(request->vg_name());
    // response->set_code(r);
    return Status::OK;
}
    
// 重命名VG
Status FlameServiceImpl::renameVolGroup(ServerContext* context,
const VGRenameRequest* request, FlameReply* response)
{
    // int r = mct_->volm()->volume_group_rename(request->old_vg_name(), request->new_vg_name());
    // response->set_code(r);
    return Status::OK;
}

//* Volume Set
// 获取指定VG内的所有Volume信息
Status FlameServiceImpl::getVolumeList(ServerContext* context,
const VolListRequest* request, VolListReply* response)
{
    // int r = mct_->volm()->volume_list(res_list, request->vg_name(), request->offset(), request->limit());
     
    // for(auto it = res_list.begin(); it != res_list.end(); ++it)
    // {
    //     VolumeItem* item = response->add_vol_list();
    //     item->set_vol_id(it->vol_id);
    //     item->set_vg_id(it->vg_id);
    //     item->set_name(it->name);
    //     item->set_ctime(it->ctime);
    //     item->set_chk_sz(it->chk_sz);
    //     item->set_size(it->size);
    //     item->set_alloced(it->alloced);
    //     item->set_used(it->used);
    //     item->set_flags(it->flags);
    //     item->set_spolicy(it->spolicy);
    //     item->set_chunks(it->chunks);
    // }

    return Status::OK;
}
    
// 创建Volume
Status FlameServiceImpl::createVolume(ServerContext* context,
const VolCreateRequest* request, FlameReply* response)
{
    // volume_meta_t vol;
    // vol.name = request->vol_name();
    // vol.ctime = utime_t::now().to_usec();
    // vol.chk_sz = request->chk_sz();
    // vol.size = request->size();
    // vol.flags = request->flags();
    // vol.spolicy = request->spolicy();

    // int r = mct_->volm()->volume_create(request->vg_name(), vol);
    // response->set_code(r);
    return Status::OK;
}

// 删除Volume
Status FlameServiceImpl::removeVolume(ServerContext* context,
const VolRemoveRequest* request, FlameReply* response)
{
    // int r = mct_->volm()->volume_remove(request->vg_name(), request->vol_name());
    // response->set_code(r);

    return Status::OK;
}
    
// 重命名Volume
Status FlameServiceImpl::renameVolume(ServerContext* context,
const VolRenameRequest* request, FlameReply* response)
{
    // int r = mct_->volm()->volume_rename(request->vg_name(), request->old_vol_name(), request->new_vol_name());
    // response->set_code(r);
    return Status::OK;
}
    
// 获取Volume信息
Status FlameServiceImpl::getVolumeInfo(ServerContext* context,
const VolInfoRequest* request, VolInfoReply* response)
{
    // volume_meta_t res;
    // int r = mct_->volm()->volume_get(res, request->vg_name(), request->vol_name());
    // response->set_retcode(r);

    // VolumeItem* item = response->mutable_vol();
    // item->set_vol_id(res.vol_id); 
    // item->set_vg_id(res.vg_id);  
    // item->set_name(res.name);   
    // item->set_ctime(res.ctime);  
    // item->set_chk_sz(res.chk_sz); 
    // item->set_size(res.size);   
    // item->set_alloced(res.alloced);
    // item->set_used(res.used);   
    // item->set_flags(res.flags);  
    // item->set_spolicy(res.spolicy);
    // item->set_chunks(res.chunks);

    return Status::OK;
}

// 更改Volume大小
Status FlameServiceImpl::resizeVolume(ServerContext* context,
const VolResizeRequest* request, FlameReply* response)
{
    // int r = mct_->volm()->volume_resize(request->vg_name(), request->vol_name(), request->new_size());
    // response->set_code(r);
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
    // list<chunk_meta_t> res_list;
    // int r = mct_->chkm()->chunk_get_maps(res_list, request->vol_id());

    // for(auto it = res_list.begin(); it != res_list.end(); ++it)
    // {
    //     ChunkItem* item = response->add_chk_list();
    //     item->set_chk_id(it->chk_id);
    //     item->set_vol_id(it->vol_id);
    //     item->set_index(it->index);
    //     item->set_stat(it->stat);
    //     item->set_spolicy(it->spolicy);
    //     item->set_primary(it->primary);
    //     item->set_size(it->size);
    //     item->set_csd_id(it->csd_id);
    //     item->set_dst_id(it->dst_id);
    // }
    return Status::OK;
}

//* Chunk Set
// 获取指定Chunk信息
Status FlameServiceImpl::getChunkMaps(ServerContext* context,
const ChunkMapsRequest* request, ChunkMapsReply* response)
{
    // list<uint64_t> chk_ids;
    // for(uint64_t i = 0; i < request->chk_id_list_size(); ++i)
    // {
    //     chk_ids.push_back(request->chk_id_list(i));
    // }

    // list<chunk_meta_t> res_list;
    // int r = mct_->chkm()->chunk_get_set(res_list, chk_ids);

    // for(auto it = res_list.begin(); it != res_list.end(); ++it)
    // {
    //     ChunkItem* item = response->add_chk_list();
    //     item->set_chk_id(it->chk_id);
    //     item->set_vol_id(it->vol_id);
    //     item->set_index(it->index);
    //     item->set_stat(it->stat);
    //     item->set_spolicy(it->spolicy);
    //     item->set_primary(it->primary);
    //     item->set_size(it->size);
    //     item->set_csd_id(it->csd_id);
    //     item->set_dst_id(it->dst_id);
    // }

    return Status::OK;
}

} // namespace service
} // namespace flame