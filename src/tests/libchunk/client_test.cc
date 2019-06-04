#include <unistd.h>
#include <cstdio>

#include "libflame/libchunk/libchunk.h"
#include "include/csdc.h"
#include "libflame/libchunk/log_libchunk.h"

#define CFG_PATH "flame_client.cfg"
using namespace flame;

void cb_func(const Response& res, void* arg){
    char* mm = (char*)arg;
    std::cout << mm[0] << mm[1] << mm[2] << std::endl; 
    std::cout << "111" << std::endl;
    return ;
}

void cb_func2(const Response& res, void* arg){
    std::cout << "222" << std::endl;
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
    /* 无inline的读取chunk的操作 */
    RdmaWorkRequest* rdma_work_request_read = cmd_client_stub->get_request();
    msg::ib::RdmaBufferAllocator* allocator = msg::Stack::get_rdma_stack()->get_rdma_allocator();
    msg::ib::RdmaBuffer* buf_read = allocator->alloc(1 << 22); //4MB
    MemoryArea* memory_read = new MemoryAreaImpl((uint64_t)buf_read->buffer(), (uint32_t)buf_read->size(), buf_read->rkey(), 1);
    cmd_t* cmd_read = (cmd_t *)rdma_work_request_read->command;
    ChunkReadCmd* read_cmd = new ChunkReadCmd(cmd_read, 0, 0, 8192, *memory_read); 
    cmd_client_stub->submit(*rdma_work_request_read, &cb_func, (void *)buf_read->buffer());

    /* 无inline的写入chunk的操作 */
    RdmaWorkRequest* rdma_work_request_write= cmd_client_stub->get_request();
    msg::ib::RdmaBuffer* buf_write = allocator->alloc(1 << 22); //4MB
    char a[8] = "1234567";
    memcpy(buf_write->buffer(), a, 8);
    MemoryArea* memory_write = new MemoryAreaImpl((uint64_t)buf_write->buffer(), (uint32_t)buf_write->size(), buf_write->rkey(), 1);
    cmd_t* cmd_write = (cmd_t *)rdma_work_request_write->command;
    ChunkWriteCmd* write_cmd = new ChunkWriteCmd(cmd_write, 0, 0, 10, *memory_write, 0); 
    cmd_client_stub->submit(*rdma_work_request_write, &cb_func2, nullptr);  //**写成功是不需要第三个参数的，只需要返回response

    /* 带inline的读取chunk的操作 */
    RdmaWorkRequest* rdma_work_request_read_inline = cmd_client_stub->get_request();
    cmd_t* cmd_read_inline = (cmd_t *)rdma_work_request_read_inline->command;
    ChunkReadCmd* read_cmd_inline = new ChunkReadCmd(cmd_read_inline, 0, 0, 10); 
    cmd_client_stub->submit(*rdma_work_request_read_inline, &cb_func, nullptr); //**这里第三个参数写nullptr但是在内部调用时因为时inline_read，会自动将参数定位到recv的rdma buffer上

    /* 带inline的写入chunk的操作 */
    RdmaWorkRequest* rdma_work_request_write_inline = cmd_client_stub->get_request();
    cmd_t* cmd_write_inline = (cmd_t *)rdma_work_request_write_inline->command;
    msg::ib::RdmaBuffer* buf_write_inline = rdma_work_request_write_inline->get_data_buf();
    char b[8] = "9876543";
    memcpy(buf_write_inline->buffer(), b, 8);
    MemoryArea* memory_write_inline = new MemoryAreaImpl((uint64_t)buf_write_inline->buffer(),\
                     (uint32_t)buf_write_inline->size(), buf_write_inline->rkey(), 1);
    ChunkWriteCmd* write_cmd_inline = new ChunkWriteCmd(cmd_write_inline, 0, 0, 10, *memory_write_inline, 1); 
    cmd_client_stub->submit(*rdma_work_request_write_inline, &cb_func2, nullptr); //**这里第三个参数写nullptr但是在内部调用时因为时inline_read，会自动将参数定位到recv的rdma buffer上

    std::getchar();
    flame_context->log()->ltrace("Start to exit!");

    return 0;
}