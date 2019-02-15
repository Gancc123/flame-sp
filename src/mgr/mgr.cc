#include "common/context.h"
#include "common/cmdline.h"
#include "common/convert.h"
#include "common/thread/thread.h"
#include "metastore/ms.h"

#include "mgr/mgr_context.h"
#include "mgr/mgr_server.h"
#include "mgr/config_mgr.h"
#include "mgr/csdm/csd_mgmt.h"
#include "mgr/chkm/chk_mgmt.h"
#include "mgr/volm/vol_mgmt.h"

#include <grpcpp/grpcpp.h>
#include "service/flame_service.h"
#include "service/internal_service.h"
#include "service/csds_client.h"

#include "cluster/clt_mgmt.h"
#include "cluster/clt_my/my_mgmt.h"

#include "layout/layout.h"
#include "layout/poll_layout.h"
#include "layout/calculator.h"
#include "layout/calculator_simple.h"

#include "mgr/log_mgr.h"

#include <memory>
#include <iostream>
#include <string>

using grpc::Server;
using grpc::ServerBuilder;

using namespace std;
using namespace flame;
using namespace flame::cli;

namespace flame {

class MgrCli final : public Cmdline {
public:
    MgrCli() : Cmdline("mgr", "Flame Manager Node") {}

    // Argument
    Argument<string>    config_path {this, 'c', "config_file", "config file path", "/etc/flame/mgr.conf"};
    Argument<string>    clt_name    {this, 'g', CFG_MGR_CLT_NAME, "cluster name", "flame"};
    Argument<string>    mgr_name    {this, 'n', CFG_MGR_NAME, "MGR Name", "mgr"};
    Argument<string>    addr        {this, 'a', CFG_MGR_ADDR, "MGR listen port", "0.0.0.0:6666"};
    Argument<string>    metastore   {this, CFG_MGR_METASTORE, "MetaStore url", ""};
    Argument<uint64_t>  hb_cycle    {this, CFG_MGR_HB_CYCLE, "heart beat cycle, unit: ms", 3000};
    Argument<uint64_t>  hb_check    {this, CFG_MGR_HB_CHECK, "heart beat check cycle, unit: ms", 30000};
    Argument<string>    log_dir     {this, CFG_MGR_LOG_DIR, "log dir", "/var/log/flame"};
    Argument<string>    log_level   {this, CFG_MGR_LOG_LEVEL, 
        "log level. {PRINT, TRACE, DEBUG, INFO, WARN, ERROR, WRONG, CRITICAL, DEAD}", "INFO"};

    // Switch
    Switch  console_log {this, CFG_MGR_CONSOLE_LOG, "print log in console"};

    HelpAction help {this};
}; // class MgrCli

class MgrAdminThread;

/**
 * Manager (MGR)
 */
class Manager final {
public:
    Manager(MgrContext* mct) : mct_(mct) {}
    ~Manager() {}

    int init(MgrCli* mgr_cli);
    int run();

    friend class MgrAdminThread;

private:
    MgrContext* mct_;
    MgrServer* server_;

    int admin_server_stat_ {0};
    unique_ptr<MgrAdminThread> admin_thread_;

    /**
     * 配置项
     */
    string      cfg_clt_name_;
    string      cfg_mgr_name_;
    node_addr_t cfg_addr_;
    string      cfg_metastore_url_;
    uint64_t    cfg_hb_cycle_ms_;
    uint64_t    cfg_hb_check_ms_;
    string      cfg_log_dir_;
    string      cfg_log_level_;

    int read_config(MgrCli* csd_cli);

    bool init_log(bool with_console);
    bool init_metastore();
    bool init_server();
    bool init_csdm();
    bool init_cltm();
    bool init_chkm();
    bool init_volm();

    bool run_server();

    int wait();
}; // class Manager

class MgrAdminThread : public Thread {
public:
    MgrAdminThread(Manager* mgr) : mgr_(mgr) {}

    virtual void entry() override {
        if (!mgr_->server_) {
            mgr_->admin_server_stat_ = -1;
            return;
        }
        mgr_->mct_->log()->linfo("start service");
        mgr_->admin_server_stat_ = mgr_->server_->run();
    }

private:
    Manager* mgr_;
};

int Manager::init(MgrCli* mgr_cli) {
    // 读取配置信息
    int r;
    if ((r = read_config(mgr_cli)) != 0) {
        mct_->log()->lerror("read config faild: %d", r);
        return 1;
    }

    // 初始化日志
    if (!init_log(mgr_cli->console_log)) {
        mct_->log()->lerror("init log faild");
        return 2;
    }

    // 初始化MetaStore
    if (!init_metastore()) {
        mct_->log()->lerror("init metastore faild");
        return 3;
    }
    
    // 初始化MgrServer
    if (!init_server()) {
        mct_->log()->lerror("init server faild");
        return 4;
    }

    // 初始化CsdManager
    if (!init_csdm()) {
        mct_->log()->lerror("init csd manager faild");
        return 5;
    }

    // 初始化集群管理组件
    if (!init_cltm()) {
        mct_->log()->lerror("init cluster manager");
        return 6;
    }

    // 初始化ChunkManager
    if (!init_chkm()) {
        mct_->log()->lerror("init chunk manager faild");
        return 7;
    }

    // 初始化VolumeManager
    if (!init_volm()) {
        mct_->log()->lerror("init volume manager faild");
        return 8;
    }

    return 0;
}

int Manager::run() {
    if (!server_) return -1;
    
    if (!run_server()) {
        mct_->log()->lerror("mgr run server faild");
        return 1;
    }

    return wait();
}

int Manager::read_config(MgrCli* mgr_cli) {
    FlameConfig* config = mct_->config();

    /**
     * cfg_clt_name_
     */
    if (mgr_cli->clt_name.done() && !mgr_cli->clt_name.get().empty()) {
        cfg_clt_name_ = mgr_cli->clt_name;
    } else if (config->has_key(CFG_MGR_CLT_NAME)) {
        cfg_clt_name_ = config->get(CFG_MGR_CLT_NAME, "");
    } else {
        mct_->log()->lerror("config[ " CFG_MGR_CLT_NAME " ] not found");
        return 1;
    }

    /**
     * cfg_mgr_name_
     */
    if (mgr_cli->mgr_name.done() && !mgr_cli->mgr_name.get().empty()) {
        cfg_mgr_name_ = mgr_cli->mgr_name;
    } else if (config->has_key(CFG_MGR_NAME)) {
        cfg_mgr_name_ = config->get(CFG_MGR_NAME, "");
    } else {
        mct_->log()->lerror("config[ " CFG_MGR_NAME " ] not found");
        return 2;
    }

    /**
     * cfg_addr_
     */
    string addr;
    if (mgr_cli->addr.done() && !mgr_cli->addr.get().empty()) {
        addr = mgr_cli->addr;
    } else if (config->has_key(CFG_MGR_ADDR)) {
        addr = config->get(CFG_MGR_ADDR, "");
    } else {
        mct_->log()->lerror("config[ " CFG_MGR_ADDR " ] not found");
        return 3;
    }

    if (!string_parse(cfg_addr_, addr)) {
        mct_->log()->lerror("invalid config[ " CFG_MGR_ADDR " ]");
        return 3;
    }

   /**
     * cfg_metastore_url_
     */
    if (mgr_cli->metastore.done() && !mgr_cli->metastore.get().empty()) {
        cfg_metastore_url_ = mgr_cli->metastore;
    } else if (config->has_key(CFG_MGR_METASTORE)) {
        cfg_metastore_url_ = config->get(CFG_MGR_METASTORE, "");
    } else {
        mct_->log()->lerror("config[ " CFG_MGR_METASTORE " ] not found");
        return 4;
    }

    /**
     * cfg_hb_cycle_ms_
     */
    if (mgr_cli->hb_cycle.done() && !mgr_cli->hb_cycle.get()) {
        cfg_hb_cycle_ms_ = mgr_cli->hb_cycle;
    } else if (config->has_key(CFG_MGR_HB_CYCLE)) {
        if (!string_parse(cfg_hb_cycle_ms_, config->get(CFG_MGR_HB_CYCLE, ""))) {
            mct_->log()->lerror("invalid config[ " CFG_MGR_HB_CYCLE " ]");
            return 5;
        }
    } else {
        mct_->log()->lerror("config[ " CFG_MGR_HB_CYCLE " ] not found");
        return 5;
    }

    /**
     * cfg_hb_check_ms_
     */
    if (mgr_cli->hb_check.done() && !mgr_cli->hb_check.get()) {
        cfg_hb_check_ms_ = mgr_cli->hb_check;
    } else if (config->has_key(CFG_MGR_HB_CHECK)) {
        if (!string_parse(cfg_hb_check_ms_, config->get(CFG_MGR_HB_CHECK, ""))) {
            mct_->log()->lerror("invalid config[ " CFG_MGR_HB_CHECK " ]");
            return 6;
        }
    } else {
        mct_->log()->lerror("config[ " CFG_MGR_HB_CHECK " ] not found");
        return 6;
    }

    /**
     * cfg_log_dir_
     */
    string log_dir;
    if (mgr_cli->log_dir.done() && !mgr_cli->log_dir.get().empty()) {
        cfg_log_dir_ = mgr_cli->log_dir;
    } else if (config->has_key(CFG_MGR_LOG_DIR)) {
        cfg_log_dir_ = config->get(CFG_MGR_LOG_DIR, "");
    } else {
        mct_->log()->lerror("config[ " CFG_MGR_LOG_DIR " ] not found");
        return 7;
    }

    /**
     * cfg_log_level_
     */
    string log_level;
    if (mgr_cli->log_level.done() && !mgr_cli->log_level.get().empty()) {
        cfg_log_level_ = mgr_cli->log_level;
    } else if (config->has_key(CFG_MGR_LOG_LEVEL)) {
        cfg_log_level_ = config->get(CFG_MGR_LOG_LEVEL, "");
    } else {
        mct_->log()->lerror("config[ " CFG_MGR_LOG_LEVEL " ] not found");
        return 8;
    }

    return 0;
}

bool Manager::init_log(bool with_console) {
    if (!mct_->fct()->init_log(cfg_log_dir_, cfg_log_level_, "mgr")) {
        mct_->fct()->log()->lerror("init log faild");
        return false;
    }
    mct_->fct()->log()->set_with_console(with_console);
    return true;
}

bool Manager::init_metastore() {
    auto ms = create_metastore(mct_->fct(), cfg_metastore_url_);
    if (!ms) {
        mct_->log()->lerror("create metastore with url(%s) faild", cfg_metastore_url_.c_str());
        return false;
    }
    mct_->ms(ms);
    return true;
}

bool Manager::init_server() {
    server_ = new MgrServer(mct_, convert2string(cfg_addr_));

    mct_->timer(shared_ptr<TimerWorker>(new TimerWorker()));
    mct_->timer()->run();
    return true;
}

bool Manager::init_csdm() {
    shared_ptr<CsdsClientFoctory> csd_client_foctory(new CsdsClientFoctoryImpl(mct_->fct()));

    // 配置Csd健康信息计算器
    shared_ptr<layout::CsdHealthCaculator> csd_hlt_calor(new layout::SimpleCsdHealthCalculator());

    shared_ptr<CsdManager> csdm(new CsdManager(
        mct_->bct(),
        csd_client_foctory,
        csd_hlt_calor   // CSD 健康信息计算器
    ));
    csdm->init();
    mct_->csdm(csdm);
    return true;
}

bool Manager::init_cltm() {
    shared_ptr<ClusterMgmt> cltm(new MyClusterMgmt(
        mct_->fct(), 
        mct_->csdm(), 
        mct_->timer(), 
        utime_t::get_by_msec(cfg_hb_check_ms_)
    ));

    mct_->cltm(cltm);
    cltm->init();
    return true;
}

bool Manager::init_chkm() {
    // 配置Chunk布局策略
    shared_ptr<layout::ChunkLayout> layout(new layout::PollLayout(mct_->csdm()));

    // 配置Chunk健康信息计算器（通常需要跟布局策略一同配置）
    shared_ptr<layout::ChunkHealthCaculator> chk_hlt_calor(new layout::SimpleChunkHealthCalulator());

    shared_ptr<ChunkManager> chkm(new ChunkManager(
        mct_->bct(),
        mct_->csdm(),
        layout,
        chk_hlt_calor   // Chunk健康信息计算器
    ));

    mct_->chkm(chkm);
    return true;
}

bool Manager::init_volm() {
    shared_ptr<VolumeManager> volm(new VolumeManager(
        mct_->bct(),
        mct_->csdm(),
        mct_->chkm()
    ));
    if (volm->init() != RC_SUCCESS)
        return false;
    mct_->volm(volm);
    return true;
}

bool Manager::run_server() {
    admin_thread_.reset(new MgrAdminThread(this));
    mct_->log()->linfo("run server");
    admin_thread_->create("mgr_admin");
    return true;
}

int Manager::wait() {
    admin_thread_->join();
    mct_->log()->linfo("join admin_thread with: %d", admin_server_stat_);
    return 0;
}

} // namespace flame

int main(int argc, char** argv) {
    MgrCli* mgr_cli = new MgrCli();
    int r = mgr_cli->parser(argc, argv);
    if (r != CmdRetCode::SUCCESS) {
        mgr_cli->print_error();
        return r;
    } else if (mgr_cli->help.done()) {
        mgr_cli->print_help();
        return 0;
    }

    // 获取全局上下文
    FlameContext* fct = FlameContext::get_context();

    if (!fct->init_config(mgr_cli->config_path)) {
        fct->log()->lerror("init config faild");
        exit(-1);
    }

    MgrBaseContext* mbct = new MgrBaseContext(fct);

    // 创建MGR上下文
    MgrContext* mct = new MgrContext(mbct);

    // 创建MGR主程序
    Manager mgr(mct);

    // 初始化MGR，Manager在初始化完成后保证不再使用MgrCli
    if (mgr.init(mgr_cli) != 0)
        exit(-1);

    // 释放MgrCli资源
    delete mgr_cli;

    // 运行MGR主程序
    return mgr.run();
}