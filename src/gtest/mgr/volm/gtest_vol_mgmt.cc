#include "gtest/mgr/volm/gtest_vol_mgmt.h"
#ifdef GTEST
#define private public
#define protected public
#endif

using namespace flame;

TEST_F(TestVolumeManager, Test1)
 {
    //ChangeStudentAge(5);
    //ChangeStudentAge(55)
    // you can refer to s here
    Print();
}
TEST_F(TestVolumeManager, Test2)
 {
    //ChangeStudentAge(55);
    // you can refer to s here
    Print();
}
TEST_F(TestVolumeManager, Test3)
 {
    Print();
    // you can refer to s here
    //print();
}

int main(int argc, char  **argv)
{
    testing::AddGlobalTestEnvironment(new TestEnvironment);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}