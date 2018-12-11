#ifndef FLAME_COMMON_THREAD_H
#define FLAME_COMMON_THREAD_H

#include <pthread.h>

namespace flame {

class Thread {
private:
    pthread_t thread_id;
    pid_t pid;
    int ioprio_class;
    int ioprio_priority;
    int cpuid;
    const char *thread_name;

    void entry_wrapper();

public:
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    Thread();
    virtual ~Thread();

    const char* get_name() { return thread_name; }

protected:
    virtual void entry() = 0;

private:
    static void* _entry_func(void *arg);

public:
    const pthread_t& get_thread_id() const;
    pid_t get_pid() const { return pid; }
    bool is_started() const;
    bool am_self() const;
    int kill(int signal);
    int try_create(size_t stacksize);
    void create(const char* name, size_t stacksize = 0);
    int join(void** prval = 0);
    int detach();
    int set_ioprio(int cls, int prio);
    // set_affinity should be called before create(), or within target thread
    int set_affinity(int cpuid);
};

}; // namespace flame

#endif // FLAME_COMMON_THREAD_H