/**
 * Interface Set for MGR MetaStore
 * @version: 0.1.0
 * 
 * @Note: Return of the function/method is MSRetCode that shows the status of the call.
 * @Note: The first parameter would be the result iff the function/method have.
 */
#ifndef FLAME_METASTORE_METASTORE_H
#define FLAME_METASTORE_METASTORE_H

#include <cstdint>
#include <string>
#include <list>

namespace flame {

enum MSRetCode {
    SUCCESS = 0,
    FAILD = 1,
    OBJ_EXIST = 2,
    OBJ_NOT_EXIST = 3
};

/**
 * Interface for Cluster Meta Store
 */
struct cluster_meta_t {
    std::string name    {};
    uint32_t    mgrs    {0};
    uint32_t    csds    {0};
    uint64_t    size    {0};
    uint64_t    alloced {0};
    uint64_t    used    {0};
}; // struct cluster_meta_t

class ClusterMS {
public:
    virtual ~ClusterMS() {}

    /**
     * Get Cluster Meta
     */
    virtual int get(cluster_meta_t& cluster) = 0;

    /**
     * Create Cluster Meta
     */
    virtual int create(const cluster_meta_t& cluster) = 0;

    /**
     * Update Cluster Meta
     */
    virtual int update(const cluster_meta_t& cluster) = 0;

protected:
    ClusterMS(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class ClusterMS

/**
 * Interface for Group Meta Store
 */
struct volume_group_meta_t {
    uint64_t    vg_id   {0};
    std::string name    {};
    uint64_t    ctime   {0};    // (ms)
    uint32_t    volumes {0};
    uint64_t    size    {0};    // total visible size
    uint64_t    alloced {0};    // total logical size
    uint64_t    used    {0};    // total physical size
}; // struct group_mate_t

class VolumeGroupMS {
public:
    virtual ~VolumeGroupMS() {}

    /**
     * Get all of the volume group
     */
    virtual int list(std::list<volume_group_meta_t>& res_list, uint32_t offset = 0, uint32_t limit = 0) = 0;

    /**
     * Get a single volume group
     * @By: vg_id or name
     */
    virtual int get(volume_group_meta_t& res_vg, uint64_t vg_id) = 0;
    virtual int get(volume_group_meta_t& res_vg, const std::string& name) = 0;

    /**
     * Create a single volume group
     * @Note: create_and_get would update vg_id iff create successfully.
     */
    virtual int create(const volume_group_meta_t& new_vg) = 0;
    virtual int create_and_get(volume_group_meta_t& new_vg) = 0;

    /**
     * Remove a single volume group
     */
    virtual int remove(uint64_t vg_id) = 0;
    virtual int remove(const std::string& name) = 0;

    /**
     * Update a single volume group
     */
    virtual int update(const volume_group_meta_t& vg) = 0;

    /**
     * Rename a single volume group
     */
    virtual int rename(uint64_t vg_id, const std::string& new_name) = 0;
    virtual int rename(const std::string& old_name, const std::string& new_name) = 0;

protected:
    VolumeGroupMS(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class GroupMS

/**
 * Interface for Volume Meta Store
 */
struct volume_meta_t {
    uint64_t    vol_id  {0};
    uint64_t    vg_id   {0};
    std::string name    {};
    uint64_t    ctime   {0};    // (ms)
    uint64_t    chk_sz  {0};    // chunk size
    uint64_t    size    {0};
    uint64_t    alloced {0};
    uint64_t    used    {0};
    uint32_t    flags   {0};
    uint32_t    spolicy {0};    // store policy
    uint32_t    chunks  {0};    // chunk number
}; // struct volume_meta_t

class VolumeMS {
public:
    virtual ~VolumeMS() {}

    /**
     * Get all volume information in same volume group
     * @By: vg_id
     */
    virtual int list(std::list<volume_meta_t>& res_list, uint64_t vg_id, uint32_t offset = 0, uint32_t limit = 0) = 0;

    /**
     * Get a single volume
     * @By: vol_id or <vg_id, name>
     */
    virtual int get(volume_meta_t& res_vol, uint64_t vol_id) = 0;
    virtual int get(volume_meta_t& res_vol, uint64_t vg_id, const std::string& name) = 0;

    /**
     * Create a single volume
     * @Note: create_and_get would update vol_id iff create successfully.
     */
    virtual int create(const volume_meta_t& new_vol) = 0;
    virtual int create_and_get(volume_meta_t& new_vol) = 0;

    /**
     * Remove a single volume
     * @By: vol_id or <vg_id, name>
     */
    virtual int remove(uint64_t vol_id) = 0;
    virtual int remove(uint64_t vg_id, const std::string& name) = 0;

    /**
     * Update a single volume
     */
    virtual int update(const volume_meta_t& vol) = 0;

    /**
     * Rename a single volume
     */
    virtual int rename(uint64_t vol_id, const std::string& new_name) = 0;
    virtual int rename(uint64_t vg_id, const std::string& old_name, const std::string& new_name) = 0;

protected:
    VolumeMS(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class VolumeMS

/**
 * Interface for Chunk Meta Store
 */
struct chunk_meta_t {
    uint64_t    chk_id  {0};
    uint64_t    vol_id  {0};
    uint32_t    index   {0};    // Thera is a same index in a chunk group
    uint32_t    stat    {0};    // chunk status
    uint32_t    spolicy {0};
    uint64_t    primary {0};    // this chunk is the primary chunk when primary == chk_id
    uint64_t    size    {0};
    uint64_t    csd_id  {0};
    uint64_t    csd_mtime {0};  // csd map time  
    uint64_t    dst_id  {0};
    uint64_t    dst_ctime {0};  // dst start time

    uint64_t key_id() const { return chk_id >> 4; }
    void key_id(uint64_t v) { chk_id = (chk_id & 0xFULL) | (v << 4); }
    uint32_t sub_id() const { return chk_id & 0xFULL; }
    void sub_id(uint32_t v) { chk_id = (chk_id & ~0xFULL) | (v & 0xF); }
}; // struct chunk_meta_t

class ChunkMS {
public:
    virtual ~ChunkMS() {}

    /**
     * Get all chunk information in same volume
     * @By: vol_id
     * @Note: It means that getting all iff len == 0
     */
    virtual int list(std::list<chunk_meta_t>& res_list, uint64_t vol_id, uint32_t off = 0, uint32_t len = 0) = 0;
    virtual int list(std::list<chunk_meta_t>& res_list, const std::list<uint64_t>& chk_ids) = 0;
    virtual int list_all(std::list<chunk_meta_t>& res_list) = 0;

    /**
     * Get a single chunk
     * @By: chk_id
     */
    virtual int get(chunk_meta_t& res_chk, uint64_t chk_id) = 0;

    /**
     * Create chunks
     * @Note: create_and_get would update chk_id iff create successfully.
     */
    virtual int create(const chunk_meta_t& new_chk) = 0;
    virtual int create_and_get(chunk_meta_t& new_chk) = 0;
    // create chunks with same parameter except index.
    virtual int create_bulk(const chunk_meta_t& new_chk, uint32_t idx_start, uint32_t idx_len) = 0;

    /**
     * Remove a single chunk
     * @By: chk_id
     */
    virtual int remove(uint64_t chk_id) = 0;

    /**
     * Update a single chunk
     */
    virtual int update(const chunk_meta_t& chk) = 0;

protected:
    ChunkMS(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class ChunkMS

/**
 * Interface for Chunk Health Meta Store
 */
struct chunk_health_meta_t {
    uint64_t    chk_id  {0};
    uint64_t    size    {0};
    uint64_t    used    {0};
    uint64_t    csd_used    {0};
    uint64_t    dst_used    {0};
    uint64_t    write_count {0};
    uint64_t    read_count  {0};
    uint64_t    last_time   {0};
    uint64_t    last_write  {0};
    uint64_t    last_read   {0};
    uint64_t    last_latency{0};    // (ns)
    uint64_t    last_alloc  {0};
    double      load_weight {0};
    double      wear_weight {0};
    double      total_weight{0};
}; // struct chunk_health_meta_t

class ChunkHealthMS {
public:
    virtual ~ChunkHealthMS() {}

    /**
     * Get a single chunk health
     * @By: chk_id
     */
    virtual int get(chunk_health_meta_t& chk_hlt, uint64_t chk_id) = 0;

    /**
     * Create a single chunk health
     * @Note: create_and_get would update chk_id iff create successfully.
     */
    virtual int create(const chunk_health_meta_t& chk_hlt) = 0;
    virtual int create_and_get(chunk_health_meta_t& chk_hlt) = 0;

    /**
     * Remove a single chunk health
     */
    virtual int remove(uint64_t chk_id) = 0;

    /**
     * Update a single chunk health
     */
    virtual int update(const chunk_health_meta_t& chk_hlt) = 0;

protected:
    ChunkHealthMS(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class ChunkHealdMs

/**
 * Interface for CSD Meta Store
 */
struct csd_meta_t {
    uint64_t    csd_id  {0};
    std::string name    {0};
    uint64_t    size    {0};
    uint64_t    ctime   {0};    // (ms)
    uint64_t    io_addr {0};
    uint64_t    admin_addr {0}; 
    uint32_t    stat    {0};
    uint64_t    latime   {0};    // last active time
}; // struct csd_meta_t

class CsdMS {
public:
    virtual ~CsdMS() {}

    /**
     * Get CSDs for given csd_ids
     */
    virtual int list(std::list<csd_meta_t>& res_list, const std::list<uint64_t>& csd_ids) = 0;

    /**
     * Get a single CSD
     */
    virtual int get(csd_meta_t& res_csd, uint64_t csd_id) = 0;

    /**
     * Create a single CSD
     */
    virtual int create(const csd_meta_t& new_csd) = 0;
    virtual int create_and_get(csd_meta_t& new_csd) = 0;

    /**
     * Remove a single CSD
     */
    virtual int remove(uint64_t csd_id) = 0;

    /**
     * Update a single CSD
     */
    virtual int update(const csd_meta_t& csd) = 0;
    virtual int update_sm(uint64_t csd_id, uint32_t stat, uint64_t io_addr = 0, uint64_t admin_addr = 0) = 0;
    virtual int update_at(uint64_t csd_id, uint64_t latime) = 0;

protected:
    CsdMS(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class CsdMS

/**
 * Interface for CSD Health Meta Store
 */
struct csd_health_meta_t {
    uint64_t    csd_id  {0};
    uint64_t    size    {0};
    uint64_t    alloced {0};
    uint64_t    used    {0};
    uint64_t    write_count {0};
    uint64_t    read_count  {0};
    uint64_t    last_time   {0};
    uint64_t    last_write  {0};
    uint64_t    last_read   {0};
    uint64_t    last_latency{0};
    uint64_t    last_alloc  {0};
    double      load_weight {0};
    double      wear_weight {0};
    double      total_weight{0};
}; // struct csd_health_meta_t

class CsdHealthMS {
public:
    virtual ~CsdHealthMS() {}

    /**
     * List CSD Health
     * 
     * @Note: 'ob' == 'order by'; 'tweight' == 'total_weight'
     */
    virtual int list_ob_tweight(std::list<csd_health_meta_t>& res_list, uint32_t limit) = 0;

    /**
     * Get a single CSD Health
     */
    virtual int get(csd_health_meta_t& csd_hlt, uint64_t csd_id) = 0;

    /**
     * Create a single CSD Health
     */
    virtual int create(const csd_health_meta_t& new_csd) = 0;
    virtual int create_and_get(csd_health_meta_t& new_csd) = 0;

    /**
     * Remove a single CSD Health
     */
    virtual int remove(uint64_t csd_id) = 0;

    /**
     * Update a single CSD Health
     */
    virtual int update(const csd_health_meta_t& csd_hlt) = 0;

protected:
    CsdHealthMS(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class CsdHealthMS

/**
 * Interface for Gateway Meta Store
 */
struct gateway_meta_t {
    uint64_t    gw_id   {0};
    uint64_t    admin_addr  {0};
    uint64_t    ltime   {0};    // connect time (ms)
    uint64_t    atime   {0};    // active time (ms)
}; // struct gateway_meta_t

class GatewayMS {
public:
    virtual ~GatewayMS() {}

    /**
     * Get a single Gateway
     */
    virtual int get(gateway_meta_t& res_gw, uint64_t gw_id) = 0;

    /**
     * Create a single Gateway
     */
    virtual int create(const gateway_meta_t& gw) = 0;

    /**
     * Remove a single Gateway
     */
    virtual int remove(uint64_t gw_id) = 0;

    /**
     * Update a single Gateway
     */
    virtual int update(const gateway_meta_t& gw) = 0;

protected:
    GatewayMS(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class GatewayMS

class MetaStore {
public:
    virtual ~MetaStore() {}

    virtual ClusterMS* get_cluster_ms() = 0;
    virtual VolumeGroupMS* get_vg_ms() = 0;
    virtual VolumeMS* get_volume_ms() = 0;
    virtual ChunkMS* get_chunk_ms() = 0;
    virtual ChunkHealthMS* get_chunk_health_ms() = 0;
    virtual CsdMS* get_csd_ms() = 0;
    virtual CsdHealthMS* get_csd_health_ms() = 0;
    virtual GatewayMS* get_gw_ms() = 0;

protected:
    MetaStore(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class MetaStore

} // namespace flame

#endif // FLAME_METASTORE_METASTORE_H