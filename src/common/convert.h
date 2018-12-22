/**
 * convert.h
 * 为保持include中文件的简洁，独立出该文件处理部分数据结构的转型，
 * 例如将meta.h里面的一些结构转换成字符串
 */
#ifndef FLAME_COMMON_CONVERT_H
#define FLAME_COMMON_CONVERT_H

#include "include/meta.h"
#include "util/str_util.h"

#include <string>
#include <sstream>

template<>
std::string convert2string<flame::node_addr_t>(const flame::node_addr_t& obj);

template<>
bool string_parse<flame::node_addr_t>(flame::node_addr_t& dst, const std::string& str);

#endif // !FLAME_COMMON_CONVERT_H