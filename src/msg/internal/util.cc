#include "util.h"

#include <algorithm>
#include <regex>

namespace flame{
namespace msg{

std::string str2upper(const std::string &v){
    std::string r_str = v;
    std::transform(r_str.begin(), r_str.end(), r_str.begin(), ::toupper);
    return r_str;
}

std::string str2lower(const std::string &v){
    std::string r_str = v;
    std::transform(r_str.begin(), r_str.end(), r_str.begin(), ::tolower);
    return r_str;
}

uint64_t size_str_to_uint64(const std::string &v) {
    static std::string unit_str = "kmgtpezy";
    auto lv = str2lower(v);
    std::regex size_regex("\\s*(\\d+)([kmgtpezy])?b?\\s*");
    std::smatch m;
    if(std::regex_match(lv, m, size_regex)){
        if(m.size() != 3){
            return -1;
        }
        uint64_t num = std::stoull(m[1].str(), nullptr, 0);
        auto unit = m[2].str();
        if(unit.empty()){
            return num;
        }
        auto index = unit_str.find(unit);
        if(index != std::string::npos){
            uint64_t new_num = num << ((index+1)*10);
            if((new_num >> ((index+1)*10)) == num ){
                //* 没有发生溢出
                return new_num;
            }
        }
    }
    return -1;
}


uint32_t gen_rand_seq(){
    static bool inited = false;
    if(!inited){
        std::srand(std::time(nullptr));
        inited = true;
    }
    return (uint32_t)std::rand();
}

} //namespace msg
} //namespace flame