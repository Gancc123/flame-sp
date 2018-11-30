#ifndef FLAME_SERVICE_INTERNAL_CLIENT_H
#define FLAME_SERVICE_INTERNAL_CLIENT_H

#include <memory>
#include <string>
#include <cstdint>

#include "include/flame.h"
#include "common/context.h"
#include "metastore/metastore.h"

#include <grpcpp/grpcpp.h>
#include "proto/internal.grpc.pb.h"

namespace flame {

struct reg_attr_t {
    std::string    csd_name    {};
    uint64_t       size        {0};
    uint64_t       io_addr     {0};
    uint64_t       admin_addr  {0};
    uint64_t       stat        {0};
}; //struct reg_attr_t

struct chk_hlt_attr_t {
    uint64_t       chk_id      {0};
    uint64_t       size        {0};
    uint32_t       stat        {0};
    uint64_t       used        {0};
    uint64_t       csd_used    {0};
    uint64_t       dst_used    {0}；
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
    uint64_t       last-alloc  {0};
    list<chk_hlt_attr_t> chk_hlt_list;
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

class InternalClientImpl {
public:
    InternalClientImpl(InternalContext* fct, std::shared_ptr<grpc::Channel> channel)
    : InternalClient(fct), stub_(InternalService::NewStub(channel)) {}
    
    // CSD注册
    int register_csd(reg_attr_t& reg_attr, uint64_t csd_id, uint64_t ctime);
    // CSD注销
    int unregister_csd(uint64_t csd_id);
    // CSD上线
    int sign_up(uint64_t csd_id, uint32_t stat, uint64_t io_addr, uint64_t admin_addr);
    // CSD下线
    int sign_out(uint64_t csd_id);
    // CSD心跳汇报
    int push_heart_beat(uint64_t csd_id);
    // CSD状态汇报
    int push_status(uint64_t csd_id, uint32_t stat);
    // CSD健康信息汇报
    int push_health(csd_hlt_attr_t& csd_hlt_attr);
    // 拉取关联Chunk信息
    int pull_related_chunk(list<uint64_t>& chk_list, list<related_chk_attr_t>& res);
    // 推送Chunk相关信息
    int push_chunk_status(list<chk_push_attr_t>& chk_list);

private:
    std::unique_ptr<InternalService::Stub> stub_;

    int return_error__(const grpc::Status& stat);
}; // class FlameClientImpl

} // namespace flame

#endif // FLAME_SERVICE_INTERNAL_CLIENT_H