#include "gtest/common/gtest_config.h"
#include "common/context.h"
#include <string>
#include <list>
using namespace flame;

TEST_F(TestConfig, Config)
{
    FlameContext* fct = FlameContext::get_context();
    FlameConfig* config = FlameConfig::create_config(fct, "../../../flame-csd/csd0/csd.conf");
    ASSERT_TRUE(config->has_key("log_level"));
    ASSERT_TRUE(config->has_key("mgr_addr"));
    ASSERT_FALSE(config->has_key("myaddress"));

    ASSERT_TRUE(strcmp(config->get("log_level").c_str(),"TRACE") == 0);
    ASSERT_TRUE(strcmp(config->get("admin_address").c_str(),"192.168.23.1:7777") == 0);
}

int main(int argc, char  **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}