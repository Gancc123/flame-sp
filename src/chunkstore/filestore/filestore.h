//
// Created by chenxuqiang on 2018/5/30.
//

#ifndef CHUNKSTORE_FILESTORE_H
#define CHUNKSTORE_FILESTORE_H

#include <iostream>
#include <string>
#include <vector>
#include <list>

#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <libaio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "common/context.h"
#include "chunkstore/filestore/chunkstorepriv.h"
#include "chunkstore/chunkstore.h"
#include "chunkstore/filestore/filestoreconf.h"

#define DEFAULT_EPOLL_SIZE          1024
#define DEFAULT_BLOCK_SIZE_SHIFT    22

#define FILESTORE_OP_SUCCESS    0x00
#define FILESTORE_INIT_ERR      0x01
#define FILESTORE_CREATE_ERR    0x02
#define FILESTORE_REMOVE_ERR    0x03
#define FILESTORE_LOAD_ERR      0x04
#define FILESTORE_PERSIST_ERR   0x05
#define FILESTORE_NO_SUPER      0x06
#define FILESTORE_MKFS_ERR      0x07
#define FILESTORE_STOP_FAILED   0x08
#define FILESTORE_MOUNT_ERR     0x09
#define FILESTORE_FORMAT_ERR    0x0a
#define FILESTORE_UNMOUNT_ERR   0x0b

#define FILESTORE_CHUNK_OPEN_ERR    0x80
#define FILESTORE_CHUNK_CLOSE_ERR   0x82
#define FILESTORE_CHUNK_CREATE_ERR  0x83
#define FILESTORE_CHUNK_REMOVE_ERR  0x84
#define FILESTORE_CHUNK_USING       0x85
#define FILESTORE_CHUNK_NO_EXIST    0x86
#define FILESTORE_CHUNK_EXIST       0x88
#define FILESTORE_CHUNK_READ_ERR    0x89
#define FILESTORE_CHUNK_WRITE_ERR   0x89
#define FILESTORE_CHUNK_SET_XATTR_ERR   0x8a
#define FILESTORE_CHUNK_GET_XATTR_ERR   0x8b
#define FILESTORE_CHUNK_XATTR_NO_NAME   0x8c
#define FILESTORE_CHUNK_REMOVE_ALL_ERR  0x8d

#define CHUNK_OP_SUCCESS            0x00
#define CHUNK_OP_CREATE_ERR         0x01
#define CHUNK_OP_STORE_ERR          0x02
#define CHUNK_OP_LOAD_ERR           0x03
#define CHUNK_OP_READ_ERR           0x04
#define CHUNK_OP_WRITE_ERR          0x05
#define CHUNK_OP_SET_XATTR_ERR      0x06
#define CHUNK_OP_GET_XATTR_ERR      0x07
#define CHUNK_OP_GET_XATTR_NO_NAME  0x08
#define CHUNK_OP_SET_INFO_ERR       0x09
#define CHUNK_OP_GET_INFO_ERR       0x0a
#define CHUNK_OP_OBJ_CREATE_ERR     0x0b
#define CHUNK_OP_OBJ_EXIST          0x0c
#define CHUNK_OP_INIT_ENV_ERR       0x0d
#define CHUNK_OP_DESTROY_ENV_ERR    0x0e
#define CHUNK_OP_REMOVE_XATTR_ERR   0x0f

#define CHUNK_NO_NAME   0x0b

#define CHUNK_MD_TYPE_BASE 0x01
#define CHUNK_MD_TYPE_XATTR 0x02

#define CHUNK_OP_READ   0x01
#define CHUNK_OP_WRITE  0x02

#define CHUNKSTORE_IO_MODE_SYNC     0x00
#define CHUNKSTORE_IO_MODE_ASYNC    0x01

#define CHUNK_CREATE_WITH_OPEN      0x01

namespace flame {
class FileChunkMap;
class FileChunk;
class Object;

struct chunk_opts {
    uint64_t chunk_id;
    //uint64_t num_objects;
    //uint32_t chunk_size;
    uint32_t block_size_shift;
    bool thin_provision;
    uint8_t chunk_flags;
    
    //struct chunk_xattr_opts xattrs;
public:
    chunk_opts() {

    }

    chunk_opts(const chunk_opts& opts) {
        chunk_id = opts.chunk_id;
        block_size_shift = opts.block_size_shift;
        thin_provision = opts.thin_provision;
    }

    chunk_opts(uint64_t cid) {
        chunk_id = cid;
        block_size_shift = (1UL << 22);
        thin_provision = false;
    }

    void print_opts() {
        std::cout << "chunk_id: " << chunk_id << std::endl;
        std::cout << "block_size_shift: " << block_size_shift << std::endl;
        std::cout << "thin_provision: " << thin_provision << std::endl;
    }

    ~chunk_opts() {

    }
};

struct chunk_xattr_opts {
    size_t count;
    char *name;
    char *value;
};

struct chunk_base_descriptor {
    uint8_t type;
    uint32_t length;

    uint64_t chunk_id;
    uint64_t vol_id;
    uint64_t index;

    uint64_t stat;
    uint64_t spolicy;
    uint64_t flags;
    uint64_t size;
    uint64_t used;
    uint64_t create_time;
    uint64_t dst_id;
    uint64_t dst_ctime;
    uint64_t xattr_num;

    uint8_t block_size_shift;
    uint64_t chunk_read_counter;
    uint64_t chunk_write_counter;
};

struct chunk_xattr {
    uint32_t index;
    std::string name;
    std::string value;
public:
    chunk_xattr():index(0), name(""), value("") { }
    chunk_xattr(uint32_t _index, 
                const std::string _name, 
                const std::string _value) :index(_index), name(_name), value(_value) {

    }
    chunk_xattr(const struct chunk_xattr& _xattr) {
        index   = _xattr.index;
        name    = _xattr.name;
        value   = _xattr.value;
    }

    ~chunk_xattr() { }
};

struct chunk_xattr_descriptor {
    uint8_t type;
    uint32_t length;

    uint16_t name_length;
    uint16_t value_length;

    char key_value[0];
};

enum FileChunkState {

};

enum FileStoreState {
    FILESTORE_FORMATED,
    FILESTORE_MOUNTED,
    FILESTORE_DOWN
};

class FileStore : public ChunkStore 
{
private:
    /*
    *ChunkStore公有字段
    */
    uint64_t id;                        //id: 新增字段, 表示chunkstore的id            
    uint64_t ftime;                     //ftime： 格式化时间，单位ms
    std::atomic<uint64_t> used;         //used： 已分配的存储空间
    std::atomic<uint64_t> size;         //size： chunkstore的存储空间
    std::atomic<uint64_t> total_chunks; //total_chunks： chunk的个数
    char cluster_name[256];             //cluster_name：表示所处的cluster的name
    char name[256];                     //name: 该chunkstore的name

    /*
     * FileStore私有字段
    */
    FileStoreConf config_file;   //配置文件实例

    pthread_attr_t complete_thread_attr;    //线程属性
    pthread_t complete_thread;              //完成线程，用于获取所有的IO结果

    /*
     * chunk_map, 用于保存所有打开的chunk的元数据信息。
     */
    FileChunkMap *chunk_map;

    /*
     * epoll_fd: 用于事件触发，在异步IO模型下，用于收集IO执行的结果
     */
    int epfd;

    /*
    io_mode：IO模式，表示ChunkStore采用的IO模型，有同步模型和异步模型。
    同步模型：意味着同步执行read,write, 并阻塞直到返回结果，不需要完成线程获取返回结果，
    异步模型：意味着异步执行read,write, 不阻塞，所以需要完成线程获取返回结果。
     */
    int io_mode;

    /*
    *state: FileState的目前所处的状态
    */
    FileStoreState state;

    //辅助私有函数
    int check_path_len(const char *path);
    bool is_meta_store(const char *dirpath);

    //持久化superblock和加载superblock
    int persist_super();
    int load_super();

    int remove_all_chunks(const char *meta_dir);
public:
    FlameContext *fct;
    /*
    do_process_result：函数指针，用于处理读写请求结果，由用户自定义。
     */
    int (*do_process_result)(void *arg);
    
    FileStore(FlameContext *_fct): ChunkStore(_fct), chunk_map(nullptr), config_file("./config"){
        epfd = 0;
        io_mode = CHUNKSTORE_IO_MODE_SYNC;
        total_chunks = 0;
        do_process_result = nullptr;
    }

    FileStore(FlameContext *_fct, std::string config_file_path): 
                            ChunkStore(_fct), chunk_map(nullptr), config_file(config_file_path) {
        epfd = 0;
        io_mode = CHUNKSTORE_IO_MODE_SYNC;
        total_chunks = 0;
        do_process_result = nullptr;
    }

    ~FileStore() {

    }

    static FileStore *create_filestore(FlameContext *fct, const std::string& url);
    //static FileStore *create_filestore(FlameContext *fct, const std::string& super_file_path);

    static void *completeLoopFunc(void *arg);

    //辅助共有函数：这些函数可能会被其他内部模块调用    
    const char *get_base_path()     const;
    const char *get_data_path()     const;
    const char *get_meta_path()     const;
    const char *get_journal_path()  const;
    const char *get_backup_path()   const;
    const char *get_super_path()    const;

    void close_active_chunks();

    int get_epfd()      const;
    FlameContext *get_flame_context();
    FileChunk *get_chunk_by_efd(const int efd);
    uint64_t get_chunk_size(const uint64_t chk_id);
    uint64_t get_chunk_size_by_xattr(const uint64_t chk_id);
    int chunk_close(Chunk *chunk);
    int stop();

    //接口函数
    int get_info(cs_info_t& info) const;
    int set_info(const cs_info_t& info);
    std::string get_driver_name() const;
    std::string get_config_info() const;
    std::string get_runtime_info() const;

    int get_io_mode() const;
    bool is_support_mem_persist() const;
    bool is_mounted();

    int dev_check();
    int dev_format();
    int dev_mount();
    int dev_unmount();

    int chunk_create(uint64_t chk_id, const chunk_create_opts_t& opts);
    int chunk_remove(uint64_t chk_id);
    bool chunk_exist(uint64_t chk_id);
    std::shared_ptr<Chunk> chunk_open(uint64_t chk_id);
};

class FileChunk : public Chunk {
private:
    FileStore *filestore;
    uint64_t    chk_id;
    uint64_t    volume_id;
    uint32_t    volume_index;
    uint32_t    chunk_stat;
    uint32_t    chunk_spolicy;
    uint32_t    flags;
    //uint64_t    size;     //在多线程环境中，改用原子类型
    //uint64_t    used;     //在多线程环境中，改用原子类型
    uint64_t    ctime;
    uint64_t    dst_id;    // 只在状态为迁移时有效
    uint64_t    dst_ctime;

    pthread_mutex_t mtx;
    pthread_cond_t cond;
    pthread_rwlock_t rwlock;

    std::atomic<uint64_t> chunk_size;
    std::atomic<uint64_t> chunk_used;

    io_context_t ioctx;
    int efd;
    uint8_t block_size_shift;

    FileChunkState state;

    std::atomic<uint64_t> open_ref;
    std::atomic<uint64_t> read_counter;
    std::atomic<uint64_t> write_counter;

    pthread_rwlock_t active_lock;
    std::map<uint64_t, Object *>active_list;

    pthread_rwlock_t xattr_lock;
    std::atomic<uint64_t> xattr_num;
    std::list<struct chunk_xattr> xattr_list;

private:
    int chunk_serial_base(struct chunk_base_descriptor *desc);
    int chunk_serial_xattrs(struct chunk_xattr_descriptor **chunk_xattr_descs);
    int chunk_deserial_base(struct chunk_base_descriptor *chunk_base_desc);
    int chunk_deserial_xattr(struct chunk_xattr *xt, struct chunk_xattr_descriptor *cxdesc, char *kv, uint32_t index);

    int create_object(uint64_t object_id, uint64_t object_size);
    int prepare_object_iocb_sync(struct oiocb *iocbs, uint32_t count, void *payload, uint64_t length, off_t offset, int opcode);
    int io_submit_sync(struct oiocb* iocbs, uint32_t count);
    int prepare_object_iocb(struct iocb **iocbs, uint32_t count, void *payload, uint64_t length, off_t offset, int opcode, void *extra_arg);
    int close_object(Object *obj);
    int close_object(const uint64_t oid);

    Object *open_object(const uint64_t oid);

    /*
     * 该函数为核心的异步IO执行函数
    */
    int read_async(void *buff, uint64_t length, off_t offset, void *extra_arg);
    int write_async(void *buff, uint64_t length, off_t offset, void *extra_arg);

public:
    FileChunk(FileStore *_filestore, 
                FlameContext* _fct):Chunk(_fct), filestore(_filestore), chunk_size(0), chunk_used(0), block_size_shift(22),
                                    open_ref(0), read_counter(0), write_counter(0), xattr_num(0) {
        pthread_mutex_init(&mtx, NULL);
        pthread_rwlock_init(&rwlock, NULL);
        pthread_cond_init(&cond, NULL);

        pthread_rwlock_init(&active_lock, NULL);
        pthread_rwlock_init(&xattr_lock, NULL);
    }

    ~FileChunk() {
        pthread_mutex_destroy(&mtx);
        pthread_cond_destroy(&cond);
        pthread_rwlock_destroy(&rwlock);
        pthread_rwlock_destroy(&active_lock);
        pthread_rwlock_destroy(&xattr_lock);
    }

    //辅助公有函数
    int close_active_objects();
    int store(const char *store_path);
    int load(const char *load_path);
    int create(const struct chunk_create_opts_t &opts);
    void set_chunk_id(uint64_t chunk_id);

    io_context_t* get_ioctx();

    int get_efd() const;
    
    uint32_t get_xattr_nums();
    uint64_t get_chunk_id() const;
    uint64_t get_open_ref();
    uint64_t get_write_counter();
    uint64_t get_read_counter();
    uint64_t get_object_size();
    uint64_t get_chunk_size() const ;
    uint64_t get_used_size();

    void     set_ref_counter(uint64_t counter);
    void     ref_counter_sub(uint64_t increment);
    void     ref_counter_add(uint64_t increment);
    void     write_counter_add(uint64_t increment);
    void     read_counter_add(uint64_t increment); 

    int      init_chunk_async_env();
    int      destroy_chunk_async_env();

    //接口函数
    int      get_info(chunk_info_t& info) const;
    uint64_t used() const;
    uint64_t size() const;
    uint32_t stat() const;
    uint64_t vol_id() const;
    uint32_t vol_index() const;
    uint32_t spolicy() const;
    bool is_preallocated() const;

    int get_xattr(const std::string& name, std::string& value);
    int set_xattr(const std::string& name, const std::string& value);

    int remove_xattr(const std::string& name);

    /*
     * 在此我们提供了两类接口，第一类是直接调用同步和异步IO的接口，其大致为read(write)_(a)sync。
     * 第二类接口则是直接通过调用read(write)_chunk，通过内部的IO模式来判定执行异步IO或者是同步IO。
    */
    int read_sync(void* buff, uint64_t off, uint64_t len);      //同步读操作
    int write_sync(void* buff, uint64_t off, uint64_t len);     //同步写操作

    int read_async(void* buff, uint64_t off, uint64_t len, chunk_opt_cb_t cb, void* cb_arg);    //异步读操作
    int write_async(void* buff, uint64_t off, uint64_t len, chunk_opt_cb_t cb, void* cb_arg);   //异步写操作

    //这两个函数作为接口函数，而异步和同步应该由filestore的io_mode来决定
    int read_chunk(void *buff, uint64_t off, uint64_t len, void *extra_arg);
    int write_chunk(void *buff, uint64_t off, uint64_t len, void *extra_arg);

};


struct chunk_store_opts {
    /*
    chunk_size: chunk的大小，默认为4GB
     */    
    uint64_t chunk_size;
    /*
    epoll_size: epoll监听的初始值。默认为1024
     */
    uint32_t epoll_size;
    /*
    io_mode: chunkStore的IO模型，默认为同步IO。
     */
    int io_mode;

    /*
    io_done:一个函数指针，用于处理完成的读写请求。
     */
    int (*io_done)(void *arg);
    /*
    fill: 表示该结构是否被填充了， true表示用户指定opts，false表示用户需要从默认的目录去初始化chunkstore。
     */
    bool fill;

    char default_base_path[256];
    char default_super[256];
    char default_data_path[256];
    char default_meta_path[256];
    char default_journal_path[256];
    char default_backup_path[256];
public:
    chunk_store_opts() {
        chunk_size = 4L * 1024L * 1024L * 1024L;
        epoll_size = DEFAULT_EPOLL_SIZE;
        io_mode = CHUNKSTORE_IO_MODE_SYNC;
        io_done = nullptr;
        sprintf(default_base_path, "%s", "/mnt/chunkstore");
        sprintf(default_super, "%s", ".superfile");
        sprintf(default_data_path, "%s", "store");
        sprintf(default_meta_path, "%s", "meta");
        sprintf(default_journal_path, "%s", "journal");
        sprintf(default_backup_path, "%s", "backup");

        fill = false;
    }

    bool is_fill() {
        return fill;
    }

    void set_fill(bool flag) {
        fill = flag;
    }

    void set_chunk_store_opts(uint64_t cs, uint32_t es, int mode, 
                        const char *base, const char *super, 
                        const char *data, const char *meta, 
                        const char *journal, const char *backup) {
        chunk_size = cs;
        epoll_size = es;
        io_mode = mode;
        sprintf(default_base_path, "%s", base);
        sprintf(default_super, "%s", super);
        sprintf(default_data_path, "%s", data);
        sprintf(default_meta_path, "%s", meta);
        sprintf(default_journal_path, "%s", journal);
        sprintf(default_backup_path, "%s", backup);

        fill = true;
    }

    void set_chunk_store_opts(const struct chunk_store_opts& opts) {
        chunk_size = opts.chunk_size;
        epoll_size = opts.epoll_size;
        io_mode = opts.io_mode;
        io_done = opts.io_done;
        
        sprintf(default_base_path, "%s", opts.default_base_path);
        sprintf(default_super, "%s", opts.default_super);
        sprintf(default_data_path, "%s", opts.default_data_path);
        sprintf(default_meta_path, "%s", opts.default_meta_path);
        sprintf(default_journal_path, "%s", opts.default_journal_path);
        sprintf(default_backup_path, "%s", opts.default_backup_path);

        fill = true;
    }

    void print_opts() {
        std::cout << "chunk_size = " << chunk_size << "\n";
        std::cout << "epoll_size = " << epoll_size << "\n";
        std::cout << "io_mode = " << io_mode << "\n";
        std::cout << "default_base_path: " << default_base_path << "\n";
        std::cout << "default_super: " << default_super << "\n";
        std::cout << "default_data_path: " << default_data_path << "\n";
        std::cout << "default_meta_path: " << default_meta_path << "\n";
        std::cout << "default_journal_path: " << default_journal_path << "\n";
        std::cout << "default_backup_path: " << default_backup_path << "\n";
    }

    ~chunk_store_opts() { }
};

struct chunk_store_descriptor {
    //char base_path[256];
    //char data_path[256];
    //char meta_path[256];
    //char journal_path[256];
    //char backup_path[256];
    //char super_path[256];

    char cluster_name[256];
    char name[256];

    uint64_t id;
    uint64_t size;
    uint64_t used;
    uint64_t ftime;
    uint64_t total_chunks;
};
}
#endif
