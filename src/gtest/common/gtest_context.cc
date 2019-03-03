#include "gtest/common/gtest_context.h"
#include <string>
#include <list>
using namespace flame;

TEST_F(TestContext, Context)
{
    FlameContext* fct = FlameContext::get_context();
    ASSERT_TRUE(fct->init_config("../../../flame-csd/csd0/csd.conf"));
    ASSERT_TRUE(fct->init_log(".","TRACE","prefix_test"));
    ASSERT_TRUE(strcmp(fct->cluster_name().c_str(),"flame"));
    fct->log()->lerror("mudule_test","gtest_context.cc",12,"TEST_F","It's just a test");
}

int main(int argc, char  **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}