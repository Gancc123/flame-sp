#ifndef FLAME_CHUNKSTORE_FILESTORECONF_H
#define FLAME_CHUNKSTORE_FILESTORECONF_H

#include <iostream>
#include <string>
#include <vector>
#include <cinttypes>
#include <atomic>

#define BYTE    1L
#define KB      1024L
#define MB      (1024L * KB)
#define GB      (1024L * MB)
#define TB      (1024L * GB)


#define FILESTORE_CONF_VALID            0x00
#define FILESTORE_CONF_INVALID          0x01
#define FILESTORE_CONF_NO_BASE          0x02
#define FILESTORE_CONF_NO_DATA          0x03
#define FILESTORE_CONF_NO_META          0x04
#define FILESTORE_CONF_NO_JOURNAL       0x05
#define FILESTORE_CONF_NO_BACKUP        0x06
#define FILESTORE_CONF_INVALID_SIZE     0x07
#define FILESTORE_CONF_INVALID_MODE     0x08

namespace flame {
class FileStoreConf {
public:
    std::string config_path;    //配置文件路径
    std::string base_path;      //根目录目录
    std::string data_path;      //数据块文件路径
    std::string meta_path;      //元数据文件路径
    std::string journal_path;   //日志文件路径
    std::string backup_path;    //备份文件路径

    uint64_t    store_size;     //指定的filestore的大小
    uint64_t    size_unit;      //size的单位

    bool io_mode;                 //IO模式
public:
    FileStoreConf(const std::string file_path): config_path(file_path), 
                                                store_size(0), base_path(""), 
                                                data_path(""), meta_path(""), 
                                                journal_path(""), backup_path(""),
                                                size_unit(BYTE), io_mode(false) {
        
    }

    ~FileStoreConf() {

    }

    const char *get_base_path()     const;
    const char *get_data_path()     const;
    const char *get_meta_path()     const;
    const char *get_journal_path()  const;
    const char *get_backup_path()   const;
    const char *get_config_path()   const;
    const char *get_super_path()    const;
    
    bool is_async_io() const;
    bool is_dir_existed(std::string &dir);
    bool is_valid_path_str(std::string &dir);

    uint64_t get_size_in_bytes();
    std::string to_string() const;
    int load();
    int store();
    int is_valid();
    void print_conf();
};

}

#endif