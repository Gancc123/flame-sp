#include "common/context.h"
#include "common/cmdline.h"
#include "common/convert.h"
#include "chunkstore/cs.h"

#include "csd/csd_context.h"
#include "csd/csd_admin.h"
#include "csd/config_csd.h"

#include "service/internal_client.h"
#include "include/retcode.h"

#include "csd/log_csd.h"

#include <memory>
#include <string>

using namespace std;
using namespace flame;
using namespace flame::cli;

namespace flame {

class CsdCli final : public Cmdline {
public:
    CsdCli() : Cmdline("csd", "Flame Chunk Store Daemon") {}

    // Argument
    Argument<string>    config_path {this, 'c', "config_file", "config file path", "/etc/flame/csd.conf"};
    Argument<string>    admin_addr  {this, 'a', "admin_addr", "CSD admin listen port", "0.0.0.0:7777"};
    Argument<string>    io_addr     {this, 'i', "io_addr", "CSD IO listen port", "0.0.0.0:9999"};
    Argument<string>    mgr_addr    {this, 'm', "mgr_addr", "Flame MGR Address", "0.0.0.0:6666"};
    Argument<string>    chunkstore  {this, "chunkstore", "ChunkStore url", ""};
    Argument<uint64_t>  heart_beat  {this, "heart_beat", "heart beat cycle, unit: ms", 3000};
    Argument<string>    log_dir     {this, "log_dir", "log dir", "/var/log/flame"};
    Argument<string>    log_level   {this, "log_level", 
        "log level. {PRINT, TRACE, DEBUG, INFO, WARN, ERROR, WRONG, CRITICAL, DEAD}", "INFO"};

    Argument<string>    csd_name    {this, 'n', "csd_name", "CSD Name which used to registering.", "csd"};

    // Switch
    Switch  console_log {this, "console_log", "print log in console"};
    Switch  force_format{this, "force_format", "force reformat the device"};

    HelpAction help {this};
}; // class CsdCli

/**
 * CSD
 */
class CSD final {
public:
    CSD(CsdContext* cct) : cct_(cct) {}
    ~CSD() { down(); }

    int init(CsdCli* csd_cli);
    int run();
    void down();

private:
    unique_ptr<CsdContext> cct_;
    unique_ptr<CsdAdminServer> server_;
    unique_ptr<InternalClientFoctory> internal_client_foctory_;

    /**
     * 配置项
     */
    node_addr_t cfg_mgr_addr_;
    node_addr_t cfg_admin_addr_;
    node_addr_t cfg_io_addr_;
    string      cfg_chunkstore_url_;
    uint64_t    cfg_heart_beat_cycle_ms_;
    string      cfg_log_dir_;
    string      cfg_log_level_;
    string      cfg_csd_name_;

    int read_config(CsdCli* csd_cli);

    bool init_log(bool with_console);
    bool init_mgr();
    bool init_chunkstore(bool force_format);
    bool init_server();

    bool csd_register();
}; // class CSD

int CSD::init(CsdCli* csd_cli) {
    // 读取配置信息
    int r;
    if ((r = read_config(csd_cli)) != 0) {
        cct_->log()->lerror("read config faild");
        return 1;
    }

    // 初始化log
    if (!init_log(csd_cli->console_log)) {
        cct_->log()->lerror("init log faild");
        return 2;
    }

    // 初始化MGR
    if (!init_mgr()) {
        cct_->log()->lerror("init mgr faild");
        return 3;
    }

    // 初始化ChunkStore
    if (!init_chunkstore(csd_cli->force_format)) {
        cct_->log()->lerror("init chunkstore faild");
        return 4;
    }

    // 初始化CsdServer
    if (!init_server()) {
        cct_->log()->lerror("init server faild");
        return 5;
    }

    return 0;
}

int CSD::run() {
    if (!server_) return -1;
    cct_->log()->linfo("start service");
    return server_->run();
}

void CSD::down() {
    if (cct_->cs())
        cct_->cs()->dev_unmount();
}

int CSD::read_config(CsdCli* csd_cli) {
    FlameConfig* config = cct_->config();
    
    /**
     * cfg_mgr_addr_
     */
    string mgr_addr;
    if (csd_cli->mgr_addr.done() && !csd_cli->mgr_addr.get().empty()) {
        mgr_addr = csd_cli->mgr_addr;
    } else if (config->has_key(CFG_CSD_MGR_ADDR)) {
        mgr_addr = config->get(CFG_CSD_MGR_ADDR, "");
    } else {
        cct_->log()->lerror("config[ mgr_addr ] not found");
        return 1;
    }

    if (!string_parse(cfg_mgr_addr_, mgr_addr)) {
        cct_->log()->lerror("invalid config[ mgr_addr ]");
        return 1;
    }

    /**
     * cfg_admin_addr_
     */
    string admin_addr;
    if (csd_cli->admin_addr.done() && !csd_cli->admin_addr.get().empty()) {
        admin_addr = csd_cli->admin_addr;
    } else if (config->has_key(CFG_CSD_ADMIN_ADDR)) {
        admin_addr = config->get(CFG_CSD_ADMIN_ADDR, "");
    } else {
        cct_->log()->lerror("config[ admin_addr ] not found");
        return 2;
    }

    if (!string_parse(cfg_admin_addr_, admin_addr)) {
        cct_->log()->lerror("invalid config[ admin_addr ]");
        return 2;
    }

    /**
     * cfg_io_addr_
     */
    string io_addr;
    if (csd_cli->io_addr.done() && !csd_cli->io_addr.get().empty()) {
        io_addr = csd_cli->io_addr;
    } else if (config->has_key(CFG_CSD_IO_ADDR)) {
        io_addr = config->get(CFG_CSD_IO_ADDR, "");
    } else {
        cct_->log()->lerror("config[ io_addr ] not found");
        return 3;
    }

    if (!string_parse(cfg_io_addr_, io_addr)) {
        cct_->log()->lerror("invalid config[ io_addr ]");
        return 3;
    }

    /**
     * cfg_chunkstore_url_
     */
    if (csd_cli->chunkstore.done() && !csd_cli->chunkstore.get().empty()) {
        cfg_chunkstore_url_ = csd_cli->chunkstore;
    } else if (config->has_key(CFG_CSD_CHUNKSTORE)) {
        cfg_chunkstore_url_ = config->get(CFG_CSD_CHUNKSTORE, "");
    } else {
        cct_->log()->lerror("config[ chunkstore ] not found");
        return 4;
    }

    /**
     * cfg_heart_beat_cycle_ms_
     */
    if (csd_cli->heart_beat.done() && !csd_cli->heart_beat.get()) {
        cfg_heart_beat_cycle_ms_ = csd_cli->heart_beat;
    } else if (config->has_key(CFG_CSD_HEART_BEAT_CYCLE)) {
        if (!string_parse(cfg_heart_beat_cycle_ms_, config->get(CFG_CSD_HEART_BEAT_CYCLE, ""))) {
            cct_->log()->lerror("invalid config[ heart_beat_cycle ]");
            return 5;
        }
    } else {
        cct_->log()->lerror("config[ heart_beat_cycle ] not found");
        return 5;
    }

    /**
     * cfg_log_dir_
     */
    string log_dir;
    if (csd_cli->log_dir.done() && !csd_cli->log_dir.get().empty()) {
        cfg_log_dir_ = csd_cli->log_dir;
    } else if (config->has_key(CFG_CSD_LOG_DIR)) {
        cfg_log_dir_ = config->get(CFG_CSD_LOG_DIR, "");
    } else {
        cct_->log()->lerror("config[ log_dir ] not found");
        return 6;
    }

    /**
     * cfg_log_level_
     */
    string log_level;
    if (csd_cli->log_level.done() && !csd_cli->log_level.get().empty()) {
        cfg_log_level_ = csd_cli->log_level;
    } else if (config->has_key(CFG_CSD_LOG_LEVEL)) {
        cfg_log_level_ = config->get(CFG_CSD_LOG_LEVEL, "");
    } else {
        cct_->log()->lerror("config[ log_level ] not found");
        return 7;
    }

    /**
     * cfg_csd_name_
     */
    if (csd_cli->csd_name.done() && !csd_cli->csd_name.get().empty()) {
        cfg_csd_name_ = csd_cli->log_level;
    } else if (config->has_key(CFG_CSD_NAME)) {
        cfg_csd_name_ = config->get(CFG_CSD_NAME, "");
    } else {
        cct_->log()->lerror("config[ log_level ] not found");
        return 8;
    }

    return 0;
}

bool CSD::init_log(bool with_console) {
    if (!cct_->fct()->init_log(cfg_log_dir_, cfg_log_level_, "csd")) {
        cct_->fct()->log()->lerror("init log faild");
        return false;
    }
    cct_->fct()->log()->set_with_console(with_console);
    return true;
}

bool CSD::init_mgr() {
    internal_client_foctory_.reset(new InternalClientFoctoryImpl(cct_->fct()));

    auto stub = internal_client_foctory_->make_internal_client(convert2string(cfg_mgr_addr_));
    if (stub.get() == nullptr) {
        cct_->log()->lerror("create mgr stub faild");
        return false;
    }

    cct_->mgr_stub(stub);

    return true;
}

bool CSD::init_chunkstore(bool force_format) {
    auto cs = create_chunkstore(cct_->fct(), cfg_chunkstore_url_);
    if (!cs) {
        cct_->log()->lerror("create metastore with url(%s) faild", cfg_chunkstore_url_.c_str());
        return false;
    }

    int r = cs->dev_check();
    switch (r) {
    case ChunkStore::DevStatus::NONE:
        cct_->log()->lerror("device not existed");
        return false;
    case ChunkStore::DevStatus::UNKNOWN:
        cct_->log()->lwarn("unknown device format");
        if (!force_format)
            return false;
        break;
    case ChunkStore::DevStatus::CLT_OUT:
        cct_->log()->lwarn("the divice belong to other cluster");
        if (!force_format)
            return false;
        break;
    case ChunkStore::DevStatus::CLT_IN:
        break;
    }

    if (force_format) {
        r = cs->dev_format();
        if (r != 0) {
            cct_->log()->lerror("format device faild");
            return false;
        }
    }

    if ((r = cs->dev_mount()) != 0) {
        cct_->log()->lerror("mount device faild (%d)", r);
        return false;
    }

    cct_->cs(cs);

    return true;
}

bool CSD::init_server() {
    server_.reset(new CsdAdminServer(cct_.get(), convert2string(cfg_admin_addr_)));

    cct_->timer(shared_ptr<TimerWorker>(new TimerWorker()));
    return true;
}

bool CSD::csd_register() {
    cs_info_t info;

    int r;
    if ((r = cct_->cs()->get_info(info)) != RC_SUCCESS) {
        cct_->log()->lerror("get chunkstore info faild");
        return false;
    }

    if (info.id == 0) {
        // 尚未注册，后期还应该检查cluster信息是否正确
        reg_attr_t reg_attr;
        reg_attr.csd_name = cfg_csd_name_;
        reg_attr.size = info.size;
        reg_attr.io_addr = cfg_io_addr_;
        reg_attr.admin_addr = cfg_admin_addr_;
        reg_attr.stat = CSD_STAT_PAUSE; // 注册完后统一状态为PAUSE
        
        reg_res_t  reg_res;
        if ((r = cct_->mgr_stub()->register_csd(reg_res, reg_attr)) != RC_SUCCESS) {
            cct_->log()->lerror("(mgr_stub) register csd faild, code(%d)", r);
            return false;
        }

        info.id = reg_res.csd_id;
        if ((r = cct_->cs()->set_info(info)) != RC_SUCCESS) {
            cct_->log()->lerror("");
            return false;
        }
    } else {
        cct_->log()->linfo("csd (%llu) have been registered", info.id);
    }

    return true;
}

} // namespace flame

int main(int argc, char** argv) {
    CsdCli* csd_cli = new CsdCli();
    int r = csd_cli->parser(argc,  argv);
    if (r != CmdRetCode::SUCCESS) {
        csd_cli->print_error();
        return r;
    } else if (csd_cli->help.done()) {
        csd_cli->print_help();
        return 0;
    }

    // 获取全局上下文
    FlameContext* fct = FlameContext::get_context();

    if (!fct->init_config(csd_cli->config_path)) {
        fct->log()->lerror("init config faild");
        exit(-1);
    }

    // 创建CSD上下文
    CsdContext* cct = new CsdContext(fct);

    // 创建CSD主程序
    CSD csd(cct);

    // 初始化CSD，CSD在初始化完成后保证不再使用CsdCli
    if (csd.init(csd_cli) != 0)
        exit(-1);
    
    // 释放CsdCli资源
    delete csd_cli;

    // 运行CSD主程序
    return csd.run();
}