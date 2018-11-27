#include "flame_client.h"

#include "log_service.h"


using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
namespace flame {

int FlameClient::connect(uint64_t gw_id, uint64_t admin_addr) {
    ConnectRequest req;
    req.set_gw_id(gw_id);
    req.set_admin_addr(admin_addr);

    FlameReply reply;

    ClientContext cxt;

    Status stat = stub_->connect(&cxt, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code(); // 注意负号
    }
}

int FlameClient::disconnect(uint64_t gw_id){
    DisconnectRequest req;
    req.set_gw_id(gw_id);

    FlameReply reply;

    ClientContext cxt;

    Status stat = stub_->disconnect(&ctx, req, &reply);

    if (stat.ok()){
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

 int FlameClient::get_cluster_info(cluster_meta_t& res){
    NoneRequest req;
    req.set_gw_id(0);

    ClusterInfoReply reply;

    ClientContext ctx;

    Status stat = stub_->getClusterInfo(&ctx, req, &reply);

    if (stat.ok()){
        res.name = reply.name;
        res.mgrs = reply.mgrs;
        res.csds = reply.csds;
        res.size = reply.size;
        res.alloced = reply.alloced;
        res.used = reply.used;

        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
 }

 int FlameClient::shutdown_cluster(){
    NoneRequest req;
    req.set_gw_id(0);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->shutdownCluster(&ctx, req, &reply);

    if (stat.ok()){
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
 }

 int FlameClient::clean_clustrt(){
    NoneRequest req;
    req.set_gw_id(0);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->cleanCluster(&ctx, req, &reply);

    if (stat.ok()){
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
 }

 int FlameClient::pull_csd_addr(const list<uint64_t>& csd_id_list, list<csd_addr_attr_t>& res){
    CsdIDListRequest req;
    for (auto it = csd_id_list.begin(); it != csd_id_list.end(); ++it)
    {
        req.add_csd_id_list()->set_from_uint64(*it);
    }

    CsdAddrListReply reply;

    ClientContext ctx;

    Status stat = stub_->pullCsdAddr(&ctx, req, &reply);

    if (stat.ok()){
        for(uint64_t i = 0; i < reply.csd_id_list_size(); ++i)
        {
            csd_addr_attr_t item;
            item.csd_id = reply(i).csd_id();
            item.io_addr = reply(i).io_addr();
            item.stat = reply(i).stat();
            res.push_back(item);
        }
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
 }

 int FlameClient::get_vol_group_list(uint32_t offset, uint32_t limit, list<volume_group_meta_t>& res){
    VGListRequest req;
    req.set_offset(offset);
    req.set_limit(limit);

    VGListReply reply;

    ClientContext ctx;

    Status stat = stub_->getVolGroupList(&ctx, req, &reply);

    if (stat.ok()){
        for(uint64_t i = 0; i < reply.vg_list_size(); ++i)
        {
            volume_group_meta_t item;
            item.vg_id = reply(i).vg_id();
            item.name = reply(i).name();
            item.ctime = reply(i).ctime();
            item.volumes = reply(i).volumes();
            item.size = reply(i).size();
            item.alloced = reply(i).alloced();
            item.used = reply(i).used();
            res.push_back(item);
        }
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
 }

int FlameClient::create_vol_group(std::string name){
    VGCreateRequest req;
    req.set_name(name);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->createVolGroup(&ctx, req, &reply);

    if (stat.ok()){
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::remove_vol_group(std::string name){
    VGRemoveRequest req;
    req.set_name(name);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->removeVolGroup(&ctx, req, &reply);

    if (stat.ok()){
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::rename_vol_group(std::string old_name, std::string new_name){
    VGRenameRequest req;
    req.set_old_vg_name(old_name);
    req.set_new_vg_name(new_name);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->renameVolGroup(&ctx, req, &reply);

    if (stat.ok()){
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::get_volume_list(std::string name, uint32_t offset, uint32_t limit, list<volume_meta_t>& res){
    VolListRequest req;
    req.set_vg_name(name);
    req.set_offset(offset);
    req.set_limit(limit);

    VolListReply reply;

    ClientContext ctx;

    Status stat = stub_->getVolumeList(&ctx, req, &reply);

    if (stat.ok()){
        for(uint64_t i = 0; i < reply.vol_list_size(); ++i)
        {
            volume_meta_t item;
            item.vol_id = reply(i).vol_id();
            item.vg_id = reply(i).vg_id();
            item.name = reply(i).name();
            item.ctime = reply(i).ctime();
            item.chk_sz = reply(i).chk_sz();
            item.size = reply(i).size();
            item.alloced = reply(i).alloced();
            item.used = reply(i).used();
            item.flags = reply(i).flags();
            item.spolicy = reply(i).spolicy();
            item.chunks = reply(i).chunks();
            res.push_back(item);
        }
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::create_volume(volume_attr_t& vol_attr){
    VolCreateRequest req;
    req.set_vg_name(vol_attr.vg_name);
    req.set_vol_name(vol_attr.vol_name);
    req.set_chk_sz(vol_attr.chk_sz);
    req.set_size(vol_attr.size);
    req.set_flags(vol_attr.flags);
    req.set_spolicy(vol_attr.spolicy);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->createVolume(&ctx, req, &reply);

    if (stat.ok()){
        
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::remove_volume(std::string vg_name, std::string vol_name){
    VolRemoveRequest req;
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->removeVolume(&ctx, req, &reply);

    if (stat.ok()){
        
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::rename_volume(std::string vg_name, std::string old_vol_name, std::string new_vol_name){
    VolRenameRequest req;
    req.set_vg_name(vg_name);
    req.set_old_vol_name(old_vol_name);
    req.set_new_vol_name(new_vol_name);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->renameVolume(&ctx, req, &reply);

    if (stat.ok()){
        
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::get_volume_info(std::string vg_name, std::string vol_name, uint32_t retcode, volume_meta_t& res){
    VolInfoRequest req;
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);

    VolInfoReply reply;

    ClientContext ctx;

    Status stat = stub_->getVolumeInfo(&ctx, req, &reply);

    if (stat.ok()){
        retcode = reply.retcode();
        res.vol_id = reply.vol_id();
        res.vg_id = reply.vg_id();
        res.name = reply.name();
        res.ctime = reply.ctime();
        res.chk_sz = reply.chk_sz();
        res.size = reply.size();
        res.alloced = reply.alloced();
        res.used = reply.used();
        res.flags = reply.flags();
        res.spolicy = reply.spolicy();
        res.chunks = reply.chunks();

        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::resize_volume(std::string vg_name, std::string vol_name, uint64_t new_size){
    VolResizeRequest req;
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);
    req.set_new_size(new_size);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->resizeVolume(&ctx, req, &reply);

    if (stat.ok()){
        
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::open_volume(uint64_t gw_id, std::string vg_name, std::string vol_name){
    VolOpenRequest req;
    req.set_gw_id(gw_id);
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->openVolume(&ctx, req, &reply);

    if (stat.ok()){
        
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::close_volume(uint64_t gw_id, std::string vg_name, std::string vol_name){
    VolCloseRequest req;
    req.set_gw_id(gw_id);
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->closeVolume(&ctx, req, &reply);

    if (stat.ok()){
        
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::lock_volume(uint64_t gw_id, std::string vg_name, std::string vol_name){
    VolLockRequest req;
    req.set_gw_id(gw_id);
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->lockVolume(&ctx, req, &reply);

    if (stat.ok()){
        
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::unlock_volume(uint64_t gw_id, std::string vg_name, std::string vol_name){
    VolUnlockRequest req;
    req.set_gw_id(gw_id);
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);

    FlameReply reply;

    ClientContext ctx;

    Status stat = stub_->unlockVolume(&ctx, req, &reply);

    if (stat.ok()){
        
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::get_volume_maps(uint64_t vol_id, list<chunk_attr_t>& res){
    VolMapsRequest req;
    req.set_vol_id(vol_id);

    VolMapsReply reply;

    ClientContext ctx;

    Status stat = stub_->getVolumeMaps(&ctx, req, &reply);

    if (stat.ok()){
        for(uint64_t i = 0; i < reply.chk_list_size(); ++i)
        {
            chunk_attr_t item;
            item.chk_id = reply(i).chk_id();
            item.vol_id = reply(i).vol_id();
            item.index = reply(i).index();
            item.stat = reply(i).stat();
            item.spolicy = reply(i).spolicy();
            item.primary = reply(i).primary();
            item.size = reply(i).size();
            item.csd_id = reply(i).csd_id();
            item.dst_id = reply(i).dst_id();

            res.push_back(item);
        }
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClient::get_chunk_maps(list<uint64_t>& chk_list, list<chunk_attr_t>& res){
    ChunkMapsRequest req;
    for(auto it = chk_list.begin(); it != chk_list.end(); ++it)
    {
        req.add_chk_id_list()->set_from_uint64(*it);
    }

    ChunkMapsReply reply;

    ClientContext ctx;

    Status stat = stub_->getChunkMaps(&ctx, req, &reply);

    if (stat.ok()){
        for(uint64_t i = 0; i < reply.chk_list_size(); ++i)
        {
            chunk_attr_t item;
            item.chk_id = reply(i).chk_id();
            item.vol_id = reply(i).vol_id();
            item.index = reply(i).index();
            item.stat = reply(i).stat();
            item.spolicy = reply(i).spolicy();
            item.primary = reply(i).primary();
            item.size = reply(i).size();
            item.csd_id = reply(i).csd_id();
            item.dst_id = reply(i).dst_id();

            res.push_back(item);
        }
        return reply.code();
    } else {
        fct_->log()->error("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

} // namespace 