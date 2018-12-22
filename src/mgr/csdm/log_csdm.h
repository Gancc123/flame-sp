#ifndef FLAME_MGR_CSDM_LOG_H
#define FLAME_MGR_CSDM_LOG_H

#include "common/log.h"

#ifdef ldead
#undef ldead
#define ldead(fmt, arg...) plog(0, "csdm", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lcritical
#define lcritical(fmt, arg...) plog(1, "csdm", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lwrong
#define lwrong(fmt, arg...) plog(2, "csdm", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lerror
#define lerror(fmt, arg...) plog(3, "csdm", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lwarn
#define lwarn(fmt, arg...) plog(4, "csdm", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef linfo
#define linfo(fmt, arg...) plog(5, "csdm", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef ldebug
#define ldebug(fmt, arg...) plog(6, "csdm", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef ltrace
#define ltrace(fmt, arg...) plog(7, "csdm", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lprint
#define lprint(fmt, arg...) plog(8, "csdm", __FILE__, __LINE__, __func__, (fmt), ##arg)

#endif

#endif // FLAME_MGR_CSDM_LOG_H
