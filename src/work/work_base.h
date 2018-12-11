#ifndef FLAME_WORK_BASE_H
#define FLAME_WORK_BASE_H

#include "common/thread/thread.h"
#include <string>

namespace flame {

/**
 * @brief 工作实体
 * 
 */
class WorkEntry {
public:
    virtual ~WorkEntry() {}

    /**
     * @brief 工作运行函数
     * 
     */
    virtual void entry() = 0;

protected:
    WorkEntry() {}
}; // class WorkEntry

class WorkerBase : public Thread {
public:
    virtual ~WorkerBase() {}

    /**
     * @brief 程序主体
     * 
     */
    virtual void entry() = 0;

    void run() { create("worker"); }

protected:
    WorkerBase(const std::string& name) : name_(name) {}

    std::string name_;
}; // class WorkerBase

} // namespace flame

#endif // FLAME_WORK_BASE_H