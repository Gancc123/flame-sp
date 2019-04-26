#include <gtest/gtest.h>
#include <iostream>

#ifdef GTEST
#define private public
#define protected public
#endif
#include "common/convert.h"

using namespace std;
using namespace flame;

//在第一个test之前，最后一个test之后调用SetUpTestCase()和TearDownTestCase()
class TestConvert:public testing::Test
{
public:
    /*下述两个函数针对一个TestSuite*/
    static void SetUpTestCase(){    
    }
 
    static void TearDownTestCase(){  
    }
    
    /* 下述两个函数针对每个TEST*/
    void SetUp(){//构造后调用   
    }

    void TearDown(){//析构前调用
    }  

private:
    
};// class TestVolumeManager

// class IsPrimeParamTest : public::testing::TestWithParam<int>{};

// TEST_P(IsPrimeParamTest, HandleTrueReturn)
// {
//     int n =  GetParam();
//     cout << n << endl;
// }
// INSTANTIATE_TEST_CASE_P(TrueReturn, IsPrimeParamTest, testing::Values(3,4,7));



