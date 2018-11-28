#ifndef FLAME_UTIL_UTIME_H
#define FLAME_UTIL_UTIME_H

#include <cmath>
#include <algorithm>
#include <cstdint>
#include <string>
#include <cstdio>

#include <errno.h>
#include <time.h>
#include <sys/time.h>


inline uint32_t cap_to_u32_max(uint64_t t) {
    return std::min(t, (uint64_t)std::numeric_limits<uint32_t>::max());;
}

class utime_t {
public:
    struct {
        uint32_t tv_sec, tv_nsec;
    } tv;

public:
    bool is_zero() const { return (tv.tv_sec == 0) && (tv.tv_nsec == 0); }
    void normalize() {
        if (tv.tv_nsec > 1000000000ul) {
            tv.tv_sec = cap_to_u32_max(tv.tv_sec + tv.tv_nsec / (1000000000ul));
            tv.tv_nsec %= 1000000000ul;
        }
    }

    // cons
    utime_t() { 
        tv.tv_sec = 0; 
        tv.tv_nsec = 0; 
    }
    utime_t(time_t s, uint32_t n) { 
        tv.tv_sec = s; 
        tv.tv_nsec = n; 
        normalize(); 
    }
    utime_t(const struct timespec &ts) { set_from_timespec(&ts); }
    utime_t(const struct timespec *ts) { set_from_timespec(ts); }
    utime_t(const struct timeval &v) { set_from_timeval(&v); }
    utime_t(const struct timeval *v) { set_from_timeval(v); }

    void set_from_timespec(const struct timespec *ts) {
        tv.tv_sec = ts->tv_sec; 
        tv.tv_nsec = ts->tv_nsec; 
    }

    void to_timespec(struct timespec *ts) const {
        ts->tv_sec = tv.tv_sec;
        ts->tv_nsec = tv.tv_nsec;
    }

    void set_from_double(double d) {
        tv.tv_sec = (uint32_t)trunc(d);
        tv.tv_nsec = (uint32_t)((d - (double)tv.tv_sec) * 1000000000.0);
    }

    void set_from_timeval(const struct timeval *v) {
        tv.tv_sec = v->tv_sec;
        tv.tv_nsec = v->tv_usec * 1000;
    }

    void to_timeval(struct timeval *v) const {
        v->tv_sec = tv.tv_sec;
        v->tv_usec = tv.tv_nsec / 1000;
    }

    // accessors
    time_t sec() const { return tv.tv_sec; }
    uint32_t usec() const { return tv.tv_nsec / 1000; }
    uint32_t nsec() const { return tv.tv_nsec; }

    // ref accessors/modifiers
    uint32_t &sec_ref() { return tv.tv_sec; }
    uint32_t &nsec_ref() { return tv.tv_nsec; }

    // timestamp
    uint64_t to_uint64() {
        return (((uint64_t)tv.tv_sec) << 32) | ((uint64_t)tv.tv_nsec);
    }

    void set_from_uint64(uint64_t v) {
        tv.tv_sec = (v & ((~(uint64_t)0ULL) << 32)) >> 32;
        tv.tv_nsec = v & ((~(uint64_t)0ULL) >> 32);
    }

    uint64_t to_nsec() const {
        return (uint64_t)tv.tv_nsec + (uint64_t)tv.tv_sec * 1000000000ull;
    }

    void set_from_nsec(uint64_t _nsec) {
        tv.tv_sec = _nsec / 1000000000ULL;
        tv.tv_nsec = _nsec % 1000000000ULL;
    }

    uint64_t to_usec() const {
        return (uint64_t)tv.tv_nsec / 1000ull + (uint64_t)tv.tv_sec * 1000000ull;
    }

    void set_from_usec(uint64_t _usec) {
        tv.tv_sec = _usec / 1000000ULL;
        tv.tv_nsec = _usec % 1000000ULL * 1000ULL;
    }

    uint64_t to_msec() const {
        return (uint64_t)tv.tv_nsec / 1000000ull + (uint64_t)tv.tv_sec * 1000ull;
    }

    void set_from_msec(uint64_t _msec) {
        tv.tv_sec = _msec / 1000ULL;
        tv.tv_nsec = _msec % 1000ULL * 1000000ULL;
    }

    void sleep() const {
        struct timespec ts;
        to_timespec(&ts);
        nanosleep(&ts, NULL);
    }

    int sprintf(char *out, int outlen) const {
        struct tm bdt;
        time_t tt = sec();
        localtime_r(&tt, &bdt);

        return ::snprintf(out, outlen,
		    "%04u-%02u-%02u %02u:%02u:%02u.%06u",
		    bdt.tm_year + 1900, bdt.tm_mon + 1, bdt.tm_mday,
		    bdt.tm_hour, bdt.tm_min, bdt.tm_sec, usec());
    }

    std::string to_str() const {
        char buff[32];
        sprintf(buff, 32);
        return std::string(buff);
    }

    static int snprintf(char *out, int outlen, time_t tt) {
        struct tm bdt;
        localtime_r(&tt, &bdt);

        return ::snprintf(out, outlen,
            "%04u-%02u-%02u %02u:%02u:%02u",
            bdt.tm_year + 1900, bdt.tm_mon + 1, bdt.tm_mday,
            bdt.tm_hour, bdt.tm_min, bdt.tm_sec);
    }

    static utime_t now() {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return utime_t(ts);
    }

    static utime_t get_by_nsec(uint64_t nsec) {
        utime_t tt;
        tt.set_from_nsec(nsec);
        return tt;
    }

    static utime_t get_by_usec(uint64_t usec) {
        utime_t tt;
        tt.set_from_usec(usec);
        return tt;
    }

    static utime_t get_by_msec(uint64_t msec) {
        utime_t tt;
        tt.set_from_msec(msec);
        return tt;
    }
};

// arithmetic operators
inline utime_t operator + (const utime_t &l, const utime_t &r) {
    uint64_t sec = (uint64_t)l.sec() + r.sec();
    return utime_t(cap_to_u32_max(sec), l.nsec() + r.nsec());
}

inline utime_t &operator += (utime_t &l, const utime_t &r) {
    l.sec_ref() = cap_to_u32_max((uint64_t)l.sec() + r.sec());
    l.nsec_ref() += r.nsec();
    l.normalize();
    return l;
}

inline utime_t &operator += (utime_t &l, double f) {
    double fs = trunc(f);
    double ns = (f - fs) * 1000000000.0;
    l.sec_ref() = cap_to_u32_max(l.sec() + (uint64_t)fs);
    l.nsec_ref() += (long)ns;
    l.normalize();
    return l;
}

inline utime_t operator - (const utime_t &l, const utime_t &r) {
    return utime_t( l.sec() - r.sec() - (l.nsec()<r.nsec() ? 1:0),
                  l.nsec() - r.nsec() + (l.nsec()<r.nsec() ? 1000000000:0) );
}

inline utime_t &operator -= (utime_t &l, const utime_t &r) {
    l.sec_ref() -= r.sec();
    if (l.nsec() >= r.nsec())
        l.nsec_ref() -= r.nsec();
    else {
        l.nsec_ref() += 1000000000L - r.nsec();
        l.sec_ref()--;
    }
    return l;
}

inline utime_t &operator -= (utime_t &l, double f) {
    double fs = trunc(f);
    double ns = (f - fs) * 1000000000.0;
    l.sec_ref() -= (long)fs;
    long nsl = (long)ns;
    if (nsl) {
        l.sec_ref()--;
        l.nsec_ref() = 1000000000L + l.nsec_ref() - nsl;
    }
    l.normalize();
    return l;
}

// comparators
inline bool operator > (const utime_t &a, const utime_t &b) {
  return (a.sec() > b.sec()) || (a.sec() == b.sec() && a.nsec() > b.nsec());
}

inline bool operator <= (const utime_t &a, const utime_t &b) {
  return !(operator > (a, b));
}

inline bool operator < (const utime_t &a, const utime_t &b) {
  return (a.sec() < b.sec()) || (a.sec() == b.sec() && a.nsec() < b.nsec());
}

inline bool operator >= (const utime_t &a, const utime_t &b) {
  return !(operator < (a, b));
}

inline bool operator == (const utime_t &a, const utime_t &b) {
  return a.sec() == b.sec() && a.nsec() == b.nsec();
}

inline bool operator != (const utime_t &a, const utime_t &b) {
  return a.sec() != b.sec() || a.nsec() != b.nsec();
}

#endif // FLAME_UTIL_UTIME_H