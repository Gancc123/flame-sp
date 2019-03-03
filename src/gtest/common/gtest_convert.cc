#include "gtest/common/gtest_convert.h"
#include <string>
#include <list>
using namespace flame;

TEST_F(TestConvert, Convert)
{
    node_addr_t addr1(0, 0, 0, 0, 3333);
    node_addr_t addr2(7777);//**ip为0**//
    node_addr_t addr3();
    ASSERT_TRUE(strcmp(convert2string(addr1).c_str(),"0.0.0.0:3333")==0);
    string_parse(addr2,"192.168.3.110");
    // ASSERT_EQ(addr3.get_ip()<<16,\
    //            (uint64_t)((uint32_t)192 | ((uint32_t)168 << 8) | \
    //                      ((uint32_t)3 << 16) | ((uint32_t)110 << 24))\
    //         );
}


int main(int argc, char  **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}