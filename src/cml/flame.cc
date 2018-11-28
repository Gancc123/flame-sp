#include "common/cmdline.h"

#include <grpcpp/grpcpp.h>

#include "include/flame.h"
#include "proto/flame.grpc.pb.h"
#include "service/flame_client.h"
#include "util/utime.h"

#include <cstdio>
#include <cstdint>
#include <string>
#include <memory>
#include <list>

using namespace std;
using namespace flame;
using namespace flame::cli;

static int success__() {
    printf("Success!\n");
    return 0;
}

static void check_faild__(int r, const std::string& msg) {
    if (r != 0) {
        printf("faild(%d): %s\n", r, msg.c_str());
        exit(-1);
    }
}

namespace flame {

static unique_ptr<FlameClientContext> make_flame_client_context() {
    FlameContext* fct = FlameContext::get_context();

    FlameClient* client = new FlameClientImpl(fct, grpc::CreateChannel(
        "localhost:6666", grpc::InsecureChannelCredentials()
    ));
    
    if (!client) {
        printf("connect flame cluster faild.\n");
        exit(-1);
    }

    FlameClientContext* fcct = new FlameClientContext(fct, client);

    if (!fcct) {
        printf("create flame client context faild.\n");
        exit(-1);
    }

    return unique_ptr<FlameClientContext>(fcct);
}

class VGShowCli : public Cmdline {
public:
    VGShowCli(Cmdline* parent) 
    : Cmdline(parent, "show", "List all of the volume group") {}

    Argument<int> offset {this, 's', "offset", "offset", 0};
    Argument<int> number {this, 'n', "number", "the number of vg that need to be showed", 0};

    HelpAction help {this};

    int def_run() {
        auto cct = make_flame_client_context();
        list<volume_group_meta_t> res;
        int r = cct->client()->get_vol_group_list(offset.get(), number.get(), res);
        check_faild__(r, "show volume group.\n");
        
        printf("Size: %d\n", res.size());
        printf("vg_id\tname\tvols\tsize\tctime\n");
        for (auto it = res.begin(); it != res.end(); it++) {
            printf("%llu\t%s\t%u\t%llu\t%s\n",
                it->vg_id,
                it->name.c_str(),
                it->volumes,
                it->size,
                utime_t::get_by_msec(it->ctime).to_str().c_str()
            );
        }
        return 0;
    }
}; // class VGShowCli

class VGCreateCli : public Cmdline {
public:
    VGCreateCli(Cmdline* parent) 
    : Cmdline(parent, "create", "create an volume group") {}

    Serial<string> name {this, 1, "name", "the name of volume group"};

    HelpAction help {this};

    int def_run() {
        auto cct = make_flame_client_context();
        
        int r = cct->client()->create_vol_group(name.get());
        check_faild__(r, "create volume group");
        
        return success__();
    }
}; // class VGCreateCli

class VGRemoveCli : public Cmdline {
public:
    VGRemoveCli(Cmdline* parent) 
    : Cmdline(parent, "remove", "remove an volume group") {}

    Serial<string> name {this, 1, "name", "the name of volume group"};

    HelpAction help {this};

    int def_run() {
        // pass
        return 0;
    }
}; // class VGRemoveCli

class VGRenameCli : public Cmdline {
public:
    VGRenameCli(Cmdline* parent)
    : Cmdline(parent, "rename", "rename an volume group") {}

    Serial<string> old_name {this, 1, "old_name", "the old name of vg"};
    Serial<string> new_name {this, 2, "new_name", "the new name of vg"};

    HelpAction help {this};
}; // class VGRenameCli

class VolGroupCli : public Cmdline {
public:
    VolGroupCli(Cmdline* parent) 
    : Cmdline(parent, "vg", "Volume Group") {}

    VGShowCli   show    {this};
    VGCreateCli create  {this};
    VGRemoveCli remove  {this};
    VGRenameCli rename  {this};

    HelpAction help {this};
}; // class VolGroupCli

class VolShowCli : public Cmdline {
public:
    VolShowCli(Cmdline* parent)
    : Cmdline(parent, "show", "show volumes") {}

    Serial<string> vg_name {this, 1, "vg_name", "the name of volume group"};

    Argument<int> offset {this, 's', "offset", "offset", 0};
    Argument<int> number {this, 'n', "number", "the number of vg that need to be showed", 0};

    HelpAction help {this};
}; // class VolShowCli

class VolCreateCli : public Cmdline {
public:
    VolCreateCli(Cmdline* parent)
    : Cmdline(parent, "create", "create an volume") {}

    Serial<string>  vg_name  {this, 1, "vg_name", "the name of volume group"};
    Serial<string>  vol_name {this, 2, "vol_name", "the name of volume"};
    Serial<int>     size     {this, 3, "size", "the size of volume, unit(GB)"};

    Argument<int>   chk_sz   {this, "chunk_size", "the size of chunk in this volume, unit(GB)", 4};
    Argument<string> spolicy {this, "store_policy", "the store policy of this volume", ""};

    Switch  prealloc    {this, "preallocate", "pre allocating the physical space for volume"};

    HelpAction help {this};
}; // class VolCreateCli

class VolRemoveCli : public Cmdline {
public:
    VolRemoveCli(Cmdline* parent)
    : Cmdline(parent, "remove", "remove an volume") {}

    Serial<string>  vg_name  {this, 1, "vg_name", "the name of volume group"};
    Serial<string>  vol_name {this, 2, "vol_name", "the name of volume"};

    HelpAction help {this};
}; // class VolRemoveCli

class VolRenameCli : public Cmdline {
public:
    VolRenameCli(Cmdline* parent)
    : Cmdline(parent, "rename", "rename an volume") {}

    Serial<string>  vg_name  {this, 1, "vg_name", "the name of volume group"};
    Serial<string>  vol_name {this, 2, "vol_name", "the name of volume"};
    Serial<string>  new_name {this, 3, "new_name", "the new name of volume"};

    HelpAction help {this};
}; // class VolRemoveCli

class VolInfoCli : public Cmdline {
public:
    VolInfoCli(Cmdline* parent)
    : Cmdline(parent, "info", "show the information of volume") {}

    Serial<string>  vg_name  {this, 1, "vg_name", "the name of volume group"};
    Serial<string>  vol_name {this, 2, "vol_name", "the name of volume"};

    HelpAction help {this};
}; // class VolInfoCli

class VolResizeCli : public Cmdline {
public:
    VolResizeCli(Cmdline* parent)
    : Cmdline(parent, "resize", "change the size of volume") {}

    Serial<string>  vg_name  {this, 1, "vg_name", "the name of volume group"};
    Serial<string>  vol_name {this, 2, "vol_name", "the name of volume"};
    Serial<int>     size     {this, 3, "size", "the new size of volume, unit(GB)"};

    HelpAction help {this};
}; // class VolResizeCli

class VolumeCli : public Cmdline {
public:
    VolumeCli(Cmdline* parent)
    : Cmdline(parent, "volume", "Volume") {}

    VolShowCli      show    {this};
    VolCreateCli    create  {this};
    VolRemoveCli    remove  {this};
    VolRenameCli    rename  {this};
    VolInfoCli      info    {this};
    VolResizeCli    resize  {this};

    HelpAction help {this};
}; // class VolumeCli

class CltInfoCli : public Cmdline {
public:
    CltInfoCli(Cmdline* parent)
    : Cmdline(parent, "info", "show the information of cluster") {}

    HelpAction help {this};

    int def_run() {
        return 0;
    }
};

class ClusterCli : public Cmdline {
public:
    ClusterCli(Cmdline* parent)
    : Cmdline(parent, "cluster", "Flame Cluster") {}

    CltInfoCli info {this};

    HelpAction help {this};
}; // class ClusterCli

class FlameCli : public Cmdline {
public:
    FlameCli() : Cmdline("flame", "Flame Command Line Interface") {}

    ClusterCli  cluster {this};
    VolGroupCli vg      {this};
    VolumeCli   volume  {this};

    HelpAction  help    {this};

}; // class FlameCli

} // namespace flame

int main(int argc, char** argv) {
    FlameCli flame_cli;
    return flame_cli.run(argc, argv);
}