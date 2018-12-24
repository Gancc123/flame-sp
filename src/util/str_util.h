#ifndef FLAME_UTIL_STR_UTIL_H
#define FLAME_UTIL_STR_UTIL_H

#include <initializer_list>
#include <cstdint>
#include <string>
#include <sstream>

void string_reverse(std::string& str, size_t off, size_t len);
inline void string_reverse(std::string& str, size_t len) { string_reverse(str, 0, len); }
inline void string_reverse(std::string& str) { string_reverse(str, 0, str.size()); } 

std::string int64_to_string(const int64_t v);
std::string uint64_to_string(const uint64_t v);

template<typename T>
inline std::string convert2string(const T& obj) { 
    std::ostringstream oss;
    oss << obj;
    return oss.str();
}

// inline std::string convert2string(char v) { return std::string(1, v); }
// inline std::string convert2string(unsigned char v) { return std::string(1, v); }
// inline std::string convert2string(int16_t v) { return int64_to_string(v); }
// inline std::string convert2string(uint16_t v) { return uint64_to_string(v); }
// inline std::string convert2string(int32_t v) { return int64_to_string(v); }
// inline std::string convert2string(uint32_t v) { return uint64_to_string(v); }
// inline std::string convert2string(int64_t v) { return int64_to_string(v); }
// inline std::string convert2string(uint64_t v) { return uint64_to_string(v); }

inline void string_append(std::string& str, char separator, const std::string& tail) {
    if (!str.empty()) str.push_back(separator);
    str.append(tail);
}

inline void string_append(std::string& str, const std::string& separator, const std::string& tail) {
    if (!str.empty()) str.append(separator);
    str.append(tail);
}

inline std::string string_concat(const std::initializer_list<std::string>& strs) {
    std::string res;
    for (auto it = strs.begin(); it != strs.end(); it++)
        res.append(*it);
    return res;
}

std::string string_encode_ez(const std::string& src, char spc);
std::string string_encode_box(const std::string& src, char spc);

template<typename T>
bool string_parse(T& dst, const std::string& str) {
    std::istringstream iss(str);
    iss >> dst;
    return true;
}

#endif // FLAME_UTIL_STR_UTIL_H