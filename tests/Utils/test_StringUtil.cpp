#include <gtest/gtest.h>
#include "duilib/Utils/StringUtil.h"

using ui::StringUtil;

// --- Printf ---

TEST(StringUtilTest, Printf_Wchar)
{
    std::wstring result = StringUtil::Printf(L"Hello %s, %d", L"World", 42);
    EXPECT_EQ(result, L"Hello World, 42");
}

TEST(StringUtilTest, Printf_Char)
{
    std::string result = StringUtil::Printf("Hello %s, %d", "World", 42);
    EXPECT_EQ(result, "Hello World, 42");
}

TEST(StringUtilTest, Printf_EmptyFormat)
{
    std::string result = StringUtil::Printf("");
    EXPECT_EQ(result, "");
}

// --- ReplaceAll ---

TEST(StringUtilTest, ReplaceAll_Wchar)
{
    std::wstring output = L"aabbcc";
    size_t count = StringUtil::ReplaceAll(L"bb", L"XX", output);
    EXPECT_EQ(output, L"aaXXcc");
    EXPECT_EQ(count, 1u);
}

TEST(StringUtilTest, ReplaceAll_Char)
{
    std::string output = "hello world hello";
    size_t count = StringUtil::ReplaceAll("hello", "hi", output);
    EXPECT_EQ(output, "hi world hi");
    EXPECT_EQ(count, 2u);
}

TEST(StringUtilTest, ReplaceAll_NotFound)
{
    std::string output = "hello";
    size_t count = StringUtil::ReplaceAll("xyz", "abc", output);
    EXPECT_EQ(output, "hello");
    EXPECT_EQ(count, 0u);
}

TEST(StringUtilTest, ReplaceAll_EmptyOutput)
{
    std::string output = "";
    size_t count = StringUtil::ReplaceAll("a", "b", output);
    EXPECT_EQ(count, 0u);
}

// --- LowerString / UpperString ---

TEST(StringUtilTest, LowerString_Char)
{
    std::string s = "Hello WORLD";
    StringUtil::LowerString(s);
    EXPECT_EQ(s, "hello world");
}

TEST(StringUtilTest, LowerString_Wchar)
{
    std::wstring s = L"Hello WORLD";
    StringUtil::LowerString(s);
    EXPECT_EQ(s, L"hello world");
}

TEST(StringUtilTest, UpperString_Char)
{
    std::string s = "Hello world";
    StringUtil::UpperString(s);
    EXPECT_EQ(s, "HELLO WORLD");
}

TEST(StringUtilTest, UpperString_Wchar)
{
    std::wstring s = L"Hello world";
    StringUtil::UpperString(s);
    EXPECT_EQ(s, L"HELLO WORLD");
}

// --- MakeLowerString / MakeUpperString ---

TEST(StringUtilTest, MakeLowerString_Char)
{
    EXPECT_EQ(StringUtil::MakeLowerString(std::string("ABC")), "abc");
}

TEST(StringUtilTest, MakeLowerString_Wchar)
{
    EXPECT_EQ(StringUtil::MakeLowerString(std::wstring(L"ABC")), L"abc");
}

TEST(StringUtilTest, MakeUpperString_Char)
{
    EXPECT_EQ(StringUtil::MakeUpperString(std::string("abc")), "ABC");
}

TEST(StringUtilTest, MakeUpperString_Wchar)
{
    EXPECT_EQ(StringUtil::MakeUpperString(std::wstring(L"abc")), L"ABC");
}

TEST(StringUtilTest, MakeLowerString_Empty)
{
    EXPECT_EQ(StringUtil::MakeLowerString(std::string("")), "");
    EXPECT_EQ(StringUtil::MakeLowerString(std::wstring(L"")), L"");
}

// --- TrimLeft / TrimRight / Trim ---

TEST(StringUtilTest, TrimLeft_Char)
{
    EXPECT_EQ(StringUtil::TrimLeft("  hello"), "hello");
}

TEST(StringUtilTest, TrimRight_Char)
{
    EXPECT_EQ(StringUtil::TrimRight("hello  "), "hello");
}

TEST(StringUtilTest, Trim_Char)
{
    EXPECT_EQ(StringUtil::Trim("  hello  "), "hello");
}

TEST(StringUtilTest, TrimLeft_Wchar)
{
    EXPECT_EQ(StringUtil::TrimLeft(L"  hello"), L"hello");
}

TEST(StringUtilTest, TrimRight_Wchar)
{
    EXPECT_EQ(StringUtil::TrimRight(L"hello  "), L"hello");
}

TEST(StringUtilTest, Trim_Wchar)
{
    EXPECT_EQ(StringUtil::Trim(L"  hello  "), L"hello");
}

TEST(StringUtilTest, TrimLeft_StringRef)
{
    std::string s = "  hello";
    StringUtil::TrimLeft(s);
    EXPECT_EQ(s, "hello");
}

TEST(StringUtilTest, TrimRight_StringRef)
{
    std::string s = "hello  ";
    StringUtil::TrimRight(s);
    EXPECT_EQ(s, "hello");
}

TEST(StringUtilTest, Trim_StringRef)
{
    std::string s = "  hello  ";
    StringUtil::Trim(s);
    EXPECT_EQ(s, "hello");
}

// --- Split ---

TEST(StringUtilTest, Split_Char)
{
    auto result = StringUtil::Split(std::string("a,b,c"), std::string(","));
    ASSERT_EQ(result.size(), 3u);
    auto it = result.begin();
    EXPECT_EQ(*it++, "a");
    EXPECT_EQ(*it++, "b");
    EXPECT_EQ(*it++, "c");
}

TEST(StringUtilTest, Split_Wchar)
{
    auto result = StringUtil::Split(std::wstring(L"hello world"), std::wstring(L" "));
    ASSERT_EQ(result.size(), 2u);
    auto it = result.begin();
    EXPECT_EQ(*it++, L"hello");
    EXPECT_EQ(*it++, L"world");
}

TEST(StringUtilTest, Split_Empty)
{
    auto result = StringUtil::Split(std::string(""), std::string(","));
    EXPECT_TRUE(result.empty());
}

TEST(StringUtilTest, Split_MultipleDelimiters)
{
    auto result = StringUtil::Split(std::string("a,,b"), std::string(","));
    ASSERT_EQ(result.size(), 2u);
    auto it = result.begin();
    EXPECT_EQ(*it++, "a");
    EXPECT_EQ(*it++, "b");
}

// --- IsEqualNoCase ---

TEST(StringUtilTest, IsEqualNoCase_Wstring)
{
    EXPECT_TRUE(StringUtil::IsEqualNoCase(std::wstring(L"Hello"), std::wstring(L"hello")));
    EXPECT_TRUE(StringUtil::IsEqualNoCase(std::wstring(L"ABC"), std::wstring(L"abc")));
    EXPECT_FALSE(StringUtil::IsEqualNoCase(std::wstring(L"abc"), std::wstring(L"abcd")));
}

TEST(StringUtilTest, IsEqualNoCase_String)
{
    EXPECT_TRUE(StringUtil::IsEqualNoCase(std::string("Hello"), std::string("hello")));
    EXPECT_FALSE(StringUtil::IsEqualNoCase(std::string("abc"), std::string("xyz")));
}

TEST(StringUtilTest, IsEqualNoCase_WcharPtr)
{
    EXPECT_TRUE(StringUtil::IsEqualNoCase(L"Hello", std::wstring(L"hello")));
    EXPECT_TRUE(StringUtil::IsEqualNoCase(std::wstring(L"Hello"), L"hello"));
}

TEST(StringUtilTest, IsEqualNoCase_CharPtr)
{
    EXPECT_TRUE(StringUtil::IsEqualNoCase("Hello", std::string("hello")));
    EXPECT_TRUE(StringUtil::IsEqualNoCase(std::string("Hello"), "hello"));
}

// --- StringToInt32 ---

TEST(StringUtilTest, StringToInt32_Wstring)
{
    EXPECT_EQ(StringUtil::StringToInt32(std::wstring(L"123")), 123);
    EXPECT_EQ(StringUtil::StringToInt32(std::wstring(L"-456")), -456);
    EXPECT_EQ(StringUtil::StringToInt32(std::wstring(L"0")), 0);
}

TEST(StringUtilTest, StringToInt32_String)
{
    EXPECT_EQ(StringUtil::StringToInt32(std::string("123")), 123);
    EXPECT_EQ(StringUtil::StringToInt32(std::string("-456")), -456);
}

TEST(StringUtilTest, StringToInt32_WcharPtr)
{
    EXPECT_EQ(StringUtil::StringToInt32(L"42"), 42);
}

TEST(StringUtilTest, StringToInt32_CharPtr)
{
    EXPECT_EQ(StringUtil::StringToInt32("42"), 42);
}

// --- StringToInt64 ---

TEST(StringUtilTest, StringToInt64_Wstring)
{
    EXPECT_EQ(StringUtil::StringToInt64(std::wstring(L"1234567890123")), 1234567890123LL);
}

TEST(StringUtilTest, StringToInt64_String)
{
    EXPECT_EQ(StringUtil::StringToInt64(std::string("1234567890123")), 1234567890123LL);
}

// --- StringToDouble ---

TEST(StringUtilTest, StringToDouble_Wstring)
{
    EXPECT_DOUBLE_EQ(StringUtil::StringToDouble(std::wstring(L"3.14")), 3.14);
}

TEST(StringUtilTest, StringToDouble_String)
{
    EXPECT_DOUBLE_EQ(StringUtil::StringToDouble(std::string("3.14")), 3.14);
}

TEST(StringUtilTest, StringToDouble_Zero)
{
    EXPECT_DOUBLE_EQ(StringUtil::StringToDouble(std::string("0.0")), 0.0);
}

// --- UInt64ToStringW / Int32ToStringW ---

TEST(StringUtilTest, UInt64ToStringW)
{
    EXPECT_EQ(StringUtil::UInt64ToStringW(0), L"0");
    EXPECT_EQ(StringUtil::UInt64ToStringW(12345), L"12345");
    EXPECT_EQ(StringUtil::UInt64ToStringW(18446744073709551615ULL), L"18446744073709551615");
}

TEST(StringUtilTest, Int32ToStringW)
{
    EXPECT_EQ(StringUtil::Int32ToStringW(0), L"0");
    EXPECT_EQ(StringUtil::Int32ToStringW(42), L"42");
    EXPECT_EQ(StringUtil::Int32ToStringW(-42), L"-42");
}

TEST(StringUtilTest, UInt64ToStringA)
{
    EXPECT_EQ(StringUtil::UInt64ToStringA(0), "0");
    EXPECT_EQ(StringUtil::UInt64ToStringA(12345), "12345");
}

TEST(StringUtilTest, Int32ToStringA)
{
    EXPECT_EQ(StringUtil::Int32ToStringA(0), "0");
    EXPECT_EQ(StringUtil::Int32ToStringA(-100), "-100");
}

TEST(StringUtilTest, StringCompare_CharPtrNullHandling)
{
    EXPECT_EQ(StringUtil::StringCompare(static_cast<const char*>(nullptr), static_cast<const char*>(nullptr)), 0);
    EXPECT_LT(StringUtil::StringCompare(static_cast<const char*>(nullptr), "abc"), 0);
    EXPECT_GT(StringUtil::StringCompare("abc", static_cast<const char*>(nullptr)), 0);
    EXPECT_LT(StringUtil::StringCompare("abc", "abd"), 0);
}

TEST(StringUtilTest, StringICompare_CaseInsensitive)
{
    EXPECT_EQ(StringUtil::StringICompare("AbC", "aBc"), 0);
    EXPECT_LT(StringUtil::StringICompare("abc", "abd"), 0);
    EXPECT_GT(StringUtil::StringICompare("abd", "abc"), 0);
}

TEST(StringUtilTest, StringToInt32AndUInt32WithRadixAndEndPtr)
{
    char* end = nullptr;
    const int32_t valueHex = StringUtil::StringToInt32("0x2Axyz", &end, 0);
    EXPECT_EQ(valueHex, 42);
    ASSERT_NE(end, nullptr);
    EXPECT_STREQ(end, "xyz");

    const uint32_t uintHex = StringUtil::StringToUInt32("FFtail", &end, 16);
    EXPECT_EQ(uintHex, 255u);
    ASSERT_NE(end, nullptr);
    EXPECT_STREQ(end, "tail");
}

TEST(StringUtilTest, StringToFloatWithEndPtr)
{
    char* end = nullptr;
    const float value = StringUtil::StringToFloat("3.5ms", &end);
    EXPECT_FLOAT_EQ(value, 3.5f);
    ASSERT_NE(end, nullptr);
    EXPECT_STREQ(end, "ms");
}

TEST(StringUtilTest, StringCopyAndStringNCopyChar)
{
    char buf[16] = {};
    EXPECT_EQ(StringUtil::StringCopy(buf, "hello"), 0);
    EXPECT_STREQ(buf, "hello");

    char buf2[16] = {};
    EXPECT_EQ(StringUtil::StringNCopy(buf2, sizeof(buf2), "abcdef", 3), 0);
    EXPECT_STREQ(buf2, "abc");
}

TEST(StringUtilTest, StringCopyAndStringNCopyWchar)
{
    wchar_t buf[16] = {};
    EXPECT_EQ(StringUtil::StringCopy(buf, L"hello"), 0);
    EXPECT_STREQ(buf, L"hello");

    wchar_t buf2[16] = {};
    EXPECT_EQ(StringUtil::StringNCopy(buf2, sizeof(buf2) / sizeof(buf2[0]), L"abcdef", 4), 0);
    EXPECT_STREQ(buf2, L"abcd");
}

TEST(StringUtilTest, StringLenHandlesNullAndValidInput)
{
    EXPECT_EQ(StringUtil::StringLen(static_cast<const char*>(nullptr)), 0u);
    EXPECT_EQ(StringUtil::StringLen(static_cast<const wchar_t*>(nullptr)), 0u);
    EXPECT_EQ(StringUtil::StringLen("abc"), 3u);
    EXPECT_EQ(StringUtil::StringLen(L"abcd"), 4u);
}
