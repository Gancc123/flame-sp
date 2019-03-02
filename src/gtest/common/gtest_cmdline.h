#include <gtest/gtest.h>
#include <iostream>


#ifdef GTEST
#define private public
#define protected public
#endif
#include "common/cmdline.h"

using namespace std;
using namespace flame::cli;

int PrintDebug(Cmdline *cmd){
    return 0;
}

class TestEnvironment : public testing::Environment{
public:
    virtual void SetUp()
    {
        cout << "TestEnvironment SetUp" << endl;
    }
    virtual void TearDown()
    {
        cout << "TestEnvironment TearDown" << endl;
    }
};//class TestEnvironment

//在第一个test之前，最后一个test之后调用SetUpTestCase()和TearDownTestCase()
class TestCommandLine:public testing::Test
{
public:
    TestCommandLine(){
        cmdline_ = new Cmdline("FlameClient","This is a FlameClient command line");
    }
    /*下述两个函数针对一个TestSuite*/
    static void SetUpTestCase()
    {
        //cout<<"SetUpTestCase()"<<endl;
    }
 
    static void TearDownTestCase()
    {
        //cout<<"TearDownTestCase()"<<endl;
    }
    /* 下述两个函数针对每个TEST*/
    void SetUp()//构造后调用
    {
        //cout<<"SetUp() is running"<<endl;
         
    }
    void TearDown()//析构前调用
    {
        //cout<<"TearDown()"<<endl;
    }  

private:
    int (*MyFunction)(Cmdline* cmd) = PrintDebug;
    Cmdline             *cmdline_;
};// class TestVolumeManager

// class IsPrimeParamTest : public::testing::TestWithParam<int>{};

// TEST_P(IsPrimeParamTest, HandleTrueReturn)
// {
//     int n =  GetParam();
//     cout << n << endl;
// }
// INSTANTIATE_TEST_CASE_P(TrueReturn, IsPrimeParamTest, testing::Values(3,4,7));



