#ifndef FLAME_MSG_INTERNAL_ERRNO_H
#define FLAME_MSG_INTERNAL_ERRNO_H

#include <string>
#include <exception>

/* Return a given error code as a string */
std::string cpp_strerror(int err);

namespace flame{

class ErrnoException : public std::exception{
public:
    int m_errno;
    std::string m_what;

    explicit ErrnoException(int e) : m_errno(e){
        m_what = cpp_strerror(m_errno);
    }

    int get_errno() const { return m_errno; }

    virtual const char* what() const throw(){
        return m_what.c_str();
    }
};


}

#endif