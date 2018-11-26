#include "flame_client.h"

#include "log_service.h"


using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
namespace flame {

int FlameClient::connect(uint64_t gw_id) {
    ConnectRequest req;
    req.set_gw_id(gw_id);

    FlameReply reply;

    ClientContext cxt;

    Status stat = stub_->connect(&cxt, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code(); // 注意负号
    }
}

} // namespace 