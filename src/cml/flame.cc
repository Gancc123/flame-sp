#include "common/cmdline.h"

#include <cstdint>
#include <string>

using namespace std;
using namespace flame;
using namespace flame::cli;

namespace flame {

class VGShowCli : public Cmdline {
public:
    VGShowCli(Cmdline* parent) 
    : Cmdline(parent, "show", "List all of the volume group") {}

    Argument<int>   offset  {this, 's', "offset", "offset", 0};
    Argument<int>   number  {this, 'n', "number", "the number of vg that need to be showed", 0};

    HelpAction  help    {this};

    int def_run() {
        // pass
        return 0;
    }
}; // class VGShowCli

class VGCreateCli : public Cmdline {
public:
    VGCreateCli(Cmdline* parent) 
    : Cmdline(parent, "create", "create an volume group") {}

    Serial<string>  name    {this, 1, "name", "the name of volume group"};

    HelpAction  help    {this};

    int def_run() {
        // pass
        return 0;
    }
}; // class VGCreateCli

class VGRemoveCli : public Cmdline {
public:
    VGRemoveCli(Cmdline* parent) 
    : Cmdline(parent, "remove", "remove an volume group") {}

    Serial<string>  name    {this, 1, "name", "the name of volume group"};

    HelpAction  help    {this};

    int def_run() {
        // pass
        return 0;
    }
}; // class VGRemoveCli

class VolGroupCli : public Cmdline {
public:
    VolGroupCli(Cmdline* parent) 
    : Cmdline(parent, "vg", "Volume Group") {}

    VGShowCli   show    {this};
    VGCreateCli create  {this};
    VGRemoveCli remove  {this};

    HelpAction  help    {this};
}; // FlameVolumeGroupCli

class FlameCli : public Cmdline {
public:
    FlameCli() : Cmdline("flame", "Flame Command Line Interface") {}

    VolGroupCli vg  {this};

    HelpAction  help    {this};

}; // class FlameCli

} // namespace flame

int main(int argc, char** argv) {
    FlameCli flame_cli;

    return flame_cli.run(argc, argv);
}