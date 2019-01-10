#ifndef FLAME_CHUNKSTORE_CS_H
#define FLAME_CHUNKSTORE_CS_H

#include "chunkstore/chunkstore.h"
#include "chunkstore/simstore/simstore.h"
#include "chunkstore/filestore/filestore.h"

#include <string>
#include <memory>

namespace flame {

std::shared_ptr<ChunkStore> create_chunkstore(FlameContext* fct, const std::string& url);

} // namespace flame

#endif // FALME_CHUNKSTORE_CS_H