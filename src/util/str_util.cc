#include "str_util.h"

#include <utility>
#include <sstream>

void string_reverse(std::string& str, size_t off, size_t len) {
    size_t lhs = off;
    size_t rhs = off + len > str.size() ? str.size() : off + len;
    while (lhs < rhs) std::swap(str[lhs++], str[--rhs]);
}


std::string int64_to_string(int64_t v) {
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

std::string uint64_to_string(uint64_t v) {
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

std::string double_to_string(const double v) {
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

std::string string_encode_ez(const std::string& src, char spc) {
    std::string dst;
    for (char c : src) {
        if (c == '\'') {
            dst.append(2, c);
        } else 
            dst.push_back(c);
    }
    return dst;
}

std::string string_encode_box(const std::string& src, char spc) {
    std::string dst(1, spc);
    for (char c : src) {
        if (c == '\'') {
            dst.append(2, c);
        } else 
            dst.push_back(c);
    }
    dst.push_back(spc);
    return dst;
}