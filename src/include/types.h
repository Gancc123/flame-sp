#ifndef FLAME_INCLUDE_FLAME_TYPES_H
#define FLAME_INCLUDE_FLAME_TYPES_H

namespace flame {

struct cluster_meta_t {
    std::string name    {};
    uint32_t    mgrs    {0};
    uint32_t    csds    {0};
    uint64_t    size    {0};
    uint64_t    alloced {0};
    uint64_t    used    {0};
}; // struct cluster_meta_t

struct volume_group_meta_t {
    uint64_t    vg_id   {0};
    std::string name    {};
    uint64_t    ctime   {0};    // (ms)
    uint32_t    volumes {0};
    uint64_t    size    {0};    // total visible size
    uint64_t    alloced {0};    // total logical size
    uint64_t    used    {0};    // total physical size
}; // struct group_mate_t

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

struct volume_attr_t {
    std::string vg_name   { };
    std::string vol_name  { };
    uint64_t    chk_sz    {0};
    uint64_t    size      {0};
    uint32_t    flags     {0};
    uint32_t    spolicy   {0};
}; // struct volume_attr_t

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
}; // struct chunk_meta_t

struct chunk_attr_t {
    uint64_t    chk_id    {0};
    uint64_t    vol_id    {0};
    uint32_t    index     {0};
    uint32_t    stat      {0};
    uint32_t    spolicy   {0};
    uint64_t    primary   {0};
    uint64_t    size      {0};
    uint64_t    csd_id    {0};
    uint64_t    dst_id    {0};
}; // struct chunk_attr_t

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

struct csd_addr_attr_t {
    uint64_t    csd_id    {0};
    uint64_t    io_addr   {0};
    uint32_t    stat      {0};
}; // struct csd_addr_attr_t

struct gateway_meta_t {
    uint64_t    gw_id   {0};
    uint64_t    admin_addr  {0};
    uint64_t    ltime   {0};    // connect time (ms)
    uint64_t    atime   {0};    // active time (ms)
}; // struct gateway_meta_t

} // namespace flame

#endif // FLAME_INCLUDE_FLAME_TYPES_H