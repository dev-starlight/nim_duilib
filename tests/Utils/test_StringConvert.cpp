#include <gtest/gtest.h>
#include "duilib/Utils/StringConvert.h"

using ui::StringConvert;

TEST(StringConvertTest, UTF8ToWStringAndBackAscii)
{
    const std::string input = "hello-world";
    const std::wstring wide = StringConvert::UTF8ToWString(input);
    const std::string output = StringConvert::WStringToUTF8(wide);
    EXPECT_EQ(output, input);
}

TEST(StringConvertTest, UTF8RoundTripWithMultibyteContent)
{
    const std::string input = "\xE4\xB8\xAD\xE6\x96\x87";
    const std::wstring wide = StringConvert::UTF8ToWString(input);
    const std::string output = StringConvert::WStringToUTF8(wide);
    EXPECT_EQ(output, input);
}

TEST(StringConvertTest, UTF8ToUTF16AndUTF16ToUTF8HandleNullAndEmpty)
{
    EXPECT_TRUE(StringConvert::UTF8ToUTF16(nullptr, 0).empty());
    EXPECT_TRUE(StringConvert::UTF16ToUTF8(nullptr, 0).empty());
}

TEST(StringConvertTest, InvalidUTF8ReturnsEmptyString)
{
    const char invalidUtf8[] = {static_cast<char>(0xC3), static_cast<char>(0x28)};
    const std::wstring wide = StringConvert::UTF8ToWString(std::string(invalidUtf8, sizeof(invalidUtf8)));
    EXPECT_TRUE(wide.empty());
}

TEST(StringConvertTest, UTF32RoundTrip)
{
    std::basic_string<DUTF32Char> utf32;
    utf32.push_back(0x0041);
    utf32.push_back(0x4E2D);
    utf32.push_back(0x6587);

    const std::string utf8 = StringConvert::UTF32ToUTF8(utf32);
    const std::basic_string<DUTF32Char> decoded = StringConvert::UTF8ToUTF32(utf8);
    EXPECT_EQ(decoded, utf32);
}

TEST(StringConvertTest, UTF8ToTAndBack)
{
    const std::string input = "abc123";
    const DString text = StringConvert::UTF8ToT(input);
    const std::string output = StringConvert::TToUTF8(text);
    EXPECT_EQ(output, input);
}
