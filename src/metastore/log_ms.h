#ifndef FLAME_METASTORE_LOG_MS_H
#define FLAME_METASTORE_LOG_MS_H

#include "common/log.h"

#ifdef dead
#undef dead
#define dead(fmt, arg...) plog(0, "metastore", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef critical
#define critical(fmt, arg...) plog(1, "metastore", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef wrong
#define wrong(fmt, arg...) plog(2, "metastore", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef error
#define error(fmt, arg...) plog(3, "metastore", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef warn
#define warn(fmt, arg...) plog(4, "metastore", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef info
#define info(fmt, arg...) plog(5, "metastore", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef debug
#define debug(fmt, arg...) plog(6, "metastore", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef trace
#define trace(fmt, arg...) plog(7, "metastore", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef print
#define print(fmt, arg...) plog(8, "metastore", __FILE__, __LINE__, __func__, (fmt), ##arg)

#endif

#endif // FLAME_METASTORE_LOG_MS_H
