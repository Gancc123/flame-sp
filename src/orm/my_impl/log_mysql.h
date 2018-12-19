#ifndef FLAME_ORM_LOG_MYSQL_H
#define FLAME_ORM_LOG_MYSQL_H

#include "common/log.h"

#ifdef ldead
#undef ldead
#define ldead(fmt, arg...) plog(0, "mysql", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lcritical
#define lcritical(fmt, arg...) plog(1, "mysql", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lwrong
#define lwrong(fmt, arg...) plog(2, "mysql", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lerror
#define lerror(fmt, arg...) plog(3, "mysql", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lwarn
#define lwarn(fmt, arg...) plog(4, "mysql", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef linfo
#define linfo(fmt, arg...) plog(5, "mysql", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef ldebug
#define ldebug(fmt, arg...) plog(6, "mysql", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef ltrace
#define ltrace(fmt, arg...) plog(7, "mysql", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lprint
#define lprint(fmt, arg...) plog(8, "mysql", __FILE__, __LINE__, __func__, (fmt), ##arg)

#endif

#endif // FLAME_ORM_LOG_MYSQL_H
