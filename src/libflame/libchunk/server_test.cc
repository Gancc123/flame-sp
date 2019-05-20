

#include <unistd.h>
#include <cstdio>

#include "libflame/libchunk/libchunk.h"
#include "log_libchunk.h"
#include "include/cmd.h"
#include "include/csdc.h"
#include "chunk_cmd_service.h"

#define CFG_PATH "flame_mgr.cfg"
using namespace flame;

int main(){
    FlameContext *flame_context = FlameContext::get_context();
    if(!flame_context->init_config(CFG_PATH)){
        clog("init config failed.");
        return -1;
    }
    if(!flame_context->init_log("", "TRACE", "mgr")){
        clog("init log failed.");
        return -1;
    }
    CmdServiceMapper *cmd_service_mapper = CmdServiceMapper::get_cmd_service_mapper();
    cmd_service_mapper->register_service(CMD_CLS_IO_CHK, CMD_CHK_IO_READ, new ReadCmdService());
    // cmd_service_mapper->register_service(CMD_CLS_IO_CHK, CMD_CHK_IO_WRITE, new WriteCmdService());

    CmdServerStubImpl* cmd_sever_stub = new CmdServerStubImpl(flame_context);

    flame_context->log()->ltrace("CmdSeverStub created!");

    std::getchar();

    flame_context->log()->ltrace("Start to exit!");


    return 0;
}