#ifndef FLAME_METASTORE_SQLMS_SQLMS_H
#define FLAME_METASTORE_SQLMS_SQLMS_H

#include "common/context.h"
#include "metastore/metastore.h"
#include "orm/orm.h"
#include "models.h"

#include <memory>

namespace flame {

class SqlMetaStore final : public MetaStore {
public:
    static SqlMetaStore* create_sqlms(FlameContext* fct, const std::string& url, size_t conn_num = DBE_DEF_CONNNUM);

    ~SqlMetaStore() {}

    /**
     * MetaStore Interface 
     */
    virtual ClusterMS* get_cluster_ms() override;
    virtual VolumeGroupMS* get_vg_ms() override;
    virtual VolumeMS* get_volume_ms() override;
    virtual ChunkMS* get_chunk_ms() override;
    virtual ChunkHealthMS* get_chunk_health_ms() override;
    virtual CsdMS* get_csd_ms() override;
    virtual CsdHealthMS* get_csd_health_ms() override;
    virtual GatewayMS* get_gw_ms() override;

    SqlMetaStore(const SqlMetaStore&) = delete;
    SqlMetaStore(SqlMetaStore&&) = delete;
    SqlMetaStore& operator = (const SqlMetaStore&) = delete;
    SqlMetaStore& operator = (SqlMetaStore&&) = delete;

    friend class SqlClusterMS;
    friend class SqlVolumeGroupMS;
    friend class SqlVolumeMS;
    friend class SqlChunkMS;
    friend class SqlChunkHealthMS;
    friend class SqlCsdMS;
    friend class SqlCsdHealthMS;
    friend class SqlGatewayMS;

private:
    SqlMetaStore(const std::shared_ptr<orm::DBEngine>& db) : db_(db) {}

    std::shared_ptr<orm::DBEngine> db_;

    /**
     * Models
     */
    ClusterModel        m_cluster   {db_};
    VolumeGroupModel    m_vg        {db_};
    VolumeModel         m_volume    {db_};
    ChunkModel          m_chunk     {db_};
    ChunkHealthModel    m_chk_health{db_};
    CsdModel            m_csd       {db_};
    CsdHealthModel      m_csd_health{db_};
    GatewayModel        m_gw        {db_};
}; // class SqlMetaStore

class SqlClusterMS final : public ClusterMS {
public:
    /**
     * Get Cluster Meta
     */
    virtual int get(cluster_meta_t& cluster) override;

    /**
     * Create Cluster Meta
     */
    virtual int create(const cluster_meta_t& cluster) override;

    /**
     * Update Cluster Meta
     */
    virtual int update(const cluster_meta_t& cluster) override;

    friend class SqlMetaStore;
private:
    SqlClusterMS(SqlMetaStore* ms) : ms_(ms) {}

    SqlMetaStore* ms_;
}; // class SqlClusterMS

class SqlVolumeGroupMS final : public VolumeGroupMS {
public:
    /**
     * Get all of the volume group
     */
    virtual int list(std::list<volume_group_meta_t>& res_list) override;

    /**
     * Get a single volume group
     * @By: vg_id or name
     */
    virtual int get(volume_group_meta_t& res_vg, uint64_t vg_id) override;
    virtual int get(volume_group_meta_t& res_vg, const std::string& name) override;

    /**
     * Create a single volume group
     * @Note: create_and_get would update vg_id iff create successfully.
     */
    virtual int create(const volume_group_meta_t& new_vg) override;
    virtual int create_and_get(volume_group_meta_t& new_vg) override;

    /**
     * Remove a single volume group
     */
    virtual int remove(uint64_t vg_id) override;
    virtual int remove(const std::string& name) override;

    /**
     * Update a single volume group
     */
    virtual int update(const volume_group_meta_t& vg) override;

    /**
     * Rename a single volume group
     */
    virtual int rename(uint64_t vg_id, const std::string& new_name) override;
    virtual int rename(const std::string& old_name, const std::string& new_name) override;

    friend class SqlMetaStore;
private:
    SqlVolumeGroupMS(SqlMetaStore* ms) : ms_(ms) {}

    SqlMetaStore* ms_;
}; // class SqlVolumeGroupMS

class SqlVolumeMS final : public VolumeMS {
public:
    /**
     * Get all volume information in same volume group
     * @By: vg_id
     */
    virtual int list(std::list<volume_meta_t>& res_list, uint64_t vg_id) override;

    /**
     * Get a single volume
     * @By: vol_id or <vg_id, name>
     */
    virtual int get(volume_meta_t& res_vol, uint64_t vol_id) override;
    virtual int get(volume_meta_t& res_vol, uint64_t vg_id, const std::string& name) override;

    /**
     * Create a single volume
     * @Note: create_and_get would update vol_id iff create successfully.
     */
    virtual int create(const volume_meta_t& new_vol) override;
    virtual int create_and_get(volume_meta_t& new_vol) override;

    /**
     * Remove a single volume
     * @By: vol_id or <vg_id, name>
     */
    virtual int remove(uint64_t vol_id) override;
    virtual int remove(uint64_t vg_id, const std::string& name) override;

    /**
     * Update a single volume
     */
    virtual int update(const volume_meta_t& vol) override;

    /**
     * Rename a single volume
     */
    virtual int rename(uint64_t vol_id, const std::string& new_name) override;
    virtual int rename(uint64_t vg_id, const std::string& old_name, const std::string& new_name) override;

    friend class SqlMetaStore;
private:
    SqlVolumeMS(SqlMetaStore* ms) : ms_(ms) {}

    SqlMetaStore* ms_;
}; // class SqlVolumeMS


class SqlChunkMS final : public ChunkMS {
public:
    /**
     * Get all chunk information in same volume
     * @By: vol_id
     * @Note: It means that getting all iff len == 0
     */
    virtual int list(std::list<chunk_meta_t>& res_list, uint64_t vol_id, uint32_t off = 0, uint32_t len = 0) override;
    virtual int list(std::list<chunk_meta_t>& res_list, const std::list<uint64_t>& chk_ids) override;
    virtual int list_all(std::list<chunk_meta_t>& res_list) override;

    /**
     * Get a single chunk
     * @By: chk_id
     */
    virtual int get(chunk_meta_t& res_chk, uint64_t chk_id) override;

    /**
     * Create chunks
     * @Note: create_and_get would update chk_id iff create successfully.
     */
    virtual int create(const chunk_meta_t& new_chk) override;
    virtual int create_and_get(volume_meta_t& new_chk) override;
    // create chunks with same parameter except index.
    virtual int create_bulk(const chunk_meta_t& new_chk, uint32_t idx_start, uint32_t idx_len) override;

    /**
     * Remove a single chunk
     * @By: chk_id
     */
    virtual int remove(uint64_t chk_id) override;

    /**
     * Update a single chunk
     */
    virtual int update(const chunk_meta_t& chk) override;

    friend class SqlMetaStore;
private:
    SqlChunkMS(SqlMetaStore* ms) : ms_(ms) {}

    SqlMetaStore* ms_;
}; // class SqlChunkMS

class SqlChunkHealthMS final : public ChunkHealthMS {
public:
    /**
     * Get a single chunk health
     * @By: chk_id
     */
    virtual int get(chunk_health_meta_t& chk_hlt, uint64_t chk_id) override;

    /**
     * Create a single chunk health
     * @Note: create_and_get would update chk_id iff create successfully.
     */
    virtual int create(const chunk_health_meta_t& chk_hlt) override;
    virtual int create_and_get(chunk_health_meta_t& chk_hlt) override;

    /**
     * Remove a single chunk health
     */
    virtual int remove(uint64_t chk_id) override;

    /**
     * Update a single chunk health
     */
    virtual int update(const chunk_health_meta_t& chk_hlt) override;

    friend class SqlMetaStore;
private:
    SqlChunkHealthMS(SqlMetaStore* ms) : ms_(ms) {}

    SqlMetaStore* ms_;
}; // class SqlChunkHealth

class SqlCsdMS final : public CsdMS {
public:
    /**
     * Get CSDs for given csd_ids
     */
    virtual int list(std::list<csd_meta_t>& res_list, const std::list<uint64_t>& csd_ids) override;

    /**
     * Get a single CSD
     */
    virtual int get(csd_meta_t& res_csd, uint64_t csd_id) override;

    /**
     * Create a single CSD
     */
    virtual int create(const csd_meta_t& new_csd) override;
    virtual int create_and_get(csd_meta_t& new_csd) override;

    /**
     * Remove a single CSD
     */
    virtual int remove(uint64_t csd_id) override;

    /**
     * Update a single CSD
     */
    virtual int update(const csd_meta_t& csd) override;
    virtual int update_sm(uint64_t csd_id, uint32_t stat, uint64_t io_addr = 0, uint64_t admin_addr = 0) override;
    virtual int update_at(uint64_t csd_id, uint64_t latime) override;

    friend class SqlMetaStore;
private:
    SqlCsdMS(SqlMetaStore* ms) : ms_(ms) {}

    SqlMetaStore* ms_;
}; // class SqlCsdMS

class SqlCsdHealthMS final : public CsdHealthMS {
public:
    /**
     * List CSD Health
     * 
     * @Note: 'ob' == 'order by'; 'tweight' == 'total_weight'
     */
    virtual int list_ob_tweight(std::list<csd_health_meta_t>& res_list, uint32_t limit) override;

    /**
     * Get a single CSD Health
     */
    virtual int get(csd_health_meta_t& csd_hlt, uint64_t csd_id) override;

    /**
     * Create a single CSD Health
     */
    virtual int create(const csd_health_meta_t& new_csd) override;
    virtual int create_and_get(csd_health_meta_t& new_csd) override;

    /**
     * Remove a single CSD Health
     */
    virtual int remove(uint64_t csd_id) override;

    /**
     * Update a single CSD Health
     */
    virtual int update(const csd_health_meta_t& csd_hlt) override;

    friend class SqlMetaStore;
private:
    SqlCsdHealthMS(SqlMetaStore* ms) : ms_(ms) {}

    SqlMetaStore* ms_;
}; // class SqlCsdHealthMS

class SqlGatewayMS final : public GatewayMS {
public:
    /**
     * Get a single Gateway
     */
    virtual int get(gateway_meta_t& res_gw, uint64_t gw_id) override;

    /**
     * Create a single Gateway
     */
    virtual int create(const gateway_meta_t& gw) override;

    /**
     * Remove a single Gateway
     */
    virtual int remove(uint64_t gw_id) override;

    /**
     * Update a single Gateway
     */
    virtual int update(const gateway_meta_t& gw) override;

    friend class SqlMetaStore;
private:
    SqlGatewayMS(SqlMetaStore* ms) : ms_(ms) {}

    SqlMetaStore* ms_;
}; // class SqlGatewayMS

} // namespace flame

#endif // FLAME_METASTORE_SQLMS_SQLMS_H