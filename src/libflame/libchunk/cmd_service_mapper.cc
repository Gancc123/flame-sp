#include "include/cmd.h"


namespace flame {

CmdServiceMapper* CmdServiceMapper::g_cmd_service_mapper = nullptr;

void CmdServiceMapper::register_service(uint8_t cmd_cls, uint8_t cmd_num, CmdService* svi){
    uint16_t cn = ( cmd_cls << 8 ) | cmd_num;
    service_mapper_[cn] = svi;
    return ;
}

CmdService* CmdServiceMapper::get_service(uint8_t cmd_cls, uint8_t cmd_num){
    uint16_t cn = ( cmd_cls << 8 ) | cmd_num;
    return service_mapper_[cn];
}

} //namespace flame
