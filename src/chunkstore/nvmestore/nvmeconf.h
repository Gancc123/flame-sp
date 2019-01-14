#ifndef FLAME_CHUNKSTORE_NVMECONF_H
#define FLAME_CHUNKSTORE_NVMECONF_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cinttypes>

#include <unistd.h>

namespace flame {
class NvmeConf {
public:
    std::string file_path;
    std::string core_mask;

    std::string read_mask;
    std::string write_mask;
    std::string meta_mask;

    uint32_t admin_core;
    uint32_t meta_core;
    std::vector<uint32_t> read_cores;
    std::vector<uint32_t> write_cores;

    uint32_t core_count;
    uint32_t num_read_core;
    uint32_t num_write_core;

    bool default_config;

    uint32_t cluster_sz;
    uint32_t num_md_pages;
    uint32_t max_md_ops;
    uint32_t max_channel_ops;

    uint64_t chunk_blob_map_id;
    uint32_t num_clusters;

    uint64_t migrate_info_blob_id;

public:
    NvmeConf(const std::string& config_file): file_path(config_file), core_mask(""), 
                                    read_mask(""), write_mask(""), 
                                    meta_mask(""), admin_core(UINT32_MAX), meta_core(UINT32_MAX),
                                    core_count(UINT32_MAX), num_read_core(UINT32_MAX), num_write_core(UINT32_MAX),
                                    default_config(true), cluster_sz(0), num_md_pages(0), max_md_ops(0), 
                                    max_channel_ops(0), chunk_blob_map_id(0), num_clusters(0), migrate_info_blob_id(0) {

    }

    NvmeConf(): file_path("../etc/nvmestore.conf"), core_mask(""),
                            read_mask(""), write_mask(""), 
                            meta_mask(""), admin_core(UINT32_MAX), meta_core(UINT32_MAX),
                            core_count(UINT32_MAX), num_read_core(UINT32_MAX), num_write_core(UINT32_MAX), 
                             default_config(true), cluster_sz(0), num_md_pages(0), max_md_ops(0),
                            max_channel_ops(0), chunk_blob_map_id(0), num_clusters(0), migrate_info_blob_id(0) {

    }

    ~NvmeConf() {

    }

    int load();
    int store();
    void print_conf();
    bool is_valid();
    bool is_valid_mask();
    uint64_t get_blob_id() const;
    uint32_t get_core_count() const;
    uint32_t get_read_core_count() const;
    uint32_t get_write_core_count() const;

    void set_blob_id(uint64_t blobid);
    void set_admin_core(uint32_t core);
    void get_core_id_by_mask(std::vector<uint32_t > &cores, std::string &mask);
    void get_core_id_by_mask_dig(std::vector<uint32_t> &cores, uint32_t mask_dig);
    uint32_t get_core_id_by_mask_dig(uint32_t mask_dig);

    uint32_t get_read_core(uint32_t index);
    uint32_t get_write_core(uint32_t index);

    void init_core_info_default();
    void init_core_info_core_count();

    void set_core_mask(std::string mask) {
        core_mask = mask;
    }

    bool is_default_config() {
        return default_config;
    }
};

}

#endif