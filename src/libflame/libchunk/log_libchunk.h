#ifndef FLAME_LIBFLAME_LIBCHUNK_LOG_H
#define FLAME_LIBFLAME_LIBCHUNK_LOG_H

#include "common/log.h"

#ifdef ldead
#undef ldead
#define ldead(fmt, arg...) plog(0, "libchunk", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lcritical
#define lcritical(fmt, arg...) plog(1, "libchunk", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lwrong
#define lwrong(fmt, arg...) plog(2, "libchunk", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lerror
#define lerror(fmt, arg...) plog(3, "libchunk", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lwarn
#define lwarn(fmt, arg...) plog(4, "libchunk", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef linfo
#define linfo(fmt, arg...) plog(5, "libchunk", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef ldebug
#define ldebug(fmt, arg...) plog(6, "libchunk", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef ltrace
#define ltrace(fmt, arg...) plog(7, "libchunk", __FILE__, __LINE__, __func__, (fmt), ##arg)

#undef lprint
#define lprint(fmt, arg...) plog(8, "libchunk", __FILE__, __LINE__, __func__, (fmt), ##arg)

#endif

#endif // FLAME_LIBFLAME_LIBCHUNK_LOG_H
