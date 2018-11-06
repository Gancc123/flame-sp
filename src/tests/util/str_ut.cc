#include "gtest/gtest.h"
#include "util/str_util.h"

#include <climits>
#include <string>

TEST(StrUtil, StringReverse) {
    std::string str = "";
    string_reverse(str);
    EXPECT_STREQ(str.c_str(), "") << "empty string";
    str = "0123456789";
    string_reverse(str);
    EXPECT_STREQ(str.c_str(), "9876543210") << "full length reverse";
    string_reverse(str, 5);
    EXPECT_STREQ(str.c_str(), "5678943210") << "part length reverse with (len)";
    string_reverse(str, 5, 5);
    EXPECT_STREQ(str.c_str(), "5678901234") << "part length reverse with (off, len)";
    string_reverse(str, 16);
    EXPECT_STREQ(str.c_str(), "4321098765") << "out of len";
    string_reverse(str, 5, 16);
    EXPECT_STREQ(str.c_str(), "4321056789") << "out of len with (off)";
    string_reverse(str, 0, 5);
    EXPECT_STREQ(str.c_str(), "0123456789") << "start with 0, and part";
}

TEST(StrUtil, Convert2String) {
    int16_t int16 = INT16_MIN;
    EXPECT_STREQ(convert2string(int16).c_str(), "-32768") << "INT16_MIN";
    int16 = INT16_MAX;
    EXPECT_STREQ(convert2string(int16).c_str(), "32767") << "INT16_MAX";
    uint16_t uint16 = UINT16_MAX;
    EXPECT_STREQ(convert2string(uint16).c_str(), "65535") << "UINT16_MAX";

    int32_t int32 = INT32_MIN;
    EXPECT_STREQ(convert2string(int32).c_str(), "-2147483648") << "INT32_MIN";
    int32 = INT32_MAX;
    EXPECT_STREQ(convert2string(int32).c_str(), "2147483647") << "INT32_MAX";
    uint32_t uint32 = UINT32_MAX;
    EXPECT_STREQ(convert2string(uint32).c_str(), "4294967295") << "UINT32_MAX";

    int64_t int64 = INT64_MIN;
    EXPECT_STREQ(convert2string(int64).c_str(), "-9223372036854775808") << "INT64_MIN";
    int64 = INT64_MAX;
    EXPECT_STREQ(convert2string(int64).c_str(), "9223372036854775807") << "INT64_MAX";
    uint64_t uint64 = UINT64_MAX;
    EXPECT_STREQ(convert2string(uint64).c_str(), "18446744073709551615") << "UINT64_MAX";

    char chr = 'a';
    EXPECT_STREQ(convert2string(chr).c_str(), "a") << "char";

    unsigned char uchr = 'a';
    EXPECT_STREQ(convert2string(uchr).c_str(), "a") << "unsigned char";
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}