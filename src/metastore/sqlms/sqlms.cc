#include "sqlms.h"

#include "metastore/log_ms.h"
#include "orm/orm.h"
#include "util/utime.h"

#include <cassert>

using namespace std;
using namespace flame::orm;

namespace flame {

/**
 * SqlMetaStore
 */

SqlMetaStore* SqlMetaStore::create_sqlms(FlameContext* fct, const std::string& url, size_t conn_num) {
    assert(fct != nullptr);
    std::shared_ptr<orm::DBEngine> engine = orm::DBEngine::create_engine(fct, url, conn_num);
    if (!engine) {
        fct->log()->error("DBEgine create faild.");
        return nullptr;
    }

    return new SqlMetaStore(fct, engine);
}

ClusterMS* SqlMetaStore::get_cluster_ms() {
    return new SqlClusterMS(fct_, this);
}

VolumeGroupMS* SqlMetaStore::get_vg_ms() {
    return new SqlVolumeGroupMS(fct_, this);
}

VolumeMS* SqlMetaStore::get_volume_ms() {
    return new SqlVolumeMS(fct_, this);
}

ChunkMS* SqlMetaStore::get_chunk_ms() {
    return new SqlChunkMS(fct_, this);    
}

ChunkHealthMS* SqlMetaStore::get_chunk_health_ms() {
    return new SqlChunkHealthMS(fct_, this);
}

CsdMS* SqlMetaStore::get_csd_ms() {
    return new SqlCsdMS(fct_, this);
}

CsdHealthMS* SqlMetaStore::get_csd_health_ms() {
    return new SqlCsdHealthMS(fct_, this);
}

GatewayMS* SqlMetaStore::get_gw_ms() {
    return new SqlGatewayMS(fct_, this);
}

/**
 * SqlClusterMS
 */

static void cluster_meta_set__(cluster_meta_t& clt, const ClusterModel& m_cluster, const DataSet::const_iterator& it) {
    clt.name = it->get(m_cluster.name);
    clt.mgrs = it->get(m_cluster.mgrs);
    clt.csds = it->get(m_cluster.csds);
    clt.size = it->get(m_cluster.size);
    clt.alloced = it->get(m_cluster.alloced);
    clt.used = it->get(m_cluster.used);
    clt.ctime = it->get(m_cluster.ctime);
}

int SqlClusterMS::get(cluster_meta_t& cluster) {
    shared_ptr<Result> res = m_cluster.query()
        .order_by(m_cluster.clt_id, Order::DESC)
        .limit(1)
        .exec();

    if (res && res->OK()) {
        shared_ptr<DataSet> ds = res->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); it++) {
            cluster_meta_set__(cluster, m_cluster, it);
            break;
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlClusterMS::create(const cluster_meta_t& cluster) {
    shared_ptr<Result> ret = m_cluster.insert()
        .column({
            m_cluster.clt_id, 
            m_cluster.name, m_cluster.mgrs, m_cluster.csds,
            m_cluster.size, m_cluster.alloced, m_cluster.used,
            m_cluster.ctime
        }).value({
            utime_t::now().to_msec(),
            cluster.name, cluster.mgrs, cluster.csds, 
            cluster.size, cluster.alloced, cluster.used,
            cluster.ctime
        }).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlClusterMS::update(const cluster_meta_t& cluster) {
    return create(cluster);
}

/**
 * SqlVolumeGroupMS
 */

static void volume_group_meta_set__(volume_group_meta_t& vg, const VolumeGroupModel& m_vg, const DataSet::const_iterator& it) {
    vg.vg_id  = it->get(m_vg.vg_id);
    vg.name   = it->get(m_vg.name);
    vg.ctime  = it->get(m_vg.ctime);
    vg.volumes = it->get(m_vg.volumes);
    vg.size   = it->get(m_vg.size);
    vg.alloced = it->get(m_vg.alloced);
    vg.used   = it->get(m_vg.used);
}

int SqlVolumeGroupMS::list(std::list<volume_group_meta_t>& res_list, uint32_t offset, uint32_t limit) {
    shared_ptr<Result> ret = m_vg.query().offset(offset).limit(limit).exec();

    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            volume_group_meta_t item;
            volume_group_meta_set__(item, m_vg, it);
            res_list.push_back(item);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeGroupMS::get(volume_group_meta_t& res_vg, uint64_t vg_id) {
    shared_ptr<Result> ret = m_vg.query().where(m_vg.vg_id == vg_id).exec();

    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for(auto it = ds->cbegin(); it != ds->cend(); ++it) {
            volume_group_meta_set__(res_vg, m_vg, it);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeGroupMS::get(volume_group_meta_t& res_vg, const std::string& name) {
    shared_ptr<Result> ret = m_vg.query().where(m_vg.name == name).exec();

    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            volume_group_meta_set__(res_vg, m_vg, it);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeGroupMS::create(const volume_group_meta_t& new_vg) {
    shared_ptr<Result> ret = m_vg.insert()
        .column({
            m_vg.name, m_vg.ctime, m_vg.volumes, 
            m_vg.size, m_vg.alloced, m_vg.used
        }).value({
            new_vg.name, new_vg.ctime, new_vg.volumes, 
            new_vg.size, new_vg.alloced, new_vg.used
        }).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeGroupMS::create_and_get(volume_group_meta_t& new_vg) {
    shared_ptr<Result> ret = m_vg.insert()
        .column({
            m_vg.name, m_vg.ctime, m_vg.volumes, 
            m_vg.size, m_vg.alloced, m_vg.used
        }).value({
            new_vg.name, new_vg.ctime, new_vg.volumes, 
            new_vg.size, new_vg.alloced, new_vg.used
        }).exec();

    if (ret && ret->OK()) {
        if (get(new_vg, new_vg.name) == MSRetCode::SUCCESS)
            return MSRetCode::SUCCESS;
        else
            return MSRetCode::FAILD;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeGroupMS::remove(uint64_t vg_id) {
    shared_ptr<Result> ret = m_vg.remove().where(m_vg.vg_id == vg_id).exec();
    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeGroupMS::remove(const std::string& name) {
    shared_ptr<Result> ret = m_vg.remove().where(m_vg.name == name).exec();
    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeGroupMS::update(const volume_group_meta_t& vg) {
    shared_ptr<Result> ret = m_vg.update()
        .assign({
            set_(m_vg.name, vg.name), 
            set_(m_vg.ctime, vg.ctime), 
            set_(m_vg.volumes, vg.volumes),
            set_(m_vg.size, vg.size), 
            set_(m_vg.alloced, vg.alloced), 
            set_(m_vg.used, vg.used)
        }).where(m_vg.vg_id == vg.vg_id).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeGroupMS::rename(uint64_t vg_id, const std::string& new_name) {
    shared_ptr<Result> ret = m_vg.update()
        .assign(set_(m_vg.name, new_name))
        .where(m_vg.vg_id == vg_id).exec();
    
    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeGroupMS::rename(const std::string& old_name, const std::string& new_name) {
    shared_ptr<Result> ret = m_vg.update()
        .assign(set_(m_vg.name, new_name))
        .where(m_vg.name == old_name).exec();
    
    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

/**
 * SqlVolumeMS
 */

static void volume_meta_set__(volume_meta_t& vol, const VolumeModel& m_volume, const DataSet::const_iterator& it) {
    vol.vol_id = it->get(m_volume.vol_id);
    vol.vg_id  = it->get(m_volume.vg_id);
    vol.name   = it->get(m_volume.name);
    vol.ctime  = it->get(m_volume.ctime);
    vol.chk_sz = it->get(m_volume.chk_sz);
    vol.size   = it->get(m_volume.size);
    vol.alloced = it->get(m_volume.alloced);
    vol.used   = it->get(m_volume.used);
    vol.flags  = it->get(m_volume.flags);
    vol.spolicy = it->get(m_volume.spolicy);
    vol.chunks = it->get(m_volume.chunks);
}

int SqlVolumeMS::list(std::list<volume_meta_t>& res_list, uint64_t vg_id, uint32_t offset, uint32_t limit) {
    shared_ptr<Result> ret = m_volume.query().where(m_volume.vg_id == vg_id).offset(offset).limit(limit).exec();

    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            volume_meta_t item;
            volume_meta_set__(item, m_volume, it);
            res_list.push_back(item);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeMS::get(volume_meta_t& res_vol, uint64_t vol_id) {
    shared_ptr<Result> ret = m_volume.query().where(m_volume.vol_id == vol_id).exec();

    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            volume_meta_set__(res_vol, m_volume, it);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeMS::get(volume_meta_t& res_vol, uint64_t vg_id, const std::string& name) {
    shared_ptr<Result> ret = m_volume.query()
        .where({m_volume.vg_id == vg_id, m_volume.name == name}).exec();

    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            volume_meta_set__(res_vol, m_volume, it);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeMS::create(const volume_meta_t& new_vol) {
    shared_ptr<Result> ret = m_volume.insert()
        .column({
            m_volume.vg_id, m_volume.name, m_volume.ctime, 
            m_volume.chk_sz, m_volume.size, m_volume.alloced, 
            m_volume.used, m_volume.flags, m_volume.spolicy, 
            m_volume.chunks
        }).value({
            new_vol.vg_id, new_vol.name, new_vol.ctime, 
            new_vol.chk_sz, new_vol.size, new_vol.alloced, 
            new_vol.used, new_vol.flags, new_vol.spolicy, 
            new_vol.chunks
        }).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeMS::create_and_get(volume_meta_t& new_vol) {
    shared_ptr<Result> ret = m_volume.insert()
        .column({
            m_volume.vg_id, m_volume.name, m_volume.ctime, 
            m_volume.chk_sz, m_volume.size, m_volume.alloced, 
            m_volume.used, m_volume.flags, m_volume.spolicy, 
            m_volume.chunks
        }).value({
            new_vol.vg_id, new_vol.name, new_vol.ctime, 
            new_vol.chk_sz, new_vol.size, new_vol.alloced, 
            new_vol.used, new_vol.flags, new_vol.spolicy, 
            new_vol.chunks
        }).exec();

    if (ret && ret->OK()) {
        if (get(new_vol, new_vol.vg_id, new_vol.name) == MSRetCode::SUCCESS)
            return MSRetCode::SUCCESS;
        else
            return MSRetCode::FAILD;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeMS::remove(uint64_t vol_id) {
    shared_ptr<Result> ret = m_volume.remove()
        .where(m_volume.vol_id == vol_id).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeMS::remove(uint64_t vg_id, const std::string& name) {
    shared_ptr<Result> ret = m_volume.remove()
        .where({m_volume.vg_id == vg_id, m_volume.name == name}).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeMS::update(const volume_meta_t& vol) {
    shared_ptr<Result> ret = m_volume.update()
        .assign({
            set_(m_volume.name, vol.name), 
            set_(m_volume.ctime, vol.ctime),
            set_(m_volume.chk_sz, vol.chk_sz), 
            set_(m_volume.size, vol.size), 
            set_(m_volume.alloced, vol.alloced), 
            set_(m_volume.used, vol.used),
            set_(m_volume.flags, vol.flags), 
            set_(m_volume.spolicy, vol.spolicy), 
            set_(m_volume.chunks, vol.chunks)
        }).where(m_volume.vol_id == vol.vol_id).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeMS::rename(uint64_t vol_id, const std::string& new_name) {
    shared_ptr<Result> ret = m_volume.update()
        .assign(set_(m_volume.name, new_name))
        .where(m_volume.vol_id == vol_id).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlVolumeMS::rename(uint64_t vg_id, const std::string& old_name, const std::string& new_name) {
    shared_ptr<Result> ret = m_volume.update()
        .assign(set_(m_volume.name, new_name))
        .where({m_volume.vg_id == vg_id, m_volume.name == old_name})
        .exec();
    
    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

/**
 * SqlChunkMS
 */

static void chunk_meta_set__(chunk_meta_t& chk, const ChunkModel& m_chunk, const DataSet::const_iterator& it) {
    chk.chk_id     = it->get(m_chunk.chk_id);
    chk.vol_id     = it->get(m_chunk.vol_id);
    chk.index      = it->get(m_chunk.index);
    chk.stat       = it->get(m_chunk.stat);
    chk.spolicy    = it->get(m_chunk.spolicy);
    chk.ctime      = it->get(m_chunk.ctime);
    chk.primary    = it->get(m_chunk.primary);
    chk.size       = it->get(m_chunk.size);
    chk.csd_id     = it->get(m_chunk.csd_id);
    chk.csd_mtime  = it->get(m_chunk.csd_mtime);
    chk.dst_id     = it->get(m_chunk.dst_id);
    chk.dst_ctime  = it->get(m_chunk.dst_ctime);
}

int SqlChunkMS::list(std::list<chunk_meta_t>& res_list, uint64_t vol_id, uint32_t off, uint32_t len) {
    shared_ptr<Result> ret = m_chunk.query()
        .where(m_chunk.vol_id == vol_id)
        .order_by(m_chunk.index).limit(len).offset(off).exec();

    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            chunk_meta_t item;
            chunk_meta_set__(item, m_chunk, it);
            res_list.push_back(item);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlChunkMS::list(std::list<chunk_meta_t>& res_list, const std::list<uint64_t>& chk_ids) {
    shared_ptr<Result> ret = m_chunk.query()
        .where(in_(m_chunk.chk_id, chk_ids)).exec();

    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            chunk_meta_t item;
            chunk_meta_set__(item, m_chunk, it);
            res_list.push_back(item);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlChunkMS::list_cg(std::list<chunk_meta_t>& res_list, uint64_t vol_id, uint32_t index) {
    shared_ptr<Result> ret = m_chunk.query()
        .where({m_chunk.vol_id == vol_id, m_chunk.index == index}).exec();

    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            chunk_meta_t item;
            chunk_meta_set__(item, m_chunk, it);
            res_list.push_back(item);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlChunkMS::list_all(std::list<chunk_meta_t>& res_list) {
    shared_ptr<Result> ret = m_chunk.query().exec();
    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            chunk_meta_t item;
            chunk_meta_set__(item, m_chunk, it);
            res_list.push_back(item);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlChunkMS::get(chunk_meta_t& res_chk, uint64_t chk_id) {
    shared_ptr<Result> ret = m_chunk.query().where(m_chunk.chk_id == chk_id).exec();

    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            chunk_meta_set__(res_chk, m_chunk, it);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlChunkMS::create(const chunk_meta_t& new_chk) {
    shared_ptr<Result> ret = m_chunk.insert()
        .column({
            m_chunk.vol_id, m_chunk.index, m_chunk.stat, 
            m_chunk.spolicy, m_chunk.primary, m_chunk.size, 
            m_chunk.csd_id, m_chunk.csd_mtime, m_chunk.dst_id, 
            m_chunk.dst_ctime
        }).value({
            new_chk.vol_id, new_chk.index, new_chk.stat, 
            new_chk.spolicy, new_chk.primary, new_chk.size, 
            new_chk.csd_id, new_chk.csd_mtime, new_chk.dst_id, 
            new_chk.dst_ctime
        }).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

//??
int SqlChunkMS::create_and_get(chunk_meta_t& new_chk) {
    shared_ptr<Result> ret = m_chunk.insert()
        .column({
            m_chunk.vol_id, m_chunk.index, m_chunk.stat, 
            m_chunk.spolicy, m_chunk.primary, m_chunk.size, 
            m_chunk.csd_id, m_chunk.csd_mtime, m_chunk.dst_id, 
            m_chunk.dst_ctime
        }).value({
            new_chk.vol_id, new_chk.index, new_chk.stat, 
            new_chk.spolicy, new_chk.primary, new_chk.size, 
            new_chk.csd_id, new_chk.csd_mtime, new_chk.dst_id, 
            new_chk.dst_ctime
        }).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

//??
int SqlChunkMS::create_bulk(const chunk_meta_t& new_chk, uint32_t idx_start, uint32_t idx_len) {
    return MSRetCode::SUCCESS;
}

int SqlChunkMS::remove(uint64_t chk_id) {
    shared_ptr<Result> ret = m_chunk.remove().where(m_chunk.chk_id == chk_id).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlChunkMS::update(const chunk_meta_t& chk) {
    shared_ptr<Result> ret = m_chunk.update()
        .assign({
            set_(m_chunk.index, chk.index),
            set_(m_chunk.stat, chk.stat), 
            set_(m_chunk.spolicy, chk.spolicy), 
            set_(m_chunk.primary, chk.primary), 
            set_(m_chunk.size, chk.size), 
            set_(m_chunk.csd_id, chk.csd_id), 
            set_(m_chunk.csd_mtime, chk.csd_mtime), 
            set_(m_chunk.dst_id, chk.dst_id), 
            set_(m_chunk.dst_ctime, chk.dst_ctime)
        }).where(m_chunk.chk_id == chk.chk_id).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

/**
 * SqlChunkHealthMS
 */

static void chunk_health_meta_set__(chunk_health_meta_t& chk_hlt, const ChunkHealthModel& m_chk_health, const DataSet::const_iterator& it) {
    chk_hlt.chk_id  = it->get(m_chk_health.chk_id);
    chk_hlt.stat    = it->get(m_chk_health.stat);
    chk_hlt.size    =  it->get(m_chk_health.size);
    chk_hlt.used    = it->get(m_chk_health.used);       
    chk_hlt.csd_used    = it->get(m_chk_health.csd_used);  
    chk_hlt.dst_used    = it->get(m_chk_health.dst_used);  
    chk_hlt.write_count = it->get(m_chk_health.write_count);
    chk_hlt.read_count  = it->get(m_chk_health.read_count);
    chk_hlt.last_time   = it->get(m_chk_health.last_time);
    chk_hlt.last_write  = it->get(m_chk_health.last_write);
    chk_hlt.last_read   = it->get(m_chk_health.last_read);
    chk_hlt.last_latency = it->get(m_chk_health.last_latency);
    chk_hlt.last_alloc  = it->get(m_chk_health.last_alloc);
    chk_hlt.load_weight = it->get(m_chk_health.load_weight);
    chk_hlt.wear_weight = it->get(m_chk_health.wear_weight);
    chk_hlt.total_weight = it->get(m_chk_health.total_weight);
}

int SqlChunkHealthMS::get(chunk_health_meta_t& chk_hlt, uint64_t chk_id) {
    shared_ptr<Result> ret = m_chk_health.query().where(m_chk_health.chk_id == chk_id).exec();
    
    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            chunk_health_meta_set__(chk_hlt, m_chk_health, it);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlChunkHealthMS::create(const chunk_health_meta_t& chk_hlt) {
    shared_ptr<Result> ret = m_chk_health.insert()
        .column({
            m_chk_health.chk_id, m_chk_health.stat, m_chk_health.size, m_chk_health.used, 
            m_chk_health.csd_used, m_chk_health.dst_used, m_chk_health.write_count, 
            m_chk_health.read_count, m_chk_health.last_time, m_chk_health.last_write, 
            m_chk_health.last_read, m_chk_health.last_latency, m_chk_health.last_alloc, 
            m_chk_health.load_weight, m_chk_health.wear_weight, m_chk_health.total_weight
        }).value({
            chk_hlt.chk_id, chk_hlt.stat, chk_hlt.size, chk_hlt.used, 
            chk_hlt.csd_used, chk_hlt.dst_used, chk_hlt.write_count, 
            chk_hlt.read_count, chk_hlt.last_time, chk_hlt.last_write, 
            chk_hlt.last_read, chk_hlt.last_latency, chk_hlt.last_alloc, 
            chk_hlt.load_weight, chk_hlt.wear_weight, chk_hlt.total_weight
        }).exec();
    
    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

//??
int SqlChunkHealthMS::create_and_get(chunk_health_meta_t& chk_hlt) {
    return MSRetCode::SUCCESS;
}

int SqlChunkHealthMS::remove(uint64_t chk_id) {
    shared_ptr<Result> ret = m_chk_health.remove().where(m_chk_health.chk_id == chk_id).exec();
    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlChunkHealthMS::update(const chunk_health_meta_t& chk_hlt) {
    shared_ptr<Result> ret = m_chk_health.update()
        .assign({
            set_(m_chk_health.size, chk_hlt.size),
            set_(m_chk_health.stat, chk_hlt.stat),
            set_(m_chk_health.used, chk_hlt.used), 
            set_(m_chk_health.csd_used, chk_hlt.csd_used), 
            set_(m_chk_health.dst_used, chk_hlt.dst_used),
            set_(m_chk_health.write_count, chk_hlt.write_count), 
            set_(m_chk_health.read_count, chk_hlt.read_count), 
            set_(m_chk_health.last_time, chk_hlt.last_time), 
            set_(m_chk_health.last_write, chk_hlt.last_write), 
            set_(m_chk_health.last_read, chk_hlt.last_read), 
            set_(m_chk_health.last_latency, chk_hlt.last_latency),
            set_(m_chk_health.last_alloc, chk_hlt.last_alloc), 
            set_(m_chk_health.load_weight, chk_hlt.load_weight), 
            set_(m_chk_health.wear_weight, chk_hlt.wear_weight), 
            set_(m_chk_health.total_weight, chk_hlt.total_weight)
        }).where(m_chk_health.chk_id == chk_hlt.chk_id).exec();
    
    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

/**
 * SqlCsdMS
 */

static void csd_meta_set__(csd_meta_t& csd, const CsdModel& m_csd, const DataSet::const_iterator& it) {
    csd.csd_id     = it->get(m_csd.csd_id);
    csd.name       = it->get(m_csd.name);
    csd.size       = it->get(m_csd.size);
    csd.ctime      = it->get(m_csd.ctime);
    csd.io_addr    = it->get(m_csd.io_addr);
    csd.admin_addr = it->get(m_csd.admin_addr);
    csd.stat       = it->get(m_csd.stat);
    csd.latime     = it->get(m_csd.latime);
}

int SqlCsdMS::list(std::list<csd_meta_t>& res_list, const std::list<uint64_t>& csd_ids) {
    shared_ptr<Result> ret = m_csd.query().where(in_(m_csd.csd_id, csd_ids)).exec();
    
    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            csd_meta_t item;
            csd_meta_set__(item, m_csd, it);
            res_list.push_back(item);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlCsdMS::get(csd_meta_t& res_csd, uint64_t csd_id) {
    shared_ptr<Result> ret = m_csd.query().where(m_csd.csd_id == csd_id).exec();

    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            csd_meta_set__(res_csd, m_csd, it);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlCsdMS::create(const csd_meta_t& new_csd) {
    shared_ptr<Result> ret = m_csd.insert()
        .column({
            m_csd.name, m_csd.size, m_csd.ctime, 
            m_csd.io_addr,m_csd.admin_addr, m_csd.stat, 
            m_csd.latime
        }).value({
            new_csd.name, new_csd.size, new_csd.ctime, 
            new_csd.io_addr, new_csd.admin_addr, new_csd.stat, 
            new_csd.latime
        }).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlCsdMS::create_and_get(csd_meta_t& new_csd) {
    shared_ptr<Result> ret = m_csd.insert()
        .column({
            m_csd.name, m_csd.size, m_csd.ctime, 
            m_csd.io_addr, m_csd.admin_addr, m_csd.stat, 
            m_csd.latime
        }).value({
            new_csd.name, new_csd.size, new_csd.ctime, 
            new_csd.io_addr, new_csd.admin_addr, new_csd.stat, 
            new_csd.latime
        }).exec();

    if (ret && ret->OK()) {
        shared_ptr<Result> res = m_csd.query().column(m_csd.csd_id)
                                 .where(m_csd.name == new_csd.name)
                                 .exec();
        if (res && res->OK()) {
            shared_ptr<DataSet> ds = res->data_set();
            for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
                new_csd.csd_id = it->get(m_csd.csd_id);
            }
            return MSRetCode::SUCCESS;
        }
        return MSRetCode::FAILD;
    }
    return MSRetCode::FAILD;
}

int SqlCsdMS::remove(uint64_t csd_id) {
    shared_ptr<Result> ret = m_csd.remove().where(m_csd.csd_id == csd_id).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlCsdMS::update(const csd_meta_t& csd) {
    shared_ptr<Result> ret = m_csd.update()
        .assign({
            set_(m_csd.name, csd.name), 
            set_(m_csd.size, csd.size), 
            set_(m_csd.ctime, csd.ctime),
            set_(m_csd.io_addr, csd.io_addr), 
            set_(m_csd.admin_addr, csd.admin_addr), 
            set_(m_csd.stat, csd.stat), 
            set_(m_csd.latime, csd.latime)
        }).where(m_csd.csd_id == csd.csd_id).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlCsdMS::update_sm(uint64_t csd_id, uint32_t stat, uint64_t io_addr, uint64_t admin_addr) {
    auto handle = m_csd.update()
        .assign(set_(m_csd.stat, stat))
        .where(m_csd.csd_id == csd_id);

    if (io_addr & 0xffff != 0xffff) {
        handle.assign(set_(m_csd.io_addr, io_addr));
    }
    if (admin_addr & 0xffff != 0xffff) {
        handle.assign(set_(m_csd.admin_addr, admin_addr));
    }
    
    shared_ptr<Result> ret = handle.exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlCsdMS::update_at(uint64_t csd_id, uint64_t latime) {
    shared_ptr<Result> ret = m_csd.update()
        .assign(set_(m_csd.latime, latime))
        .where(m_csd.csd_id == csd_id)
        .exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

/**
 * SqlCsdHealthMS
 */

static void csd_health_meta_set__(csd_health_meta_t& csd_hlt, const CsdHealthModel& m_csd_health, const DataSet::const_iterator& it) {
    csd_hlt.csd_id      = it->get(m_csd_health.csd_id);
    csd_hlt.stat        = it->get(m_csd_health.stat);
    csd_hlt.size        = it->get(m_csd_health.size);
    csd_hlt.alloced     = it->get(m_csd_health.alloced);
    csd_hlt.used        = it->get(m_csd_health.used);
    csd_hlt.write_count = it->get(m_csd_health.write_count);
    csd_hlt.read_count  = it->get(m_csd_health.read_count);
    csd_hlt.last_time   = it->get(m_csd_health.last_time);
    csd_hlt.last_write  = it->get(m_csd_health.last_write);
    csd_hlt.last_read   = it->get(m_csd_health.last_read);
    csd_hlt.last_latency = it->get(m_csd_health.last_latency);
    csd_hlt.last_alloc  = it->get(m_csd_health.last_alloc);
    csd_hlt.load_weight = it->get(m_csd_health.load_weight);
    csd_hlt.wear_weight = it->get(m_csd_health.wear_weight);
    csd_hlt.total_weight = it->get(m_csd_health.total_weight);
}

int SqlCsdHealthMS::list_ob_tweight(std::list<csd_health_meta_t>& res_list, uint32_t limit) {
    shared_ptr<Result> ret = m_csd_health.query()
        .order_by(m_csd_health.total_weight, Order::DESC).limit(limit).exec();

    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            csd_health_meta_t csd_hlt;
            csd_health_meta_set__(csd_hlt, m_csd_health, it);
            res_list.push_back(csd_hlt);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlCsdHealthMS::get(csd_health_meta_t& csd_hlt, uint64_t csd_id) {
    shared_ptr<Result> ret = m_csd_health.query()
        .where(m_csd_health.csd_id == csd_id).exec();
    
    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            csd_health_meta_set__(csd_hlt, m_csd_health, it);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlCsdHealthMS::create(const csd_health_meta_t& new_csd) {
    shared_ptr<Result> ret = m_csd_health.insert()
        .column({
            m_csd_health.csd_id, m_csd_health.stat, m_csd_health.size, m_csd_health.alloced, 
            m_csd_health.used, m_csd_health.write_count, m_csd_health.read_count,
            m_csd_health.last_time, m_csd_health.last_write, m_csd_health.last_read, 
            m_csd_health.last_latency, m_csd_health.last_alloc, m_csd_health.load_weight, 
            m_csd_health.wear_weight, m_csd_health.total_weight
        }).value({
            new_csd.csd_id, new_csd.stat, new_csd.size, new_csd.alloced, 
            new_csd.used, new_csd.write_count, new_csd.read_count,
            new_csd.last_time, new_csd.last_write, new_csd.last_read, 
            new_csd.last_latency, new_csd.last_alloc, new_csd.load_weight, 
            new_csd.wear_weight, new_csd.total_weight
        }).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

//??
int SqlCsdHealthMS::create_and_get(csd_health_meta_t& new_csd) {
    return MSRetCode::SUCCESS;
}

int SqlCsdHealthMS::remove(uint64_t csd_id) {
    shared_ptr<Result> ret = m_csd_health.remove().where(m_csd_health.csd_id == csd_id).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlCsdHealthMS::update(const csd_health_meta_t& csd_hlt) {
    shared_ptr<Result> ret = m_csd_health.update()
        .assign({
            set_(m_csd_health.stat, csd_hlt.stat),
            set_(m_csd_health.size, csd_hlt.size), 
            set_(m_csd_health.alloced, csd_hlt.alloced), 
            set_(m_csd_health.used, csd_hlt.used),
            set_(m_csd_health.write_count, csd_hlt.write_count), 
            set_(m_csd_health.read_count, csd_hlt.read_count), 
            set_(m_csd_health.last_time, csd_hlt.last_time),
            set_(m_csd_health.last_write, csd_hlt.last_write), 
            set_(m_csd_health.last_read, csd_hlt.last_read), 
            set_(m_csd_health.last_latency, csd_hlt.last_latency),
            set_(m_csd_health.last_alloc, csd_hlt.last_alloc), 
            set_(m_csd_health.load_weight, csd_hlt.load_weight), 
            set_(m_csd_health.wear_weight, csd_hlt.wear_weight),
            set_(m_csd_health.total_weight, csd_hlt.total_weight)
        }).where(m_csd_health.csd_id == csd_hlt.csd_id).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

/**
 * SqlGatewayMS
 */

static void gateway_meta_set__(gateway_meta_t& gw, const GatewayModel& m_gw, const DataSet::const_iterator& it) {
    gw.gw_id = it->get(m_gw.gw_id);
    gw.admin_addr = it->get(m_gw.admin_addr);
    gw.ltime = it->get(m_gw.ltime);
    gw.atime = it->get(m_gw.atime);
}

int SqlGatewayMS::get(gateway_meta_t& res_gw, uint64_t gw_id) {
    shared_ptr<Result> ret = m_gw.query().where(m_gw.gw_id == gw_id).exec();
    if (ret && ret->OK()) {
        shared_ptr<DataSet> ds = ret->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); ++it) {
            gateway_meta_set__(res_gw, m_gw, it);
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlGatewayMS::create(const gateway_meta_t& gw) {
    shared_ptr<Result> ret = m_gw.insert()
        .column({
            m_gw.gw_id, m_gw.admin_addr, m_gw.ltime, m_gw.atime
        }).value({
            gw.gw_id, gw.admin_addr, gw.ltime, gw.atime
        }).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlGatewayMS::remove(uint64_t gw_id) {
    shared_ptr<Result> ret = m_gw.remove().where(m_gw.gw_id == gw_id).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
    
}

int SqlGatewayMS::update(const gateway_meta_t& gw) {
    shared_ptr<Result> ret = m_gw.update()
        .assign({
            set_(m_gw.admin_addr, gw.admin_addr), 
            set_(m_gw.ltime, gw.ltime), 
            set_(m_gw.atime, gw.atime)
        }).where(m_gw.gw_id == gw.gw_id).exec();

    if (ret && ret->OK()) {
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

} // namespace flame
