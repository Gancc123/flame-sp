#include "flame_client.h"
#include "proto/flame.pb.h"

#include "log_service.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace flame {

int FlameClientImpl::connect(uint64_t gw_id, uint64_t admin_addr) {
    ConnectRequest req;
    req.set_gw_id(gw_id);
    req.set_admin_addr(admin_addr);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->connect(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code(); // 注意负号
    }
}

int FlameClientImpl::disconnect(uint64_t gw_id) {
    DisconnectRequest req;
    req.set_gw_id(gw_id);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->disconnect(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

 int FlameClientImpl::get_cluster_info(cluster_meta_t& res) {
    NoneRequest req;
    req.set_gw_id(0);

    ClusterInfoReply reply;
    ClientContext ctx;
    Status stat = stub_->getClusterInfo(&ctx, req, &reply);

    if (stat.ok()) {
        res.name = reply.name();
        res.mgrs = reply.mgrs();
        res.csds = reply.csds();
        res.size = reply.size();
        res.alloced = reply.alloced();
        res.used = reply.used();
        return 0;
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
 }

 int FlameClientImpl::shutdown_cluster() {
    NoneRequest req;
    req.set_gw_id(0);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->shutdownCluster(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
 }

 int FlameClientImpl::clean_clustrt() {
    NoneRequest req;
    req.set_gw_id(0);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->cleanCluster(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
 }

 int FlameClientImpl::pull_csd_addr(std::list<csd_addr_t>& res, const std::list<uint64_t>& csd_id_list) {
    CsdIDListRequest req;
    for (auto it = csd_id_list.begin(); it != csd_id_list.end(); ++it) {
        req.add_csd_id_list(*it);
    }

    CsdAddrListReply reply;
    ClientContext ctx;
    Status stat = stub_->pullCsdAddr(&ctx, req, &reply);

    if (stat.ok()) {
        for (int i = 0; i < reply.csd_list_size(); ++i) {
            csd_addr_t item;
            item.csd_id = reply.csd_list(i).csd_id();
            item.io_addr = reply.csd_list(i).io_addr();
            item.stat = reply.csd_list(i).stat();
            res.push_back(item);
        }
        return 0;
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
 }

 int FlameClientImpl::get_vol_group_list(std::list<volume_group_meta_t>& res, uint32_t offset, uint32_t limit) {
    VGListRequest req;
    req.set_offset(offset);
    req.set_limit(limit);

    VGListReply reply;
    ClientContext ctx;
    Status stat = stub_->getVolGroupList(&ctx, req, &reply);

    if (stat.ok()) {
        for (uint64_t i = 0; i < reply.vg_list_size(); ++i) {
            volume_group_meta_t item;
            item.vg_id = reply.vg_list(i).vg_id();
            item.name = reply.vg_list(i).name();
            item.ctime = reply.vg_list(i).ctime();
            item.volumes = reply.vg_list(i).volumes();
            item.size = reply.vg_list(i).size();
            item.alloced = reply.vg_list(i).alloced();
            item.used = reply.vg_list(i).used();
            res.push_back(item);
        }
        return 0;
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
 }

int FlameClientImpl::create_vol_group(const std::string& name) {
    VGCreateRequest req;
    req.set_vg_name(name);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->createVolGroup(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::remove_vol_group(const std::string& name) {
    VGRemoveRequest req;
    req.set_vg_name(name);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->removeVolGroup(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::rename_vol_group(const std::string& old_name, const std::string& new_name) {
    VGRenameRequest req;
    req.set_old_vg_name(old_name);
    req.set_new_vg_name(new_name);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->renameVolGroup(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::get_volume_list(std::list<volume_meta_t>& res, const std::string& name, uint32_t offset, uint32_t limit) {
    VolListRequest req;
    req.set_vg_name(name);
    req.set_offset(offset);
    req.set_limit(limit);

    VolListReply reply;
    ClientContext ctx;
    Status stat = stub_->getVolumeList(&ctx, req, &reply);

    if (stat.ok()) {
        for (uint64_t i = 0; i < reply.vol_list_size(); ++i) {
            volume_meta_t item;
            item.vol_id = reply.vol_list(i).vol_id();
            item.vg_id = reply.vol_list(i).vg_id();
            item.name = reply.vol_list(i).name();
            item.ctime = reply.vol_list(i).ctime();
            item.chk_sz = reply.vol_list(i).chk_sz();
            item.size = reply.vol_list(i).size();
            item.alloced = reply.vol_list(i).alloced();
            item.used = reply.vol_list(i).used();
            item.flags = reply.vol_list(i).flags();
            item.spolicy = reply.vol_list(i).spolicy();
            item.chunks = reply.vol_list(i).chunks();
            res.push_back(item);
        }
        return 0;
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::create_volume(const std::string& vg_name, const std::string& vol_name, const vol_attr_t& attr) {
    VolCreateRequest req;
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);
    req.set_chk_sz(attr.chk_sz);
    req.set_size(attr.size);
    req.set_flags(attr.flags);
    req.set_spolicy(attr.spolicy);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->createVolume(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::remove_volume(const std::string& vg_name, const std::string& vol_name) {
    VolRemoveRequest req;
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->removeVolume(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::rename_volume(const std::string& vg_name, const std::string& old_vol_name, const std::string& new_vol_name) {
    VolRenameRequest req;
    req.set_vg_name(vg_name);
    req.set_old_vol_name(old_vol_name);
    req.set_new_vol_name(new_vol_name);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->renameVolume(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::get_volume_info(volume_meta_t& res, const std::string& vg_name, const std::string& vol_name, uint32_t retcode) {
    VolInfoRequest req;
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);

    VolInfoReply reply;
    ClientContext ctx;
    Status stat = stub_->getVolumeInfo(&ctx, req, &reply);

    if (stat.ok()) {
        retcode = reply.retcode();
        if (retcode != 0)
            return retcode;
        res.vol_id = reply.vol().vol_id();
        res.vg_id = reply.vol().vg_id();
        res.name = reply.vol().name();
        res.ctime = reply.vol().ctime();
        res.chk_sz = reply.vol().chk_sz();
        res.size = reply.vol().size();
        res.alloced = reply.vol().alloced();
        res.used = reply.vol().used();
        res.flags = reply.vol().flags();
        res.spolicy = reply.vol().spolicy();
        res.chunks = reply.vol().chunks();
        return 0;
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::resize_volume(const std::string& vg_name, const std::string& vol_name, uint64_t new_size) {
    VolResizeRequest req;
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);
    req.set_new_size(new_size);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->resizeVolume(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::open_volume(uint64_t gw_id, const std::string& vg_name, const std::string& vol_name) {
    VolOpenRequest req;
    req.set_gw_id(gw_id);
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->openVolume(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::close_volume(uint64_t gw_id, const std::string& vg_name, const std::string& vol_name) {
    VolCloseRequest req;
    req.set_gw_id(gw_id);
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->closeVolume(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::lock_volume(uint64_t gw_id, const std::string& vg_name, const std::string& vol_name) {
    VolLockRequest req;
    req.set_gw_id(gw_id);
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->lockVolume(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::unlock_volume(uint64_t gw_id, const std::string& vg_name, const std::string& vol_name) {
    VolUnlockRequest req;
    req.set_gw_id(gw_id);
    req.set_vg_name(vg_name);
    req.set_vol_name(vol_name);

    FlameReply reply;
    ClientContext ctx;
    Status stat = stub_->unlockVolume(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::get_volume_maps(std::list<chunk_attr_t>& res, uint64_t vol_id) {
    VolMapsRequest req;
    req.set_vol_id(vol_id);

    VolMapsReply reply;
    ClientContext ctx;
    Status stat = stub_->getVolumeMaps(&ctx, req, &reply);

    if (stat.ok()) {
        for (uint64_t i = 0; i < reply.chk_list_size(); ++i) {
            chunk_attr_t item;
            item.chk_id = reply.chk_list(i).chk_id();
            item.vol_id = reply.chk_list(i).vol_id();
            item.index = reply.chk_list(i).index();
            item.stat = reply.chk_list(i).stat();
            item.spolicy = reply.chk_list(i).spolicy();
            item.primary = reply.chk_list(i).primary();
            item.size = reply.chk_list(i).size();
            item.csd_id = reply.chk_list(i).csd_id();
            item.dst_id = reply.chk_list(i).dst_id();

            res.push_back(item);
        }
        return 0;
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int FlameClientImpl::get_chunk_maps(std::list<chunk_attr_t>& res, const std::list<uint64_t>& chk_list) {
    ChunkMapsRequest req;
    for (auto it = chk_list.begin(); it != chk_list.end(); ++it) {
        req.add_chk_id_list(*it);
    }

    ChunkMapsReply reply;
    ClientContext ctx;
    Status stat = stub_->getChunkMaps(&ctx, req, &reply);

    if (stat.ok()) {
        for (uint64_t i = 0; i < reply.chk_list_size(); ++i) {
            chunk_attr_t item;
            item.chk_id = reply.chk_list(i).chk_id();
            item.vol_id = reply.chk_list(i).vol_id();
            item.index = reply.chk_list(i).index();
            item.stat = reply.chk_list(i).stat();
            item.spolicy = reply.chk_list(i).spolicy();
            item.primary = reply.chk_list(i).primary();
            item.size = reply.chk_list(i).size();
            item.csd_id = reply.chk_list(i).csd_id();
            item.dst_id = reply.chk_list(i).dst_id();

            res.push_back(item);
        }
        return 0;
    } else {
        fct_->log()->lerror("RPC Failed(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

} // namespace flame
