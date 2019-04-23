#include "chunkstore/simstore/simstore.h"
#include "include/retcode.h"
#include "util/utime.h"

#include "chunkstore/log_cs.h"

#include <memory>
#include <sstream>
#include <fstream>
#include <cctype>
#include <exception>
#include <vector>
#include <regex>

using namespace std;

namespace flame {

void simstore_chunk_t::init_blocks__() {
    uint32_t blk_num = info.size / SIMSTORE_BLOCK_SIZE + (info.size % SIMSTORE_BLOCK_SIZE ? 1 : 0);
    uint32_t idx = 0;
    while (idx < blk_num) {
        simstore_block_t blk;
        blocks.push_back(blk);
        idx++;
    }
}

SimStore* SimStore::create_simstore(FlameContext* fct, const std::string& url) {
    string pstr = "^(\\w+)://(\\d+)([tmgTMG])(:.+)?$";
    regex pattern(pstr);
    smatch result;
    if (!regex_match(url, result, pattern)) {
        fct->log()->lerror("format error for url: %s", url.c_str());
        return nullptr;
    }

    string driver = result[1];
    string size_str = result[2];
    string size_unit = result[3];
    string path = result[4];
    if (!path.empty())
        path = path.substr(1);//去冒号
    uint64_t size;
    try {
        size = stoull(size_str);
    } catch (exception& e) {
        fct->log()->lerror("size (%s) is invalid", size_str.c_str());
        return nullptr;
    }

    string units = "MmGgTt";
    size_t pos = units.find(size_unit[0]);
    if (pos == string::npos) {
        fct->log()->lerror("size unit (%s) is invalid", size_unit.c_str());
        return nullptr;
    }

    switch (pos / 2) {
    case 0:
        size <<= 20;
        break;
    case 1:
        size <<= 30;
        break;
    case 2:
        size <<= 40;
        break;
    }

    return new SimStore(fct, size, path);
}

SimStore* SimStore::create_simstore(FlameContext* fct, uint64_t size, const std::string& bk_file) {
    return new SimStore(fct, size, bk_file);
}

int SimStore::get_info(cs_info_t& info) const {
    info = info_;
    return 0;
}

std::string SimStore::get_driver_name() const {
    return "SimStore";
}

std::string SimStore::get_config_info() const {
    ostringstream oss;
    oss << get_driver_name() << "://" << (info_.size >> 30) << "G";
    if (!bk_file_name_.empty())
        oss << ":" << bk_file_name_;
    return oss.str();
}

std::string SimStore::get_runtime_info() const {
    return "SimStore: Runtime Information";
}

int SimStore::get_io_mode() const {
    return ChunkStore::IOMode::ASYNC;
}

int SimStore::dev_check() {
    // 当配置bk_file时，检查文件是否存在
    if (!bk_file_name_.empty()) {
        fstream fs;
        fs.open(bk_file_name_, fstream::in);
        if (!fs.is_open()){
            return ChunkStore::DevStatus::NONE;
        }     
    }
    return ChunkStore::DevStatus::CLT_IN;
}

int SimStore::dev_format() {
    if (!bk_file_name_.empty()) {
        fstream fs;
        fs.open(bk_file_name_, fstream::out);
        if (!fs.is_open())
            return RC_OBJ_NOT_FOUND;
    }
    formated_ = true;
    fct_->log()->linfo("simstore: device (%llu:%s) format", size_, bk_file_name_.c_str());
    return RC_SUCCESS;
}

int SimStore::dev_mount() {
    if (!bk_file_name_.empty()) {
        int r = backup_load__();
        if (r != 0) {
            fct_->log()->lerror("simstore: load backup faild");
            return RC_FAILD;
        }
    } else 
        info_init__();
        
    mounted_ = true;
    fct_->log()->linfo("simstore: device mount success");
    return RC_SUCCESS;
}

int SimStore::dev_unmount() {
    if (!mounted_)
        return RC_SUCCESS;
    if (!bk_file_name_.empty()) {
        int r = backup_store__();
        if (r == 0) {
            mounted_ = false;
            return RC_SUCCESS;
        }
        fct_->log()->lerror("faild to store data to backup file");
        return RC_FAILD;
    }
    mounted_ = false;
    fct_->log()->linfo("simstore: device unmount success");
    return RC_SUCCESS;
}

bool SimStore::is_mounted() {
    return mounted_;
}

int SimStore::flush() {
    if (!bk_file_name_.empty()) {
        int r = backup_store__();
        if (r == 0) {
            return RC_SUCCESS;
        }
        fct_->log()->lerror("faild to store data to backup file");
        return RC_FAILD;
    }
    return RC_SUCCESS;
}

int SimStore::chunk_create(uint64_t chk_id, const chunk_create_opts_t& opts) {
    auto it = chk_map_.find(chk_id);
    if (it != chk_map_.end()) 
        return RC_OBJ_EXISTED;
    
    simstore_chunk_t chk;
    chunk_info_t& chk_info = chk.info;
    chk_info.chk_id = chk_id;
    chk_info.vol_id = opts.vol_id;
    chk_info.index = opts.index;
    chk_info.stat = 0;
    chk_info.spolicy = opts.spolicy;
    chk_info.flags = opts.flags;
    chk_info.size = opts.size;
    if (opts.is_prealloc())
        chk_info.used = opts.size;
    chk_info.dst_id = 0;
    chk_info.dst_ctime = 0;
    
    chk.init_blocks__();
    chk_map_[chk_id] = chk;
    info_.chk_num++;
    fct_->log()->linfo("simstore: create chunk: chk_id(%llu), vol_id(%llu), index(%u), size(%llu)", 
        chk_info.chk_id, chk_info.vol_id, chk_info.index, chk_info.size);
    return RC_SUCCESS;
}

int SimStore::chunk_remove(uint64_t chk_id) {
    auto it = chk_map_.find(chk_id);
    if (it == chk_map_.end())
        return RC_OBJ_NOT_FOUND;
    
    chk_map_.erase(chk_id);
    info_.chk_num--;
    fct_->log()->linfo("simstore: remove chunk: chk_id(%llu)", chk_id);
    return RC_SUCCESS;
}

bool SimStore::chunk_exist(uint64_t chk_id) {
    return chk_map_.find(chk_id) != chk_map_.end();
}

shared_ptr<Chunk> SimStore::chunk_open(uint64_t chk_id) {
    auto it = chk_map_.find(chk_id);
    if (it == chk_map_.end())
        return nullptr;
    fct_->log()->linfo("simstore: open chunk: chk_id(%llu)", chk_id);
    return shared_ptr<Chunk>(new SimChunk(fct_, this, &it->second));
}

int SimStore::info_init__() {
    info_.id = 0;
    info_.cluster_name = fct_->cluster_name();
    info_.name = fct_->node_name();
    info_.size = size_;
    info_.used = 0;
    info_.ftime = utime_t::now().to_usec();
    info_.chk_num = 0;
    return RC_SUCCESS;
}

int SimStore::backup_load__() {
    fstream fin;
    fin.open(bk_file_name_, fstream::in);
    if (!fin.is_open()) {
        fct_->log()->lerror("simstore: backup file not found");
        return RC_OBJ_NOT_FOUND;
    }

    return ld_upper__(fin);
}

int SimStore::backup_store__() {
    fstream fout;
    fout.open(bk_file_name_, fstream::out);
    if (!fout.is_open()) {
        fct_->log()->linfo("simstore: backup file not found");
        return RC_OBJ_NOT_FOUND;
    }
    
    // store ChunkStore Information
    fout << "@kv" << endl;
    fout << "id=" << info_.id << endl;
    fout << "cluster_name=" <<  info_.cluster_name << endl;
    fout << "name=" << info_.name << endl;
    fout << "size=" << info_.size << endl;
    fout << "used=" << info_.used << endl;
    fout << "ftime=" << info_.ftime << endl; 
    fout << "chk_num=" << info_.chk_num << endl;

    // store Chunk Label
    fout << "@table" << endl;
    fout << "chk_id" << SIMSTORE_SEP_L1;
    fout << "vol_id" << SIMSTORE_SEP_L1;
    fout << "index" << SIMSTORE_SEP_L1;
    fout << "stat" << SIMSTORE_SEP_L1;
    fout << "spolicy" << SIMSTORE_SEP_L1;
    fout << "flags" << SIMSTORE_SEP_L1;
    fout << "size" << SIMSTORE_SEP_L1;
    fout << "used" << SIMSTORE_SEP_L1;
    fout << "ctime" << SIMSTORE_SEP_L1;
    fout << "dst_id" << SIMSTORE_SEP_L1;
    fout << "dst_time" << SIMSTORE_SEP_L1;
    fout << "xattr" << SIMSTORE_SEP_L1;
    fout << "blocks" << SIMSTORE_SEP_L1;
    fout << endl;

    // store Chunk Information
    for (auto it = chk_map_.begin(); it != chk_map_.end(); it++) {
        simstore_chunk_t& chk = it->second;
        
        chunk_info_t& info = chk.info;
        fout << info.chk_id << SIMSTORE_SEP_L1;
        fout << info.vol_id << SIMSTORE_SEP_L1;
        fout << info.index << SIMSTORE_SEP_L1;
        fout << info.stat << SIMSTORE_SEP_L1;
        fout << info.spolicy << SIMSTORE_SEP_L1;
        fout << info.flags << SIMSTORE_SEP_L1;
        fout << info.size << SIMSTORE_SEP_L1;
        fout << info.used << SIMSTORE_SEP_L1;
        fout << info.ctime << SIMSTORE_SEP_L1;
        fout << info.dst_id << SIMSTORE_SEP_L1;
        fout << info.dst_ctime << SIMSTORE_SEP_L1;

        // xattr
        for (auto xit = chk.xattr.begin(); xit != chk.xattr.end(); xit++) {
            fout << xit->first << SIMSTORE_SEP_KW << xit->second << SIMSTORE_SEP_L2;
        }
        fout << SIMSTORE_SEP_L1;

        // blocks
        for (uint32_t blk_id = 0; blk_id < chk.blocks.size(); blk_id++) {
            simstore_block_t& blk = chk.blocks[blk_id];
            fout << blk.ctime << SIMSTORE_SEP_L2;
            fout << blk.cnt.rd << SIMSTORE_SEP_L2;
            fout << blk.cnt.wr << SIMSTORE_SEP_L2;
        }
        fout << SIMSTORE_SEP_L1;

        fout << endl;
    }

    fout << "@end" << endl;

    fout.close();
    return RC_SUCCESS;
}

int SimStore::ld_upper__(fstream& fin) {
    char chr;
    while (fin.get(chr)) {
        if (isspace(chr)) 
            continue;
        else if (chr == '#')
            ld_annotation__(fin);
        else if (chr == '@') {
            int r = ld_sub_name__(fin);
            if (r != 0) {
                fct_->log()->lerror("simstore: load sub name faild");
                return r;
            }
        }
    }
    return RC_SUCCESS;
}

int SimStore::ld_annotation__(fstream& fin) {
    char chr;
    while (fin.get(chr)) {
        if (chr == '\n')
            break;
    }
    return 0;
}

int SimStore::ld_sub_name__(fstream& fin) {
    char chr;
    string name;
    while (fin.get(chr)) {
        if (chr == '\n')
            break;
        name.push_back(chr);
    }

    if (name == "kv") {
        return ld_sub_kv__(fin);
    } else if (name == "table") {
        return ld_sub_table__(fin);
    } else if (name == "end") {
        return 0;
    } else {
        fct_->log()->lerror("simstore: faild in sub name");
        return 1;
    }
}

int SimStore::ld_sub_kv__(fstream& fin) {
    char chr;
    string key;
    string value;
    bool in_value = false;
    while (fin.get(chr)) {
        if (in_value) {
            if (chr == '\n') {
                if (key.empty() || value.empty()) {
                    fct_->log()->lerror("simstore: key or value is empty");
                    return 2;
                }
                
                int r = st_info__(key, value);
                if (r != 0) {
                    fct_->log()->lerror("simstore: set key-value faild");
                    return 2;
                }

                key.clear();
                value.clear();
                in_value = false;
            } else
                value.push_back(chr);
        } else {
            if (chr == '\n') {
                fct_->log()->lerror("simstore: format invalid");
                return 2;
            } else if (chr == SIMSTORE_SEP_KW)
                in_value = true;
            else if (chr == '@') {
                if (key.empty())
                    return ld_sub_name__(fin);
                key.push_back(chr);
            } else
                key.push_back(chr);
        }
    }
}

int SimStore::ld_sub_table__(fstream& fin) {
    vector<string> header;
    
    int r = ld_sub_table_header__(fin, header);
    if (r != 0) {
        fct_->log()->lerror("simstore: load sub table header faild");
        return 3;
    }
    
    while (true) {
        r = ld_sub_table_row__(fin, header);
        if (r == 1)
            break;
        if (r > 1) {
            fct_->log()->lerror("simstore: load sub table row faild");
            return 3;
        }
    }
    return 0;
}

int SimStore::ld_sub_table_header__(fstream& fin, vector<string>& header) {
    char chr;
    string col;
    while (fin.get(chr)) {
        if (chr == '\n') 
            return 0;
        else if (chr == SIMSTORE_SEP_L1) {
            if (col.empty()) {
                fct_->log()->lerror("simstore: sub table col name is empty");
                return 1;
            }
            header.push_back(col);
            col.clear();
        } else 
            col.push_back(chr);
    }
    fct_->log()->lerror("simstore: sub table header format invalid");
    return 2;
}

int SimStore::ld_sub_table_row__(fstream& fin, vector<string>& header) {
    char chr;
    string value;
    int cols = 0;
    simstore_chunk_t chk;
    while (fin.get(chr)) {
        if (chr == '@') {
            if (cols == 0) {
                int r = ld_sub_name__(fin);
                if (r != 0) {
                    fct_->log()->lerror("simstore: load sub name faild");
                    return r;
                }
            } else {
                fct_->log()->lerror("simstore: sub table row format invalid");
                return 2;
            }
        } else if (chr == '\n') {
            if (value.empty() && cols == 0)
                return 1;   // is tail
            if (cols != header.size()) {
                fct_->log()->lerror("simstore: sub table row format invalid");
                return 2;   // 2 is faild
            }
            chk_map_[chk.info.chk_id] = chk;
        } else if (chr == SIMSTORE_SEP_L1) {
            if (value.empty() || cols >= header.size()) {
                fct_->log()->lerror("simstore: sub table row value invalid");
                return 2;
            }

            int r = st_chunk__(chk, header[cols], value);
            if (r != 0) {
                fct_->log()->lerror("simstore: set chunk faild");
                return 2;
            }

            cols++;
        } else 
            value.push_back(chr);
    }
}

int SimStore::st_info__(const std::string& key, const std::string& value) {
    if (key == "id") {
        try {
            uint64_t id = stoull(value);
            info_.id = id;
        } catch (exception& e) {
            fct_->log()->lerror("simstore: set id faild");
            return 1;
        }
    } else if (key == "cluster_name") {
        info_.cluster_name = value;
    } else if (key == "name") {
        info_.name = value;
    } else if (key == "size") {
        try {
            uint64_t size = stoull(value);
            info_.size = size;
        } catch (exception& e) {
            fct_->log()->lerror("simstore: set size faild");
            return 1;
        }
    } else if (key == "used") {
        try {
            uint64_t used = stoull(value);
            info_.used = used;
        } catch (exception& e) {
            fct_->log()->lerror("simstore: set used faild");
            return 1;
        }
    } else if (key == "ftime") {
        try {
            uint64_t ftime = stoull(value);
            info_.ftime = ftime;
        } catch (exception& e) {
            fct_->log()->lerror("simstore: set ftime faild");
            return 1;
        }
    } else if (key == "chk_num") {
        try {
            uint32_t chk_num = stoull(value);
            info_.chk_num = chk_num;
        } catch (exception& e) {
            fct_->log()->lerror("simstore: set chk_num faild");
            return 1;
        }
    } else 
        return 2;
    return 0;
}

int SimStore::st_chunk__(simstore_chunk_t& chk, const string& key, const string& value) {
    if (key == "xattr") {
        return st_chunk_xattr__(chk, value);
    } else if (key == "blocks") {
        return st_chunk_blocks__(chk, value);
    }

    uint64_t v;
    try {
        v = stoull(value);
    } catch (exception& e) {
        fct_->log()->lerror("simstore: parse value faild");
        return 1;
    }

    if (key == "chk_id") {
        chk.info.chk_id = v;
        return 0;
    } else if (key == "vol_id") {
        chk.info.vol_id = v;
        return 0;
    } else if (key == "size") {
        chk.info.size = v;
        return 0;
    } else if (key == "used") {
        chk.info.used = v;
        return 0;
    } else if (key == "ctime") {
        chk.info.ctime = v;
        return 0;
    } else if (key == "dst_id") {
        chk.info.dst_id = v;
        return 0;
    } else if (key == "dst_ctime") {
        chk.info.dst_ctime = v;
        return 0;
    } else if (key == "index") {
        chk.info.index = v;
    } else if (key == "stat") {
        chk.info.stat = v;
    } else if (key == "spolicy") {
        chk.info.spolicy = v;
    } else if (key == "flags") {
        chk.info.flags = v;
    }

    return 0;
}

int SimStore::st_chunk_xattr__(simstore_chunk_t& chk, const string& value) {
    size_t off = 0;
    size_t pos;
    
    while ((pos = value.find(SIMSTORE_SEP_L2, off)) != string::npos) {
        size_t eq_pos = value.find(SIMSTORE_SEP_KW, off);
        if (eq_pos == string::npos)
            return 1;
        string key = value.substr(off, eq_pos - off);
        string val = value.substr(eq_pos + 1, pos - eq_pos -1);
        if (key.empty() || val.empty())
            return 1;
        chk.xattr[key] = val;
        off = pos + 1;
    }
    return 0;
}
    
int SimStore::st_chunk_blocks__(simstore_chunk_t& chk, const string& value) {
    size_t off = 0;
    size_t pos;
    while ((pos = value.find(SIMSTORE_SEP_L2, off)) != string::npos) {
        int idx = 0;
        size_t doff = off;
        size_t dpos;
        simstore_block_t blk;
        while ((dpos = value.find(SIMSTORE_SEP_L3, doff)) != string::npos) {
            string val = value.substr(doff, dpos - doff);
            uint64_t v;
            try {
                v = stoull(val);
            } catch (exception& e) {
                return 1;
            }

            if (idx == 0) {
                blk.ctime = v;
            } else if (idx == 1) {
                blk.cnt.rd = v;
            } else if (idx == 2) {
                blk.cnt.wr = v;
            } else 
                return 1;
            doff = dpos + 1;
        }
        chk.blocks.push_back(blk);
        off = pos + 1;
    }
    return 0;
}

int SimChunk::get_info(chunk_info_t& info) const {
    info = chk_->info;
    return RC_SUCCESS;
}

uint64_t SimChunk::size() const {
    return chk_->info.size;
}

uint64_t SimChunk::used() const {
    return chk_->info.used;
}

uint32_t SimChunk::stat() const {
    return chk_->info.stat;
}

uint64_t SimChunk::vol_id() const {
    return chk_->info.vol_id;
}

uint32_t SimChunk::vol_index() const {
    return chk_->info.index;
}

uint32_t SimChunk::spolicy() const {
    return chk_->info.spolicy;
}

bool SimChunk::is_preallocated() const {
    return chk_->info.flags & CHK_FLG_PREALLOC;
}

int SimChunk::read_sync(void* buff, uint64_t off, uint64_t len) {
    rd_count__(off, len);
    fct_->log()->linfo("simstore: read sync chk_id(%llu), off(%llu), len(%llu)",
        chk_->info.chk_id, off, len);
    return RC_SUCCESS;
}

int SimChunk::write_sync(void* buff, uint64_t off, uint64_t len) {
    wr_count__(off, len);
    fct_->log()->linfo("simstore: write sync chk_id(%llu), off(%llu), len(%llu)",
        chk_->info.chk_id, off, len);
    return RC_SUCCESS;
}

int SimChunk::get_xattr(const std::string& name, std::string& value) {
    auto it = chk_->xattr.find(name);
    if (it == chk_->xattr.end())
        return RC_OBJ_NOT_FOUND;
    value = chk_->xattr[name];
    fct_->log()->linfo("simstore: xattr get chk_id(%llu), name(%s), value(%s)",
        chk_->info.chk_id, name.c_str(), value.c_str());
    return RC_SUCCESS;
}

int SimChunk::set_xattr(const std::string& name, const std::string& value) {
    chk_->xattr[name] = value;
    fct_->log()->linfo("simstore: xattr set chk_id(%llu), name(%s), value(%s)",
        chk_->info.chk_id, name.c_str(), value.c_str());
    return RC_SUCCESS;
}

int SimChunk::read_async(void* buff, uint64_t off, uint64_t len, chunk_opt_cb_t cb, void* cb_arg) {
    rd_count__(off, len);
    fct_->log()->linfo("simstore: read async chk_id(%llu), off(%llu), len(%llu)",
        chk_->info.chk_id, off, len);
    if (cb != nullptr)
        cb(cb_arg);
    return RC_SUCCESS;
}

int SimChunk::write_async(void* buff, uint64_t off, uint64_t len, chunk_opt_cb_t cb, void* cb_arg) {
    wr_count__(off, len);
    fct_->log()->linfo("simstore: write async chk_id(%llu), off(%llu), len(%llu)",
        chk_->info.chk_id, off, len);
    if (cb != nullptr)
        cb(cb_arg);
    return RC_SUCCESS;
}

void SimChunk::blk_range__(uint32_t& begin, uint32_t& end, uint64_t off, uint64_t len) {
    int chk_size = chk_->info.size;
    uint64_t sz;
    end = begin = off / SIMSTORE_BLOCK_SIZE;
    sz = off % SIMSTORE_BLOCK_SIZE;
    if (!sz) sz = SIMSTORE_BLOCK_SIZE;
    len -= sz;
    while (len > 0) {
        end++;
        len -= SIMSTORE_BLOCK_SIZE;
    }
}

void SimChunk::rd_count__(uint64_t off, uint64_t len) {
    uint32_t begin, end;
    blk_range__(begin, end, off, len);
    for (int idx = begin; idx < end; idx++) {
        chk_->blocks[idx].cnt.rd++;
    }
}

void SimChunk::wr_count__(uint64_t off, uint64_t len) {
    uint32_t begin, end;
    blk_range__(begin, end, off, len);
    for (int idx = begin; idx < end; idx++) {
        chk_->blocks[idx].cnt.wr++;
    }
}

} // namespace flame