#include "common/convert.h"

#include <sstream>
#include <regex>

using namespace std;

template<>
string convert2string<flame::node_addr_t>(const flame::node_addr_t& obj) {
    ostringstream oss;
    uint32_t _ip = obj.get_ip();
    uint8_t* _p = (uint8_t*)&_ip;
    oss << *_p << "." << *(_p + 1) << "." << *(_p + 2) << "." << *(_p + 3);
    oss << ":" << obj.get_port();
    return oss.str();
}

template<>
bool string_parse<flame::node_addr_t>(flame::node_addr_t& dst, const std::string& str) {
    regex pattren("(\\d+).(\\d+).(\\d+).(\\d+):(\\d+)");
    smatch res;
    
    if (!regex_match(str, res, pattren))
        return false;

    int ip[4];
    for (int i = 0; i < 4; i++) {
        ip[i] = stoi(res[i + 1]);
        if (ip[i] < 0 || ip[i] > 255)
            return false;
    }
    int port = stoi(res[5]);
    if (port < 0 || port > 65535)
        return false;

    dst.set_ip(ip[0], ip[1], ip[2], ip[3]);
    dst.set_port(port);
    return true;
}
