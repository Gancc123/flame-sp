#include "chunkstore/simstore/simstore.h"

#include "util/utime.h"

#include <memory>
#include <sstream>
#include <fstream>
#include <cctype>
#include <exception>
#include <vector>
#include <regex>

using namespace std;

namespace flame {

SimStore* SimStore::create_simstore(FlameContext* fct, const std::string& url) {
    string pstr = "^(\\w+)://(\\d+)([tmgTMG])(:.+)?$";
    regex pattern(pstr);
    smathc result;
    if (!regex_match(url, result, pattern)) {
        fct->log()->error("");
    }
}

SimStore* SimStore::create_simstore(FlameContext* fct, uint64_t size, const std::string& bk_file = "") {
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
    if (!bk_file_.empty())
        oss << ":" << bk_file_;
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
    if (!bk_file_.empty()) {
        fstream fs;
        fs.open(bk_file_, fstream::in);
        if (!fp.is_open())
            return ChunkStore::DevStatus::NONE;
    }
    return ChunkStore::DevStatus::CLT_IN;
}

int SimStore::dev_format() {
    formated_ = true;
    return 0;
}

int SimStore::dev_mount() {
    if (!bk_file_.empty()) {
        mounted_ = true;
        return DevRetCode::SUCCESS;
    }
}

int SimStore::dev_umount() {
    if (!mounted_)
        return DevRetCode::SUCCESS;
}

bool SimStore::is_mounted() {
    return mounted_;
}

int SimStore::chunk_create(uint64_t chk_id, const chunk_create_opts_t& opts) {
    auto it = chk_map_.find(chk_id);
    if (it != chk_map_.end()) 
        return OpRetCode::OBJ_EXISTED;
    
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
    
    chk_map_[chk_id] = chk_info;
    info_.chk_num++;
    return OpRetCode::SUCCESS;
}

int SimStore::chunk_remove(uint64_t chk_id) {
    auto it = chk_map_.find(chk_id);
    if (it == chk_map_.end())
        return OpRetCode::OBJ_NOTFOUND;
    
    chk_map_.erase(chk_id);
    info_.chk_num--;
    return OpRetCode::SUCCESS;
}

bool SimStore::chunk_exist(uint64_t chk_id) {
    return chk_map_.find(chk_id) != chk_map_.end();
}

shared_ptr<Chunk> SimStore::chunk_open(uint64_t chk_id) {
    auto it = chk_map_.find(chk_id);
    if (it == chk_map_.end())
        return nullptr;
    return shared_ptr<Chunk>(new SimChunk(fct_, this, &it->second));
}

int SimStore::info_init__() {
    info_.id = 0;
    info_.cluster_name = fct_->cluster_name();
    info_.name = fct_->node_name();
    info_.size = size_;
    info_.used = 0;
    info_.ftime = utime_t::now().to_msec();
    info_.chk_num = 0;
    return OpRetCode::SUCCESS;
}

int SimStore::backup_load__() {
    fstream fin;
    fin.open(bk_file_, fstream::in);
    if (!fin.is_open())
        return OpRetCode::OBJ_NOTFOUND;

    char chr;
    while (fin.get(chr)) {

    }
}

int SimStore::backup_store__() {
    fstream fout;
    fout.open(bk_file_, fstream::out);
    if (!fout.is_open())
        return OpRetCode::OBJ_NOTFOUND;
    
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
        blk_num = info.size / SIMSTORE_BLOCK_SIZE + (info.size % SIMSTORE_BLOCK_SIZE ? 1 : 0);
        for (uint32_t blk_id = 0; blk_id < blk_num; blk_id++) {
            auto bit = chk.blocks.find(blk_id);
            if (bit == chk.blocks.end()) {
                // not found
                fout << 0 << SIMSTORE_SEP_L2;
                fout << -1 << SIMSTORE_SEP_L2;
                fout << -1 << SIMSTORE_SEP_L2;
            } else {
                // founded
                simstore_block_t& blk = bit->second; 
                fout << blk.ctime << SIMSTORE_SEP_L2;
                fout << blk.cnt.rd << SIMSTORE_SEP_L2;
                fout << blk.cnt.wr << SIMSTORE_SEP_L2;
            }
        }
        fout << SIMSTORE_SEP_L1;

        fout << endl;
    }

    fout << "@end" << endl;

    fout.close();
    return OpRetCode::SUCCESS;
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
            if (r != 0)
                return r;
        }
    }
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
    } else 
        return 1;
}

int SimStore::ld_sub_kv__(fstream& fin) {
    char chr;
    string key;
    string value;
    bool in_value = false;
    while (fin.get(chr)) {
        if (in_value) {
            if (chr == '\n') {
                if (key.empty() || value.empty())
                    return 2;
                
                int r = st_info__(key, value);
                if (r != 0)
                    return 2;

                key.clear();
                value.clear();
                in_value = false;
            } else
                value.push_back(chr);
        } else {
            if (chr == '\n')
                return 2;
            else if (chr == SIMSTORE_SEP_KW)
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
    map<int, string> header;
    
    int r = ld_sub_table__(fin, header);
    if (r != 0)
        return 3;
    
    while (true) {
        r = ld_sub_table_row__(fin, header);
        if (r == 1)
            break;
        if (r > 1)
            return 3;
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
            if (col.empty())
                return 1;
            header.push_back(col);
            idx++;
            col.clear();
        } else 
            col.push_back(chr);
    }
    return 2;
}

int SimStore::ld_sub_table_row__(fstream& fin, vector<string>& header) {
    char chr;
    string value;
    int cols = 0;
    simstore_chunk_t chk;
    while (fin.get(chr)) {
        if (chr == '\n') {
            if (value.empty() && cols == 0)
                return 1;   // is tail
            if (cols != header.size())
                return 2;   // 2 is faild
            chk_map_[chk.info.chk_id] = chk;
        } else if (chr == SIMSTORE_SEP_L1) {
            if (value.empty() || cols >= header.size())
                return 2;

            int r = st_chunk__(chk, header[cols], value);
            if (r != 0)
                return 2;

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
            return 1;
        }
    } else if (key == "used") {
        try {
            uint64_t used = stoull(value);
            info_.used = used;
        } catch (exception& e) {
            return 1;
        }
    } else if (key == "ftime") {
        try {
            uint64_t ftime = stoull(value);
            info_.ftime = ftime;
        } catch (exception& e) {
            return 1;
        }
    } else if (key == "chk_num") {
        try {
            uint32_t chk_num = stou(value);
            info_.chk_num = chk_num;
        } catch (exception& e) {
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
    }
    
    uint32_t sv;
    try {
        sv = stou(value);
    } catch (exception& e) {
        return 1;
    }
    
    if (key == "index") {
        chk.info.index = sv;
    } else if (key == "stat") {
        chk.info.stat = sv;
    } else if (key == "spolicy") {
        chk.info.spolicy = sv;
    } else if (key == "flags") {
        chk.info.flags = sv;
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
        if (key.empty() !! val.empty())
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
    info = info_;
    return ChunkStore::OpRetCode::SUCCESS;
}

uint64_t SimChunk::size() const {
    return chk_->info_.size;
}

uint64_t SimChunk::used() const {
    return chk_->info_.used;
}

uint64_t SimChunk::stat() const {
    return chk_->info_.stat;
}

uint64_t SimChunk::vol_id() const {
    return chk_->info_.vol_id;
}

uint32_t SimChunk::vol_index() const {
    return chk_->info_.index;
}

uint32_t SimChunk::spolicy() const {
    return chk_->info_.spolicy;
}

bool SimChunk::is_preallocated() const {
    return chk_->info_.flags & ChunkFlags::PREALLOC;
}

int SimChunk::read_sync(void* buff, uint64_t off, uint64_t len) {
    return ChunkStore::OpRetCode::SUCCESS;
}

int SimChunk::write_sync(void* buff, uint64_t off, uint64_t len) {
    return ChunkStore::OpRetCode::SUCCESS;
}

int SimChunk::get_xattr(const std::string& name, std::string& value) {
    auto it = chk_->xattr.find(name);
    if (it == chk_->xattr.end())
        return ChunkStore::OpRetCode::OBJ_NOTFOUND;
    value = chk_->xattr[name];
    return ChunkStore::OpRetCode::SUCCESS;
}

int SimChunk::set_xattr(const std::string& name, const std::string& value) {
    chk_->xattr[name] = value;
    return ChunkStore::OpRetCode::SUCCESS;
}

int SimChunk::read_async(void* buff, uint64_t off, uint64_t len, chunk_opt_cb_t cb, void* cb_arg) {
    if (cb != nullptr)
        cb(cb_arg);
    return ChunkStore::OpRetCode::SUCCESS;
}

int SimChunk::write_async(void* buff, uint64_t off, uint64_t len, chunk_opt_cb_t cb, void* cb_arg) {
    if (cb != nullptr)
        cb(cb_arg);
    return ChunkStore::OpRetCode::SUCCESS;
}

} // namespace flame