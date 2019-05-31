#include "memzone/rdma_mz.h"
#include "common/context.h"
#include "common/log.h"
#include "memzone/spdk_common.h"
#include "allocator_test.h"

using namespace flame;
using namespace flame::memory;
using namespace flame::memory::ib;
using namespace flame::msg;

using FlameContext = flame::FlameContext;

#define KB 1024L
#define MB (1024L * (KB))
#define GB (1024L * (MB))
#define CFG_PATH "flame_mgr.cfg"

perf_config_t global_config;

struct test_context {
    FlameContext *fct;
    MsgContext *mct;
public:
    test_context(FlameContext *_fct, MsgContext *_mct): fct(_fct), mct(_mct) {

    }
};

static void test_start(void *arg1, void *arg2) {
    struct test_context *tctx = (struct test_context*)arg1;

    FlameContext *fct = tctx->fct;
    MsgContext *mct = tctx->mct;

    std::cout << "test start..." << std::endl;

    MemoryConfig *mem_cfg = MemoryConfig::load_config(fct);
    assert(mem_cfg);

    std::cout << "load config completed.." << std::endl;

    flame::msg::ib::ProtectionDomain *pd = flame::msg::Stack::get_rdma_stack()->get_manager()->get_ib().get_pd();

    //初始化一个RDMA env然后将RDMA的pd作为参数传递给RdmaAllocator。
    BufferAllocator *allocator = new RdmaAllocator(fct, pd, mem_cfg);

    std::cout << "alloctor init completed.." << std::endl;

    Buffer rdma_buf = allocator->allocate(4 * MB);
    BufferPtr *rdma_buf_ptr = rdma_buf.get();

    if(rdma_buf_ptr == nullptr) {
        fct->log()->lerror("allocator alloc memory failed.");
        spdk_app_stop(-1);
        return ;
    } else {
        fct->log()->linfo("allocator alloc memory success.");
        
        void *buffer = rdma_buf_ptr->addr();
        size_t size = rdma_buf_ptr->size();

        std::cout << "size = " << size << std::endl;
    }

    spdk_app_stop(0);
}

int main(int argc, char *argv[])
{
    auto parser = init_parser();
    optparse::Values options = parser.parse_args(argc, argv);
    auto config_path = options.get("config");

    FlameContext *fct = FlameContext::get_context();
    if(!fct->init_config(std::string(config_path).empty() ? CFG_PATH : config_path)) {
        clog("init config failed.");
        return -1;
    }

    if(!fct->init_log("", str2upper(std::string(options.get("log_level"))), 
                fmt::format("memory{}", std::string(options.get("index"))))) {
        clog("init log failed.");
        return -1;
    }

    auto mct = new MsgContext(fct);
    global_config.num = (int)options.get("num");
    global_config.result_file = std::string(options.get("result_file"));
    global_config.perf_type = perf_type_from_str(std::string(options.get("type")));
    global_config.use_imm_resp = (bool)options.get("imm_resp");
    global_config.inline_size = std::string(options.get("inline"));
    global_config.size = size_str_to_uint64(std::string(options.get("size")));
    global_config.no_thr_optimize = (bool)options.get("no_thr_opt");

    init_resource(global_config);

    auto rdma_msger = new RdmaMsger(mct, &global_config);
    mct->load_config();
    mct->config->set_msg_log_level(std::string(options.get("log_level")));
    mct->config->set_rdma_max_inline_data(std::string(options.get("inline")));

    mct->init(rdma_msger, nullptr);

    std::cout << "mct init success..." << std::endl;

    int rc = 0;
    struct spdk_app_opts opts = {};
    spdk_app_opts_init(&opts);
    opts.name = "allocator_test";
    opts.reactor_mask = "0x01";


    struct test_context *t_ctx = new test_context(fct, mct);
    rc = spdk_app_start(&opts, test_start, t_ctx, nullptr);
    if(rc) {
        SPDK_NOTICELOG("spdk app start: ERROR!\n");
    } else {
        SPDK_NOTICELOG("SUCCESS!\n");
    }

    spdk_app_fini();

    if(rdma_msger)
        delete rdma_msger;

    return rc;
}