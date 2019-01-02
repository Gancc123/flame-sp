#include "csds_client.h"
#include "proto/csds.pb.h"

#include <sstream>

#include "log_service.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace std;

namespace flame {

int CsdsClientImpl::chunk_fetch(std::list<chunk_version_t>& res, const std::list<uint64_t>& chk_id_list) {
    ChunkFetchRequest req;
    for (auto it = chk_id_list.begin(); it != chk_id_list.end(); it++)
        req.add_chk_id_list(*it);
    
    ChunkFetchReply reply;
    ClientContext ctx;
    Status stat = stub_->fetchChunk(&ctx, req, &reply);

    if (stat.ok()) {
        for (int i = 0; i < reply.chk_ver_list_size(); i++) {
            ChunkVersionItem* chk_ver = reply.add_chk_ver_list();
            chunk_version_t ver;
            ver.chk_id  = chk_ver->chk_id();
            ver.epoch   = chk_ver->epoch();
            ver.version = chk_ver->version();
            ver.used    = chk_ver->used();
            res.push_back(ver);
        }
        return 0;
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::chunk_signal(const std::list<chunk_signal_t>& chk_sgn_list) {
    ChunkSignalRequest req;
    for (auto it = chk_sgn_list.begin(); it != chk_sgn_list.end(); it++) {
        ChunkSignalItem* item = req.add_signal_list();
        item->set_chk_id(it->chk_id);
        item->set_signal(it->signal);
    }

    CsdsReply reply;
    ClientContext ctx;
    Status stat = stub_->pushChunkSignal(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::shutdown(uint64_t csd_id) {
    ShutdownRequest req;
    req.set_csd_id(csd_id);

    CsdsReply reply;
    ClientContext ctx;
    Status stat = stub_->shutdown(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::clean(uint64_t csd_id) {
    CleanRequest req;
    req.set_csd_id(csd_id);

    CsdsReply reply;
    ClientContext ctx;
    Status stat = stub_->clean(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::chunk_create(std::list<chunk_bulk_res_t>& res, const chk_attr_t& attr, const std::list<uint64_t>& chk_id_list) {
    ChunkCreateRequest req;
    req.set_flags(attr.flags);
    req.set_spolicy(attr.spolicy);
    req.set_size(attr.size);
    for (auto it = chk_id_list.begin(); it != chk_id_list.end(); it++)
        req.add_chk_id_list(*it);

    ChunkBulkReply reply;
    ClientContext ctx;
    Status stat = stub_->createChunk(&ctx, req, &reply);

    if (stat.ok()) {
        for (int i = 0; i < reply.res_list_size(); i++) {
            auto chk = reply.res_list(i);
            chunk_bulk_res_t chkr;
            chkr.chk_id = chk.chk_id();
            chkr.res = chk.res();
            res.push_back(chkr);
        }
        return 0;
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::chunk_remove(uint64_t chk_id) {
    ChunkRemoveRequest req;
    req.add_chk_id_list(chk_id);

    ChunkBulkReply reply;
    ClientContext ctx;
    Status stat = stub_->removeChunk(&ctx, req, &reply);

    if (stat.ok()) {
        if (reply.res_list_size() == 0) return 1;
        for (int i = 0; i < reply.res_list_size(); i++) {
            auto res = reply.res_list(i);
            if (res.chk_id() == chk_id)
                return res.res();
        }
        return 2;
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::chunk_remove(std::list<chunk_bulk_res_t>& res, const std::list<uint64_t>& chk_id_list) {
    ChunkRemoveRequest req;
    for (auto it = chk_id_list.begin(); it != chk_id_list.end(); it++)
        req.add_chk_id_list(*it);

    ChunkBulkReply reply;
    ClientContext ctx;
    Status stat = stub_->removeChunk(&ctx, req, &reply);

    if (stat.ok()) {
        for (int i = 0; i < reply.res_list_size(); i++) {
            auto chk = reply.res_list(i);
            chunk_bulk_res_t chkr;
            chkr.chk_id = chk.chk_id();
            chkr.res = chk.res();
            res.push_back(chkr);
        }
        return 0;
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::chunk_chooss(std::list<chunk_bulk_res_t>& res, const std::list<uint64_t>& chk_id_list) {
    ChunkChooseRequest req;
    for (auto it = chk_id_list.begin(); it != chk_id_list.end(); it++)
        req.add_chk_id_list(*it);

    ChunkBulkReply reply;
    ClientContext ctx;
    Status stat = stub_->chooseChunk(&ctx, req, &reply);

    if (stat.ok()) {
        for (int i = 0; i < reply.res_list_size(); i++) {
            auto chk = reply.res_list(i);
            chunk_bulk_res_t chkr;
            chkr.chk_id = chk.chk_id();
            chkr.res = chk.res();
            res.push_back(chkr);
        }
        return 0;
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::chunk_move(std::list<chunk_bulk_res_t>& res, const std::list<chunk_move_attr_t>& attr_list) {
    ChunkMoveRequest req;
    for (auto it = attr_list.begin(); it != attr_list.end(); it++) {
        auto attr = req.add_chk_list();
        attr->set_chk_id(it->chk_id);
        attr->set_src_id(it->src_id);
        attr->set_dst_id(it->dst_id);
        attr->set_signal(it->signal);
    }

    ChunkBulkReply reply;
    ClientContext ctx;
    Status stat = stub_->moveChunk(&ctx, req, &reply);

    if (stat.ok()) {
        for (int i = 0; i < reply.res_list_size(); i++) {
            auto chk = reply.res_list(i);
            chunk_bulk_res_t chkr;
            chkr.chk_id = chk.chk_id();
            chkr.res = chk.res();
            res.push_back(chkr);
        }
        return 0;
    } else {
        fct_->log()->lerror("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

std::shared_ptr<CsdsClient> CsdsClientFoctoryImpl::make_csds_client(node_addr_t addr) {
    uint32_t ip = addr.get_ip();
    uint8_t* part = (uint8_t*)&ip;

    ostringstream oss;
    oss << *part << "." << *(part + 1) << "." << *(part + 2) << "." << *(part + 3);
    oss << ":" << addr.get_port();

    CsdsClientImpl* client = new CsdsClientImpl(fct_, grpc::CreateChannel(
        oss.str(), grpc::InsecureChannelCredentials()
    ));

    return std::shared_ptr<CsdsClient>(client);
}

} // namespace flame