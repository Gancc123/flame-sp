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

typedef void (*callback_t)(Status stat, void* arg);

class FLAME_API Config {
public:
    Config();
    ~Config();

    int setAddr(std::string& addr);

}; // class FlameConfig

struct FLAME_API AioCompletion {
    callback_t cb;
    void* arg;
};

class FLAME_API Volume;
class FLAME_API VolumeAttr;
class FLAME_API VolumeMeta;

class FLAME_API Cluster {
public:
    static Cluster* connect(FlameConfig& cfg);
    static Cluster* connect(std::string& path);
    int shutdown();

    // Cluster API
    // return info of cluster by a argurment
    Status cluster_info(const std::string& arg, std::string& rst);

    // Group API
    // create an group.
    Status group_create(const std::string& name);
    // list group. return an list of group name.
    Status group_list(std::vector<std::string>>& rst);
    // remove an group.
    Status group_remove(const std::string& name);
    // rename an group.
    Status group_rename(const std::string& src, const std::string& dst);

    // Volume API
    // create an volume.
    Status volume_create(const std::string& group, const std::string& name, const VolumeAttr& attr);
    // list volumes. return an list of volume name.
    Status volume_list(const std::string& group, std::vector<std::string>>& rst);
    // remove an volume.
    Status volume_remove(const std::string& group, const std::string& name);
    // rename an volume.
    Status volume_rename(const std::string& group, const std::string& src, const std::string& dst);
    // move an volume to another group.
    Status volume_move(const std::string& group, const std::string& name, const std::string& target);
    // clone an volume
    Status volume_clone(const std::string& src_group, const std::string& src_name, const std::string& dst_group, const std::string& dst_name);
    // read info of volume.
    Status volume_meta(const std::string& group, const std::string& name, VolumeMeta& info);
    // open an volume, and return the io context of volume.
    Status volume_open(const std::string& group, const std::string& name, Volume** rst);
    // lock an volume.
    Status volume_lock_open(const std::string& group, const std::string& name, Volume** rst);
    // unlock an volume.
    Status volume_unlock(const std::string& group, const std::string& name);

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
    Status aio_reset(uint64_t offset, uint64_t len);
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