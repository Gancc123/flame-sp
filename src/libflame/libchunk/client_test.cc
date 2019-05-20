#include <unistd.h>
#include <cstdio>

#include "libflame/libchunk/libchunk.h"
#include "include/csdc.h"
#include "log_libchunk.h"

#define CFG_PATH "flame_client.cfg"
using namespace flame;

void cb_func(const Response& res, void* arg){
    return ;
}

int main(){
    FlameContext *flame_context = FlameContext::get_context();
    if(!flame_context->init_config(CFG_PATH)){
        clog("init config failed.");
        return -1;
    }
    if(!flame_context->init_log("", "TRACE", "client")){
        clog("init log failed.");
        return -1;
    }

    std::shared_ptr<CmdClientStubImpl> cmd_client_stub = CmdClientStubImpl::create_stub("127.0.0.1", 7778);
    RdmaWorkRequest* rdma_work_request = cmd_client_stub->get_request();
    flame_context->log()->ltrace("CmdClientStub created!");

    msg::ib::RdmaBufferAllocator* allocator = msg::Stack::get_rdma_stack()->get_rdma_allocator();
    msg::ib::RdmaBuffer* buf = allocator->alloc(1 << 22); //4MB

    MemoryArea* memory = new MemoryAreaImpl((uint64_t)buf->buffer(), (uint32_t)buf->size(), buf->rkey(), 1);

    flame_context->log()->ltrace("(uint64_t)buf->buffer() = %llu",(uint64_t)buf->buffer());
    flame_context->log()->ltrace("(uint32_t)buf->size() = %llu",(uint32_t)buf->size());
    flame_context->log()->ltrace(" buf->rkey() = %llu", buf->rkey());
    
    cmd_t* cmd = (cmd_t *)rdma_work_request->command;
    ChunkReadCmd* read_cmd = new ChunkReadCmd(cmd, 0, 0, 10, *memory); 
    // read_cmd->copy(cmd);
    // read_cmd->set_cq(2, 20);
    flame_context->log()->ltrace("ChunkReadCmd created!");
    int i = 1;
    cmd_client_stub->submit(*rdma_work_request, &cb_func, (void *)&i);
    
    std::getchar();
    char* mm = (char*)buf->buffer();
    std::cout << mm[0] << mm[1] << mm[2] << mm[3] << std::endl;
    flame_context->log()->ltrace("Start to exit!");

    return 0;
}