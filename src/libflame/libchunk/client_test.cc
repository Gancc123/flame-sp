#include <unistd.h>
#include <cstdio>

#include "libflame/libchunk/libchunk.h"
#include "include/csdc.h"
#include "log_libchunk.h"

#define CFG_PATH "flame_client.cfg"
using namespace flame;

void cb_func(const Response& res, void* arg){
    char* mm = (char*)arg;
    std::cout << mm[0] << mm[1] << mm[2] << std::endl; 
    std::cout << "111" << std::endl;
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
    msg::ib::RdmaBufferAllocator* allocator = msg::Stack::get_rdma_stack()->get_rdma_allocator();
    msg::ib::RdmaBuffer* buf = allocator->alloc(1 << 22); //4MB

    MemoryArea* memory = new MemoryAreaImpl((uint64_t)buf->buffer(), (uint32_t)buf->size(), buf->rkey(), 1);
    
    cmd_t* cmd = (cmd_t *)rdma_work_request->command;
    ChunkReadCmd* read_cmd = new ChunkReadCmd(cmd, 0, 0, 10, *memory); 

    cmd_client_stub->submit(*rdma_work_request, &cb_func, (void *)buf->buffer());
    
    std::getchar();
    // char* mm = (char*)buf->buffer();
    // std::cout << mm[0] << mm[1] << mm[2] << mm[3] << std::endl;
    flame_context->log()->ltrace("Start to exit!");

    return 0;
}