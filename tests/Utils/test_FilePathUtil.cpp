#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include "duilib/Utils/FilePathUtil.h"

using ui::FilePath;
using ui::FilePathUtil;

namespace {

std::filesystem::path MakeUniqueTempPath(const char* prefix)
{
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (std::string(prefix) + std::to_string(stamp));
}

} // namespace

TEST(FilePathUtilTest, JoinFilePath)
{
    FilePath p = FilePathUtil::JoinFilePath(FilePath(_T("a")), FilePath(_T("b")));
    const std::string s = p.ToStringA();
    EXPECT_NE(s.find('a'), std::string::npos);
    EXPECT_NE(s.find('b'), std::string::npos);
}

TEST(FilePathUtilTest, NormalizeFilePathString)
{
    const DString normalized = FilePathUtil::NormalizeFilePath(_T("a/./b/../c.txt"));
    EXPECT_EQ(FilePathUtil::GetFileExtension(normalized), _T("TXT"));
    EXPECT_EQ(normalized.find(_T("..")), DString::npos);
}

TEST(FilePathUtilTest, NormalizeFilePathObject)
{
    FilePath normalized = FilePathUtil::NormalizeFilePath(FilePath(_T("a/./b/../c")));
    const std::string s = normalized.ToStringA();
    EXPECT_NE(s.find('c'), std::string::npos);
    EXPECT_EQ(s.find(".."), std::string::npos);
}

TEST(FilePathUtilTest, CreateOneDirectoryAndCreateDirectories)
{
    const std::filesystem::path oneDir = MakeUniqueTempPath("duilib_fp_one_");
    const DString oneDirText = FilePath(oneDir.string()).ToString();
    EXPECT_TRUE(FilePathUtil::CreateOneDirectory(oneDirText));
    EXPECT_FALSE(FilePathUtil::CreateOneDirectory(oneDirText));

    const std::filesystem::path nested = oneDir / "a" / "b";
    const DString nestedText = FilePath(nested.string()).ToString();
    EXPECT_TRUE(FilePathUtil::CreateDirectories(nestedText));
    EXPECT_FALSE(FilePathUtil::CreateDirectories(nestedText));

    std::filesystem::remove_all(oneDir);
}

TEST(FilePathUtilTest, GetCurrentModuleDirectory)
{
    FilePath dir = FilePathUtil::GetCurrentModuleDirectory();
    EXPECT_FALSE(dir.IsEmpty());
    EXPECT_TRUE(dir.IsExistsDirectory());
}

TEST(FilePathUtilTest, GetFileExtension)
{
    EXPECT_EQ(FilePathUtil::GetFileExtension(_T("abc.txt")), _T("TXT"));
    EXPECT_EQ(FilePathUtil::GetFileExtension(_T("/tmp/abc.tar.gz")), _T("GZ"));
    EXPECT_TRUE(FilePathUtil::GetFileExtension(_T("abc")).empty());
}
