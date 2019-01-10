#ifndef FLAME_MSG_INTERNAL_UTIL_H
#define FLAME_MSG_INTERNAL_UTIL_H

#include <string>
#include <ctime>

namespace flame{
namespace msg{

/**
 * string
 */
std::string str2upper(const std::string &);
std::string str2lower(const std::string &);
uint64_t size_str_to_uint64(const std::string &);


/**
 * rand helper
 */
class rand_init_helper{
    rand_init_helper(){
        std::srand(std::time(nullptr));
        ::srand48(std::time(nullptr));
    }
public:
    static void init() {
        static rand_init_helper instance;
    }
};

uint32_t gen_rand_seq();

} //namespace msg
} //namespace flame

#endif //FLAME_MSG_INTERNAL_UTIL_H