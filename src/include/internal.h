#ifndef FLAME_INCLUDE_INTERNAL_H
#define FLAME_INCLUDE_INTERNAL_H

#include "common/context.h"
#include "include/meta.h"

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

struct csd_hlt_attr_t {
    csd_hlt_sub_t             csd_hlt_sub;
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
}; // class InternalClient

class InternalClientFoctory {
public:
    virtual ~InternalClientFoctory() {}

    virtual std::shared_ptr<InternalClient> make_internal_client(node_addr_t addr) = 0;

    virtual std::shared_ptr<InternalClient> make_internal_client(const std::string& addr) = 0;

protected:
    InternalClientFoctory(FlameContext* fct) : fct_(fct) {}

    FlameContext* fct_;
}; // class InternalClientFoctory

class InternalClientContext {
public:
    InternalClientContext(FlameContext* fct, InternalClient* client) 
    : fct_(fct), client_(client) {}

    ~InternalClientContext() {}

    FlameContext* fct() const { return fct_; }
    Logger* log() const { return fct_->log(); }
    FlameConfig* config() const { return fct_->config(); }
    
    InternalClient* client() const { return client_.get(); }
    void set_client(InternalClient* c) { client_.reset(c); }

private:
    FlameContext* fct_;
    std::unique_ptr<InternalClient> client_;
}; // class InternalClientContext

} // namespace flame

#endif // FLAME_INCLUDE_INTERNAL_H