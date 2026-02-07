#include <gtest/gtest.h>

#include "duilib/Core/CompiledResourceLoader.h"

#include <algorithm>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

namespace ui {
namespace test {

namespace {

struct TestEntry {
    std::string path;
    std::vector<uint8_t> data;
};

void AppendU32(std::vector<uint8_t>& out, uint32_t value)
{
    out.push_back(static_cast<uint8_t>(value & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void WriteU32(std::vector<uint8_t>& out, size_t offset, uint32_t value)
{
    ASSERT_GE(out.size(), offset + 4);
    out[offset + 0] = static_cast<uint8_t>(value & 0xFF);
    out[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    out[offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
    out[offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
}

std::vector<uint8_t> BuildCompiledBlob(const std::vector<TestEntry>& entries)
{
    const uint32_t kMagic = 0x44554952; // 'DUIR'
    const uint32_t kVersion = 1;

    size_t tableSize = 12;
    for (const auto& entry : entries) {
        tableSize += 12 + entry.path.size();
    }

    std::vector<uint8_t> blob;
    blob.reserve(tableSize);

    AppendU32(blob, kMagic);
    AppendU32(blob, kVersion);
    AppendU32(blob, static_cast<uint32_t>(entries.size()));

    uint32_t dataOffset = static_cast<uint32_t>(tableSize);
    for (const auto& entry : entries) {
        AppendU32(blob, static_cast<uint32_t>(entry.path.size()));
        AppendU32(blob, dataOffset);
        AppendU32(blob, static_cast<uint32_t>(entry.data.size()));
        blob.insert(blob.end(), entry.path.begin(), entry.path.end());
        dataOffset += static_cast<uint32_t>(entry.data.size());
    }

    for (const auto& entry : entries) {
        blob.insert(blob.end(), entry.data.begin(), entry.data.end());
    }

    return blob;
}

bool ContainsPath(const std::vector<DString>& values, const DString& target)
{
    return std::find(values.begin(), values.end(), target) != values.end();
}

} // namespace

TEST(CompiledResourceLoaderTest, InitializeRejectsNullOrZero)
{
    CompiledResourceLoader loader;
    EXPECT_FALSE(loader.Initialize(nullptr, 0));

    const uint8_t oneByte = 0;
    EXPECT_FALSE(loader.Initialize(&oneByte, 0));
}

TEST(CompiledResourceLoaderTest, InvalidMagicProducesEmptyIndex)
{
    auto blob = BuildCompiledBlob({{"images/icon.png", {0x01, 0x02}}});
    WriteU32(blob, 0, 0x12345678);

    CompiledResourceLoader loader;
    ASSERT_TRUE(loader.Initialize(blob.data(), blob.size()));
    EXPECT_FALSE(loader.Exists(_T(":/images/icon.png")));

    const uint8_t* data = nullptr;
    size_t size = 0;
    EXPECT_FALSE(loader.GetResourceData(_T(":/images/icon.png"), data, size));
}

TEST(CompiledResourceLoaderTest, ExistsAndGetResourceData)
{
    const std::vector<TestEntry> entries = {
        {"images/icon.png", {0x89, 0x50, 0x4E, 0x47}},
        {"xml/window.xml", {'<', 'w', 'i', 'n', 'd', 'o', 'w', '/', '>'}}
    };
    auto blob = BuildCompiledBlob(entries);

    CompiledResourceLoader loader;
    ASSERT_TRUE(loader.Initialize(blob.data(), blob.size()));

    EXPECT_TRUE(loader.Exists(_T(":/images/icon.png")));
    EXPECT_TRUE(loader.Exists(_T("images\\icon.png")));
    EXPECT_TRUE(loader.Exists(_T("IMAGES/ICON.PNG")));
    EXPECT_FALSE(loader.Exists(_T(":/missing.file")));

    const uint8_t* ptr = nullptr;
    size_t size = 0;
    ASSERT_TRUE(loader.GetResourceData(_T(":/images/icon.png"), ptr, size));
    ASSERT_EQ(size, entries[0].data.size());
    EXPECT_EQ(0, std::memcmp(ptr, entries[0].data.data(), size));
}

TEST(CompiledResourceLoaderTest, GetResourceDataVectorAndString)
{
    const std::vector<TestEntry> entries = {
        {"xml/window.xml", {'<', 'x', 'm', 'l', '>', '\n'}}
    };
    auto blob = BuildCompiledBlob(entries);

    CompiledResourceLoader loader;
    ASSERT_TRUE(loader.Initialize(blob.data(), blob.size()));

    std::vector<uint8_t> copied;
    ASSERT_TRUE(loader.GetResourceData(_T(":/xml/window.xml"), copied));
    EXPECT_EQ(copied, entries[0].data);

    const DString text = loader.GetResourceString(_T(":/xml/window.xml"));
#if defined(DUILIB_UNICODE)
    EXPECT_EQ(text, DString(L"<xml>\n"));
#else
    EXPECT_EQ(text, DString("<xml>\n"));
#endif

    EXPECT_TRUE(loader.GetResourceString(_T(":/not_found")).empty());
}

TEST(CompiledResourceLoaderTest, CaseSensitivitySwitch)
{
    auto blob = BuildCompiledBlob({{"Case/File.txt", {'o', 'k'}}});

    CompiledResourceLoader loader;
    loader.SetCaseSensitive(true);
    EXPECT_TRUE(loader.IsCaseSensitive());
    ASSERT_TRUE(loader.Initialize(blob.data(), blob.size()));

    EXPECT_FALSE(loader.Exists(_T("case/file.txt")));
    EXPECT_TRUE(loader.Exists(_T("Case/File.txt")));
}

TEST(CompiledResourceLoaderTest, ListDirectoryAndGetAllResourcePaths)
{
    auto blob = BuildCompiledBlob({
        {"images/icon.png", {0x01}},
        {"images/logo.png", {0x02}},
        {"xml/main.xml", {0x03}},
        {"readme.txt", {'r'}}
    });

    CompiledResourceLoader loader;
    ASSERT_TRUE(loader.Initialize(blob.data(), blob.size()));

    const auto rootItems = loader.ListDirectory(DString());
    EXPECT_TRUE(ContainsPath(rootItems, _T("images")));
    EXPECT_TRUE(ContainsPath(rootItems, _T("xml")));
    EXPECT_TRUE(ContainsPath(rootItems, _T("readme.txt")));

    const auto imageItems = loader.ListDirectory(_T(":/images"));
    EXPECT_TRUE(ContainsPath(imageItems, _T("icon.png")));
    EXPECT_TRUE(ContainsPath(imageItems, _T("logo.png")));
    EXPECT_FALSE(ContainsPath(imageItems, _T("main.xml")));

    const auto all = loader.GetAllResourcePaths();
    EXPECT_EQ(all.size(), 4u);
    EXPECT_TRUE(ContainsPath(all, _T("images/icon.png")));
    EXPECT_TRUE(ContainsPath(all, _T("xml/main.xml")));
}

TEST(CompiledResourceLoaderTest, InvalidPathLengthIsHandledSafely)
{
    auto blob = BuildCompiledBlob({{"images/icon.png", {0xAA}}});

    // Corrupt path length of first entry.
    WriteU32(blob, 12, 0x00FFFFFF);

    CompiledResourceLoader loader;
    ASSERT_TRUE(loader.Initialize(blob.data(), blob.size()));
    EXPECT_FALSE(loader.Exists(_T(":/images/icon.png")));
    EXPECT_TRUE(loader.GetAllResourcePaths().empty());
}

TEST(CompiledResourceLoaderTest, OutOfRangeDataOffsetEntryIgnored)
{
    auto blob = BuildCompiledBlob({
        {"images/icon.png", {0x10, 0x11}},
        {"xml/main.xml", {0x20, 0x21}}
    });

    // Corrupt second entry data offset and size.
    // Header 12 bytes + first entry (12 + len("images/icon.png") = 27)
    // second entry dataOffset starts at 12 + 27 + 4 = 43
    const size_t secondDataOffsetPos = 43;
    const size_t secondDataSizePos = 47;
    WriteU32(blob, secondDataOffsetPos, 0xFFFFFF00);
    WriteU32(blob, secondDataSizePos, 0x00000040);

    CompiledResourceLoader loader;
    ASSERT_TRUE(loader.Initialize(blob.data(), blob.size()));

    EXPECT_TRUE(loader.Exists(_T(":/images/icon.png")));
    EXPECT_FALSE(loader.Exists(_T(":/xml/main.xml")));
}

TEST(CompiledResourceLoaderTest, NormalizePathHandlesPrefixAndSlashes)
{
    auto blob = BuildCompiledBlob({{"assets/icons/menu.svg", {0x31}}});

    CompiledResourceLoader loader;
    ASSERT_TRUE(loader.Initialize(blob.data(), blob.size()));

    EXPECT_TRUE(loader.Exists(_T(":/assets/icons/menu.svg")));
    EXPECT_TRUE(loader.Exists(_T("/assets/icons/menu.svg")));
    EXPECT_TRUE(loader.Exists(_T("assets\\icons\\menu.svg")));
}

TEST(CompiledResourceLoaderTest, EmptyPathLookupReturnsFalse)
{
    auto blob = BuildCompiledBlob({{"a.txt", {'a'}}});

    CompiledResourceLoader loader;
    ASSERT_TRUE(loader.Initialize(blob.data(), blob.size()));

    EXPECT_FALSE(loader.Exists(_T(":/")));
    EXPECT_FALSE(loader.Exists(DString()));

    const uint8_t* data = nullptr;
    size_t size = 0;
    EXPECT_FALSE(loader.GetResourceData(_T(":/"), data, size));

    std::vector<uint8_t> copied;
    EXPECT_FALSE(loader.GetResourceData(DString(), copied));
}

TEST(CompiledResourceLoaderTest, ListDirectoryUnknownReturnsEmpty)
{
    auto blob = BuildCompiledBlob({
        {"images/icon.png", {0x01}},
        {"xml/main.xml", {0x02}}
    });

    CompiledResourceLoader loader;
    ASSERT_TRUE(loader.Initialize(blob.data(), blob.size()));

    const auto values = loader.ListDirectory(_T(":/not_exists"));
    EXPECT_TRUE(values.empty());
}

TEST(CompiledResourceLoaderTest, DuplicatePathUsesLastEntry)
{
    auto blob = BuildCompiledBlob({
        {"dup/file.bin", {0x11}},
        {"dup/file.bin", {0x22, 0x33}}
    });

    CompiledResourceLoader loader;
    ASSERT_TRUE(loader.Initialize(blob.data(), blob.size()));

    std::vector<uint8_t> copied;
    ASSERT_TRUE(loader.GetResourceData(_T(":/dup/file.bin"), copied));
    ASSERT_EQ(copied.size(), 2u);
    EXPECT_EQ(copied[0], 0x22);
    EXPECT_EQ(copied[1], 0x33);
}

TEST(CompiledResourceLoaderTest, SetCaseSensitiveNoOpWhenUnchanged)
{
    auto blob = BuildCompiledBlob({{"x/y.txt", {'o', 'k'}}});

    CompiledResourceLoader loader;
    ASSERT_TRUE(loader.Initialize(blob.data(), blob.size()));
    EXPECT_TRUE(loader.Exists(_T("X/Y.TXT")));

    loader.SetCaseSensitive(false);
    EXPECT_FALSE(loader.IsCaseSensitive());
    EXPECT_TRUE(loader.Exists(_T("X/Y.TXT")));
}

} // namespace test
} // namespace ui
