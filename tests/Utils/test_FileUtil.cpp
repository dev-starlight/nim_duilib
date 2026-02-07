#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FileUtil.h"

using ui::FilePath;
using ui::FileUtil;

namespace {

std::filesystem::path MakeUniqueTempPath(const char* prefix)
{
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (std::string(prefix) + std::to_string(stamp));
}

} // namespace

TEST(FileUtilTest, WriteAndReadBinaryRoundTrip)
{
    const std::filesystem::path rootDir = MakeUniqueTempPath("duilib_fileutil_");
    std::filesystem::create_directories(rootDir);
    const std::filesystem::path filePath = rootDir / "data.bin";
    const FilePath duiPath(filePath.string());

    const std::vector<uint8_t> payload = {0x00, 0x01, 0x7F, 0x80, 0xFE, 0xFF};
    ASSERT_TRUE(FileUtil::WriteFileData(duiPath, payload));

    std::vector<uint8_t> output;
    ASSERT_TRUE(FileUtil::ReadFileData(duiPath, output));
    EXPECT_EQ(output, payload);

    std::vector<uint8_t> header;
    ASSERT_TRUE(FileUtil::ReadFileHeaderData(duiPath, 3, header));
    ASSERT_EQ(header.size(), 3u);
    EXPECT_EQ(header[0], payload[0]);
    EXPECT_EQ(header[1], payload[1]);
    EXPECT_EQ(header[2], payload[2]);

    std::filesystem::remove_all(rootDir);
}

TEST(FileUtilTest, WriteAndReadAnsiString)
{
    const std::filesystem::path rootDir = MakeUniqueTempPath("duilib_fileutil_a_");
    std::filesystem::create_directories(rootDir);
    const std::filesystem::path filePath = rootDir / "text.txt";
    const FilePath duiPath(filePath.string());

    const std::string input = "duilib-fileutil-string";
    ASSERT_TRUE(FileUtil::WriteFileData(duiPath, input));

    std::vector<uint8_t> data;
    ASSERT_TRUE(FileUtil::ReadFileData(duiPath, data));
    const std::string output(data.begin(), data.end());
    EXPECT_EQ(output, input);

    std::filesystem::remove_all(rootDir);
}

TEST(FileUtilTest, ReadMissingFileReturnsFalse)
{
    const std::filesystem::path filePath = MakeUniqueTempPath("duilib_fileutil_missing_");
    const FilePath duiPath(filePath.string());

    std::vector<uint8_t> data;
    EXPECT_FALSE(FileUtil::ReadFileData(duiPath, data));
    EXPECT_TRUE(data.empty());

    std::vector<uint8_t> header;
    EXPECT_FALSE(FileUtil::ReadFileHeaderData(duiPath, 4, header));
    EXPECT_TRUE(header.empty());
}

TEST(FileUtilTest, WriteEmptyBinaryReturnsFalse)
{
    const std::filesystem::path rootDir = MakeUniqueTempPath("duilib_fileutil_empty_");
    std::filesystem::create_directories(rootDir);
    const std::filesystem::path filePath = rootDir / "empty.bin";
    const FilePath duiPath(filePath.string());

    const std::vector<uint8_t> emptyData;
    EXPECT_FALSE(FileUtil::WriteFileData(duiPath, emptyData));

    std::filesystem::remove_all(rootDir);
}
