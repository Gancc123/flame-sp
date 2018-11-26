#ifndef FLAME_SERVICE_FLAME_CLIENT_H
#define FLAME_SERVICE_FLAME_CLIENT_H

#include <memory>
#include <string>
#include <cstdint>


#include "common/context.h"
#include "proto/flame.grpc.pb.h"
#include "metastore/metastore.h"

namespace flame {

class FlameClient {
public:
    FlameClient(FlameContext* fct, std::shared_ptr<grpc::Channel> channel)
    : fct_(fct), stub_(FlameService::NewStub(channel)) {}

    //* Gateway Set
    // GW注册： 建立一个GW连接
    int connect(uint64_t gw_id);
    
    // 获取Flame集群信息
    int get_cluster_info(cluster_meta_t& res);


private:
    FlameContext* fct_;
    std::unique_ptr<FlameService::Stub> stub_;
}; // class FlameClient

} // namespace flame


#endif // FLAME_SERVICE_FLAME_CLIENT_H