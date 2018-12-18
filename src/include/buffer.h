#ifndef FLAME_INCLUDE_BUFFER_H
#define FLAME_INCLUDE_BUFFER_H

#include <cassert>
#include <cstdint>
#include <list>
#include <memory>

namespace flame {

class BufferPtr {
public:
    virtual ~BufferPtr() {}

    virtual uint8_t* addr() const = 0;

    virtual size_t size() const = 0;

    virtual bool null() const { return addr() == nullptr || size() == 0; }

    virtual uint8_t* end() const { return addr() + size(); }

    /**
     * @brief Buffer类型
     * 用于区分不同Buffer分配器所分配的类型，0表示无类型
     * @return int 
     */
    virtual int type() const { return 0; }

    /**
     * @brief 按指定大小调整Buffer空间
     * 默认拒绝重新分配
     * @param sz 新大小
     * @return true 调整成功
     * @return false 调整失败
     */
    virtual bool resize(size_t sz) { return size() == sz ? true : false; }
 
protected:
    BufferPtr() {}
}; // class BufferPtr

inline bool operator == (const BufferPtr& x, const BufferPtr& y) {
    return (x.addr() == y.addr()) && (x.size() == y.size());
}

inline bool operator != (const BufferPtr& x, const BufferPtr& y) {
    return !(x == y);
}

template<T>
class Pointer : public BufferPtr {
public:
    explicit Pointer() : ptr_(nullptr), cnt_(0) {}
    explicit Pointer(T* ptr) : ptr_(ptr), cnt_(1) {}
    explicit Pointer(T* ptr, size_t cnt) : ptr_(ptr), cnt_(cnt) {}

    virtual uint8_t* addr() const override { return static_cast<uint8_t*>(ptr_); }

    virtual size_t size() const override { return sizeof(T) * cnt_; }

    void set(T* ptr, size_t cnt = 1) { ptr_ = ptr; cnt_ = cnt; }

    T* get(size_t idx = 0) { return idx < cnt_ ? ptr_ + idx : nullptr; }

protected:
    T* ptr_;
    size_t cnt_;
}; // class Pointer

class Buffer {
public:
    Buffer() {}
    Buffer(const std::shared_ptr<BufferPtr>& ptr) : ptr_(ptr) {}

    void set(const std::shared_ptr<BufferPtr>& ptr) { ptr_ = ptr; }

    bool null() const { return ptr_.get() == nullptr || ptr_->null(); }

    bool valid() const { return !null(); }

    uint8_t* addr() const { return ptr_->addr(); }

    size_t size() const { return ptr_->size(); }

    uint8_t* end() const { return ptr_->end(); }

    int type() const { return ptr_->type(); }

    bool resize(size_t sz) { return ptr_->resize(sz); }

    Buffer(const Buffer&) = default;
    Buffer(Buffer&&) = default;
    Buffer& operator = (const Buffer&) = default;
    Buffer& operator = (Buffer&&) = default;

private:
    std::shared_ptr<BufferPtr> ptr_ {nullptr};
}; // class Buffer

class BufferList {
public:
    BufferList() {}
    explicit BufferList(const Buffer& buff) { }

    /**
     * @brief Buffer总大小
     * 
     * @return size_t 
     */
    size_t size() const { return bsize_; }

    /**
     * @brief Buffer分段数量
     * 
     * @return size_t 
     */
    size_t count() const { return blist_.size(); }

    bool empty() const { return count() == 0 || size() == 0; }

    std::list<Buffer>::const_iterator begin() { return list_.cbegin(); }
    std::list<Buffer>::const_iterator end() { return list_.cend(); }
    std::list<Buffer>::const_iterator rbegin() { return list_.crbegin(); }
    std::list<Buffer>::const_iterator rend() { return list_.crend(); }

    void push_front(const Buffer& buff) {
        blist_.push_front(buff);
        bsize_ += buff.size();
    }

    void push_front(const BufferList& bl) {
        if (&bl == this || bl.empty()) 
            return;
        for (auto it = bl.rbegin(); it != bl.rend(); it++)
            blist_.push_front(*it);
        bsize_ += bl.size();
    }

    void pop_front() {
        if (blist_.empty())
            return;
        bsize_ -= blist_.front().size();
        blist_.pop_front();
    }

    void push_back(const Buffer& buff) {
        blist_.push_back(buff);
        bsize_ += buff.size();
    }

    void push_back(const BufferList& bl) {
        if (&bl == this || bl.empty())
            return;
        for (auto it = bl.begin(); it != bl.end(); it++)
            blist_.push_back(*it);
        bsize_ += bl.size();
    }

    void pop_back() {
        if (blist_.empty())
            return;
        bsize_ -= blist_.back().size();
        blist_.pop_back();
    }

    BufferList(const BufferList&) = default;
    BufferList(BufferList&&) = default;
    BufferList& operator = (const BufferList&) = default;
    BufferList& operator = (BufferList&&) = default;

private:
    std::list<Buffer> blist_;
    size_t bsize_ {0};
}; // class BufferList

/**
 * @brief Buffer分配器
 * 
 * 所有具体实现都需要定义其对应的BufferAllocator和BufferPtr,
 * Buffer分配器没有定义回收的方法，回收动作在BufferPtr指针被Buffer释放时触发
 * 
 */
class BufferAllocator {
public:
    virtual ~BufferAllocator() {}

    /**
     * @brief Buffer分配器所能分配的最大大小（不是Free空间）
     * 
     * @return size_t 
     */
    virtual size_t max_size() const = 0;

    /**
     * @brief 分配指定大小的Buffer
     * 需要检查Buffer是否有效(valid)，
     * 当Buffer的null()返回true或valid()返回false时，
     * 表示内存分配器不能够再分配指定大小的内存
     * @param sz 
     * @return Buffer 
     */
    virtual Buffer allocate(size_t sz) = 0;

protected:
    BufferAllocator() {}
}; // class BufferAllocator

} // namespace flame

#endif // !FLAME_INCLUDE_BUFFER_H
