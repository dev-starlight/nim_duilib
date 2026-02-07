#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <vector>
#include "duilib/Utils/FilePath.h"

using ui::FilePath;

namespace {

std::filesystem::path MakeUniqueTempPath(const char* prefix)
{
    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (std::string(prefix) + std::to_string(stamp));
}

char PathSepChar()
{
#ifdef DUILIB_BUILD_FOR_WIN
    return '\\';
#else
    return '/';
#endif
}

} // namespace

TEST(FilePathTest, EmptyAndClear)
{
    FilePath path;
    EXPECT_TRUE(path.IsEmpty());
    EXPECT_EQ(path.HashValue(), static_cast<size_t>(0));

    path = _T("abc");
    EXPECT_FALSE(path.IsEmpty());
    path.Clear();
    EXPECT_TRUE(path.IsEmpty());
}

TEST(FilePathTest, JoinAndFileName)
{
    FilePath path(_T("root"));
    path.JoinFilePath(FilePath(_T("folder")));
    path.JoinFilePath(FilePath(_T("file.txt")));

    EXPECT_EQ(path.GetFileName(), _T("file.txt"));
    EXPECT_EQ(path.GetFileExtension(), _T(".txt"));
}

TEST(FilePathTest, FormatAndTrimPathSeparator)
{
    FilePath path((MakeUniqueTempPath("duilib_fp_sep_") / "dir").string());
    path.FormatPathAsDirectory();
    std::string formatted = path.ToStringA();
    ASSERT_FALSE(formatted.empty());
    EXPECT_EQ(formatted.back(), PathSepChar());

    path.TrimRightPathSeparator();
    std::string trimmed = path.ToStringA();
    ASSERT_FALSE(trimmed.empty());
    EXPECT_NE(trimmed.back(), PathSepChar());
}

TEST(FilePathTest, ExistsAndFileSize)
{
    const std::filesystem::path dir = MakeUniqueTempPath("duilib_fp_exists_");
    const std::filesystem::path file = dir / "sample.txt";
    std::filesystem::create_directories(dir);
    {
        std::ofstream out(file, std::ios::binary);
        out << "hello";
    }

    FilePath dirPath(dir.string());
    FilePath filePath(file.string());

    EXPECT_TRUE(dirPath.IsExistsPath());
    EXPECT_TRUE(dirPath.IsExistsDirectory());
    EXPECT_FALSE(dirPath.IsExistsFile());

    EXPECT_TRUE(filePath.IsExistsPath());
    EXPECT_TRUE(filePath.IsExistsFile());
    EXPECT_FALSE(filePath.IsExistsDirectory());
    EXPECT_EQ(filePath.GetFileSize(), static_cast<uint64_t>(5));

    std::filesystem::remove_all(dir);
}

TEST(FilePathTest, SubDirectoryAndParentList)
{
    const std::filesystem::path base = MakeUniqueTempPath("duilib_fp_parent_");
    const std::filesystem::path child = base / "a" / "b";
    FilePath basePath(base.string());
    FilePath childPath(child.string());

    EXPECT_TRUE(childPath.IsSubDirectory(basePath));
    EXPECT_FALSE(basePath.IsSubDirectory(childPath));

    std::vector<FilePath> parents;
    childPath.GetParentPathList(parents);
    ASSERT_FALSE(parents.empty());
    EXPECT_EQ(parents.back().GetFileName(), _T("a"));
}

TEST(FilePathTest, Operators)
{
    FilePath left(_T("ab"));
    left += _T("cd");
    EXPECT_NE(left.ToStringA().find("abcd"), std::string::npos);

    FilePath joined(_T("x"));
    joined /= FilePath(_T("y"));
    std::string value = joined.ToStringA();
    EXPECT_NE(value.find('x'), std::string::npos);
    EXPECT_NE(value.find('y'), std::string::npos);
}

TEST(FilePathTest, RelativeAndAbsolute)
{
    FilePath relative(_T("relative/path.txt"));
    EXPECT_TRUE(relative.IsRelativePath());
    EXPECT_FALSE(relative.IsAbsolutePath());

    FilePath absolute(std::filesystem::temp_directory_path().string());
    EXPECT_TRUE(absolute.IsAbsolutePath());
}

TEST(FilePathTest, CompareAndHash)
{
    FilePath a(_T("path_a"));
    FilePath b(_T("path_b"));
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a < b || b < a);
    EXPECT_NE(a.HashValue(), static_cast<size_t>(0));
}
