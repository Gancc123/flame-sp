#ifndef FLAME_INCLUDE_INTERNAL_H
#define FLAME_INCLUDE_INTERNAL_H

#include "common/context.h"

#include <cstdint>
#include <list>

namespace flame {

struct reg_attr_t {
    std::string     csd_name    {};
    uint64_t        size        {0};
    uint64_t        io_addr     {0};
    uint64_t        admin_addr  {0};
    uint64_t        stat        {0};
}; // struct reg_attr_t

struct reg_res_t {
    uint64_t        csd_id      {0};
    uint64_t        ctime       {0};
}; // struct reg_res_t

struct chk_hlt_attr_t {
    uint64_t       chk_id      {0};
    uint64_t       size        {0};
    uint32_t       stat        {0};
    uint64_t       used        {0};
    uint64_t       csd_used    {0};
    uint64_t       dst_used    {0};
    uint64_t       last_time   {0};
    uint64_t       last_write  {0};
    uint64_t       last_read   {0};
    uint64_t       last_latency{0};
    uint64_t       last_alloc  {0};
}; //struct chk_hlt_attr_t

struct csd_hlt_attr_t {
    uint64_t       csd_id      {0};
    uint64_t       size        {0};
    uint64_t       alloced     {0};
    uint64_t       used        {0};
    uint64_t       last_time   {0};
    uint64_t       last_write  {0};
    uint64_t       last_read   {0};
    uint64_t       last_latency{0};
    uint64_t       last_alloc  {0};
    std::list<chk_hlt_attr_t> chk_hlt_list;
}; //struct csd_hlt_attr_t

struct related_chk_attr_t {
    uint64_t       org_id      {0};
    uint64_t       chk_id      {0};
    uint64_t       csd_id      {0};
    uint64_t       dst_id      {0};
}; //struct related_chk_attr_t

struct chk_push_attr_t {
    uint64_t       chk_id      {0};
    uint32_t       stat        {0};
    uint64_t       csd_id      {0};
    uint64_t       dst_id      {0};
    uint64_t       dst_mtime   {0};
}; //struct chk_push_attr_t

class InternalClient {
public:
    virtual ~InternalClient() {}
    
    // CSD注册
    virtual int register_csd(reg_res_t& res, const reg_attr_t& reg_attr) = 0;

    // CSD注销
    virtual int unregister_csd(uint64_t csd_id) = 0;

    // CSD上线
    virtual int sign_up(uint64_t csd_id, uint32_t stat, uint64_t io_addr, uint64_t admin_addr) = 0;

    // CSD下线
    virtual int sign_out(uint64_t csd_id) = 0;

    // CSD心跳汇报
    virtual int push_heart_beat(uint64_t csd_id) = 0;

    // CSD状态汇报
    virtual int push_status(uint64_t csd_id, uint32_t stat) = 0;

    // CSD健康信息汇报
    virtual int push_health(const csd_hlt_attr_t& csd_hlt_attr) = 0;

    // 拉取关联Chunk信息
    virtual int pull_related_chunk(std::list<related_chk_attr_t>& res, const std::list<uint64_t>& chk_list) = 0;

    // 推送Chunk相关信息
    virtual int push_chunk_status(const std::list<chk_push_attr_t>& chk_list) = 0;

protected:
    InternalClient(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class FlameClientImpl

} // namespace flame

#endif // FLAME_INCLUDE_INTERNAL_H