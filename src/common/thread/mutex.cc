#include "mutex.h"

namespace flame {

static inline void __mutex_settype(pthread_mutex_t *mutex, int type) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, type);
    pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

Mutex::Mutex(MutexType t)
: m_type(t), m_nlock(0), m_owner(0) {
#ifndef __USE_GNU
    if(m_type == MUTEX_TYPE_ADAPTIVE_NP){
        m_type = MUTEX_TYPE_DEFAULT;
    }
#endif
    switch(m_type) {
    case MUTEX_TYPE_NORMAL:
        __mutex_settype(&m_mutex, PTHREAD_MUTEX_NORMAL);
        break;
    case MUTEX_TYPE_ERRORCHECK:
        __mutex_settype(&m_mutex, PTHREAD_MUTEX_ERRORCHECK);
        break;
    case MUTEX_TYPE_RECURSIVE:
        __mutex_settype(&m_mutex, PTHREAD_MUTEX_RECURSIVE);
        break;
#ifdef __USE_GNU
    case MUTEX_TYPE_ADAPTIVE_NP:
        __mutex_settype(&m_mutex, PTHREAD_MUTEX_ADAPTIVE_NP);
        break;
#endif
    case MUTEX_TYPE_DEFAULT:
    default:
        pthread_mutex_init(&m_mutex, NULL);
        break;
    }
}

Mutex::~Mutex() {
    assert(m_nlock == 0);
    pthread_mutex_destroy(&m_mutex);
}

} // namespace flame