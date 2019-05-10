#ifndef FLAME_MSG_RDMA_RDMA_CONNECTION_H
#define FLAME_MSG_RDMA_RDMA_CONNECTION_H

#include "msg/Msg.h"
#include "msg/Connection.h"
#include "MemoryManager.h"
#include "RdmaMem.h"

#include <atomic>
#include <string>
#include <deque>
#include <vector>
#include <functional>

#define FLAME_MSG_RDMA_SEL_SIG_WRID_MATIC_PREFIX_SHIFT 24
//The magic prefix helps distinguishing whether wrid is a pointer.
#define FLAME_MSG_RDMA_SEL_SIG_WRID_MAGIC_PREFIX 0x19941224ff000000ULL

namespace flame{
namespace msg{

//"inline" allows the function to be declared in multiple translation units.
//The only meaning of "inline" to C++ is allowing multiple definitions.
inline bool is_sel_sig_wrid(uint64_t wrid){
    int shift = FLAME_MSG_RDMA_SEL_SIG_WRID_MATIC_PREFIX_SHIFT;
    uint64_t prefix = FLAME_MSG_RDMA_SEL_SIG_WRID_MAGIC_PREFIX >> shift;
    return (wrid >> shift) == prefix;
}

inline uint32_t num_from_sel_sig_wrid(uint64_t wrid){
    int shift = FLAME_MSG_RDMA_SEL_SIG_WRID_MATIC_PREFIX_SHIFT;
    return (uint32_t)(wrid & ~((~0ULL) << shift));
}

//num max is limited by "prefix shift".
inline uint64_t sel_sig_wrid_from_num(uint32_t num){
    int shift = FLAME_MSG_RDMA_SEL_SIG_WRID_MATIC_PREFIX_SHIFT;
    uint64_t prefix = FLAME_MSG_RDMA_SEL_SIG_WRID_MAGIC_PREFIX;
    uint64_t num_clean = num & ~((~0ULL) << shift);
    return prefix | num_clean;
}

class RdmaWorker;
struct RdmaRwWork;

extern const uint32_t RDMA_RW_WORK_BUFS_LIMIT;
extern const uint32_t RDMA_BATCH_SEND_WR_MAX;
extern const uint8_t RDMA_QP_MAX_RD_ATOMIC;

class RdmaConnection : public Connection{
public:
    enum class RdmaStatus : uint8_t{
        ERROR,
        INIT,
        CAN_WRITE,
        CLOSING_POSITIVE,// for fin msg sender, cannot write but still can read.
        CLOSING_PASSIVE, // for fin msg recver, cannot read but still can write.
        CLOSED
    };
private:
    using Chunk = ib::Chunk; 
    using RdmaBuffer = ib::RdmaBuffer;
    ib::QueuePair *qp = nullptr;
    ib::IBSYNMsg peer_msg;
    ib::IBSYNMsg my_msg;
    bool active = false;
    RdmaWorker *rdma_worker = nullptr;

    Mutex send_mutex;
    std::list<Msg *> msg_list;
    std::list<RdmaRwWork *> rw_work_list;
    std::deque<uint32_t> imm_data_list;

    //for recv msg
    std::list<ibv_wc> recv_wc;
    uint32_t recv_cur_msg_offset = 0;
    MsgBuffer recv_cur_msg_header_buffer;
    Msg *recv_cur_msg = nullptr;

    std::atomic<RdmaStatus> status;
    std::atomic<bool> fin_msg_pending;

    void recv_msg_cb(Msg *msg);
    int post_work_request(std::vector<Chunk *> &tx_buffers);
    int post_rdma_send(std::list<Msg *> &msg);
    int submit_send_works();
    int submit_rw_works();
    int decode_rx_buffer(ib::Chunk *chunk);
    int reap_send_msg();
    RdmaConnection(MsgContext *mct);
public:
    static RdmaConnection *create(MsgContext *mct, RdmaWorker *w, uint8_t sl=0);
    ~RdmaConnection();
    virtual msg_ttype_t get_ttype() override { return msg_ttype_t::RDMA; }

    virtual ssize_t send_msg(Msg *msg, bool more=false) override;
    virtual Msg* recv_msg() override;
    virtual ssize_t send_msgs(std::list<Msg *> &msgs) override;
    virtual int pending_msg() override {
        MutexLocker l(send_mutex);
        return this->msg_list.size();
    };
    size_t pending_rw_works(){
        MutexLocker l(send_mutex);
        return this->rw_work_list.size();
    }
    virtual bool is_connected() override{
        return status == RdmaStatus::CAN_WRITE;
    }
    virtual void close() override;
    virtual bool has_fd() const override { return false; }
    virtual bool is_owner_fixed() const override { return true; }

    std::atomic<bool> is_dead_pending;
    std::atomic<uint32_t> inflight_rx_buffers;

    void pass_wc(std::list<ibv_wc> &wc);
    void get_wc(std::list<ibv_wc> &wc);

    size_t recv_data();

    ssize_t submit(bool more=false);

    int activate();

    int post_rdma_rw(RdmaRwWork *work, bool enqueue=true);
    int post_imm_data(uint32_t imm_data);
    int post_imm_data(std::vector<uint32_t> *imm_data_vec);

    const char* get_qp_state() { 
        return ib::Infiniband::qp_state_string(qp->get_state()); 
    }
    void fin();
    void fault();
    bool is_closed() const { return status == RdmaStatus::CLOSED; }
    bool is_error() const { return status == RdmaStatus::ERROR; }
    ib::QueuePair *get_qp() const { return this->qp; }
    uint32_t get_tx_wr() const { return this->qp->get_tx_wr(); }

    ib::IBSYNMsg &get_my_msg() {
        return my_msg;
    }

    ib::IBSYNMsg &get_peer_msg() {
        return peer_msg;
    }

    RdmaWorker *get_rdma_worker() const{
        return rdma_worker;
    }

    static inline RdmaConnection *get_by_qp(ib::QueuePair *qp){
        return (RdmaConnection *)qp->user_ctx;
    }

    static std::string status_str(RdmaStatus s){
        switch(s){
        case RdmaStatus::ERROR:
            return "error";
        case RdmaStatus::INIT:
            return "init";
        case RdmaStatus::CAN_WRITE:
            return "can_write";
        case RdmaStatus::CLOSING_POSITIVE:
            return "closing_positive";
        case RdmaStatus::CLOSING_PASSIVE:
            return "closing_passive";
        case RdmaStatus::CLOSED:
            return "closed";
        default:
            return "unknown";
        }
    }

};

} //namespace msg
} //namespace flame

#endif