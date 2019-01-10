#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/file.h>
#include <fcntl.h>
#include <libaio.h>
#include <strings.h>
#include <inttypes.h>

#include "common/context.h"
#include "chunkstore/chunkstore.h"
#include "chunkstore/filestore/filestore.h"
#include "chunkstore/filestore/object.h"
#include "chunkstore/filestore/chunkstorepriv.h"
#include "util/utime.h"
#include "chunkstore/log_cs.h"

using namespace flame;

int Object::init() {
    char obj_path[512];
    snprintf(obj_path, sizeof(obj_path), "%s/%s/%" PRIx64 "/%" PRIx64, filestore->get_base_path(), filestore->get_data_path(), chunk->get_chunk_id(), oid);

#ifdef DEBUG
    std::cout << "obj_path = " << obj_path << std::endl;
#endif
    if(filestore->get_io_mode() == CHUNKSTORE_IO_MODE_ASYNC) {
        open_flags |= O_DIRECT;
    }

    state = OBJECT_LOADING;
    fd = open(obj_path, open_flags, util::def_fmode);
    if(fd < 0) {
        fct->log()->error("open object file failed: %s", strerror(errno));
        return -1;
    }
    open_time = (unsigned long long)time(NULL);

#ifdef DEBUG
    std::cout << "objct fd = " << fd << std::endl;
#endif
    state = OBJECT_OPENED;
    return 0;
}

bool Object::is_open() const {
    return this->get_state() == OBJECT_OPENED;
}

ObjectState Object::get_state() const {
    return this->state;
}

int Object::get_fd() const {
    if(is_open())
        return fd;
    else {
        fct->log()->error("object is not open");
        return -1;
    }
}

void Object::write_counter_add(uint64_t increment) {
    write_counter.fetch_add(increment, std::memory_order_relaxed);
}

void Object::read_counter_add(uint64_t increment) { 
    read_counter.fetch_add(increment, std::memory_order_relaxed);
}

uint64_t Object::get_read_counter() {
    return read_counter.load(std::memory_order_relaxed);
}

uint64_t Object::get_write_counter() {
    return write_counter.load(std::memory_order_relaxed);
}

uint64_t Object::get_oid() const {
    return this->oid;
}

void Object::update_access_time() {
    access_time = (uint64_t)time(NULL);
}

uint64_t Object::get_access_time() const {
    return access_time;
}
