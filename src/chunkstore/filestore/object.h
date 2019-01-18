
#ifndef OBJECTSTORE_OBJECT_H
#define OBJECTSTORE_OBJECT_H

#include <iostream>
#include <string>
#include <atomic>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/file.h>
#include <fcntl.h>

#include "common/context.h"
#include "chunkstore/filestore/chunkutil.h"
#include "chunkstore/filestore/filestore.h"

namespace flame{

enum ObjectState {
    OBJECT_LOADING = 0,
    OBJECT_OPENED,
    OBJECT_CLOSED,
    OBJECT_MIGRATE
};

class Object {
//Object的元数据都是非持久化的；
//如果未来出现了需要持久化的元数据信息，则利用文件的扩展属性存取

private:
    FileStore *filestore;
    FileChunk* chunk;

    uint64_t cid;
    uint64_t oid;
    uint64_t size;

    uint64_t open_time;     //记录该object被open的时间
    uint64_t access_time;   //记录该object最近一次被存取的时间

    ObjectState state;

    int fd;
    int open_flags;

    //记录该object自open开始一段时间内的读写次数，用于关闭某些不常用的object，不持久化
    std::atomic<uint64_t> read_counter;
    std::atomic<uint64_t> write_counter;

public:
    FlameContext *fct;
    Object(FlameContext *_fct, FileStore *_filestore, 
                    FileChunk *_chunk, uint64_t id):fct(_fct), filestore(_filestore), 
                    chunk(_chunk), oid(id), read_counter(0), write_counter(0) {
        open_flags = O_RDWR;
    }

    virtual int init();
    virtual int get_fd() const;
    virtual bool is_open() const;
    virtual ObjectState get_state() const;
    virtual uint64_t get_oid() const;

    virtual void update_access_time();
    virtual uint64_t get_access_time() const;

    virtual void write_counter_add(uint64_t increment);
    virtual void read_counter_add(uint64_t increment);
    virtual uint64_t get_read_counter();
    virtual uint64_t get_write_counter();

    virtual ~Object() {
        close(fd);
    }
};

}

#endif