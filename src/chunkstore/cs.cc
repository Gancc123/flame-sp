#include "chunkstore/cs.h"
#include "chunkstore/log_cs.h"

using namespace std;

namespace flame {

shared_ptr<ChunkStore> create_chunkstore(FlameContext* fct, const string& url) {
    size_t pos = url.find("://");
    if (pos == string::npos) {
        fct->log()->error("invalid url: %s", url.c_str());
        return nullptr;
    }

    string driver = url.substr(0, pos);
    if (driver == "simstore") {
        return shared_ptr<ChunkStore>(SimStore::create_simstore(fct, url));
    }

    fct->log()->error("invalid driver: %s", driver.c_str());
    return nullptr;
}

} // namespace flame