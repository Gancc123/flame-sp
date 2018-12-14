#ifndef FLAME_INCLUDE_BUFFER_H
#define FLAME_INCLUDE_BUFFER_H

#include <cstdint>

namespace flame {

class BufferBase {
public:
    virtual ~BufferBase() {}

    virtual void* data() const = 0;
    virtual size_t size() const = 0;

    virtual bool is_null() const { return data() == nullptr || size() == 0; }

    operator void* () const { return data(); }

protected:
    BufferBase() {}
}; // class BufferBase

template<typename T>
class Buffer : public BufferBase {
public:
    explicit Buffer(T* p) 
    : data_(p), len_(sizeof(T)) {}

    explicit Buffer(T* p, size_t n)
    : data_(p), len_(sizeof(T) * n) {}

    virtual ~Buffer() {}

    virtual T* data() const { return nullptr; }
    virtual size_t size() const { return len_; }

protected:
    T* data_;
    size_t len_;
}; // class Buffer

} // namespace flame

#endif // !FLAME_INCLUDE_BUFFER_H
