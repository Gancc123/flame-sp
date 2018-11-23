#include "sqlms.h"

#include "metastore/log_ms.h"
#include "orm/orm.h"

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

    return new SqlMetaStore(engine);
}

ClusterMS* SqlMetaStore::get_cluster_ms() {
    return new SqlClusterMS(this);
}

VolumeGroupMS* SqlMetaStore::get_vg_ms() {
    return new SqlVolumeGroupMS(this);
}

VolumeMS* SqlMetaStore::get_volume_ms() {
    return new SqlVolumeMS(this);
}

ChunkMS* SqlMetaStore::get_chunk_ms() {
    return new SqlChunkMS(this);    
}

ChunkHealthMS* SqlMetaStore::get_chunk_health_ms() {
    return new SqlChunkHealthMS(this);
}

CsdMS* SqlMetaStore::get_csd_ms() {
    return new SqlCsdMS(this);
}

CsdHealthMS* SqlMetaStore::get_csd_health_ms() {
    return new SqlCsdHealthMS(this);
}

GatewayMS* SqlMetaStore::get_gw_ms() {
    return new SqlGatewayMS(this);
}

/**
 * SqlClusterMS
 */

int SqlClusterMS::get(cluster_meta_t& cluster) {
    shared_ptr<Result> = m_cluster.query()
        .order_by(m_cluster.clt_id, Order::DESC)
        .limit(1)
        .exec();
    if (res && res->OK()) {
        shared_ptr<DataSet> ds = res->data_set();
        for (auto it = ds->cbegin(); it != ds->cend(); it++) {
            cluster.name = it->get(m_cluster.name);
            cluster.mgrs = it->get(m_cluster.mgrs);
            cluster.csds = it->get(m_cluster.csds);
            cluster.size = it->get(m_cluster.size);
            cluster.alloced = it->get(m_cluster.alloced);
            cluster.used = it->get(m_cluster.used);
            break;
        }
        return MSRetCode::SUCCESS;
    }
    return MSRetCode::FAILD;
}

int SqlClusterMS::create(const cluster_meta_t& cluster) {
    return MSRetCode::SUCCESS;
}

int SqlClusterMS::update(const cluster_meta_t& cluster) {
    return MSRetCode::SUCCESS;
}

/**
 * SqlVolumeGroupMS
 */

int SqlVolumeGroupMS::list(std::list<volume_group_meta_t>& res_list) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeGroupMS::get(volume_group_meta_t& res_vg, uint64_t vg_id) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeGroupMS::get(volume_group_meta_t& res_vg, const std::string& name) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeGroupMS::create(const volume_group_meta_t& new_vg) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeGroupMS::create_and_get(volume_group_meta_t& new_vg) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeGroupMS::remove(uint64_t vg_id) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeGroupMS::remove(const std::string& name) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeGroupMS::update(const volume_group_meta_t& vg) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeGroupMS::rename(uint64_t vg_id, const std::string& new_name) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeGroupMS::rename(const std::string& old_name, const std::string& new_name) {
    return MSRetCode::SUCCESS;
}

/**
 * SqlVolumeMS
 */

int SqlVolumeMS::list(std::list<volume_meta_t>& res_list, uint64_t vg_id) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeMS::get(volume_meta_t& res_vol, uint64_t vol_id) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeMS::get(volume_meta_t& res_vol, uint64_t vg_id, const std::string& name) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeMS::create(const volume_meta_t& new_vol) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeMS::create_and_get(volume_meta_t& new_vol) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeMS::remove(uint64_t vol_id) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeMS::remove(uint64_t vg_id, const std::string& name) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeMS::update(const volume_meta_t& vol) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeMS::rename(uint64_t vol_id, const std::string& new_name) {
    return MSRetCode::SUCCESS;
}

int SqlVolumeMS::rename(uint64_t vg_id, const std::string& old_name, const std::string& new_name) {
    return MSRetCode::SUCCESS;
}

/**
 * SqlChunkMS
 */

int SqlChunkMS::list(std::list<chunk_meta_t>& res_list, uint64_t vol_id, uint32_t off, uint32_t len) {
    return MSRetCode::SUCCESS;
}

int SqlChunkMS::list(std::list<chunk_meta_t>& res_list, const std::list<uint64_t>& chk_ids) {
    return MSRetCode::SUCCESS;
}

int SqlChunkMS::list_all(std::list<chunk_meta_t>& res_list) {
    return MSRetCode::SUCCESS;
}

int SqlChunkMS::get(chunk_meta_t& res_chk, uint64_t chk_id) {
    return MSRetCode::SUCCESS;
}

int SqlChunkMS::create(const chunk_meta_t& new_chk) {
    return MSRetCode::SUCCESS;
}

int SqlChunkMS::create_and_get(volume_meta_t& new_chk) {
    return MSRetCode::SUCCESS;
}

int SqlChunkMS::create_bulk(const chunk_meta_t& new_chk, uint32_t idx_start, uint32_t idx_len) {
    return MSRetCode::SUCCESS;
}

int SqlChunkMS::remove(uint64_t chk_id) {
    return MSRetCode::SUCCESS;
}

int SqlChunkMS::update(const chunk_meta_t& chk) {
    return MSRetCode::SUCCESS;
}

/**
 * SqlChunkHealthMS
 */

int SqlChunkHealthMS::get(chunk_health_meta_t& chk_hlt, uint64_t chk_id) {
    return MSRetCode::SUCCESS;
}

int SqlChunkHealthMS::create(const chunk_health_meta_t& chk_hlt) {
    return MSRetCode::SUCCESS;
}

int SqlChunkHealthMS::create_and_get(chunk_health_meta_t& chk_hlt) {
    return MSRetCode::SUCCESS;
}

int SqlChunkHealthMS::remove(uint64_t chk_id) {
    return MSRetCode::SUCCESS;
}

int SqlChunkHealthMS::update(const chunk_health_meta_t& chk_hlt) {
    return MSRetCode::SUCCESS;
}

/**
 * SqlCsdMS
 */

int SqlCsdMS::list(std::list<csd_meta_t>& res_list, const std::list<uint64_t>& csd_ids) {
    return MSRetCode::SUCCESS;
}

int SqlCsdMS::get(csd_meta_t& res_csd, uint64_t csd_id) {
    return MSRetCode::SUCCESS;
}

int SqlCsdMS::create(const csd_meta_t& new_csd) {
    return MSRetCode::SUCCESS;
}

int SqlCsdMS::create_and_get(csd_meta_t& new_csd) {
    return MSRetCode::SUCCESS;
}

int SqlCsdMS::remove(uint64_t csd_id) {
    return MSRetCode::SUCCESS;
}

int SqlCsdMS::update(const csd_meta_t& csd) {
    return MSRetCode::SUCCESS;
}

int SqlCsdMS::update_sm(uint64_t csd_id, uint32_t stat, uint64_t io_addr, uint64_t admin_addr) {
    return MSRetCode::SUCCESS;
}

int SqlCsdMS::update_at(uint64_t csd_id, uint64_t latime) {
    return MSRetCode::SUCCESS;
}

/**
 * SqlCsdHealthMS
 */

int SqlCsdHealthMS::list_ob_tweight(std::list<csd_health_meta_t>& res_list, uint32_t limit) {
    return MSRetCode::SUCCESS;
}

int SqlCsdHealthMS::get(csd_health_meta_t& csd_hlt, uint64_t csd_id) {
    return MSRetCode::SUCCESS;
}

int SqlCsdHealthMS::create(const csd_health_meta_t& new_csd) {
    return MSRetCode::SUCCESS;
}

int SqlCsdHealthMS::create_and_get(csd_health_meta_t& new_csd) {
    return MSRetCode::SUCCESS;
}

int SqlCsdHealthMS::remove(uint64_t csd_id) {
    return MSRetCode::SUCCESS;
}

int SqlCsdHealthMS::update(const csd_health_meta_t& csd_hlt) {
    return MSRetCode::SUCCESS;
}

/**
 * SqlGatewayMS
 */

int SqlGatewayMS::get(gateway_meta_t& res_gw, uint64_t gw_id) {
    return MSRetCode::SUCCESS;
}

int SqlGatewayMS::create(const gateway_meta_t& gw) {
    return MSRetCode::SUCCESS;
}

int SqlGatewayMS::remove(uint64_t gw_id) {
    return MSRetCode::SUCCESS;
}

int SqlGatewayMS::update(const gateway_meta_t& gw) {
    return MSRetCode::SUCCESS;
}

} // namespace flame