#include "ms.h"
#include "orm/orm.h"
#include "log_ms.h"

namespace flame {

std::shared_ptr<MetaStore> create_metastore(FlameContext* fct, const std::string& url) {
    orm::DBUrlParser parser(url);
    if (!parser.matched()) {
        fct->log()->lerror("metastore url is invalid");
        return nullptr;
    }
    
    if (parser.driver() == "mysql") {
        return std::shared_ptr<MetaStore> (SqlMetaStore::create_sqlms(fct, url));
    }

    return nullptr;
}

} // namespace flame