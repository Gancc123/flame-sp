#include <iostream>
#include <string>
#include <regex>

#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libaio.h>
#include <fcntl.h>

#include "chunkstore/filestore/filechunkmap.h"
#include "chunkstore/filestore/filestore.h"
#include "chunkstore/filestore/chunkutil.h"
#include "util/utime.h"
#include "chunkstore/log_cs.h"

#define MAX_NUM_EVENTS 1024

using namespace flame;

const char* FileStore::get_base_path() const {
    return this->config_file.get_base_path();
}

const char* FileStore::get_data_path() const {
    return this->config_file.get_data_path();
}

const char* FileStore::get_meta_path() const {
    return this->config_file.get_meta_path();
}

const char* FileStore::get_journal_path() const {
    return this->config_file.get_journal_path();
}

const char *FileStore::get_backup_path() const {
    return this->config_file.get_backup_path();
}

const char *FileStore::get_super_path() const {
    return this->config_file.get_super_path();
}

int FileStore::get_epfd() const {
    return epfd;
}

FlameContext* FileStore::get_flame_context() {
    return fct_;
}

bool FileStore::is_meta_store(const char *dirpath) {
    char config_path[256];
    char epoch_path[256];
    sprintf(config_path, "%s/%s", dirpath, "/config");
    sprintf(epoch_path, "%s/%s", dirpath, "/epoch");

    if(!access(config_path, F_OK) && !access(epoch_path, F_OK))
           return true;

    return false;
}

std::string FileStore::get_driver_name() const {
    return "FileStore";
}

std::string FileStore::get_config_info() const {
    //std::ostringstream oss;
    //oss << get_driver_name() << "://" << get_base_path() << ":" << (size >> 30) << "G";
    //return oss.str();

    return config_file.to_string();
}

std::string FileStore::get_runtime_info() const {
    return "FileStore: Runtime Information";
}

int FileStore::get_io_mode() const {
    return io_mode;
}

bool FileStore::is_support_mem_persist() const {
    return false;
}

bool FileStore::is_mounted() {
    return state == FILESTORE_MOUNTED;
}

/*
*get_info：用于获取filestore的public info
*/
int FileStore::get_info(cs_info_t &info) const {
    info.id = id;
    info.cluster_name = cluster_name;
    info.name = name;
    info.ftime = ftime;

    info.size = size.load(std::memory_order_relaxed);
    info.used = used.load(std::memory_order_relaxed);
    info.chk_num = total_chunks.load(std::memory_order_relaxed);
}

int FileStore::set_info(const cs_info_t& info) {
    id = info.id;
    ftime = info.ftime;

    sprintf(cluster_name, "%s", info.cluster_name.c_str());
    sprintf(name, "%s", info.name.c_str());

    size.store(info.size, std::memory_order_relaxed);
    used.store(info.used, std::memory_order_relaxed);
    total_chunks.store(info.chk_num, std::memory_order_relaxed);
}

FileStore* FileStore::create_filestore(FlameContext *fct, const std::string& url) {
    std::string pstr = "^(\\w+)://(.+)";
    std::regex pattern(pstr);
    std::smatch result;

    if(!regex_match(url, result, pattern)) {
        fct->log()->lerror("format error for url: %s", url.c_str());
        return nullptr;
    }

    std::string driver = result[1];
    if(driver != "filestore" && driver != "FileStore") {
        fct->log()->lerror("unknown driver: %s", driver.c_str());
        return nullptr;
    }

    std::string config_file_path = result[2];

    return new FileStore(fct, config_file_path);
}

/*
    FileStore* FileStore::create_filestore(FlameContext *fct, 
                                    const std::string& config_file_path) {
        return new FileStore(fct, config_file_path);
}
*/

/*
 * close_active_chunks: 该函数用于关闭目前FileStore中所有被打开的chunk，该函数一般用于格式化操作
*/
void FileStore::close_active_chunks() {
    if(chunk_map) {
        while(!chunk_map->is_empty()) {
            Chunk *chunk = chunk_map->get_active_chunk();
            if(chunk != nullptr) {
                chunk_close(chunk);
            }
        }
    }
}

int FileStore::dev_check() {
    int ret;
    ret = config_file.load();
    if(ret != FILESTORE_CONF_VALID) {
        fct_->log()->lerror("load config file <%s> failed.", config_file.get_config_path());
        return NONE;
    }

    config_file.print_conf();

    if((ret = config_file.is_valid()) != FILESTORE_CONF_VALID) {
        switch(ret) {
            case FILESTORE_CONF_NO_BASE:
                fct_->log()->lerror("config_file is invalid: no base_path: %s", config_file.get_base_path());
                break;
            case FILESTORE_CONF_NO_DATA:
                fct_->log()->lerror("config_file is invalid: no data_path: %s", config_file.get_data_path());
                break;
            case FILESTORE_CONF_NO_META:
                fct_->log()->lerror("config_file is invalid: no meta_path: %s", config_file.get_meta_path());
                break;
            case FILESTORE_CONF_NO_JOURNAL:
                fct_->log()->lerror("config file is invalid: no journal_path: %s", config_file.get_journal_path());
                break;
            case FILESTORE_CONF_NO_BACKUP:
                fct_->log()->lerror("config file is invalid: no backup_path: %s", config_file.get_backup_path());
                break;
            case FILESTORE_CONF_INVALID_SIZE:
                fct_->log()->lerror("config file is invalid: invalid size.");
                break;
            case FILESTORE_CONF_INVALID_MODE:
                fct_->log()->lerror("config file is invalid: invalid mode");
                break;
            default:
                fct_->log()->lerror("config file is invalid.");
                break;
        }

        return NONE;
    }

    return CLT_IN;
}

/*
 * remove_all_chunk:用于删除一个filestore下的所有chunk，一般用于dev_format
*/
int FileStore::remove_all_chunks(const char *meta_dir) {
    char chunk_meta_obj[512];
    char chunk_data_dir[512];
    char chunk_meta_temp[512];

    DIR *dir;
    struct dirent* dir_info;
    struct stat st;

    if(dir = opendir(meta_dir)) {
        while(dir_info = readdir(dir)) {
            //首先判断如果是当前目录，上级目录，或者是superblock超级块文件，则跳过，不删除（因为superblock没有对应的data block）
            if(strcmp(dir_info->d_name, ".") == 0 || 
                strcmp(dir_info->d_name, "..") == 0 || 
                strcmp(dir_info->d_name, "superblock") == 0 ||
                strcmp(dir_info->d_name, "config") == 0) {
                continue;
            }

            sprintf(chunk_meta_obj, "%s/%s", meta_dir, dir_info->d_name);
            if(stat(chunk_meta_obj, &st) == 0) {
                if(S_ISREG(st.st_mode)) {
                    sprintf(chunk_data_dir, "%s/%s/%s", get_base_path(), get_data_path(), dir_info->d_name);
                    if(access(chunk_data_dir, F_OK) == 0) {
                        if(util::remove_dir(chunk_data_dir) == 0) {
                            if(util::xremove(chunk_meta_obj) != 0) {
                                fct_->log()->lerror("remove chunk_meta_obj %s failed: %s", chunk_meta_obj, strerror(errno));
                                sprintf(chunk_meta_temp, "%s/.%s", meta_dir, dir_info->d_name);
                                util::xrename(chunk_meta_obj, chunk_meta_temp);
                            }

                            total_chunks--;
                        }
                    }
                } else if(S_ISDIR(st.st_mode)){
                    util::remove_dir(chunk_meta_obj);
                }
            }

        }
    } else {
        return FILESTORE_CHUNK_REMOVE_ALL_ERR;
    }

    return FILESTORE_OP_SUCCESS;
}

/*
*persist_super：持久化超级块信息
*/
int FileStore::persist_super() {
    char super_path[256];
    int ret;
    int open_flags = O_WRONLY;

    sprintf(super_path, "%s/%s", get_base_path(), get_super_path());
    if(util::xaccess(super_path, F_OK) != 0) {
        open_flags |= O_CREAT;
    }

    int fd = open(super_path, open_flags, util::def_fmode);
    if(fd < 0) {
        fct_->log()->lerror("open super file <%s> failed: %s", super_path, strerror(errno));
        return FILESTORE_PERSIST_ERR;
    }

    struct chunk_store_descriptor csdesc;

    //snprintf(csdesc.data_path,    sizeof(csdesc.data_path), "%s", data_path);
    //snprintf(csdesc.meta_path,    sizeof(csdesc.meta_path), "%s", meta_path);
    //snprintf(csdesc.journal_path, sizeof(csdesc.journal_path), "%s", journal_path);
    //snprintf(csdesc.backup_path,  sizeof(csdesc.backup_path), "%s", backup_path);
    //snprintf(csdesc.super_path,   sizeof(csdesc.super_path), "%s", super_file);

    snprintf(csdesc.cluster_name, sizeof(csdesc.cluster_name), "%s", cluster_name);
    snprintf(csdesc.name, sizeof(csdesc.name), "%s", name);

    //csdesc.chunk_size = chunk_size;
    csdesc.id = id; 
    csdesc.size = size.load(std::memory_order_relaxed);
    csdesc.used = used.load(std::memory_order_relaxed);
    csdesc.total_chunks = total_chunks.load(std::memory_order_relaxed);
    csdesc.ftime = ftime;

    ret = write(fd, &csdesc, sizeof(struct chunk_store_descriptor));
    if(ret < sizeof(struct chunk_store_descriptor)) {
        fct_->log()->lerror("write super file: %s failed: %s", super_path, strerror(errno));
        close(fd);
        return FILESTORE_PERSIST_ERR;
    }
    close(fd);

    return FILESTORE_OP_SUCCESS;
}

/**
*load_super(): 加载超级块信息
*/
int FileStore::load_super() {
    char super_path[256];
    int ret;
    sprintf(super_path, "%s/%s", get_base_path(), get_super_path());
    if(util::xaccess(super_path, F_OK) != 0) {
        fct_->log()->lerror("super block is not existed.");
        return FILESTORE_NO_SUPER;
    }

    int fd = open(super_path, O_RDONLY, util::def_fmode);
    if(fd < 0) {
        if(errno == ENOENT) {
            fct_->log()->lwarn("open super file %s failed: %s", super_path, strerror(errno));
            return FILESTORE_NO_SUPER;
        } else {
            fct_->log()->lerror("open super file %s failed: %s", super_path, strerror(errno));
            return FILESTORE_INIT_ERR;
        }
    }

    struct chunk_store_descriptor csdesc;
    ret = read(fd, &csdesc, sizeof(struct chunk_store_descriptor));
    if(ret < sizeof(struct chunk_store_descriptor)) {
        fct_->log()->lerror("read super file %s failed: %s", super_path, strerror(errno));
        close(fd);
        return FILESTORE_INIT_ERR;
    }

    //snprintf(data_path, sizeof(data_path), "%s", csdesc.data_path);
    //snprintf(meta_path, sizeof(meta_path), "%s", csdesc.meta_path);
    //snprintf(journal_path, sizeof(journal_path), "%s", csdesc.journal_path);
    //snprintf(backup_path, sizeof(backup_path), "%s", csdesc.backup_path);
    //snprintf(super_file, sizeof(super_file), "%s", csdesc.super_path);

    snprintf(cluster_name, sizeof(cluster_name), "%s", csdesc.cluster_name);
    snprintf(name, sizeof(name), "%s", csdesc.name);

    id    = csdesc.id;
    ftime = csdesc.ftime;
    size.store(csdesc.size, std::memory_order_relaxed);
    used.store(csdesc.used, std::memory_order_relaxed);
    total_chunks.store(csdesc.total_chunks, std::memory_order_relaxed);

    close(fd);
    return FILESTORE_OP_SUCCESS;
}

/*
 * dev_format：对设备进行格式化，分为两种情况，第一种是对设备第一次进行格式化，此时需要创建各个子目录
 * 第二种情况则是对一个正在运行的设备进行格式化，需要关闭所有的正在工作的chunk，并删除它们
*/
int FileStore::dev_format() {
    //格式化
    int ret;
    char journal_dir[256];
    char data_dir[256];
    char meta_dir[256];
    char backup_dir[256];

    DIR *dir;
    struct stat st;
    struct dirent *dir_info;
    char subfile[256];

    sprintf(backup_dir,  "%s/%s", get_base_path(), get_backup_path());
    sprintf(journal_dir, "%s/%s", get_base_path(), get_journal_path());
    sprintf(meta_dir,    "%s/%s", get_base_path(), get_meta_path());
    sprintf(data_dir,    "%s/%s", get_base_path(), get_data_path());

    this->close_active_chunks();

    if(dir = opendir(get_base_path())) {
        while(dir_info = readdir(dir)) {
            if(strcmp(dir_info->d_name, ".") == 0 || strcmp(dir_info->d_name, "..") == 0)
                continue;
            
            if(strcmp(dir_info->d_name, get_backup_path()) == 0) {
                if(util::remove_dir(backup_dir) != 0) {
                    fct_->log()->lerror("format filestore failed: format backup_path <%s> failed.", backup_dir);
                    closedir(dir);
                    return FILESTORE_FORMAT_ERR;
                }
            } else if(strcmp(dir_info->d_name, get_journal_path()) == 0) {
                if(util::remove_dir(journal_dir) != 0) {
                    fct_->log()->lerror("format filestore failed: format journal_path <%s> failed", journal_dir);
                    closedir(dir);
                    return FILESTORE_FORMAT_ERR;
                }
            } else if(strcmp(dir_info->d_name, get_meta_path()) == 0) {
                if(this->remove_all_chunks(meta_dir) != FILESTORE_OP_SUCCESS) {
                    fct_->log()->lerror("format filestore failed: format meta_path <%s> failed.", meta_dir);
                    closedir(dir);
                    return FILESTORE_FORMAT_ERR;
                }
            } else if(strcmp(dir_info->d_name, get_data_path()) == 0) {
                if(util::remove_dir(data_dir) != 0) {
                    fct_->log()->lerror("format filestore failed: format data_path <%s> failed", data_dir);
                    closedir(dir);
                    return FILESTORE_FORMAT_ERR;
                }
            } else {
                sprintf(subfile, "%s/%s", get_base_path(), dir_info->d_name);
                stat(subfile, &st);

                if(S_ISREG(st.st_mode))
                    util::xremove(subfile);
                else if(S_ISDIR(st.st_mode)) {
                    //递归删除子目录
                    util::remove_dir(subfile);
                }
            }
        }
    }
    closedir(dir);

    if(access(backup_dir, F_OK) != 0) {
        if(util::xmkdir(backup_dir, util::def_dmode) != 0) {
            fct_->log()->lerror("create backup_path <%s> failed: %s", backup_dir, strerror(errno));
            return FILESTORE_FORMAT_ERR;
        }
    }

    if(access(journal_dir, F_OK) != 0) {
        if(util::xmkdir(journal_dir, util::def_dmode) != 0) {
            fct_->log()->lerror("create journal_path <%s> failed: %s", journal_dir, strerror(errno));
            return FILESTORE_FORMAT_ERR;
        }
    }

    if(access(meta_dir, F_OK) != 0) {
        if(util::xmkdir(meta_dir, util::def_dmode) != 0) {
            fct_->log()->lerror("create meta_path <%s> failed: %s", meta_dir, strerror(errno));
            return FILESTORE_FORMAT_ERR;
        }
    }

    if(access(data_dir, F_OK) != 0) {
        if(util::xmkdir(data_dir, util::def_dmode) != 0) {
            fct_->log()->lerror("create data_path <%s> failed: %s", data_dir, strerror(errno));
            return FILESTORE_FORMAT_ERR;
        }
    }

    used.store(0, std::memory_order_relaxed);
    size.store(config_file.get_size_in_bytes(), std::memory_order_relaxed);

    this->persist_super();
    ftime = time(NULL);
    return FILESTORE_OP_SUCCESS;
}

int FileStore::dev_mount() {

    this->load_super();

    this->chunk_map = new FileChunkMap(this);
    if(this->chunk_map == nullptr) {
        fct_->log()->lerror("new ChunkMap failed.");
        return FILESTORE_MOUNT_ERR;
    }

    if(config_file.is_async_io())
        io_mode = CHUNKSTORE_IO_MODE_ASYNC;
    else
        io_mode = CHUNKSTORE_IO_MODE_SYNC;

    if(io_mode == CHUNKSTORE_IO_MODE_ASYNC) {
        if((epfd = epoll_create(1)) == -1) {
            fct_->log()->lerror("epoll create fialed: %s", strerror(errno));
            delete this->chunk_map;
            return FILESTORE_INIT_ERR;
        }

        if(pthread_create(&complete_thread, NULL, completeLoopFunc, (void *)this) != 0) {
            fct_->log()->lerror("create complete thread failed: %s", strerror(errno));
            delete this->chunk_map;
            close(epfd);
            return FILESTORE_INIT_ERR;
        }
    }

    return FILESTORE_OP_SUCCESS;
}

//判断一个chunk是否已经存在了，通过判断chunk的元数据文件是否存在
bool FileStore::chunk_exist(const uint64_t chk_id) {
    char full_path[256];
    sprintf(full_path, "%s/%s/%" PRIx64, get_base_path(), get_meta_path(), chk_id);
    if(access(full_path, F_OK) == 0) {
        return true;
    }

    return false;
}

/*
*chunk_create(): 该函数用于创建一个chunk
*/
int FileStore::chunk_create(uint64_t chk_id, const chunk_create_opts_t& opts) {
    int ret = 0;
    if(chunk_exist(chk_id)) {
        fct_->log()->lerror("chunk <%" PRIx64 "> has already existed.", chk_id);
        return FILESTORE_CHUNK_EXIST;
    }

    FileChunk *chunk = new FileChunk(this, fct_);

    chunk->set_chunk_id(chk_id);
    ret = chunk->create(opts);

    if(ret != CHUNK_OP_SUCCESS) {
        fct_->log()->lerror("the chunk %" PRIx64 "create failed.", chk_id);
        return FILESTORE_CHUNK_CREATE_ERR;
    }

    if(chunk != nullptr) {
        delete chunk;
    }

    fct_->log()->linfo("the chunk %" PRIx64 " create successfully.", chk_id);

    total_chunks.fetch_add(1, std::memory_order_relaxed);
    used.fetch_add(opts.size, std::memory_order_relaxed);

    return FILESTORE_OP_SUCCESS;
}

/*
*chunk_remove：删除一个chunk，如果该chunk正在被使用，则返回删除失败
*/
int FileStore::chunk_remove(uint64_t chk_id) {
    int ret;
    Chunk *chunk = chunk_map->get_chunk(chk_id);
    if(chunk == nullptr) {
        char chunk_path[256];
        char chunk_data_sour[256];
        char chunk_data_dest[256];

        sprintf(chunk_data_sour, "%s/%s/%" PRIx64, get_base_path(), get_data_path(), chk_id);
        sprintf(chunk_data_dest, "%s/%s/.%" PRIx64, get_base_path(), get_data_path(), chk_id);

        if(util::xrename(chunk_data_sour, chunk_data_dest) != 0) {
            fct_->log()->lerror("remove chunk data failed.");
            return FILESTORE_CHUNK_REMOVE_ERR;
        }

        sprintf(chunk_path, "%s/%s/%" PRIx64, get_base_path(), get_meta_path(), chk_id);
        if(util::xremove(chunk_path) != 0) {
            fct_->log()->lerror("chunk remove failed.");
            util::xrename(chunk_data_dest, chunk_data_sour);
            return FILESTORE_CHUNK_REMOVE_ERR;
        }

        uint64_t chunk_size = get_chunk_size(chk_id);

        total_chunks.fetch_sub(1, std::memory_order_relaxed);
        used.fetch_sub(chunk_size, std::memory_order_relaxed);

    } else {
        fct_->log()->lerror("this chunk <% " PRIx64 "> is using.", chk_id);
        return FILESTORE_CHUNK_USING;
    }

    return FILESTORE_OP_SUCCESS;
}

/*
*chunk_open：打开一个chunk，并将它加载到内存中，获取一个可以执行IO的chunk句柄，本质上就是一个chunk指针
*/
std::shared_ptr<Chunk> FileStore::chunk_open(uint64_t chk_id) {
    int ret;
    FileChunk *chunk = chunk_map->get_chunk(chk_id);

    if(chunk != nullptr) {
        chunk->ref_counter_add(1);
        return std::shared_ptr<Chunk>(chunk);
    } else {
        char chunk_path[256];
        sprintf(chunk_path, "%s/%s/%" PRIx64, get_base_path(), get_meta_path(), chk_id);
        chunk = new FileChunk(this, fct_);
        if(chunk->load(chunk_path) != 0) {
            fct_->log()->lerror("open chunk failed.");
            return nullptr;
        }

        if(io_mode == CHUNKSTORE_IO_MODE_ASYNC) {
            if(chunk->init_chunk_async_env() != CHUNK_OP_SUCCESS) {
                fct_->log()->lerror("init chunk async env failed.");
                return nullptr;
            }
            fct_->log()->lerror("init chunk async env successed.");
        }

        fct_->log()->linfo("open chunk %" PRIx64 " success.", chk_id);
        chunk->ref_counter_add(1);
        chunk_map->insert_chunk(chunk);
    }

    return std::shared_ptr<Chunk>(chunk);
}

/**
*chunk_close：根据一个chunk指针关闭一个chunk，将该chunk从内存中持久化到设备上
*/
int FileStore::chunk_close(Chunk* chunk) {
    FileChunk *chunk_ptr = dynamic_cast<FileChunk *>(chunk);
    if(chunk_ptr == nullptr) {
        return FILESTORE_OP_SUCCESS;
    }

    if(chunk_ptr->get_open_ref() > 1) {
        chunk_ptr->ref_counter_sub(1);
    } else if(chunk_ptr->get_open_ref() == 1){
        //这是一个只被打开了1次的chunk，需要持久化之后，
        //销毁上下文环境（包括打开的object和其它evnetfd等），并从chunk_map中移除，最后释放chunk_ptr指向的空间
        char store_path[256];
        sprintf(store_path, "%s/%s/%" PRIx64, get_base_path(), get_meta_path(), chunk_ptr->get_chunk_id());
        if((chunk_ptr->store(store_path)) != 0) {
            fct_->log()->lerror("close chunk failed.");
            return FILESTORE_CHUNK_CLOSE_ERR;
        }

        if((chunk_ptr->close_active_objects()) != CHUNK_OP_SUCCESS) {
            fct_->log()->lerror("close active list failed.");
            return FILESTORE_CHUNK_CLOSE_ERR;
        }

        if(io_mode == CHUNKSTORE_IO_MODE_ASYNC) {
            if(chunk_ptr->destroy_chunk_async_env() != CHUNK_OP_SUCCESS) {
                fct_->log()->lerror("destroy chunk aync env failed.");
                return FILESTORE_CHUNK_CLOSE_ERR;
            }
        }

        chunk_map->remove_chunk(chunk_ptr->get_chunk_id());
        if(chunk_ptr != nullptr) {
            delete chunk_ptr;
            chunk_ptr = nullptr;
        }
    } else {
        //这是一个无效的chunk_ptr，不应该出现在chunk_map中，所以需要从chunk_map中删除，并释放它
        fct_->log()->lerror("invalid chunk ptr.");
        chunk_map->remove_chunk(chunk_ptr->get_chunk_id());
        if(chunk_ptr != nullptr) {
            delete chunk_ptr;
            chunk_ptr = nullptr;
        }
    }

    return FILESTORE_OP_SUCCESS;
}

int FileStore::stop(void) {
    if(pthread_join(complete_thread, NULL) != 0) {
        fct_->log()->lerror("pthread_join failed: %s", strerror(errno));
        return FILESTORE_STOP_FAILED;
    }

    return FILESTORE_OP_SUCCESS;
}

void *FileStore::completeLoopFunc(void *arg) {
    FileStore *filestore = (FileStore *)arg;

    struct io_event events[MAX_NUM_EVENTS];
    struct timespec timeout;
    int ret;
    int complete_count = 0;
    struct epoll_event eplevs[MAX_NUM_EVENTS];
    int nfds = 0;
    int epfd = filestore->get_epfd();

    int efd;
    io_context_t *ioctx; 

    while(true) {
        uint64_t finished_aio = 0;
        nfds = epoll_wait(epfd, eplevs, MAX_NUM_EVENTS, -1);

#ifdef DEBUG
        std::cout << "nfds = " << nfds << std::endl;
#endif

        for(int i = 0; i < nfds; i++) {
            FileChunk *chunk = filestore->get_chunk_by_efd(eplevs[i].data.fd);
            if(chunk == nullptr)
                continue;

            ioctx = chunk->get_ioctx();
            efd = eplevs[i].data.fd;

            if(read(efd, &finished_aio, sizeof(finished_aio)) != sizeof(finished_aio)) {
                filestore->get_flame_context()->log()->lwarn("read efd faild: %s", strerror(errno));
                break;
            }
#ifdef DEBUG
            std::cout << "finished_aio = " << finished_aio << std::endl;
#endif
            while(finished_aio > 0) {
                //timeout.tv_sec = 0;
                //timeout.tv_nsec = 100000000;
                ret = io_getevents(*ioctx, 1, MAX_NUM_EVENTS, events, NULL);
                if(ret > 0) {
                    for(int j = 0; j < ret; j++) {
#ifdef DEBUG
                        std::cout << "io completed...\n";
                        std::cout << "res1 = " << events[j].res << std::endl;
                        std::cout << "res2 = " << events[j].res2 << std::endl;
#endif
                        struct chunk_async_opt_entry_t *async_opt = (struct chunk_async_opt_entry_t *)(events[j].data);
                        async_opt->cb(async_opt->cb_arg);
                    }

                    complete_count += ret;
                    finished_aio -= ret;
                }
            }
        }
    }
    
    return NULL;
}

int FileStore::dev_unmount() {
    if(chunk_map->get_chunk_nums() > 0) {
        fct_->log()->lerror("dev could not umount, because filestore is wroking.");
        return FILESTORE_UNMOUNT_ERR;
    }

    state = FILESTORE_DOWN;
    if(io_mode == CHUNKSTORE_IO_MODE_ASYNC) {
        if(pthread_cancel(complete_thread) != 0) {
            fct_->log()->lerror("complted thread cancel failed.");
            return FILESTORE_UNMOUNT_ERR;
        }

        close(epfd);
    }

    this->persist_super();

    if(chunk_map != nullptr) {
        delete chunk_map;
        chunk_map = nullptr;
    }

    return 0;
}

FileChunk* FileStore::get_chunk_by_efd(const int efd) {
    FileChunk *chunk = chunk_map->get_chunk_by_efd(efd);
    if(chunk == nullptr) {
        fct_->log()->lerror("unknown efd: there is no corresponding chunk.");
    }
    
    return chunk;
}

uint64_t FileStore::get_chunk_size(const uint64_t chk_id) {
//该方法通过遍历chunk下的所有object的大小，获取总的chunk大小；
//因为此时chunk是没有被打开的，所有获取该chunk大小是较为困难的。
    char chunk_path[256];
    char file_name[256];
    uint64_t chunk_size;

    DIR *dir;
    struct dirent *dir_info;
    struct stat st;

    sprintf(chunk_path, "%s/%s/%" PRIx64, get_base_path(), get_data_path(), chk_id);
    if(dir = opendir(chunk_path)) {
        while(dir_info = readdir(dir)) {
            if(!strcmp(dir_info->d_name, ".") || !strcmp(dir_info->d_name, ".."))
                continue;
            
            sprintf(file_name, "%s/%s", chunk_path, dir_info->d_name);
            stat(file_name, &st);
            if(S_ISREG(st.st_mode)) {
                chunk_size += st.st_size;
            }
        }
    }
    closedir(dir);

    return chunk_size;
}

//该方法也用于获取一个未打开的chunk的size
//方法则是通过读取chunk元数据文件的扩展属性，这样可以非常快速获取信息，而不需要那么复杂
uint64_t FileStore::get_chunk_size_by_xattr(const uint64_t chk_id) {
    char chunk_path[256];
    uint64_t chunk_size;

    sprintf(chunk_path, "%s/%s/%" PRIx64, get_base_path(), get_meta_path(), chk_id);
    if(util::get_xattr(chunk_path, "user.chunk_size", &chunk_size, sizeof(uint64_t)) != 0) {
        fct_->log()->lerror("get chunk_size failed: %s", strerror(errno));
        return 0;
    }

    return chunk_size;
}