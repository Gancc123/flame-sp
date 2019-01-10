#include <string>
#include <iostream>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstdint>

#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <libgen.h>
#include <dirent.h>
#include <libaio.h>
#include <inttypes.h>

#include "chunkstore/chunkstore.h"
#include "chunkstore/chunkmap.h"

#include "chunkstore/filestore/filestore.h"
#include "chunkstore/filestore/chunkutil.h"
#include "chunkstore/filestore/object.h"
#include "chunkstore/filestore/chunkstorepriv.h"
#include "util/utime.h"
#include "chunkstore/log_cs.h"

using namespace flame;

struct fdSet {
    int filenums;
    int fd[0];
};

void FileChunk::set_chunk_id(uint64_t chunk_id) {
    chk_id = chunk_id;
}

uint32_t FileChunk::stat() const {
    return chunk_stat;
}

uint64_t FileChunk::vol_id() const {
    return volume_id;
}

uint32_t FileChunk::vol_index() const {
    return volume_index;
}

uint32_t FileChunk::spolicy() const {
    return chunk_spolicy;
}

uint64_t FileChunk::size() const {
    return chunk_size.load(std::memory_order_relaxed);
}

uint64_t FileChunk::used() const {
    return chunk_used.load(std::memory_order_relaxed);
}

bool FileChunk::is_preallocated() const {
    return flags & ChunkFlags::PREALLOC;
}

int FileChunk::chunk_serial_base(struct chunk_base_descriptor *desc) {
    if(desc == nullptr)
        return -1;

    desc->type = CHUNK_MD_TYPE_BASE;
    desc->length = sizeof(struct chunk_base_descriptor);

    desc->chunk_id = chk_id;
    desc->vol_id = volume_id;
    desc->index = volume_index;
    desc->stat = chunk_stat;
    desc->spolicy = chunk_spolicy;
    desc->flags = flags;
    desc->size = chunk_size.load(std::memory_order_relaxed);
    desc->used = chunk_used.load(std::memory_order_relaxed);
    desc->create_time = ctime;
    desc->dst_id = dst_id;
    desc->dst_ctime = dst_ctime;
    desc->xattr_num = xattr_num.load(std::memory_order_relaxed);

    desc->block_size_shift    = block_size_shift;
    desc->chunk_read_counter  = read_counter.load(std::memory_order_relaxed);
    desc->chunk_write_counter = write_counter.load(std::memory_order_relaxed);

    return 0;
}

int FileChunk::chunk_serial_xattrs(struct chunk_xattr_descriptor **chunk_xattr_descs) {
    std::list<struct chunk_xattr>::iterator iter;
    int i = 0;
    uint32_t new_len = 0;
    for(i = 0, iter = xattr_list.begin(); iter != xattr_list.end(); i++, iter++) {
        uint32_t name_len = (iter->name).length();
        uint32_t value_len = (iter->value).length();
        uint32_t new_len = sizeof(struct chunk_xattr_descriptor) + name_len + value_len + 1;

        chunk_xattr_descs[i] = (struct chunk_xattr_descriptor *)malloc(new_len);
        if(chunk_xattr_descs[i] == nullptr) {
            fct_->log()->error("malloc failed: %s", strerror(errno));
            return -1;
        }

        chunk_xattr_descs[i]->type = CHUNK_MD_TYPE_XATTR;
        chunk_xattr_descs[i]->length = new_len;
        chunk_xattr_descs[i]->name_length = name_len;
        chunk_xattr_descs[i]->value_length = value_len;
        memcpy((char *)(chunk_xattr_descs[i]->key_value), (iter->name).c_str(), name_len);
        memcpy((char *)(chunk_xattr_descs[i]->key_value + name_len), (iter->value).c_str(), value_len);
        chunk_xattr_descs[i]->key_value[name_len + value_len + 1] = '\0';
    }

    return 0;
}

int FileChunk::store(const char *store_path) {
    int ret;
    int fd;
    int open_flag = O_RDWR | O_CREAT | O_EXCL;
    off_t offset = sizeof(struct chunk_base_descriptor);

    char temp_path[256];
    //避免将数据写入到原来的元数据文件，导致数据破坏。
    sprintf(temp_path, "%s.temp", store_path);

    fd = open(temp_path, open_flag, util::def_fmode);
    if(fd < 0) {
        return CHUNK_OP_STORE_ERR;
    }

    struct chunk_base_descriptor cdesc;
    struct chunk_xattr_descriptor* cxdescs[this->xattr_num];

    if(chunk_serial_base(&cdesc) != 0)
        goto close_fd;

    if(chunk_serial_xattrs(cxdescs) != 0) 
        goto close_fd;

    ret = pwrite(fd, &cdesc, sizeof(cdesc), 0);
    if(ret < sizeof(struct chunk_base_descriptor)) {
        fct_->log()->error("persist chunk base info failed: %s", strerror(errno));
        goto close_fd;
    }

    //这里有个问题，即把扩展属性头和扩展属性的kv分开存储，导致了一个扩展属性就需要
    //两次pwrite来存储，这导致了两次系统调用，我们考虑是否通过一次系统调用完成所有的扩展属性的存储
    //这个问题对于load操作而言，一样存在。
    for(int i = 0; i < this->xattr_num; i++) {
        ret = pwrite(fd, cxdescs[i], sizeof(struct chunk_xattr_descriptor), offset);
        if(ret < sizeof(struct chunk_xattr_descriptor)) {
            fct_->log()->error("persist chunk xattr info failed: %s", strerror(errno));
            goto close_fd;
        }

        offset += sizeof(struct chunk_xattr_descriptor);
        ret = pwrite(fd, cxdescs[i]->key_value, cxdescs[i]->name_length + cxdescs[i]->value_length, offset);
        if(ret < cxdescs[i]->name_length + cxdescs[i]->value_length) {
            fct_->log()->error("persist chunk xattr info failed: %s", strerror(errno));
            goto close_fd;
        }
        offset += (cxdescs[i]->name_length + cxdescs[i]->value_length);
    }

    close(fd);

    //将chunk_size等一些信息存储到chunk元数据文件的扩展属性中
    //这样做的主要目的是为了能够在不打开chunk的情况下，快速获取chunk的大小
    char chunk_size_str[64];
    sprintf(chunk_size_str, "%" PRIx64, chunk_size.load(std::memory_order_relaxed));

    if(util::set_xattr(temp_path, "user.chunk_size", 
                chunk_size_str, sizeof(chunk_size_str), 0) != 0) {
        fct_->log()->error("set chunk_size failed: %s", strerror(errno));
    }

    if(cxdescs != nullptr) {
        for(int i = 0; i < this->xattr_num; i++) {
            free(cxdescs[i]);
        }
    }

    //将新写入的临时文件替换旧文件，这样就可以保证元数据的一致性了。
    if(util::xrename(temp_path, store_path) != 0) {
        fct_->log()->error("rename failed: %s", strerror(errno));
        goto remove_temp; 
    }

    close(fd);
    return CHUNK_OP_SUCCESS;

close_fd:
    close(fd);

    if(cxdescs != nullptr) {
        for(int i = 0; i < this->xattr_num; i++) {
            free(cxdescs[i]);
        }
    }
remove_temp:
    util::xremove(temp_path);

    return CHUNK_OP_STORE_ERR;
}

int FileChunk::chunk_deserial_base(struct chunk_base_descriptor *chunk_base_desc) {
    if(chunk_base_desc == nullptr || chunk_base_desc->type != CHUNK_MD_TYPE_BASE) {
        return -1;
    }

    chk_id = chunk_base_desc->chunk_id;
    volume_id = chunk_base_desc->vol_id;
    volume_index = chunk_base_desc->index;
    chunk_stat = chunk_base_desc->stat;
    chunk_spolicy = chunk_base_desc->spolicy;
    flags = chunk_base_desc->flags;

    chunk_size.store(chunk_base_desc->size, std::memory_order_relaxed);
    chunk_used.store(chunk_base_desc->used, std::memory_order_relaxed);

    ctime = chunk_base_desc->create_time;
    dst_id = chunk_base_desc->dst_id;
    dst_ctime = chunk_base_desc->dst_ctime;

    xattr_num.store(chunk_base_desc->xattr_num, std::memory_order_relaxed);

    block_size_shift = chunk_base_desc->block_size_shift;
    read_counter.store(chunk_base_desc->chunk_read_counter, std::memory_order_relaxed);
    write_counter.store(chunk_base_desc->chunk_write_counter, std::memory_order_relaxed);

    return 0;
}

int FileChunk::chunk_deserial_xattr(struct chunk_xattr *xt, 
                        struct chunk_xattr_descriptor *cxdesc, 
                        char *kv, uint32_t index) {
    if(xt == nullptr || cxdesc == nullptr || cxdesc->type != CHUNK_MD_TYPE_XATTR) {
        return -1;
    }

    std::string name_str(&kv[0], &kv[cxdesc->name_length]);
    std::string value_str(&kv[cxdesc->name_length], &kv[cxdesc->name_length + cxdesc->value_length]);

    xt->index   = index;
    xt->name    = name_str;
    xt->value   = value_str;

    return 0;
}

int FileChunk::load(const char *load_path) {
    int ret;
    int fd;
    int open_flag = O_RDWR;

    off_t offset = 0;

    if((fd = open(load_path, open_flag, util::def_fmode)) < 0) {
        fct_->log()->error("load chunk failed: %s", strerror(errno));
        return CHUNK_OP_LOAD_ERR;
    }

    struct chunk_base_descriptor cbdesc;

    ret = pread(fd, &cbdesc, sizeof(struct chunk_base_descriptor), 0);
    if(ret < sizeof(struct chunk_base_descriptor)) {
        fct_->log()->error("load chunk base info failed: %s", strerror(errno));
        close(fd);
        return CHUNK_OP_LOAD_ERR;
    }

    if(chunk_deserial_base(&cbdesc) != 0) {
        fct_->log()->error("deserial chunk base info failed");
        close(fd);
        return CHUNK_OP_LOAD_ERR;
    }
    
    uint32_t xattr_nums = this->xattr_num.load(std::memory_order_relaxed);
    struct chunk_xattr_descriptor xattr_descs[xattr_nums];

    for(int i = 0; i < xattr_nums; i++) {
        struct chunk_xattr xattr;
        uint32_t kv_len = 0;
        char *kv = nullptr;
        ret = pread(fd, &xattr_descs[i], sizeof(struct chunk_xattr_descriptor), offset);
        if(ret < sizeof(struct chunk_xattr_descriptor)) {
            fct_->log()->error("load chunk xattr info faild: %s", strerror(errno));
            goto close_fd;
        }
        offset += sizeof(struct chunk_xattr_descriptor);

        kv_len = (xattr_descs[i].name_length + xattr_descs[i].value_length);
        kv = (char *)malloc(sizeof(char) * kv_len);

        if(kv == nullptr) {
            fct_->log()->error("load chunk xattr kv info failed: %s", strerror(errno));
            goto close_fd;
        }

        ret = pread(fd, kv, kv_len, offset);
        if(ret < kv_len) {
            fct_->log()->error("load chunk xattr kv failed: %s", strerror(errno));
            goto close_fd;
        }
        offset += kv_len;

        if(chunk_deserial_xattr(&xattr, &xattr_descs[i], kv, i) != 0) {
            fct_->log()->error("deserial chunk xattr info failed.");
            goto close_fd;
        }

        this->xattr_list.push_back(xattr);

        if(kv != nullptr) {
            free(kv);
            kv = nullptr;
        }
    }

    close(fd);
    return CHUNK_OP_SUCCESS;

close_fd:
    close(fd);
    return CHUNK_OP_LOAD_ERR;
}

uint64_t FileChunk::get_chunk_id() const {
    return chk_id;
}

uint64_t FileChunk::get_open_ref() {
    return open_ref.load(std::memory_order_relaxed);
}

void FileChunk::set_ref_counter(uint64_t counter) {
    open_ref.store(counter, std::memory_order_relaxed);
}

void FileChunk::ref_counter_sub(uint64_t increment) {
    open_ref.fetch_sub(increment, std::memory_order_relaxed);
}

void FileChunk::ref_counter_add(uint64_t increment) {
    open_ref.fetch_add(increment, std::memory_order_relaxed);
}

void FileChunk::write_counter_add(uint64_t increment) {
    write_counter.fetch_add(increment, std::memory_order_relaxed);
}

void FileChunk::read_counter_add(uint64_t increment) {
    read_counter.fetch_add(increment, std::memory_order_relaxed);
}

uint64_t FileChunk::get_write_counter() {
    return write_counter.load(std::memory_order_relaxed);
}

uint64_t FileChunk::get_read_counter() {
    return read_counter.load(std::memory_order_relaxed);
}

uint64_t FileChunk::get_object_size() {
    return (1ULL << block_size_shift);
}

uint64_t FileChunk::get_chunk_size() const {
    return chunk_size.load(std::memory_order_relaxed);
}

io_context_t *FileChunk::get_ioctx() {
    return &ioctx;
}

int FileChunk::get_efd() const {
    return efd;
}

uint32_t FileChunk::get_xattr_nums() {
    return xattr_num.load(std::memory_order_relaxed);
}

int FileChunk::init_chunk_async_env() {
    struct epoll_event epevent;

    bzero(&ioctx, sizeof(io_context_t));
    if(io_setup(1024, &ioctx) != 0) {
        fct_->log()->error("io setup failed.");
        return CHUNK_OP_INIT_ENV_ERR;
    }

    if((efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)) == -1) {
        fct_->log()->error("eventfd failed: %s", strerror(errno));
        return CHUNK_OP_INIT_ENV_ERR;
    }

    epevent.events = EPOLLIN | EPOLLOUT | EPOLLET;
    epevent.data.fd = efd;

    if(epoll_ctl(filestore->get_epfd(), EPOLL_CTL_ADD, efd, &epevent) != 0) {
        fct_->log()->error("epoll ctl failed: %s", strerror(errno));
        return CHUNK_OP_INIT_ENV_ERR;
    }

    return CHUNK_OP_SUCCESS;
}

int FileChunk::destroy_chunk_async_env() {
    if(epoll_ctl(filestore->get_epfd(), EPOLL_CTL_DEL, efd, NULL) != 0) {
        fct_->log()->error("epoll ctl failed: %s", strerror(errno));
        return CHUNK_OP_DESTROY_ENV_ERR;
    }

    if(close(efd) != 0) {
        fct_->log()->error("close efd failed: %s", strerror(errno));
        return CHUNK_OP_DESTROY_ENV_ERR;
    }

    if(io_destroy(ioctx) != 0) {
        fct_->log()->error("io_destroy failed.");
        return CHUNK_OP_DESTROY_ENV_ERR;
    }

    return CHUNK_OP_SUCCESS;
}

int FileChunk::create(const struct chunk_create_opts_t &opts) {
    char chunk_meta_obj[512];
    char chunk_data_dir[512];

    char *base_path;

    snprintf(chunk_meta_obj, sizeof(chunk_meta_obj), "%s/%s/%" PRIx64, filestore->get_base_path(), filestore->get_meta_path(), chk_id);
    snprintf(chunk_data_dir, sizeof(chunk_data_dir), "%s/%s/%" PRIx64, filestore->get_base_path(), filestore->get_data_path(), chk_id);

    std::cout << chunk_meta_obj << std::endl;
    std::cout << chunk_data_dir << std::endl;

    ctime   = (unsigned)time(NULL);
    flags   = opts.flags;
    volume_id  = opts.vol_id;
    chunk_spolicy = opts.spolicy;
    block_size_shift = DEFAULT_BLOCK_SIZE_SHIFT;

    chunk_size.store(opts.size, std::memory_order_relaxed);
    chunk_used.store(0, std::memory_order_relaxed);

#ifdef DEBUG
    std::cout << "object_size = " << object_size << ", object_nums = " << object_nums << std::endl;
    //chunk_id = opts->chunk_id;
#endif

    uint64_t object_size = (1ULL << block_size_shift);
    uint64_t object_nums = chunk_size / object_size;

    if(util::xmkdir(chunk_data_dir, util::def_dmode) != 0) {
        fct_->log()->error("failed to mkdir %s: %s", chunk_data_dir, strerror(errno));
        return CHUNK_OP_CREATE_ERR;
    } else {
        if(this->is_preallocated()) {
            for(uint64_t i = 0; i < object_nums; i++) {
                uint64_t object_id = i;
                if(create_object(object_id, object_size) == CHUNK_OP_OBJ_CREATE_ERR) {
                    fct_->log()->error("create object failed.");
                    return CHUNK_OP_CREATE_ERR;
                }
            }
            chunk_used = chunk_size.load(std::memory_order_relaxed);
        }
    }

    if(this->store(chunk_meta_obj) != 0) {
        fct_->log()->error("failed to store chunk <%" PRIx64 ">.", this->chk_id);
        util::xremove(chunk_data_dir);
        return CHUNK_OP_CREATE_ERR;
    }
    return CHUNK_OP_SUCCESS;
}

int FileChunk::create_object(uint64_t object_id, uint64_t object_size) {
    int ret;
    int fd;
    char object_name[256];
    sprintf(object_name, "%s/%s/%" PRIx64 "/%" PRIx64, filestore->get_base_path(), filestore->get_data_path(), chk_id, object_id);
    if(access(object_name, F_OK) == 0) {
        fct_->log()->error("object<%s> has already existed.", object_name);
        return CHUNK_OP_OBJ_EXIST;
    }

    if((fd = open(object_name, O_CREAT | O_RDWR | O_CLOEXEC, util::def_fmode)) <= 0) {
        fct_->log()->error("object<%s> create fiailed: %s", object_name, strerror(errno));
        return CHUNK_OP_OBJ_CREATE_ERR;        
    }

    if(util::prealloc(fd, object_size) != 0) {
        fct_->log()->error("object<%s> alloc disk space failed: %s", object_name, strerror(errno));
        close(fd);
        util::xremove(object_name);
        return CHUNK_OP_OBJ_CREATE_ERR;
    }

    close(fd);

    return CHUNK_OP_SUCCESS;
}

int FileChunk::get_info(chunk_info_t& info) const {
    info.chk_id = chk_id;
    info.vol_id = volume_id;
    info.index  = volume_index;
    info.stat   = chunk_stat;
    info.flags  = flags;
    info.spolicy = chunk_spolicy;

    info.size = chunk_size.load(std::memory_order_relaxed);
    info.used = chunk_used.load(std::memory_order_relaxed);

    info.ctime  = ctime;
    info.dst_id = dst_id;
    info.dst_ctime = dst_ctime;
}

int FileChunk::set_xattr(const std::string& name, const std::string& value) {
    if(name.length() <= 0 || value.length() <= 0) {
        return CHUNK_OP_SET_XATTR_ERR;
    }

    pthread_rwlock_wrlock(&xattr_lock);
    std::list<struct chunk_xattr>::iterator iter;
    for(iter = xattr_list.begin(); iter != xattr_list.end(); iter++) {
        if(iter->name == name) {
            iter->value = value;
            break;
        }
    }

    if(iter == xattr_list.end()) {
        struct chunk_xattr cxattr;
        cxattr.index = this->get_xattr_nums() + 1;
        cxattr.name = name;
        cxattr.value = value;
        
        xattr_list.push_back(cxattr);
        xattr_num++;
    }

    pthread_rwlock_unlock(&xattr_lock);
    return CHUNK_OP_SUCCESS;
}

int FileChunk::get_xattr(const std::string& name, std::string& value) {
    if(name.length() <= 0) {
        return CHUNK_OP_GET_XATTR_ERR;
    }

    pthread_rwlock_rdlock(&xattr_lock);
    std::list<struct chunk_xattr>::iterator iter;
    for(iter = xattr_list.begin(); iter != xattr_list.end(); iter++) {
        if(iter->name == name) {
            value = iter->value;
            break;
        }
    }
    pthread_rwlock_unlock(&xattr_lock);

    if(iter == xattr_list.end()) {
        fct_->log()->error("xattr<%s> is not exist.", name.c_str());
        return CHUNK_OP_GET_XATTR_NO_NAME;
    }
    
    return CHUNK_OP_SUCCESS;
}

int FileChunk::remove_xattr(const std::string& name) {
    if(name.length() < 0)
        return CHUNK_OP_REMOVE_XATTR_ERR;

    pthread_rwlock_wrlock(&xattr_lock);
    std::list<struct chunk_xattr>::iterator iter;
    for(iter = xattr_list.begin(); iter != xattr_list.end(); iter++) {
        if(iter->name == name) {
            xattr_list.erase(iter);
            xattr_num--;
            pthread_rwlock_unlock(&xattr_lock);
            return CHUNK_OP_SUCCESS;
        }
    }
    pthread_rwlock_unlock(&xattr_lock);

    return CHUNK_OP_REMOVE_XATTR_ERR;
}

int FileChunk::prepare_object_iocb_sync(struct oiocb *oiocbs, uint32_t count, void *payload, uint64_t length, off_t offset, int opcode) {
    int ret;
    if(oiocbs == nullptr) {
        return -1;
    }

    uint64_t object_size = get_object_size();
    uint64_t first_obj_idx = offset / object_size;
    uint64_t first_obj_offset = offset % object_size;
    uint64_t last_obj_idx = (offset + length) / object_size;
    uint64_t last_obj_offset = (offset + length) % object_size;

    uint64_t obj_idx = first_obj_idx;
    off_t op_offset = first_obj_offset;
    uint64_t op_length = 0;
    char *buffer_ptr = (char *)payload;

    for(int i = 0; i < count; i++) {
        if(obj_idx < last_obj_idx) {
            op_length = object_size - op_offset;
        } else if(obj_idx == last_obj_idx && obj_idx == first_obj_idx){
            op_length = length;
        } else if(obj_idx == last_obj_idx) {
            op_length = last_obj_offset;
        }

#ifdef DEBUG
        std::cout << "obj_idx = " << obj_idx << std::endl;
        std::cout << "op_length = " << op_length << std::endl;
        std::cout << "op_offset = " << op_offset << std::endl;
#endif
        Object *obj = open_object(obj_idx);
        if(obj == nullptr) {
            fct_->log()->error("open object faild.");
            return -1;
        }

        switch(opcode) {
            case CHUNK_OP_READ:
                obj->read_counter_add(1);
                break;
            case CHUNK_OP_WRITE:
                obj->write_counter_add(1);
                break;
            default:
                break;
        }
        obj->update_access_time();

        oiocbs[i].fd = obj->get_fd();
        oiocbs[i].length = op_length;
        oiocbs[i].offset = op_offset;
        oiocbs[i].buffer = (void *)buffer_ptr;
        oiocbs[i].opcode = opcode;

        obj_idx += 1;
        op_offset = 0;
        buffer_ptr += op_length;
    }

    return 0;
}

int FileChunk::io_submit_sync(struct oiocb *iocbs, uint32_t count) {
    int ret;
    for(int i = 0; i < count; i++) {
        switch(iocbs[i].opcode) {
            case CHUNK_OP_READ:
                ret = pread(iocbs[i].fd, iocbs[i].buffer, iocbs[i].length, iocbs[i].offset);
                if(ret < iocbs[i].length)
                    return i;
                break;
            case CHUNK_OP_WRITE:
                ret = pwrite(iocbs[i].fd, iocbs[i].buffer, iocbs[i].length, iocbs[i].offset);
                if(ret < iocbs[i].length)
                    return i;
                break;
            default:
                break;
        }
    }

    return count;
}

int FileChunk::read_sync(void *payload, uint64_t offset, uint64_t length) {
    int ret;
    uint64_t object_size = get_object_size();
    uint64_t first_obj_idx = offset / object_size;
    uint64_t first_obj_offset = offset % object_size;
    uint64_t last_obj_idx = (offset + length) / object_size;
    uint64_t last_obj_offset = (offset + length) % object_size;

    uint32_t request_num = last_obj_idx - first_obj_idx + 1;
    struct oiocb iocbs[request_num];
    if(prepare_object_iocb_sync(iocbs, request_num, payload, length, offset, CHUNK_OP_READ) != 0) {
        fct_->log()->error("convert request into obejct iocb faild.");
        return CHUNK_OP_READ_ERR;
    }

#ifdef DEBUG
    std::cout << "request_num = " << request_num << std::endl;
    for(int i = 0; i < request_num; i++) {
        iocbs[i].print_arg();
    }
    std::cout << "=================================\n";
#endif

    ret = io_submit_sync(iocbs, request_num);
    if(ret < request_num) {
        fct_->log()->error("read_sync faild.");
        return CHUNK_OP_READ_ERR;
    }

    return CHUNK_OP_SUCCESS;
}

int FileChunk::write_sync(void *payload, uint64_t offset, uint64_t length) {
    int ret;
    uint64_t object_size = get_object_size();
    uint64_t first_obj_idx = offset / object_size;
    uint64_t first_obj_offset = offset % object_size;
    uint64_t last_obj_idx = (offset + length) / object_size;
    uint64_t last_obj_offset = offset % object_size;

    uint32_t request_num = last_obj_idx - first_obj_idx + 1;
    //std::cout << "request_num = " << request_num << std::endl;
    struct oiocb iocbs[request_num];
    if(prepare_object_iocb_sync(iocbs, request_num, payload, length, offset, CHUNK_OP_WRITE) != 0) {
        fct_->log()->error("convert request into obejct iocb faild.");
        return CHUNK_OP_WRITE_ERR;
    }

#ifdef DEBUG
    for(int i = 0; i < request_num; i++) {
        iocbs[i].print_arg();
    }
#endif

    ret = io_submit_sync(iocbs, request_num);
    if(ret < request_num) {
        fct_->log()->error("write_sync faild.");
        return CHUNK_OP_WRITE_ERR;
    }

    return CHUNK_OP_SUCCESS;
}

int FileChunk::prepare_object_iocb(struct iocb **iocbs, uint32_t count, void *payload, 
                            uint64_t length, off_t offset, int opcode, void *extra_arg) {
    if(iocbs == nullptr) {
        return -1;
    }

    int req_num = 0;
    uint64_t object_size = get_object_size();

    uint64_t first_obj_idx = offset / object_size;
    uint64_t first_obj_offset = offset % object_size;

    uint64_t last_obj_idx = (offset + length) / object_size;
    uint64_t last_obj_offset = (offset + length) % object_size;

    uint64_t obj_idx = first_obj_idx;
    off_t op_offset = first_obj_offset;
    uint64_t op_length = 0;
    char *buffer_ptr = (char *)payload;

    struct epoll_event epevent;

    for(int i = 0; i < count; i++) {
        if(obj_idx < last_obj_idx) {
            op_length = object_size - op_offset;
        } else if(obj_idx == last_obj_idx && obj_idx == first_obj_idx){
            op_length = length;
        } else if(obj_idx == last_obj_idx) {
            op_length = last_obj_offset;
        }

        Object * obj = open_object(obj_idx);
        if(obj == nullptr) {
            fct_->log()->error("open object faild.");
            return -1;            
        }

        switch(opcode) {
            case CHUNK_OP_READ:
                io_prep_pread(iocbs[i], obj->get_fd(), buffer_ptr, op_length, op_offset);
                io_set_eventfd(iocbs[i], this->efd);
                iocbs[i]->data = extra_arg;
                obj->read_counter_add(1);
                obj->update_access_time();
                break;
            case CHUNK_OP_WRITE:
                io_prep_pwrite(iocbs[i], obj->get_fd(), buffer_ptr, op_length, op_offset);
                io_set_eventfd(iocbs[i], this->efd);
                iocbs[i]->data = extra_arg;
                obj->write_counter_add(1);
                obj->update_access_time();
                break;
            default:
                fct_->log()->error("invalid op code.");
                return -1;
                break;
        }

        obj_idx += 1;
        op_offset = 0;
        buffer_ptr += op_length;
    }

    return 0;
}

int FileChunk::read_async(void *buff, uint64_t off, uint64_t len, chunk_opt_cb_t cb, void *cb_arg) {
    struct chunk_async_opt_entry_t *extra_arg = (struct chunk_async_opt_entry_t *)malloc(sizeof(struct chunk_async_opt_entry_t));
    if(extra_arg == nullptr) {
        fct_->log()->error("malloc callback_arg failed.");
        return CHUNK_OP_READ_ERR;
    }

    extra_arg->buff = buff;
    extra_arg->off  = off;
    extra_arg->len  = len;
    extra_arg->cb   = cb;
    extra_arg->cb_arg = cb_arg;

    return read_async(buff, len, off, extra_arg);
    
}

int FileChunk::read_async(void *payload, uint64_t length, off_t offset, void *extra_arg) {
    int ret;

    uint64_t object_size = get_object_size();
    uint64_t first_obj_idx = offset / object_size;
    uint64_t first_obj_offset = offset % object_size;
    uint64_t last_obj_idx = (offset + length) / object_size;
    uint64_t last_obj_offset = (offset + length) % object_size;

    uint32_t request_num = last_obj_idx - first_obj_idx + 1;
    struct iocb *iocbps[request_num];

    for(int i = 0; i < request_num; i++) {
        iocbps[i] = (struct iocb *)malloc(sizeof(struct iocb));
        if(iocbps[i] == nullptr) {
            fct_->log()->error("prepare iocbs failed: %s", strerror(errno));
            for(int j = i - 1; j >= 0; j--) {
                free(iocbps[i]);
            }

            return CHUNK_OP_READ_ERR;
        }
    }

    if(prepare_object_iocb(iocbps, request_num, payload, length, offset, CHUNK_OP_READ, extra_arg) != 0) {
        fct_->log()->error("convert request into chunk_iocb failed.");
        return CHUNK_OP_READ_ERR;
    }

#ifdef DEBUG
    for(int i = 0; i < request_num; i++) {
        std::cout << iocbps[i]->aio_fildes << std::endl;
        std::cout << iocbps[i]->u.c.nbytes << std::endl;
        std::cout << iocbps[i]->u.c.offset << std::endl;
    }
#endif

    ret = io_submit(ioctx, request_num, iocbps);
    if(ret != request_num) {
        fct_->log()->error("read io submit failed: %s", strerror(errno));
        return CHUNK_OP_READ_ERR;
    }

    return CHUNK_OP_SUCCESS;
}

int FileChunk::write_async(void *buff, uint64_t off, uint64_t len, chunk_opt_cb_t cb, void *cb_arg) {
    struct chunk_async_opt_entry_t *extra_arg = (struct chunk_async_opt_entry_t *)malloc(sizeof(struct chunk_async_opt_entry_t));
    if(extra_arg == nullptr) {
        fct_->log()->error("malloc callback_arg failed.");
        return CHUNK_OP_WRITE_ERR;
    }

    extra_arg->buff = buff;
    extra_arg->off  = off;
    extra_arg->len  = len;
    extra_arg->cb   = cb;
    extra_arg->cb_arg = cb_arg;

    return write_async(buff, len, off, extra_arg);
}

int FileChunk::write_async(void *payload, uint64_t length, off_t offset, void *extra_arg) {
    int ret;

    uint64_t object_size = get_object_size();
    uint64_t first_obj_idx = offset / object_size;
    uint64_t first_obj_offset = offset % object_size;
    uint64_t last_obj_idx = (offset + length) / object_size;
    uint64_t last_obj_offset = (offset + length) % object_size;

    long request_num = last_obj_idx - first_obj_idx + 1;
    struct iocb *iocbps[request_num];

    //std::cout << "request_num = " << request_num << std::endl;

    for(int i = 0; i < request_num; i++) {
        iocbps[i] = (struct iocb *)malloc(sizeof(struct iocb));

        if(iocbps[i] == nullptr) {
            fct_->log()->error("prepare iocbs failed: %s", strerror(errno));
            for(int j = i - 1; j >= 0; j--) {
                free(iocbps[j]);
            }
            return CHUNK_OP_WRITE_ERR;
        }
    }

    if(prepare_object_iocb(iocbps, request_num, payload, length, offset, CHUNK_OP_WRITE, extra_arg) != 0) {
        fct_->log()->error("convert request into object_iocb faild.");
        return CHUNK_OP_WRITE_ERR;
    }
   
    //iocbps = iocbs;
#ifdef DEBUG
    for(int i = 0; i < request_num; i++) {
        std::cout << iocbps[i]->aio_fildes << std::endl;
        std::cout << iocbps[i]->u.c.nbytes << std::endl;
        std::cout << iocbps[i]->u.c.offset << std::endl;
    }
#endif 

    ret = io_submit(ioctx, request_num, iocbps);
    if(ret != request_num) {
        fct_->log()->error("write, io submit faild: %s", strerror(errno));
        return CHUNK_OP_WRITE_ERR;
    } 

    return CHUNK_OP_SUCCESS;
}

int FileChunk::write_chunk(void *buff, uint64_t off, uint64_t len, void *extra_arg) {
    int ret;
    switch(filestore->get_io_mode()) {
        case CHUNKSTORE_IO_MODE_SYNC:
            if(write_sync(buff, len, off) != CHUNK_OP_SUCCESS) {
                fct_->log()->error("chunk write failed.");
                return CHUNK_OP_WRITE_ERR;
            }
            break;
        case CHUNKSTORE_IO_MODE_ASYNC:
            if(write_async(buff, len, off, extra_arg) != CHUNK_OP_SUCCESS) {
                fct_->log()->error("chunk write failed.");
                return CHUNK_OP_WRITE_ERR;
            }
            break;
        default:
            return CHUNK_OP_WRITE_ERR;
    }

    return CHUNK_OP_SUCCESS;
}

int FileChunk::read_chunk(void *buff, uint64_t off, uint64_t len, void *extra_arg) {
    int ret;
    switch(filestore->get_io_mode()) {
        case CHUNKSTORE_IO_MODE_SYNC:
            if(read_sync(buff, len, off) != CHUNK_OP_SUCCESS) {
                fct_->log()->error("chunk read failed.");
                return CHUNK_OP_READ_ERR;
            }
            break;
        case CHUNKSTORE_IO_MODE_ASYNC:
            if(read_async(buff, len, off, extra_arg) != CHUNK_OP_SUCCESS) {
                fct_->log()->error("chunk write failed.");
                return CHUNK_OP_READ_ERR;
            }
            break;
        default:
            return CHUNK_OP_READ_ERR;
    }

    return CHUNK_OP_SUCCESS;
}

Object *FileChunk::open_object(const uint64_t oid) {
    Object *result = nullptr;
    pthread_rwlock_rdlock(&active_lock);
    std::map<uint64_t, Object *>::iterator iter;
    iter = active_list.find(oid);
    if(iter != active_list.end()) {
        result = iter->second;
    }
    pthread_rwlock_unlock(&active_lock);

    if(iter == active_list.end()) {
        result = new Object(this->fct_, this->filestore, this, oid);
        result->init();
        pthread_rwlock_wrlock(&active_lock);
        active_list[oid] = result;
        pthread_rwlock_unlock(&active_lock);
    }

    return result;
}

int FileChunk::close_object(Object *obj) {
    uint64_t oid;
    pthread_rwlock_wrlock(&active_lock);
    if(obj != nullptr) {
        oid = obj->get_oid();
        close(obj->get_fd());
        delete(obj);
    }

    std::map<uint64_t, Object*>::iterator iter;
    iter = active_list.find(oid);
    if(iter != active_list.end()) {
        active_list.erase(iter);
    }
    pthread_rwlock_unlock(&active_lock);
       
    return 0;
}

int FileChunk::close_object(const uint64_t oid) {
    Object *result = nullptr;
    Object *temp_obj = nullptr;
    pthread_rwlock_wrlock(&active_lock);
    std::map<uint64_t, Object *>::iterator iter;
    iter = this->active_list.find(oid);
    if(iter != active_list.end()) {
        int fd = iter->second->get_fd();
        close(fd);
        temp_obj = iter->second;
        active_list.erase(iter);
    }
    pthread_rwlock_unlock(&active_lock);

    if(temp_obj != nullptr) {
        delete temp_obj;
        temp_obj = nullptr;
    }

    return 0;
}

int FileChunk::close_active_objects() {
    int ret;
    pthread_rwlock_wrlock(&active_lock);
    std::map<uint64_t, Object *>::iterator iter;
    while(!active_list.empty()) {
        iter = active_list.begin();
        Object *object = iter->second;
        if(object != nullptr) {
            close(object->get_fd());
            delete object;
        }
        active_list.erase(iter);
    }
    pthread_rwlock_unlock(&active_lock);

    return CHUNK_OP_SUCCESS;
}

/*
FileChunk::~FileChunk() {
    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cond);
    pthread_rwlock_destroy(&rwlock);
    pthread_rwlock_destroy(&active_lock);
    pthread_rwlock_destroy(&xattr_lock);

    if(get_open_ref() > 1) {
        ref_counter_sub(1);
    } else {
        char store_path[256];
        sprintf(store_path, "%s/%s/%" PRIx64, filestore->get_base_path(), filestore->get_meta_path(), get_chunk_id());
        if(store(store_path) != 0) {
            fct_->log()->error("close chunk failed.");
            exit(-1);
        }

        if(close_active_objects() != CHUNK_OP_SUCCESS) {
            fct_->log()->error("close active list failed.");
            exit(-1);
        }

        if(filestore->get_io_mode() == CHUNKSTORE_IO_MODE_ASYNC) {
            if(destroy_chunk_async_env() != CHUNK_OP_SUCCESS) {
                fct->log()->error("destroy chunk async env failed.");
                exit(-1);
            }
        }

        filestore->get_chunk_map()->remove_chunk(get_chunk_id());
    }
}
*/