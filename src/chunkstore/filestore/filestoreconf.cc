
#include <fstream>
#include <regex>
#include <sys/vfs.h>
#include <sys/statfs.h>

#include "chunkstore/filestore/filestoreconf.h"
#include "chunkstore/filestore/chunkutil.h"

using namespace flame;

int FileStoreConf::load() {
    std::fstream config_stream;
    config_stream.open(config_path, std::ios::in);
    if(!config_stream.is_open()) {
        return FILESTORE_CONF_INVALID;
    }

    std::string dump;
    std::string key;
    std::string size_str;

    while(config_stream >> key) {
        if(key == "data_path") {
            config_stream >> data_path;
        } else if(key == "meta_path") {
            config_stream >> meta_path;
        } else if(key == "base_path") {
            config_stream >> base_path;
        } else if(key == "journal_path") {
            config_stream >> journal_path;
        } else if(key == "backup_path") {
            config_stream >> backup_path;
        } else if(key == "store_size" || key == "size") {
            config_stream >> size_str;
            std::regex size_re("(\\d+)([TGMBtgm])");
            std::smatch result;
            std::string temp_size;
            std::string temp_unit;

            if(regex_match(size_str, result, size_re)) {
                temp_size = result[1];
                temp_unit = result[2];
                sscanf(temp_size.c_str(), "%" PRId64, &store_size);
                char unit = temp_unit[0];

                switch(unit) {
                    case 'T':
                    case 't':
                        size_unit = TB;
                        break;
                    case 'G':
                    case 'g':
                        size_unit = GB;
                        break;
                    case 'M':
                    case 'm':
                        size_unit = MB;
                        break;
                    case 'K':
                    case 'k':
                        size_unit = KB;
                        break;
                    default:
                        size_unit = BYTE;
                        break;
                }
            } else {
                return FILESTORE_CONF_INVALID_SIZE;
            }
        } else if(key == "config_path") {
            config_stream >> dump;
            if(dump != config_path) {
                config_stream.close();
                return FILESTORE_CONF_INVALID;
            }
        } else if(key == "io_mode") {
            config_stream >> dump;
            if(dump == "async" || dump == "Async")
                io_mode = true;
            else if(dump == "sync" || dump == "Sync") {
                io_mode = false;
            } else {
                config_stream.close();
                return FILESTORE_CONF_INVALID_MODE;
            }
        } else {
            config_stream >> dump;
        }
    }

    config_stream.close();
    return 0;
}

int FileStoreConf::store() {
    std::fstream config_stream;
    config_stream.open(config_path, std::ios::out);
    if(!config_stream.is_open())
        return FILESTORE_CONF_INVALID;

    std::string mode;
    if(io_mode)
        mode = "async";
    else
        mode = "sync";

    std::string unit;
    switch(size_unit) {
        case TB:
            unit = "T";
            break;
        case GB:
            unit = "G";
            break;
        case MB:
            unit = "M";
            break;
        case KB:
            unit = "K";
            break;
        default:
            unit = "B";
            break;
    }

    config_stream << "config_path"  << " " << config_path   << "\n";
    config_stream << "base_path"    << " " << base_path     << "\n";
    config_stream << "data_path"    << " " << data_path     << "\n";
    config_stream << "meta_path"    << " " << meta_path     << "\n";
    config_stream << "journal_path" << " " << journal_path  << "\n";
    config_stream << "backup_path"  << " " << backup_path   << "\n";
    config_stream << "store_size"   << " " << store_size    << unit << "\n";
    config_stream << "io_mode"      << " " << mode          << "\n";

    config_stream.close();
    return FILESTORE_CONF_VALID;
}

/*
 * is_dir_existed(): 判断一个给定的子目录是否存在
*/
bool FileStoreConf::is_dir_existed(std::string &dir) {
    int ret = 0;
    std::string full_path = base_path + "/" + dir;
    if((ret = util::xaccess(full_path.c_str(), F_OK)) != 0) {
        return false;
    }

    return true;
}

/*
 * is_valid_path_str(): 判断一个给定的子目录是否有效，
 * 要求给定的子目录不能为空，且只能有一级，且不允许字母，数字和下划线以外的其它字符
*/
bool FileStoreConf::is_valid_path_str(std::string &dir) {
    if(dir.length() <= 0)
        return false;
    
    for(size_t i = 0; i < dir.length(); i++) {
        if((dir[i] <= 'z' && dir[i] >= 'a') || 
                (dir[i] <= 'Z' && dir[i] >= 'A') || (dir[i] <= '9' && dir[i] >= '0') || dir[i] == '_') {
            continue;
        } else {
            return false;
        }
    }

    return true;
}

/*
 *  is_valid()：判断配置文件是否有效
*/
int FileStoreConf::is_valid() {
    int ret = 0;

    //首先判断base_path是否为空，或者是base_path是否存在
    if(base_path.length() <= 0 || util::xaccess(base_path.c_str(), F_OK) != 0)
        return FILESTORE_CONF_NO_BASE;

    //判断data_path的指定字段是否有效
    if(!is_valid_path_str(data_path))
        return FILESTORE_CONF_NO_DATA;

    //判断meta_path的指定字段是否有效
    if(!is_valid_path_str(meta_path))
        return FILESTORE_CONF_NO_META;

    //判断journal_path的指定字段是否有效
    if(!is_valid_path_str(journal_path))
        return FILESTORE_CONF_NO_JOURNAL;

    //判断backup_path的指定字段是否有效
    if(!is_valid_path_str(backup_path))
        return FILESTORE_CONF_NO_BACKUP;

    //判断指定的存储空间是否大于了磁盘的固有空间
    struct statfs fs_stat;
    if((ret = util::xstatfs(base_path.c_str(), &fs_stat)) != 0)
        return FILESTORE_CONF_INVALID;

    std::cout << "device size: " << fs_stat.f_bsize * fs_stat.f_bavail << std::endl;

    switch(size_unit) {
        case TB:
            if((fs_stat.f_bsize * fs_stat.f_bavail / TB) < store_size)
                return FILESTORE_CONF_INVALID_SIZE;
            break;
        case GB:
            if((fs_stat.f_bsize * fs_stat.f_bavail / GB) < store_size)
                return FILESTORE_CONF_INVALID_SIZE;
            break;
        case MB:
            if((fs_stat.f_bsize * fs_stat.f_bavail / MB) < store_size)
                return FILESTORE_CONF_INVALID_SIZE;
            break;
        case KB:
            if((fs_stat.f_bsize * fs_stat.f_bavail / KB) < store_size)
                return FILESTORE_CONF_INVALID_SIZE;
            break;
        default:
            if((fs_stat.f_bsize * fs_stat.f_bavail) < store_size)
                return FILESTORE_CONF_INVALID_SIZE;
            break;
    }

    return FILESTORE_CONF_VALID;
}

const char *FileStoreConf::get_base_path() const {
    return base_path.c_str();
}

const char *FileStoreConf::get_data_path() const {
    return data_path.c_str();
}

const char *FileStoreConf::get_meta_path() const {
    return meta_path.c_str();
}

const char *FileStoreConf::get_journal_path() const {
    return journal_path.c_str();
}

const char *FileStoreConf::get_backup_path() const {
    return backup_path.c_str();
}

const char *FileStoreConf::get_config_path() const {
    return config_path.c_str();
}

const char *FileStoreConf::get_super_path() const {
    std::string super_path = meta_path + "/superblock";
    return super_path.c_str();
}

uint64_t FileStoreConf::get_size_in_bytes() {
    uint64_t size = 0;
    switch(size_unit) {
        case TB:
            size = store_size * TB;
            break;
        case GB:
            size = store_size * GB;
            break;
        case MB:
            size = store_size * MB;
            break;
        case KB:
            size = store_size * KB;
            break;
        default:
            size = store_size;
            break;
    }

    return size;
}

std::string FileStoreConf::to_string() const {
    std::ostringstream oss;

    oss << "filestore" << "://" << base_path << ":" << data_path << ":";
    oss << meta_path << ":" << journal_path << ":" << backup_path << ":";
    switch(size_unit) {
        case TB:
            oss << store_size << "T";
            break;
        case GB:
            oss << store_size << "G";
            break;
        case MB:
            oss << store_size << "M";
            break;
        case KB:
            oss << store_size << "K";
            break;
        default:
            oss << store_size << "B";
            break;
    }

    oss << ":";

    if(io_mode)
        oss << "async";
    else
        oss << "sync";

    return oss.str();
}

bool FileStoreConf::is_async_io() const {
    return io_mode;
}

void FileStoreConf::print_conf() {
    std::cout << "-----config_file info-----------------\n";
    std::cout << "| config_path: "  << config_path  << "\n";
    std::cout << "| base_path: "    << base_path    << "\n";
    std::cout << "| data_path: "    << data_path    << "\n";
    std::cout << "| meta_path: "    << meta_path    << "\n";
    std::cout << "| journal_path: " << journal_path << "\n";
    std::cout << "| backup_path: "  << backup_path  << "\n";
    std::cout << "| store_size: "   << store_size   << "\n";
    std::cout << "| store_unit: "   << size_unit    << "\n";
    std::cout << "| io_mode: "      << io_mode      << "\n";
    std::cout << "--------------------------------------\n";
}