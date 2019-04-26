#include "mgr/volm/vol_mgmt.h"
#include <gtest/gtest.h>
#include <iostream>
using namespace std;

namespace flame {
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
class TestVolumeManager:public testing::Test
{
public:
    TestVolumeManager(){
        //volume_manager_ = new VolumeManager();
    }
    TestVolumeManager(int n){
        //volume_manager_ = new VolumeManager();
    }

    void ChangeStudentAge(int n){
        //share_ = n;
        //s_->SetAge(n);
        //cout << "Student age is " << s_->GetAge() << "; share_ is "<< share_ <<endl;
        cout << "Success" << endl;
    }
    void Print(){
        //cout << "Student age is " << s_->GetAge() << "; share_ is "<< share_ <<endl;
        cout << "Success" << endl;
    }

    /*下述两个函数针对一个TestSuite*/
    static void SetUpTestCase()
    {
        cout<<"SetUpTestCase()"<<endl;
    }
 
    static void TearDownTestCase()
    {
        //delete s;
        cout<<"TearDownTestCase()"<<endl;
    }
    /* 下述两个函数针对每个TEST*/
    void SetUp()//构造后调用
    {
        cout<<"SetUp() is running"<<endl;
         
    }
    void TearDown()//析构前调用
    {
        cout<<"TearDown()"<<endl;
    }  
private:
    VolumeManager *volume_manager_;
};// class TestVolumeManager

// class IsPrimeParamTest : public::testing::TestWithParam<int>{};

// TEST_P(IsPrimeParamTest, HandleTrueReturn)
// {
//     int n =  GetParam();
//     cout << n << endl;
// }
// INSTANTIATE_TEST_CASE_P(TrueReturn, IsPrimeParamTest, testing::Values(3,4,7));

}// FLAME_GTEST_VOL_MGMT_H


