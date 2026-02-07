#include <gtest/gtest.h>

#include "duilib/Core/ResourceParam.h"

using ui::CompiledResParam;
using ui::FilePath;
using ui::LocalFilesResParam;
using ui::ResourceType;
using ui::ZipFileResParam;

TEST(ResourceParamTest, LocalFilesDefaultsAndCtor)
{
    LocalFilesResParam defaults;
    EXPECT_EQ(defaults.GetResType(), ResourceType::kLocalFiles);
    EXPECT_TRUE(defaults.resourcePath.IsEmpty());
    EXPECT_FALSE(defaults.themePath.IsEmpty());
    EXPECT_FALSE(defaults.fontFilePath.IsEmpty());
    EXPECT_FALSE(defaults.languagePath.IsEmpty());
    EXPECT_FALSE(defaults.languageFileName.empty());
    EXPECT_FALSE(defaults.globalXmlFileName.empty());

    const FilePath inputPath(_T("C:/duilib/resources"));
    LocalFilesResParam withPath(inputPath);
    EXPECT_EQ(withPath.GetResType(), ResourceType::kLocalFiles);
    EXPECT_EQ(withPath.resourcePath.ToString(), inputPath.ToString());
}

TEST(ResourceParamTest, ZipFileDefaults)
{
    ZipFileResParam zipParam;
    EXPECT_EQ(zipParam.GetResType(), ResourceType::kZipFile);
    EXPECT_TRUE(zipParam.zipFilePath.IsEmpty());
    EXPECT_TRUE(zipParam.zipPassword.empty());
}

TEST(ResourceParamTest, CompiledResourceDefaults)
{
    CompiledResParam compiled;
    EXPECT_EQ(compiled.GetResType(), ResourceType::kCompiledResources);
    EXPECT_EQ(compiled.compiledData, nullptr);
    EXPECT_EQ(compiled.compiledDataSize, 0u);
    EXPECT_EQ(compiled.resourcePrefix, _T(":/"));
}

#ifdef DUILIB_BUILD_FOR_WIN
TEST(ResourceParamTest, ResZipDefaultsOnWindows)
{
    ui::ResZipFileResParam resZip;
    EXPECT_EQ(resZip.GetResType(), ResourceType::kResZipFile);
    EXPECT_EQ(resZip.hResModule, nullptr);
    EXPECT_STREQ(resZip.resourceName, _T(""));
    EXPECT_STREQ(resZip.resourceType, _T(""));
    EXPECT_TRUE(resZip.zipPassword.empty());
}
#endif
