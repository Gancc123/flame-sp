// Flame API
// @version: 1.0

#ifndef LIBFLAME_H
#define LIBFLAME_H

#include <string>
#include <vector>

#include "status.h"
#include "buffer.h"

#if __GNUC__ >= 4
    #define FLAME_API __attribute__((visibility ("default")))
#else
    #define FLAME_API
#endif

namespace libflame {

typedef void (*callback_fn_t)(const Status& stat, void* arg1, void* arg2);

class FLAME_API Config {
public:
    Config();
    ~Config();

    int setAddr(std::string& addr);

}; // class Config

struct FLAME_API AioCompletion {
    callback_fn_t fn { nullptr };
    void* arg1 { nullptr };
    void* arg2 { nullptr };

    inline void call(const Status& stat) {
        if (fn != nullptr) fn(stat, arg1, arg2);
    }
};

class FLAME_API Volume;
class FLAME_API VolumeAttr;
class FLAME_API VolumeMeta;

class FLAME_API Cluster {
public:
    static Cluster* connect(Config& cfg);
    static Cluster* connect(std::string& path);
    int shutdown();

    // Cluster API
    // return info of cluster by a argurment
    Status cluster_info(const std::string& arg, std::string& rst);

    // Group API
    // create an group.
    Status vg_create(const std::string& name);
    // list group. return an list of group name.
    Status vg_list(std::vector<std::string>>& rst);
    // remove an group.
    Status vg_remove(const std::string& name);
    // rename an group. not support now
    // Status vg_rename(const std::string& src, const std::string& dst);

    // Volume API
    // create an volume.
    Status vol_create(const std::string& group, const std::string& name, const VolumeAttr& attr);
    // list volumes. return an list of volume name.
    Status vol_list(const std::string& group, std::vector<std::string>>& rst);
    // remove an volume.
    Status vol_remove(const std::string& group, const std::string& name);
    // rename an volume. not support now
    // Status vol_rename(const std::string& group, const std::string& src, const std::string& dst);
    // move an volume to another group.
    Status vol_move(const std::string& group, const std::string& name, const std::string& target);
    // clone an volume
    Status vol_clone(const std::string& src_group, const std::string& src_name, const std::string& dst_group, const std::string& dst_name);
    // read info of volume.
    Status vol_meta(const std::string& group, const std::string& name, VolumeMeta& info);
    // open an volume, and return the io context of volume.
    Status vol_open(const std::string& group, const std::string& name, Volume** rst);
    // lock an volume. not support now
    // Status vol_open_locked(const std::string& group, const std::string& name, Volume** rst);
    // unlock an volume. not support now
    // Status vol_unlock(const std::string& group, const std::string& name);

private:
    Cluster();
    ~Cluster();

    Cluster(const Cluster& rhs) = delete;
    Cluster& operator=(const Cluster& rhs) = delete;

}; // class Cluster

class FLAME_API Volume {
public:
    std::string get_id();
    std::string get_name();
    std::string get_group();
    uint64_t size();
    VolumeMeta get_meta();

    // manage
    Status resize(uint64_t size);
    Status lock();
    Status unlock();

    // sync io call
    Status read(const BufferList& buffs, uint64_t offset, uint64_t len);
    Status write(const BufferList& buffs, uint64_t offset, uint64_t len);
    Status reset(uint64_t offset, uint64_t len);
    Status flush();

    // async io call
    Status aio_read(const BufferList& buffs, uint64_t offset, uint64_t len, const AioCompletion& cpl);
    Status aio_write(const BufferList& buffs, uint64_t offset, uint64_t len, const AioCompletion& cpl);
    Status aio_reset(uint64_t offset, uint64_t len, const AioCompletion& cpl);
    Status aio_flush(const AioCompletion& cpl);

private:
    Volume();
    ~Volume;

    friend class Cluster;
}; // class Volume

class FLAME_API VolumeAttr {
public:
    VolumeAttr(uint64_t size) : size_(size), prealloc_(false) {}
    ~VolumeAttr() {}

    void set_size(uint64_t s) { size_ = s; }
    void set_prealloc(bool prealloc) { prealloc_ = prealloc; }

private:
    uint64_t size_;
    bool prealloc_;
}; // struct VolumeAttr

class FLAME_API VolumeMeta {
public:
    ~VolumeMeta() {}
    
    std::string& get_name() { return name_; }
    std::string& get_group() { return group_; }
    uint64_t get_size() { return size_; }
    uint64_t get_ctime() { return ctime_; }
    bool is_prealloc() { return prealloc_; }

private:
    VolumeMeta() {}

    friend class Volume;

    std::string name_;
    std::string group_;
    uint64_t size_;
    uint64_t ctime_; // create time
    bool prealloc_;
}; // class VolumeMeta

} // namespace libflame

#endif // LIBFLAME_H