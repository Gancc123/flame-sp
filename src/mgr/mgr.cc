#include "common/context.h"
#include "common/cmdline.h"
#include "metastore/ms.h"

#include "mgr/mgr_context.h"
#include "mgr/mgr_server.h"
#include "mgr/config_mgr.h"
#include "mgr/csdm/csd_mgmt.h"

#include <grpcpp/grpcpp.h>
#include "service/flame_service.h"
#include "service/internal_service.h"
#include "service/csds_client.h"

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
    Argument<string>    addr        {this, 'a', "address", "MGR listen port", "0.0.0.0:6666"};
    Argument<string>    metastore   {this, "metastore", "MetaStore url", ""};
    Argument<string>    log_dir     {this, "log_dir", "log dir", "/var/log/flame"};
    Argument<string>    log_level   {this, "log_level", 
        "log level. {PRINT, TRACE, DEBUG, INFO, WARN, ERROR, WRONG, CRITICAL, DEAD}", "INFO"};

    HelpAction help {this};
}; // class MgrCli

/**
 * Manager (MGR)
 */
class Manager final {
public:
    Manager(MgrContext* mct) : mct_(mct) {}
    ~Manager() {}

    int init(MgrCli* mgr_cli);
    int run();

private:
    MgrContext* mct_;
    MgrServer* server_;

    bool init_metastore(MgrCli* mgr_cli);
    bool init_server(MgrCli* mgr_cli);
    bool init_csdm(MgrCli* mgr_cli);
};

int Manager::init(MgrCli* mgr_cli) {
    // 初始化MetaStore
    if (!init_metastore(mgr_cli)) {
        mct_->log()->lerror("init metastore faild");
        return 1;
    }
    
    // 初始化MgrServer
    if (!init_server(mgr_cli)) {
        mct_->log()->lerror("init server faild");
        return 2;
    }

    // 初始化CsdManager
    if (!init_csdm(mgr_cli)) {
        mct_->log()->lerror("init csd manager faild");
        return 3;
    }

    return 0;
}

int Manager::run() {
    if (!server_) return -1;
    mct_->log()->linfo("start service");
    return server_->run();
}

bool Manager::init_metastore(MgrCli* mgr_cli) {
    FlameConfig* config = mct_->config();
    string ms_url;
    if (mgr_cli->metastore.done() && !mgr_cli->metastore.get().empty()) {
        ms_url = mgr_cli->metastore;
    } else if (config->has_key(CFG_MGR_METASTORE)) {
        ms_url = config->get(CFG_MGR_METASTORE, "");
    } else
        return false;
    
    auto ms = create_metastore(mct_->fct(), ms_url);
    if (!ms) {
        mct_->log()->lerror("create metastore with url(%s) faild", ms_url.c_str());
        return false;
    }
    mct_->ms(ms);
    return true;
}

bool Manager::init_server(MgrCli* mgr_cli) {
    FlameConfig* config = mct_->config();
    string addr;
    if (mgr_cli->addr.done() && !mgr_cli->addr.get().empty()) {
        addr = mgr_cli->addr;
    } else if (config->has_key(CFG_MGR_ADDR)) {
        addr = config->get(CFG_MGR_ADDR, "");
    } else 
        return false;
    
    server_ = new MgrServer(mct_, addr);
    return true;
}

bool Manager::init_csdm(MgrCli* mgr_cli) {
    shared_ptr<CsdsClientFoctory> csd_client_foctory(new CsdsClientFoctoryImpl(mct_->fct()));

    shared_ptr<CsdManager> csdm(new CsdManager(
        mct_->fct(),    // Flame上下文文
        mct_->ms(),     // MetaStore
        csd_client_foctory
    ));

    mct_->csdm(csdm);
    return true;
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

    if (!fct->init_log(
        mgr_cli->log_dir.done() ? mgr_cli->log_dir.get() : "", 
        mgr_cli->log_level, 
        "mgr"
    )) {
        fct->log()->lerror("init log faild");
        exit(-1);
    }

    // 创建MGR上下文
    MgrContext* mct = new MgrContext(fct);

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