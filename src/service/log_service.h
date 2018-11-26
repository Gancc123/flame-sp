#ifndef FLAME_SERVICE_LOG_H
#define FLAME_SERVICE_LOG_H

#include "common/log.h"

#ifdef dead
#undef dead
#define dead(fmt, arg...) plog(0, "service", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef critical
#define critical(fmt, arg...) plog(1, "service", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef wrong
#define wrong(fmt, arg...) plog(2, "service", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef error
#define error(fmt, arg...) plog(3, "service", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef warn
#define warn(fmt, arg...) plog(4, "service", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef info
#define info(fmt, arg...) plog(5, "service", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef debug
#define debug(fmt, arg...) plog(6, "service", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef trace
#define trace(fmt, arg...) plog(7, "service", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef print
#define print(fmt, arg...) plog(8, "service", __FILE__, __LINE__, __func__, (fmt), ##arg)

#endif

#endif // FLAME_METASTORE_LOG_MS_H
