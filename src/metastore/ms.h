#ifndef FLAME_METASTORE_MS_H
#define FLAME_METASTORE_MS_H

#include "common/context.h"
#include "metastore/metastore.h"
#include "metastore/sqlms/sqlms.h"

#include <string>
#include <memory>

namespace flame {

std::shared_ptr<MetaStore> create_metastore(FlameContext* fct, const std::string& url);

} // namespace flame

#endif // FLAME_METASTORE_MS_H