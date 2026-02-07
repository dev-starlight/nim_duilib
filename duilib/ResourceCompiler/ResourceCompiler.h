#ifndef DUILIB_RESOURCE_COMPILER_COMPILER_H_
#define DUILIB_RESOURCE_COMPILER_COMPILER_H_

#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace duilib {
namespace rc {

struct ResourceEntry {
    std::string filePath;
    std::string alias;
    std::string resourcePath;
    bool compress;
};

struct QrcFile {
    std::string prefix;
    std::string baseDir;
    std::vector<ResourceEntry> entries;
};

class ResourceCompiler {
public:
    ResourceCompiler();
    ~ResourceCompiler();

    bool ParseQrcFile(const std::filesystem::path& qrcPath);
    
    bool GenerateCppCode(const std::filesystem::path& outputPath, 
                          const std::string& resourceName);
    
    void SetCompressionThreshold(size_t threshold);
    void SetCompressionEnabled(bool enabled);
    
    const std::vector<std::string>& GetErrors() const;
    const std::vector<std::string>& GetWarnings() const;

private:
    bool ReadFileContent(const std::filesystem::path& path, std::vector<uint8_t>& content);
    
    std::string GenerateHeaderGuard(const std::string& resourceName);
    
    std::string EscapeString(const std::string& input);
    
    void ReportError(const std::string& message);
    void ReportWarning(const std::string& message);

private:
    QrcFile m_qrc;
    std::vector<std::string> m_errors;
    std::vector<std::string> m_warnings;
    
    size_t m_compressionThreshold;
    bool m_compressionEnabled;
};

}
}

#endif
