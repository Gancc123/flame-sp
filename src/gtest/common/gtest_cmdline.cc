#include "gtest/common/gtest_cmdline.h"
#include <string>
#include <list>
using namespace flame::cli;

TEST_F(TestCommandLine, SingleCmdline)
{
    ASSERT_STREQ(cmdline_->name().c_str(), "FlameClient");
    ASSERT_STREQ(cmdline_->des().c_str(), "This is a FlameClient command line");
    ASSERT_TRUE(cmdline_->parent()==nullptr);
    ASSERT_TRUE(cmdline_->active_module()==cmdline_);
}

TEST_F(TestCommandLine, Switch)//**包含了对基类CmdBase的测试;Switch -> CmdBase
{
    Switch *myswitch = new Switch(cmdline_, 'a', "all", "print all log");
    ASSERT_EQ(myswitch->cmd_type(), SWITCH);
    ASSERT_TRUE(myswitch->short_name()=='a');
    ASSERT_STREQ(myswitch->long_name().c_str(), "all");
    ASSERT_STREQ(myswitch->des().c_str(), "print all log");
    ASSERT_FALSE(myswitch->done());
    ASSERT_STREQ(myswitch->get_def().c_str(),"false");
    ASSERT_FALSE(myswitch->default_value());
    ASSERT_FALSE(myswitch->get());
    ASSERT_TRUE(cmdline_->ln_map_["all"] == myswitch);//**默认调用了register_self()注册到cmdline_中
    ASSERT_FALSE(*myswitch);

    myswitch->set(true);
    ASSERT_TRUE(myswitch->get());
    ASSERT_TRUE(*myswitch);

    myswitch->set_done();
    ASSERT_TRUE(myswitch->done());
}

TEST_F(TestCommandLine, Argument)//**包含了对ArgumentBase的测试；Argument -> ArgumentBase -> CmdBase
{
    /**根据cmdline创建一个Argument**/
    Argument<int> *myargument = new Argument<int> {cmdline_, 't', "time", "set running time", 3000};
    ASSERT_EQ(myargument->cmd_type(), ARGUMENT);
    ASSERT_FALSE(myargument->must_);
    ASSERT_STREQ(myargument->get_def().c_str(),"3000");
    ASSERT_EQ(myargument->get(), 3000);
    ASSERT_TRUE(cmdline_->sn_map_['t'] == myargument);//**默认调用了register_self()注册到cmdline_中
    ASSERT_EQ(*myargument, 3000);

    myargument->set_with_str("4000");
    ASSERT_EQ(myargument->get(), 4000);
    ASSERT_EQ(myargument->def_val(), 3000);
}

/**   Serial相对于Argument就多了一个index，故不测试  **/

/**   Test Action  **/
TEST_F(TestCommandLine, Action)//**Action -> CmdBase
{
    /**根据cmdline创建一个Serial**/
    Action *myaction = new Action {cmdline_, "print_debug", "print debug infomation", MyFunction};
    ASSERT_EQ(myaction->cmd_type(), CmdType::ACTION);
    ASSERT_EQ(myaction->run(cmdline_), 0);
}

/**   Test Cmdline  **/
TEST_F(TestCommandLine, Cmdline)//**Cmdline
{
    /**创建多个Cmdline形成父子关系 volumecreatecmdline -> volumecmdline -> cmdline**/
    Cmdline *volumecmdline        = new Cmdline {cmdline_, "volume", "volume operations"};
    Cmdline *volumecreatecmdline  = new Cmdline {volumecmdline,"create","create a volume"};

    Switch           *switch_preallocate   = new Switch {volumecreatecmdline, 'p', "preallocate", "pre allocating the physical space for volume"};
    Switch           *switch_console       = new Switch {volumecreatecmdline, 'c', "console ", "show something on console"};
    Argument<int>    *argument_size        = new Argument<int> {volumecreatecmdline, 's', "chunk_size", "the size of chunk in this volume, unit(GB)", 4};
    Argument<string> *argument_spolicy     = new Argument<string> {volumecreatecmdline, "store_policy", "the store policy of this volume"};//**无默认值，必须指定policy
    Serial<string>   *serial_volume_group  = new Serial<string> {volumecreatecmdline, 1, "vg_name" , "the name of volume group"};
    Serial<string>   *serial_name          = new Serial<string> {volumecreatecmdline, 2, "vol_name", "the name of volume"};
    Serial<int>      *serial_size          = new Serial<int> {volumecreatecmdline, 3, "vol_size", "the size of volume"};
    
    /**逐个进行测试**/
    /*private function*/
    ASSERT_TRUE(volumecreatecmdline->check_def__());
    ASSERT_TRUE(cmdline_->check_submodule__("volume") == volumecmdline);
    ASSERT_TRUE(volumecmdline->check_submodule__("create") == volumecreatecmdline);
    ASSERT_TRUE(volumecreatecmdline->top_module__() == cmdline_);
    volumecreatecmdline->change_active__();
    ASSERT_TRUE(cmdline_->active_ == volumecreatecmdline);

    int fack_argc = 7;
    char arg0[] = "create";
    char arg1[] = "vg1";
    char arg2[] = "vol1";
    char arg3[] = "50";
    char arg4[] = "--store_policy";
    char arg5[] = "threecopies";
    char arg6[] = "error_input";
    // char **fake_argv1= new char *[fack_argc]{ arg0, arg1, arg2, arg3, arg4, arg5, arg6};
    // ASSERT_TRUE(volumecreatecmdline->do_parser__(fack_argc,fake_argv1) == CmdRetCode::FORMAT_ERROR);
    fack_argc--;
    char **fake_argv2= new char *[fack_argc]{ arg0, arg1, arg2, arg3, arg4, arg5};
    ASSERT_TRUE(volumecreatecmdline->do_parser__(fack_argc,fake_argv2) == CmdRetCode::SUCCESS);
    // bool check_def__(); //
    // Cmdline* check_submodule__(const std::string& str); // 
    // Cmdline* top_module__(); //
    // void change_active__(); //
    // int do_parser__(int argc, char** argv);


    ASSERT_TRUE(volumecmdline->parent() == cmdline_); 
    ASSERT_TRUE(volumecmdline->active_module() == volumecmdline); 
    ASSERT_TRUE(strcmp(volumecmdline->active_name().c_str(),"volume") == 0); 
    //volumecreatecmdline.register_type()在构造Switch,Argument和Serial时自动调用，前述TEST已测试
    //parent.register_submodule()在构造父子关系时自动调用
    ASSERT_TRUE(volumecmdline->sub_map_["create"] == volumecreatecmdline);
    
// bool has_tail() const { return tail_; }
// int tail_size() const { return tail_vec_.size(); }
// std::string tail_list(int idx) const { return tail_vec_[idx]; }
    volumecmdline->print_help();//**只有submodule，没有真正意义上的参数
    volumecreatecmdline->print_help();//**真正实现功能的Cmdline

    
// void print_error();

    // int parser(int argc, char** argv);
    // int run(int argc, char** argv); 

    
}

int main(int argc, char  **argv)
{
    testing::AddGlobalTestEnvironment(new TestEnvironment);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}