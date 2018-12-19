#include "common/context.h"
#include "common/cmdline.h"
#include "chunkstore/cs.h"

#include "csd/csd_context.h"
#include "csd/csd_admin.h"
#include "csd/config_csd.h"
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
    Argument<string>    admin_addr  {this, 'a', "admin_addr", "CSD admin listen port", "0.0.0.0:6666"};
    Argument<string>    io_addr     {this, 'i', "io_addr", "CSD IO listen port", "0.0.0.0:9999"};
    Argument<string>    chunkstore  {this, "chunkstore", "ChunkStore url", ""};
    Argument<string>    log_dir     {this, "log_dir", "log dir", "/var/log/flame"};
    Argument<string>    log_level   {this, "log_level", 
        "log level. {PRINT, TRACE, DEBUG, INFO, WARN, ERROR, WRONG, CRITICAL, DEAD}", "INFO"};

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
    CsdContext* cct_;
    CsdAdminServer* server_;

    bool init_chunkstore(CsdCli* csd_cli);
    bool init_server(CsdCli* csd_cli);
}; // class CSD

int CSD::init(CsdCli* csd_cli) {
    // 初始化ChunkStore
    if (!init_chunkstore(csd_cli)) {
        cct_->log()->lerror("init chunkstore faild");
        return 1;
    }

    // 初始化CsdServer
    if (!init_server(csd_cli)) {
        cct_->log()->lerror("init server faild");
        return 2;
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

bool CSD::init_chunkstore(CsdCli* csd_cli) {
    FlameConfig* config = cct_->config();
    string cs_url;
    if (csd_cli->chunkstore.done() && !csd_cli->chunkstore.get().empty()) {
        cs_url = csd_cli->chunkstore;
    } else if (config->has_key(CFG_CSD_CHUNKSTORE)) {
        cs_url = config->get(CFG_CSD_CHUNKSTORE, "");
    } else
        return false;
    
    auto cs = create_chunkstore(cct_->fct(), cs_url);
    if (!cs) {
        cct_->log()->lerror("create metastore with url(%s) faild", cs_url.c_str());
        return false;
    }

    int r = cs->dev_check();
    switch (r) {
    case ChunkStore::DevStatus::NONE:
        cct_->log()->lerror("device not existed");
        return false;
    case ChunkStore::DevStatus::UNKNOWN:
        cct_->log()->lwarn("unknown device format");
        if (!csd_cli->force_format)
            return false;
        break;
    case ChunkStore::DevStatus::CLT_OUT:
        cct_->log()->lwarn("the divice belong to other cluster");
        if (!csd_cli->force_format)
            return false;
        break;
    case ChunkStore::DevStatus::CLT_IN:
        break;
    }

    if (csd_cli->force_format) {
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

bool CSD::init_server(CsdCli* csd_cli) {
    FlameConfig* config = cct_->config();
    string admin_addr;
    if (csd_cli->admin_addr.done() && !csd_cli->admin_addr.get().empty()) {
        admin_addr = csd_cli->admin_addr;
    } else if (config->has_key(CFG_CSD_ADMIN_ADDR)) {
        admin_addr = config->get(CFG_CSD_ADMIN_ADDR, "");
    } else 
        return false;
    
    server_ = new CsdAdminServer(cct_, admin_addr);
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

    if (!fct->init_log(
        csd_cli->log_dir.done() ? csd_cli->log_dir.get() : "", 
        csd_cli->log_level, 
        "csd"
    )) {
        fct->log()->lerror("init log faild");
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