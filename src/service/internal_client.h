#ifndef FLAME_SERVICE_INTERNAL_CLIENT_H
#define FLAME_SERVICE_INTERNAL_CLIENT_H

#include <memory>
#include <string>
#include <cstdint>

#include "include/internal.h"
#include "common/context.h"
#include "metastore/metastore.h"

#include <grpcpp/grpcpp.h>
#include "proto/internal.grpc.pb.h"

namespace flame {

class InternalClientImpl final : public InternalClient {
public:
    InternalClientImpl(FlameContext* fct, std::shared_ptr<grpc::Channel> channel)
    : InternalClient(fct), stub_(InternalService::NewStub(channel)) {}
    
    // CSD注册
    virtual int register_csd(reg_res_t& res, const reg_attr_t& reg_attr);
    
    // CSD注销
    virtual int unregister_csd(uint64_t csd_id);
    
    // CSD上线
    virtual int sign_up(uint64_t csd_id, uint32_t stat, uint64_t io_addr, uint64_t admin_addr);
    
    // CSD下线
    virtual int sign_out(uint64_t csd_id);
    
    // CSD心跳汇报
    virtual int push_heart_beat(uint64_t csd_id);
    
    // CSD状态汇报
    virtual int push_status(uint64_t csd_id, uint32_t stat);
    
    // CSD健康信息汇报
    virtual int push_health(const csd_hlt_attr_t& csd_hlt_attr);
    
    // 拉取关联Chunk信息
    virtual int pull_related_chunk(std::list<related_chk_attr_t>& res, const std::list<uint64_t>& chk_list);
    
    // 推送Chunk相关信息
    virtual int push_chunk_status(const std::list<chk_push_attr_t>& chk_list);

private:
    std::unique_ptr<InternalService::Stub> stub_;
}; // class FlameClientImpl

class InternalClientFoctoryImpl : public InternalClientFoctory {
public:
    InternalClientFoctoryImpl(FlameContext* fct) 
    : InternalClientFoctory(fct) {}

    virtual std::shared_ptr<InternalClient> make_internal_client(node_addr_t addr) override;

    virtual std::shared_ptr<InternalClient> make_internal_client(const std::string& addr) override;
}; // class InternalClientFoctory

} // namespace flame

#endif // FLAME_SERVICE_INTERNAL_CLIENT_H