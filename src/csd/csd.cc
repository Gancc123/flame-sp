#include "common/context.h"
#include "common/cmdline.h"
#include "chunkstore/cs.h"

#include "csd_context.h"

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

    HelpAction help {this};
}; // class CsdCli

/**
 * CSD
 */
class CSD final {
public:
    CSD(CsdContext* cct)

private:
    CsdContext* cct_;
    
}; // class CSD


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
        fct->log()->error("init config faild");
        exit(-1);
    }

    if (!fct->init_log(
        csd_cli->log_dir.done() ? csd_cli->log_dir.get() : "", 
        csd_cli->log_level, 
        "csd"
    )) {
        fct->log()->error("init log faild");
        exit(-1);
    }

    CsdContext* cct = new CsdContext(fct);


    return 0;
}