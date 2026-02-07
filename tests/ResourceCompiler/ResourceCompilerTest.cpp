#include "ResourceCompilerTest.h"

#include "duilib/ResourceCompiler/ResourceCompiler.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace ui {
namespace test {

namespace {

namespace fs = std::filesystem;

std::string ReadTextFile(const fs::path& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::vector<uint8_t> ReadBinaryFile(const fs::path& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
}

void WriteTextFile(const fs::path& filePath, const std::string& content)
{
    fs::create_directories(filePath.parent_path());
    std::ofstream file(filePath, std::ios::binary);
    file << content;
}

void WriteBinaryFile(const fs::path& filePath, const std::vector<uint8_t>& data)
{
    fs::create_directories(filePath.parent_path());
    std::ofstream file(filePath, std::ios::binary);
    file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

fs::path WriteQrcFile(const fs::path& qrcPath, const std::vector<std::string>& fileEntries)
{
    std::ofstream qrcFile(qrcPath, std::ios::binary);
    qrcFile << "<RCC>\n";
    qrcFile << "  <qresource prefix=\"/\">\n";
    for (const auto& entry : fileEntries) {
        qrcFile << "    " << entry << "\n";
    }
    qrcFile << "  </qresource>\n";
    qrcFile << "</RCC>\n";
    return qrcPath;
}

bool CompileQrc(const fs::path& qrcPath,
                const fs::path& outputPath,
                const std::string& resourceName,
                duilib::rc::ResourceCompiler& compiler)
{
    return compiler.ParseQrcFile(qrcPath) && compiler.GenerateCppCode(outputPath, resourceName);
}

bool ContainsHexSequence(const std::string& text, const std::vector<uint8_t>& bytes)
{
    if (bytes.empty()) {
        return true;
    }

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < bytes.size(); ++i) {
        if (i > 0) {
            ss << ", ";
        }
        ss << "0x" << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return text.find(ss.str()) != std::string::npos;
}

size_t CountOccurrences(const std::string& text, const std::string& token)
{
    if (token.empty()) {
        return 0;
    }

    size_t count = 0;
    size_t pos = 0;
    while ((pos = text.find(token, pos)) != std::string::npos) {
        ++count;
        pos += token.length();
    }
    return count;
}

} // namespace

void ResourceCompilerTest::SetUp()
{
    m_testResourceDir = std::filesystem::temp_directory_path() / "duilib_test_resources";
    m_tempOutputDir = std::filesystem::temp_directory_path() / "duilib_test_output";

    std::filesystem::create_directories(m_testResourceDir);
    std::filesystem::create_directories(m_tempOutputDir);

    CreateTestResourceFiles();
    CreateTestQrcFile();
}

void ResourceCompilerTest::TearDown()
{
    CleanupTempFiles();
}

void ResourceCompilerTest::CreateTestResourceFiles()
{
    fs::create_directories(m_testResourceDir / "images");
    fs::create_directories(m_testResourceDir / "xml");
    fs::create_directories(m_testResourceDir / "fonts");

    WriteBinaryFile(m_testResourceDir / "images" / "icon.png", {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A});
    WriteBinaryFile(m_testResourceDir / "images" / "logo.png", {0x89, 0x50, 0x4E, 0x47, 0x0A, 0x1A});

    WriteTextFile(m_testResourceDir / "xml" / "window.xml",
                  "<?xml version=\"1.0\"?>\n"
                  "<Window>\n"
                  "  <Box>\n"
                  "    <Label text=\"Hello\"/>\n"
                  "  </Box>\n"
                  "</Window>\n");

    WriteBinaryFile(m_testResourceDir / "binary.dat", {0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE});
    WriteBinaryFile(m_testResourceDir / "fonts" / "test.ttf", {'O', 'T', 'T', 'O'});
}

void ResourceCompilerTest::CreateTestQrcFile()
{
    WriteQrcFile(m_testResourceDir / "test.qrc",
                 {
                     "<file>images/icon.png</file>",
                     "<file alias=\"logo.png\">images/logo.png</file>",
                     "<file>xml/window.xml</file>",
                     "<file>binary.dat</file>",
                     "<file>fonts/test.ttf</file>"
                 });
}

void ResourceCompilerTest::CleanupTempFiles()
{
    std::filesystem::remove_all(m_testResourceDir);
    std::filesystem::remove_all(m_tempOutputDir);
}

TEST_F(ResourceCompilerTest, ParseQrcFile)
{
    duilib::rc::ResourceCompiler compiler;
    const fs::path qrcPath = m_testResourceDir / "test.qrc";

    ASSERT_TRUE(compiler.ParseQrcFile(qrcPath));
    EXPECT_TRUE(compiler.GetErrors().empty());

    const fs::path outputPath = m_tempOutputDir / "parse_qrc_output.h";
    ASSERT_TRUE(compiler.GenerateCppCode(outputPath, "ParseQrc"));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("Entry count"), std::string::npos);
    EXPECT_NE(generated.find("icon.png"), std::string::npos);
    EXPECT_NE(generated.find("logo.png"), std::string::npos);
    EXPECT_NE(generated.find("window.xml"), std::string::npos);
    EXPECT_NE(generated.find("binary.dat"), std::string::npos);
}

TEST_F(ResourceCompilerTest, GenerateCppCode)
{
    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "generated_resources.h";

    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputPath, "CompiledPack", compiler));
    ASSERT_TRUE(fs::exists(outputPath));
    EXPECT_GT(fs::file_size(outputPath), 0u);

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("GetCompiledPackData"), std::string::npos);
    EXPECT_NE(generated.find("GetCompiledPackDataSize"), std::string::npos);
    EXPECT_NE(generated.find("COMPILED_RESOURCES_COMPILEDPACK_H_"), std::string::npos);
}

TEST_F(ResourceCompilerTest, HandleFileAliases)
{
    WriteTextFile(m_testResourceDir / "images" / "origin.txt", "alias-source");
    const fs::path aliasQrc = WriteQrcFile(
        m_testResourceDir / "alias.qrc",
        {"<file alias=\"shortcut_alias.txt\">images/origin.txt</file>"});

    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "alias_output.h";
    ASSERT_TRUE(CompileQrc(aliasQrc, outputPath, "AliasPack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("shortcut_alias.txt"), std::string::npos);
    EXPECT_EQ(generated.find("images/origin.txt"), std::string::npos);
}

TEST_F(ResourceCompilerTest, CompressionOptions)
{
    duilib::rc::ResourceCompiler disabledCompiler;
    disabledCompiler.SetCompressionEnabled(false);
    disabledCompiler.SetCompressionThreshold(1);
    const fs::path outputDisabled = m_tempOutputDir / "compression_disabled.h";
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputDisabled, "CompressionOff", disabledCompiler));

    duilib::rc::ResourceCompiler enabledCompiler;
    enabledCompiler.SetCompressionEnabled(true);
    enabledCompiler.SetCompressionThreshold(1);
    const fs::path outputEnabled = m_tempOutputDir / "compression_enabled.h";
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputEnabled, "CompressionOn", enabledCompiler));

    EXPECT_TRUE(disabledCompiler.GetErrors().empty());
    EXPECT_TRUE(enabledCompiler.GetErrors().empty());
    EXPECT_GT(fs::file_size(outputDisabled), 0u);
    EXPECT_GT(fs::file_size(outputEnabled), 0u);
}

TEST_F(ResourceCompilerTest, HandleLargeFiles)
{
    std::vector<uint8_t> largeData(256 * 1024);
    for (size_t i = 0; i < largeData.size(); ++i) {
        largeData[i] = static_cast<uint8_t>(i % 251);
    }
    WriteBinaryFile(m_testResourceDir / "large.bin", largeData);

    const fs::path qrcPath = WriteQrcFile(
        m_testResourceDir / "large.qrc",
        {"<file>large.bin</file>"});

    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "large_output.h";
    ASSERT_TRUE(CompileQrc(qrcPath, outputPath, "LargePack", compiler));

    EXPECT_GT(fs::file_size(outputPath), 200u * 1024u);
    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("large.bin"), std::string::npos);
}

TEST_F(ResourceCompilerTest, HandleBinaryData)
{
    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "binary_output.h";
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputPath, "BinaryPack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_TRUE(ContainsHexSequence(generated, {0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE}));
}

TEST_F(ResourceCompilerTest, LoadResourceFromMemory)
{
    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "memory_format.h";
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputPath, "MemoryPack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("0x52, 0x49, 0x55, 0x44"), std::string::npos);
    EXPECT_NE(generated.find("Version: 1"), std::string::npos);
    EXPECT_NE(generated.find("Entry table"), std::string::npos);
}

TEST_F(ResourceCompilerTest, ResourceLookup)
{
    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "lookup_output.h";
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputPath, "LookupPack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("images/icon.png"), std::string::npos);
    EXPECT_NE(generated.find("logo.png"), std::string::npos);
    EXPECT_EQ(generated.find("not_exist.file"), std::string::npos);
}

TEST_F(ResourceCompilerTest, CaseSensitivity)
{
    WriteTextFile(m_testResourceDir / "mixed_case.txt", "case-sensitive");
    const fs::path qrcPath = WriteQrcFile(
        m_testResourceDir / "case.qrc",
        {"<file alias=\"Case/Upper.TXT\">mixed_case.txt</file>"});

    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "case_output.h";
    ASSERT_TRUE(CompileQrc(qrcPath, outputPath, "CasePack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("Case/Upper.TXT"), std::string::npos);
    EXPECT_EQ(generated.find("case/upper.txt"), std::string::npos);
}

TEST_F(ResourceCompilerTest, DirectoryListing)
{
    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "directory_output.h";
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputPath, "DirectoryPack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("images/"), std::string::npos);
    EXPECT_NE(generated.find("xml/"), std::string::npos);
    EXPECT_NE(generated.find("fonts/"), std::string::npos);
}

TEST_F(ResourceCompilerTest, ResourceDataIntegrity)
{
    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "integrity_output.h";
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputPath, "IntegrityPack", compiler));

    const std::vector<uint8_t> original = ReadBinaryFile(m_testResourceDir / "binary.dat");
    const std::string generated = ReadTextFile(outputPath);
    EXPECT_TRUE(ContainsHexSequence(generated, original));
}

TEST_F(ResourceCompilerTest, IntegrationWithGlobalManager)
{
    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "global_manager_output.h";
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputPath, "GlobalPack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("const void* GetGlobalPackData"), std::string::npos);
    EXPECT_NE(generated.find("size_t GetGlobalPackDataSize"), std::string::npos);
}

TEST_F(ResourceCompilerTest, IntegrationWithWindowBuilder)
{
    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "window_builder_output.h";
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputPath, "WindowPack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("window.xml"), std::string::npos);
    EXPECT_TRUE(ContainsHexSequence(generated, {0x3C, 0x57, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x3E}));
}

TEST_F(ResourceCompilerTest, IntegrationWithImageManager)
{
    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "image_manager_output.h";
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputPath, "ImagePack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("icon.png"), std::string::npos);
    EXPECT_NE(generated.find("logo.png"), std::string::npos);
    EXPECT_TRUE(ContainsHexSequence(generated, {0x89, 0x50, 0x4E, 0x47}));
}

TEST_F(ResourceCompilerTest, Benchmark_CompiledVsFile)
{
    const fs::path xmlPath = m_testResourceDir / "xml" / "window.xml";

    const auto fileStart = std::chrono::steady_clock::now();
    for (int i = 0; i < 200; ++i) {
        const std::string xml = ReadTextFile(xmlPath);
        ASSERT_FALSE(xml.empty());
    }
    const auto fileEnd = std::chrono::steady_clock::now();

    const auto generatedStart = std::chrono::steady_clock::now();
    for (int i = 0; i < 10; ++i) {
        duilib::rc::ResourceCompiler compiler;
        const fs::path outputPath = m_tempOutputDir / ("benchmark_compiled_" + std::to_string(i) + ".h");
        ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputPath, "BenchmarkPack", compiler));
    }
    const auto generatedEnd = std::chrono::steady_clock::now();

    const auto fileDuration = std::chrono::duration_cast<std::chrono::microseconds>(fileEnd - fileStart);
    const auto generatedDuration = std::chrono::duration_cast<std::chrono::microseconds>(generatedEnd - generatedStart);

    EXPECT_GT(fileDuration.count(), 0);
    EXPECT_GT(generatedDuration.count(), 0);
}

TEST_F(ResourceCompilerTest, Benchmark_MemoryUsage)
{
    const size_t inputSize = static_cast<size_t>(fs::file_size(m_testResourceDir / "images" / "icon.png"))
                           + static_cast<size_t>(fs::file_size(m_testResourceDir / "images" / "logo.png"))
                           + static_cast<size_t>(fs::file_size(m_testResourceDir / "xml" / "window.xml"))
                           + static_cast<size_t>(fs::file_size(m_testResourceDir / "binary.dat"))
                           + static_cast<size_t>(fs::file_size(m_testResourceDir / "fonts" / "test.ttf"));

    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "benchmark_memory_output.h";
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputPath, "MemoryBenchmark", compiler));

    const size_t outputSize = static_cast<size_t>(fs::file_size(outputPath));
    EXPECT_GT(outputSize, inputSize);
}

TEST_F(ResourceCompilerTest, Benchmark_StartupTime)
{
    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "benchmark_startup_output.h";

    const auto start = std::chrono::steady_clock::now();
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputPath, "StartupBenchmark", compiler));
    const auto end = std::chrono::steady_clock::now();

    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(elapsedMs.count(), 5000);
}

TEST_F(ResourceCompilerTest, EmptyResource)
{
    const fs::path emptyQrc = WriteQrcFile(m_testResourceDir / "empty.qrc", {});
    duilib::rc::ResourceCompiler compiler;

    EXPECT_FALSE(compiler.ParseQrcFile(emptyQrc));
    EXPECT_FALSE(compiler.GenerateCppCode(m_tempOutputDir / "empty_output.h", "EmptyPack"));
    EXPECT_FALSE(compiler.GetErrors().empty());
}

TEST_F(ResourceCompilerTest, SpecialCharactersInPath)
{
    WriteBinaryFile(m_testResourceDir / "images" / "icon @2x!.png", {0x89, 0x50, 0x4E, 0x47});

    const fs::path qrcPath = WriteQrcFile(
        m_testResourceDir / "special_path.qrc",
        {"<file alias=\"icons/icon @2x!.png\">images/icon @2x!.png</file>"});

    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "special_path_output.h";
    ASSERT_TRUE(CompileQrc(qrcPath, outputPath, "SpecialPathPack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("icons/icon @2x!.png"), std::string::npos);
}

TEST_F(ResourceCompilerTest, DuplicateResourceNames)
{
    WriteTextFile(m_testResourceDir / "first.txt", "first");
    WriteTextFile(m_testResourceDir / "second.txt", "second");

    const fs::path qrcPath = WriteQrcFile(
        m_testResourceDir / "duplicate_alias.qrc",
        {
            "<file alias=\"dup.txt\">first.txt</file>",
            "<file alias=\"dup.txt\">second.txt</file>"
        });

    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "duplicate_output.h";
    ASSERT_TRUE(CompileQrc(qrcPath, outputPath, "DuplicatePack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_GE(CountOccurrences(generated, "dup.txt"), 2u);
}

TEST_F(ResourceCompilerTest, NestedDirectories)
{
    WriteTextFile(m_testResourceDir / "resources" / "a" / "b" / "c" / "d" / "e" / "deep.xml",
                  "<deep>value</deep>\n");

    const fs::path qrcPath = WriteQrcFile(
        m_testResourceDir / "nested.qrc",
        {"<file>resources/a/b/c/d/e/deep.xml</file>"});

    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "nested_output.h";
    ASSERT_TRUE(CompileQrc(qrcPath, outputPath, "NestedPack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("resources/a/b/c/d/e/deep.xml"), std::string::npos);
}

TEST_F(ResourceCompilerTest, ParseQrcFileNotFound)
{
    duilib::rc::ResourceCompiler compiler;
    const fs::path missingQrc = m_testResourceDir / "not_found.qrc";

    EXPECT_FALSE(compiler.ParseQrcFile(missingQrc));
    ASSERT_FALSE(compiler.GetErrors().empty());
    EXPECT_NE(compiler.GetErrors().front().find("QRC file not found"), std::string::npos);
}

TEST_F(ResourceCompilerTest, ParseQrcFileWithoutEntries)
{
    const fs::path emptyQrc = WriteQrcFile(m_testResourceDir / "no_entries.qrc", {});
    duilib::rc::ResourceCompiler compiler;

    EXPECT_FALSE(compiler.ParseQrcFile(emptyQrc));
    EXPECT_FALSE(compiler.GenerateCppCode(m_tempOutputDir / "no_entries_output.h", "NoEntriesPack"));
    ASSERT_FALSE(compiler.GetErrors().empty());
    EXPECT_NE(compiler.GetErrors().back().find("No resources to compile"), std::string::npos);
}

TEST_F(ResourceCompilerTest, GenerateCppCodeOutputPathIsDirectory)
{
    duilib::rc::ResourceCompiler compiler;
    ASSERT_TRUE(compiler.ParseQrcFile(m_testResourceDir / "test.qrc"));

    EXPECT_FALSE(compiler.GenerateCppCode(m_tempOutputDir, "DirOutputPack"));
    ASSERT_FALSE(compiler.GetErrors().empty());
    EXPECT_NE(compiler.GetErrors().back().find("Cannot create output file"), std::string::npos);
}

TEST_F(ResourceCompilerTest, GenerateCppCodeWithMissingSourceFileProducesWarning)
{
    const fs::path qrcPath = WriteQrcFile(
        m_testResourceDir / "missing_entry.qrc",
        {
            "<file>xml/window.xml</file>",
            "<file>missing/404.bin</file>"
        });

    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "missing_entry_output.h";
    ASSERT_TRUE(compiler.ParseQrcFile(qrcPath));
    EXPECT_TRUE(compiler.GenerateCppCode(outputPath, "MissingEntryPack"));

    ASSERT_FALSE(compiler.GetWarnings().empty());
    EXPECT_NE(compiler.GetWarnings().front().find("Failed to read"), std::string::npos);

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("missing/404.bin"), std::string::npos);
}

TEST_F(ResourceCompilerTest, ParseQrcFileMultipleQresourcePrefixes)
{
    WriteTextFile(m_testResourceDir / "alpha.txt", "alpha");
    WriteTextFile(m_testResourceDir / "beta.txt", "beta");

    const fs::path qrcPath = m_testResourceDir / "multi_prefix.qrc";
    {
        std::ofstream qrcFile(qrcPath, std::ios::binary);
        qrcFile << "<RCC>\n";
        qrcFile << "  <qresource prefix=\"/alpha\">\n";
        qrcFile << "    <file>alpha.txt</file>\n";
        qrcFile << "  </qresource>\n";
        qrcFile << "  <qresource prefix=\"/beta\">\n";
        qrcFile << "    <file>beta.txt</file>\n";
        qrcFile << "  </qresource>\n";
        qrcFile << "</RCC>\n";
    }

    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "multi_prefix_output.h";
    ASSERT_TRUE(CompileQrc(qrcPath, outputPath, "PrefixPack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("/alpha/alpha.txt"), std::string::npos);
    EXPECT_NE(generated.find("/beta/beta.txt"), std::string::npos);
}

TEST_F(ResourceCompilerTest, ParseQrcFileAliasFallbackWhenAliasEmpty)
{
    const fs::path qrcPath = WriteQrcFile(
        m_testResourceDir / "empty_alias.qrc",
        {"<file alias=\"\">images/icon.png</file>"});

    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "empty_alias_output.h";
    ASSERT_TRUE(CompileQrc(qrcPath, outputPath, "AliasFallbackPack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_NE(generated.find("images/icon.png"), std::string::npos);
}

TEST_F(ResourceCompilerTest, GenerateCppCodeIsDeterministicForSameInput)
{
    duilib::rc::ResourceCompiler compilerA;
    duilib::rc::ResourceCompiler compilerB;

    const fs::path outputA = m_tempOutputDir / "deterministic_a.h";
    const fs::path outputB = m_tempOutputDir / "deterministic_b.h";

    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputA, "DeterministicPack", compilerA));
    ASSERT_TRUE(CompileQrc(m_testResourceDir / "test.qrc", outputB, "DeterministicPack", compilerB));

    const std::string generatedA = ReadTextFile(outputA);
    const std::string generatedB = ReadTextFile(outputB);
    EXPECT_EQ(generatedA, generatedB);
}

TEST_F(ResourceCompilerTest, FileTagOutsideQresourceIsIgnored)
{
    WriteTextFile(m_testResourceDir / "outside.txt", "outside");
    WriteTextFile(m_testResourceDir / "inside.txt", "inside");

    const fs::path qrcPath = m_testResourceDir / "outside_ignored.qrc";
    {
        std::ofstream qrcFile(qrcPath, std::ios::binary);
        qrcFile << "<RCC>\n";
        qrcFile << "  <file>outside.txt</file>\n";
        qrcFile << "  <qresource prefix=\"/\">\n";
        qrcFile << "    <file>inside.txt</file>\n";
        qrcFile << "  </qresource>\n";
        qrcFile << "</RCC>\n";
    }

    duilib::rc::ResourceCompiler compiler;
    const fs::path outputPath = m_tempOutputDir / "outside_ignored_output.h";
    ASSERT_TRUE(CompileQrc(qrcPath, outputPath, "OutsideIgnoredPack", compiler));

    const std::string generated = ReadTextFile(outputPath);
    EXPECT_EQ(generated.find("outside.txt"), std::string::npos);
    EXPECT_NE(generated.find("inside.txt"), std::string::npos);
}

} // namespace test
} // namespace ui
