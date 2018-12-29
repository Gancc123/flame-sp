#ifndef FLAME_INCLUDE_OBJECTS_H
#define FLAME_INCLUDE_OBJECTS_H

#include "include/meta.h"
#include "util/utime.h"

#include <string>

namespace flame {

class ObjectBase {
public:
    virtual ~ObjectBase() {}

protected:
    ObjectBase() {}
}; // class ObjectBase

class VolumeGroupObject {
public:
    VolumeGroupObject() {}
    ~VolumeGroupObject() {}

    void set_vg_id(uint64_t v) { meta_.vg_id = v; }
    uint64_t get_vg_id() const { return meta_.vg_id; }

    void set_name(const std::string& v) { meta_.name = v; }
    std::string get_name() const { return meta_.name; }

    void set_ctime(uint64_t v) { meta_.ctime = v; }
    uint64_t get_ctime() const { return meta_.ctime; }
    void set_ctime_ut(const utime_t& ut) { meta_.ctime = ut.to_usec(); }
    utime_t get_ctime_ut() const { return utime_t::get_by_usec(meta_.ctime); }

    void set_volumes(uint32_t v) { meta_.volumes = v; }
    uint32_t get_volumes() const { return meta_.volumes; }

    void set_size(uint64_t v) { meta_.size = v; }
    uint64_t get_size() const { return meta_.size; }

    void set_alloced(uint64_t v) { meta_.alloced = v; }
    uint64_t get_alloced() const { return meta_.size; }

    void set_used(uint64_t v) { meta_.used = v; }
    uint64_t get_used() const {return meta_.used; }

    VolumeGroupObject(const VolumeGroupObject&) = default;
    VolumeGroupObject(VolumeGroupObject&&) = default;
    VolumeGroupObject& operator = (const VolumeGroupObject&) = default;
    VolumeGroupObject& operator = (VolumeGroupObject&&) = default;

protected:
    volume_group_meta_t meta_;
};

class VolumeObject : public ObjectBase {
public:
    VolumeObject() {}
    ~VolumeObject() {}

    /**
     * Volume MetaData
     */
    void set_vol_id(uint64_t v) { meta_.vol_id = v; }
    uint64_t get_vol_id() const { return meta_.vol_id; }

    void set_vg_id(uint64_t v) { meta_.vg_id = v; }
    uint64_t get_vg_id() const { return meta_.vg_id; }

    void set_name(const std::string& v) { meta_.name = v; }
    std::string get_name() const { return meta_.name; }

    void set_ctime(uint64_t v) { meta_.ctime = v; }
    uint64_t get_ctime() const { return meta_.ctime; }

    void set_chk_sz(uint64_t v) { meta_.chk_sz = v; }
    uint64_t get_chk_sz() const { return meta_.chk_sz; }

    void set_size(uint64_t v) { meta_.size = v; }
    uint64_t get_size() const { return meta_.size; }

    void set_alloced(uint64_t v) { meta_.alloced = v; }
    uint64_t get_alloced() const { return meta_.alloced; }

    void set_used(uint64_t v) { meta_.used = v; }
    uint64_t get_used() const { return meta_.used; }

    void set_flags(uint32_t v) { meta_.flags = v; }
    uint32_t get_flags() const { return meta_.flags; }

    void set_spolicy(uint32_t v) { meta_.spolicy = v; }
    uint32_t get_spolicy() const { return meta_.spolicy; }

    void set_chunks(uint32_t v) { meta_.chunks = v; }
    uint32_t get_chunks() const { return meta_.chunks; }

    void set_ctime_ut(const utime_t& ut) { meta_.ctime = ut.to_usec(); }
    utime_t get_ctime_ut() const { return utime_t::get_by_usec(meta_.ctime); }

    VolumeObject(const VolumeObject&) = default;
    VolumeObject(VolumeObject&&) = default;
    VolumeObject& operator = (const VolumeObject&) = default;
    VolumeObject& operator = (VolumeObject&&) = default;

protected:
    volume_meta_t meta_;
}; // class VolumeObject

class ChunkObject : public ObjectBase {
public:
    ChunkObject() {}
    ~ChunkObject() {}

    /**
     * Chunk ID Setter/Getter
     */
    void set_id(uint64_t v) { id_.val = v; sync_id__(); }
    uint64_t get_id() const { return id_.val; }

    void set_vol_id(uint64_t v) { id_.set_vol_id(v); sync_id__(); }
    uint64_t get_vol_id() const { return id_.get_vol_id(); }

    void set_index(uint32_t v) { id_.set_index(v); sync_id__(); }
    uint32_t get_index() const { return id_.get_index(); }

    void set_sub_id(uint16_t v) { id_.set_sub_id(v); sync_id__(); }
    uint16_t get_sub_id() const { return id_.get_sub_id(); }

    /**
     * Chunk Meta
     */
    void set_stat(uint32_t v) { meta_.stat = hlt_.stat = v; }
    uint32_t get_stat() const { return meta_.stat; }

    void set_spolicy(uint32_t v) { meta_.spolicy = v; }
    uint32_t get_spolicy() const { return meta_.spolicy; }

    void set_ctime(uint64_t v) { meta_.ctime = v; }
    uint64_t get_ctime() const { return meta_.ctime; }
    void set_ctime_ut(const utime_t& ut) { meta_.ctime = ut.to_usec(); }
    utime_t get_ctime_ut() const { return utime_t::get_by_usec(meta_.ctime); }

    void set_primary(uint64_t v) { meta_.primary = v; }
    uint64_t get_primary() const { return meta_.primary; }

    void set_size(uint64_t v) { meta_.size = hlt_.size = v; }
    uint64_t get_size() const { return meta_.size; }

    void set_csd_id(uint64_t v) { meta_.csd_id = v; }
    uint64_t get_csd_id() const { return meta_.csd_id; }

    void set_csd_mtime(uint64_t v) { meta_.csd_mtime = v; }
    uint64_t get_csd_mtime() const { return meta_.csd_mtime; }
    void set_csd_mtime_ut(const utime_t& ut) { meta_.csd_mtime = ut.to_usec(); }
    utime_t get_csd_mtime_ut() const { return utime_t::get_by_usec(meta_.csd_mtime); }

    void set_dst_id(uint64_t v) { meta_.dst_id = v; }
    uint64_t get_dst_id() const { return meta_.dst_id; }

    void set_dst_ctime(uint64_t v) { meta_.dst_ctime = v; }
    uint64_t get_dst_ctime() const { return meta_.dst_ctime; }
    void set_dst_ctime_ut(const utime_t& ut) { meta_.dst_ctime = ut.to_usec(); }
    utime_t get_dst_ctime_ut() const { return utime_t::get_by_usec(meta_.dst_ctime); }

    /**
     * Chunk Health
     */
    void set_used(uint64_t v) { hlt_.used = v; }
    uint64_t get_used() const { return hlt_.used; }

    void set_csd_used(uint64_t v) { hlt_.csd_used = v; }
    uint64_t get_csd_used() const { return hlt_.csd_used; }

    void set_dst_used(uint64_t v) { hlt_.dst_used = v; }
    uint64_t get_dst_used() const { return hlt_.dst_used; }

    void set_write_count(uint64_t v) { hlt_.write_count = v; }
    uint64_t get_write_count() const { return hlt_.write_count; }
    void add_write_count(uint64_t v) { hlt_.write_count += v; }

    void set_read_count(uint64_t v) { hlt_.read_count = v; }
    uint64_t get_read_count() const { return hlt_.read_count; }
    void add_read_count(uint64_t v) { hlt_.read_count += v; }

    void set_last_time(uint64_t v) { hlt_.hlt_meta.last_time = v; }
    uint64_t get_last_time() const { return hlt_.hlt_meta.last_time; }
    void set_last_time_ut(const utime_t& ut) { hlt_.hlt_meta.last_time = ut.to_usec(); }
    utime_t get_last_time_ut() const { return utime_t::get_by_usec(hlt_.hlt_meta.last_time); }

    void set_last_write(uint64_t v) { hlt_.hlt_meta.last_write = v; }
    uint64_t get_last_write() const { return hlt_.hlt_meta.last_write; }

    void set_last_read(uint64_t v) { hlt_.hlt_meta.last_read = v; }
    uint64_t get_last_read() const { return hlt_.hlt_meta.last_read; }

    void set_last_latency(uint64_t v) { hlt_.hlt_meta.last_latency = v; }
    uint64_t get_last_latency() const { return hlt_.hlt_meta.last_latency; }

    void set_last_alloc(uint64_t v) { hlt_.hlt_meta.last_alloc = v; }
    uint64_t get_last_alloc() const { return hlt_.hlt_meta.last_alloc; }

    void set_load_weight(double v) { hlt_.weight_meta.load_weight = v; }
    double get_load_weight() const { return hlt_.weight_meta.load_weight; }

    void set_wear_weight(double v) { hlt_.weight_meta.wear_weight = v; }
    double get_wear_weight() const { return hlt_.weight_meta.wear_weight; }

    void set_total_weight(double v) { hlt_.weight_meta.total_weight = v; }
    double get_total_weight() const { return hlt_.weight_meta.total_weight; }

    ChunkObject(const ChunkObject&) = default;
    ChunkObject(ChunkObject&&) = default;
    ChunkObject& operator = (const ChunkObject&) = default;
    ChunkObject& operator = (ChunkObject&&) = default;

protected:
    chunk_id_t id_;
    chunk_meta_t meta_;
    chunk_health_meta_t hlt_;

private:
    void sync_id__() {
        meta_.chk_id = id_.val;
        meta_.vol_id = id_.get_vol_id();
        meta_.index = id_.get_index();
        hlt_.chk_id = id_.val;
    }
}; // class ChunkObject

class CsdObject : public ObjectBase {
public:
    CsdObject() {}
    ~CsdObject() {}

    /**
     * CSD ID
     */
    void set_csd_id(uint64_t v) { meta_.csd_id = hlt_.csd_id = v; }
    uint64_t get_csd_id() const { return meta_.csd_id; }

    /**
     * CSD MetaData
     */
    void set_name(const std::string& name) { meta_.name = name; }
    std::string get_name() const { return meta_.name; }

    void set_stat(uint32_t v) { meta_.stat = hlt_.stat = v; }
    uint32_t get_stat() const { return meta_.stat; }

    void set_size(uint64_t v) { meta_.size = hlt_.size = v; }
    uint64_t get_size() const { return meta_.size; }

    void set_ctime(uint64_t v) { meta_.ctime = v; } 
    uint64_t get_ctime() const { return meta_.ctime; }
    void set_ctime_ut(const utime_t& ut) { meta_.ctime = ut.to_usec(); }
    utime_t get_ctime_ut() const { return utime_t::get_by_usec(meta_.ctime); }

    void set_io_addr(uint64_t v) { meta_.io_addr = v; } 
    uint64_t get_io_addr() const { return meta_.io_addr; }
    void set_io_ip(uint32_t v) {
        node_addr_t addr(meta_.io_addr);
        addr.set_ip(v);
        meta_.io_addr = addr.val; 
    }
    uint32_t get_io_ip() const { return node_addr_t(meta_.io_addr).get_ip(); }
    void set_io_port(uint16_t v) {
        node_addr_t addr(meta_.io_addr);
        addr.set_port(v);
        meta_.io_addr = addr.val;
    }

    void set_admin_addr(uint64_t v) { meta_.admin_addr = v; } 
    uint64_t get_admin_addr() const { return meta_.admin_addr; }
    void set_admin_ip(uint32_t v) {
        node_addr_t addr(meta_.admin_addr);
        addr.set_ip(v);
        meta_.admin_addr = addr.val;
    }
    uint32_t get_admin_ip() const { return node_addr_t(meta_.admin_addr).get_ip(); }
    void set_admin_port(uint16_t v) {
        node_addr_t addr(meta_.admin_addr);
        addr.set_port(v);
        meta_.admin_addr = addr.val;
    }
    uint16_t get_admin_port() const { return node_addr_t(meta_.admin_addr).get_port(); }

    void set_latime(uint64_t v) { meta_.latime = v; } 
    uint64_t get_latime() const { return meta_.latime; }
    void set_latime_ut(const utime_t& ut) { meta_.latime = ut.to_usec(); }
    utime_t get_latime_ut() const { return utime_t::get_by_usec(meta_.latime); }

    /**
     * CSD Health
     */
    void set_alloced(uint64_t v) { hlt_.alloced = v; } 
    uint64_t get_alloced() const { return hlt_.alloced; }

    void set_used(uint64_t v) { hlt_.used = v; } 
    uint64_t get_used() const { return hlt_.used; }

    void set_write_count(uint64_t v) { hlt_.write_count = v; } 
    uint64_t get_write_count() const { return hlt_.write_count; }
    void add_write_count(uint64_t v) { hlt_.write_count += v; }

    void set_read_count(uint64_t v) { hlt_.read_count = v; } 
    uint64_t get_read_count() const { return hlt_.read_count; }
    void add_read_count(uint64_t v) { hlt_.read_count += v; }

    void set_last_time(uint64_t v) { hlt_.hlt_meta.last_time = v; } 
    uint64_t get_last_time() const { return hlt_.hlt_meta.last_time; }
    void set_last_time_ut(const utime_t& ut) { hlt_.hlt_meta.last_time = ut.to_usec(); }
    utime_t get_last_time_ut() const { return utime_t::get_by_usec(hlt_.hlt_meta.last_time); }

    void set_last_write(uint64_t v) { hlt_.hlt_meta.last_write = v; } 
    uint64_t get_last_write() const { return hlt_.hlt_meta.last_write; }

    void set_last_read(uint64_t v) { hlt_.hlt_meta.last_read = v; } 
    uint64_t get_last_read() const { return hlt_.hlt_meta.last_read; }

    void set_last_latency(uint64_t v) { hlt_.hlt_meta.last_latency = v; } 
    uint64_t get_last_latency() const { return hlt_.hlt_meta.last_latency; }

    void set_last_alloc(uint64_t v) { hlt_.hlt_meta.last_alloc = v; } 
    uint64_t get_last_alloc() const { return hlt_.hlt_meta.last_alloc; }

    void set_load_weight(double v) { hlt_.weight_meta.load_weight = v;}
    double get_load_weight() const { return hlt_.weight_meta.load_weight; }

    void set_wear_weight(double v) { hlt_.weight_meta.wear_weight = v;}
    double get_wear_weight() const { return hlt_.weight_meta.wear_weight; }

    void set_total_weight(double v) { hlt_.weight_meta.total_weight = v;}
    double get_total_weight() const { return hlt_.weight_meta.total_weight; }

    uint64_t get_left() const { return meta_.size - hlt_.alloced; }
    double get_space_usage() const { return (double)hlt_.used / meta_.size; }

    csd_meta_t& meta() { return meta_; }
    csd_health_meta_t& health() { return hlt_; }

    CsdObject(const CsdObject&) = default;
    CsdObject(CsdObject&&) = default;
    CsdObject& operator = (const CsdObject&) = default;
    CsdObject& operator = (CsdObject&&) = default;

protected:
    csd_meta_t meta_;
    csd_health_meta_t hlt_;
}; // class CsdObject

class GatewayObject : public ObjectBase {
public:
    GatewayObject() {}
    ~GatewayObject() {}

    void set_gw_id(uint64_t v) { meta_.gw_id = v; }
    uint64_t get_gw_id() const { return  meta_.gw_id; }

    void set_admin_addr(uint64_t v) { meta_.admin_addr  = v; }
    uint64_t get_admin_addr() const { return meta_.admin_addr; }
    void set_admin_ip(uint32_t v) {
        node_addr_t addr(meta_.admin_addr);
        addr.set_ip(v);
        meta_.admin_addr = addr.val;
    }
    uint32_t get_admin_ip() const { return node_addr_t(meta_.admin_addr).get_ip(); }
    void set_admin_port(uint16_t v) {
        node_addr_t addr(meta_.admin_addr);
        addr.set_port(v);
        meta_.admin_addr = addr.val;
    }
    uint16_t get_admin_port() const { return node_addr_t(meta_.admin_addr).get_port(); }

    void set_ltime(uint64_t v) { meta_.ltime = v; }
    uint64_t get_ltime() const { return meta_.ltime; }
    void set_ltime_ut(const utime_t& ut) { meta_.ltime = ut.to_usec(); }
    utime_t get_ltime_ut() const { return utime_t::get_by_usec(meta_.ltime); }

    void set_atime(uint64_t v) { meta_.atime = v; }
    uint64_t get_atime() const { return meta_.atime; }
    void set_atime_ut(const utime_t& ut) { meta_.atime = ut.to_usec(); }
    utime_t get_atime_ut() const { return utime_t::get_by_usec(meta_.atime); }

    GatewayObject(const GatewayObject&) = default;
    GatewayObject(GatewayObject&&) = default;
    GatewayObject& operator = (const GatewayObject&) = default;
    GatewayObject& operator = (GatewayObject&&) = default;

protected:
    gateway_meta_t meta_;
};

} // namespace flame


#endif // FLAME_INCLUDE_OBJECTS_H