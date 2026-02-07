#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "duilib/Utils/StringCharset.h"

using ui::CharsetType;
using ui::StringCharset;

TEST(StringCharsetTest, GetBOMSize)
{
    EXPECT_EQ(StringCharset::GetBOMSize(CharsetType::UTF8), 3u);
    EXPECT_EQ(StringCharset::GetBOMSize(CharsetType::UTF16_LE), 2u);
    EXPECT_EQ(StringCharset::GetBOMSize(CharsetType::UTF16_BE), 2u);
    EXPECT_EQ(StringCharset::GetBOMSize(CharsetType::ANSI), 0u);
    EXPECT_EQ(StringCharset::GetBOMSize(CharsetType::UNKNOWN), 0u);
}

TEST(StringCharsetTest, DetectCharsetByBOM)
{
    const char utf8Data[] = {char(0xEF), char(0xBB), char(0xBF), 'a'};
    EXPECT_EQ(StringCharset::GetDataCharsetByBOM(utf8Data, static_cast<uint32_t>(sizeof(utf8Data))), CharsetType::UTF8);

    const char utf16LeData[] = {char(0xFF), char(0xFE), 'A', char(0x00)};
    EXPECT_EQ(StringCharset::GetDataCharsetByBOM(utf16LeData, static_cast<uint32_t>(sizeof(utf16LeData))), CharsetType::UTF16_LE);

    const char utf16BeData[] = {char(0xFE), char(0xFF), char(0x00), 'A'};
    EXPECT_EQ(StringCharset::GetDataCharsetByBOM(utf16BeData, static_cast<uint32_t>(sizeof(utf16BeData))), CharsetType::UTF16_BE);

    EXPECT_EQ(StringCharset::GetDataCharsetByBOM(nullptr, 0), CharsetType::UNKNOWN);
}

TEST(StringCharsetTest, DetectDataCharset)
{
    const std::string ascii = "abc123";
    EXPECT_EQ(StringCharset::GetDataCharset(ascii.data(), static_cast<uint32_t>(ascii.size())), CharsetType::ANSI);

    const char utf8Text[] = {char(0xE4), char(0xB8), char(0xAD)};
    EXPECT_EQ(StringCharset::GetDataCharset(utf8Text, static_cast<uint32_t>(sizeof(utf8Text))), CharsetType::UTF8);

    const char invalidBytes[] = {char(0xFF)};
    EXPECT_EQ(StringCharset::GetDataCharset(invalidBytes, static_cast<uint32_t>(sizeof(invalidBytes))), CharsetType::UNKNOWN);
}

TEST(StringCharsetTest, GetDataAsStringWithUtf8Bom)
{
    const char utf8Data[] = {char(0xEF), char(0xBB), char(0xBF), 'h', 'i'};

    std::wstring result;
    CharsetType outCharset = CharsetType::UNKNOWN;
    uint32_t bomSize = 0;
    ASSERT_TRUE(StringCharset::GetDataAsString(utf8Data, static_cast<uint32_t>(sizeof(utf8Data)), result, outCharset, bomSize));
    EXPECT_EQ(result, L"hi");
    EXPECT_EQ(outCharset, CharsetType::UTF8);
    EXPECT_EQ(bomSize, 3u);
}

TEST(StringCharsetTest, GetDataAsStringWithUtf16BeBom)
{
    const char utf16BeData[] = {
        char(0xFE), char(0xFF),
        char(0x00), char(0x41),
        char(0x00), char(0x42)
    };

    std::wstring result;
    CharsetType outCharset = CharsetType::UNKNOWN;
    uint32_t bomSize = 0;
    ASSERT_TRUE(StringCharset::GetDataAsString(utf16BeData, static_cast<uint32_t>(sizeof(utf16BeData)), result, outCharset, bomSize));
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], L'A');
    EXPECT_EQ(result[1], L'B');
    EXPECT_EQ(outCharset, CharsetType::UTF16_BE);
    EXPECT_EQ(bomSize, 2u);
}

TEST(StringCharsetTest, GetDataAsStringMismatchWithBomReturnsFalse)
{
    const char utf16LeData[] = {char(0xFF), char(0xFE), 'A', char(0x00)};

    std::wstring result;
    CharsetType outCharset = CharsetType::UNKNOWN;
    uint32_t bomSize = 0;
    EXPECT_FALSE(StringCharset::GetDataAsString(
        utf16LeData,
        static_cast<uint32_t>(sizeof(utf16LeData)),
        CharsetType::UTF8,
        result,
        outCharset,
        bomSize));
}

TEST(StringCharsetTest, GetDataAsStringWithSpecifiedUtf8NoBom)
{
    const char utf8Text[] = {char(0xE4), char(0xB8), char(0xAD)};

    std::wstring result;
    CharsetType outCharset = CharsetType::UNKNOWN;
    uint32_t bomSize = 0;
    ASSERT_TRUE(StringCharset::GetDataAsString(
        utf8Text,
        static_cast<uint32_t>(sizeof(utf8Text)),
        CharsetType::UTF8,
        result,
        outCharset,
        bomSize));
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(outCharset, CharsetType::UTF8);
    EXPECT_EQ(bomSize, 0u);
}

TEST(StringCharsetTest, ValidateStreamHelpers)
{
    const char ascii[] = "abc";
    EXPECT_TRUE(StringCharset::IsValidateASCIIStream(ascii, 3));

    const char validUtf8[] = {char(0xE4), char(0xB8), char(0xAD)};
    EXPECT_TRUE(StringCharset::IsValidateUTF8Stream(validUtf8, static_cast<uint32_t>(sizeof(validUtf8))));

    const char invalidUtf8[] = {char(0xE4), char(0x00)};
    EXPECT_FALSE(StringCharset::IsValidateUTF8Stream(invalidUtf8, static_cast<uint32_t>(sizeof(invalidUtf8))));
}
