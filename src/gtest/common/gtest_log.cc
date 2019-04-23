#include "gtest/common/gtest_log.h"
#include <string>
#include <list>
using namespace flame;

TEST_F(TestLog, LogPrinter)
{
    FILE *fp;
    ASSERT_TRUE( (fp = fopen("test.log","wb") )!=NULL);

    LogPrinter *logprinter = new LogPrinter(fp, LogLevel::TRACE);
    logprinter->plog(LogLevel::TRACE,"test","gtest_log.cc",9,"TEST_F","It's just a test");
    logprinter->set_with_console(true);
    logprinter->set_imm_flush(false);
    ASSERT_FALSE(logprinter->is_imm_flush());//**只是不立即刷回，还是会写日志
    logprinter->plog(LogLevel::TRACE,"test","gtest_log.cc",14,"TEST_F","It's just a test");
    logprinter->set_level(LogLevel::ERROR);
    logprinter->plog(LogLevel::TRACE,"test","gtest_log.cc",18,"TEST_F","It's just a test");
}

TEST_F(TestLog, Logger)
{
    FILE *fp;
    ASSERT_TRUE( (fp = fopen("test.log","wb") )!=NULL);

    Logger *logger = new Logger(".","test_logger");
    logger->plog(LogLevel::INFO,"test","gtest_log.cc",27,"TEST_F","It's just a test");
}

int main(int argc, char  **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}