#include "csds_client.h"
#include "proto/csds.pb.h"

#include "log_service.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

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
            ChunkVersionItem& chk_ver = reply.chk_ver_list(i);
            chunk_version_t ver;
            ver.chk_id  = chk_ver.chk_id();
            ver.epoch   = chk_ver.epoch();
            ver.version = chk_ver.version();
            ver.used    = chk_ver.used();
            res.push_back(ver);
        }
        return 0;
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
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
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
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
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
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
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::chunk_create(const chunk_create_attr_t& attr) {
    ChunkCreateRequest req;
    req.set_chk_id(attr.chk_id);
    req.set_vol_id(attr.vol_id);
    req.set_index(attr.index);
    req.set_stat(attr.stat);
    req.set_spolicy(attr.spolicy);
    req.set_size(attr.size);

    CsdsReply reply;
    ClientContext ctx;
    Status stat = stub_->createChunk(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::chunk_remove(uint64_t chk_id) {
    ChunkRemoveRequest req;
    req.add_chk_id_list(chk_id);

    CsdsReply reply;
    ClientContext ctx;
    Status stat = stub_->removeChunk(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::chunk_remove(const std::list<uint64_t>& chk_id_list) {
    ChunkRemoveRequest req;
    for (auto it = chk_id_list.begin(); it != chk_id_list.end(); it++)
        req.add_chk_id_list(*it);

    CsdsReply reply;
    ClientContext ctx;
    Status stat = stub_->removeChunk(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::chunk_chooss(const std::list<uint64_t>& chk_id_list) {
    ChunkChooseRequest req;
    for (auto it = chk_id_list.begin(); it != chk_id_list.end(); it++)
        req.add_chk_id_list(*it);

    CsdsReply reply;
    ClientContext ctx;
    Status stat = stub_->chooseChunk(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

int CsdsClientImpl::chunk_move(const chunk_move_attr_t& attr) {
    ChunkMoveRequest req;
    req.set_chk_id(attr.chk_id);
    req.set_src_id(attr.src_id);
    req.set_dst_id(attr.dst_id);
    req.set_signal(attr.signal);

    CsdsReply reply;
    ClientContext ctx;
    Status stat = stub_->moveChunk(&ctx, req, &reply);

    if (stat.ok()) {
        return reply.code();
    } else {
        fct_->log()->error("RPC Faild(%d): %s", stat.error_code(), stat.error_message().c_str());
        return -stat.error_code();
    }
}

} // namespace flame