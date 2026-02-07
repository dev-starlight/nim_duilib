#include "duilib/ResourceCompiler/ResourceCompiler.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>

#ifdef DUILIB_BUILD_FOR_WIN
#include <windows.h>
#endif

namespace duilib {
namespace rc {

ResourceCompiler::ResourceCompiler()
    : m_compressionThreshold(1024)
    , m_compressionEnabled(true)
{
}

ResourceCompiler::~ResourceCompiler() = default;

bool ResourceCompiler::ParseQrcFile(const std::filesystem::path& qrcPath)
{
    m_qrc.entries.clear();
    m_errors.clear();
    m_warnings.clear();

    if (!std::filesystem::exists(qrcPath)) {
        ReportError("QRC file not found: " + qrcPath.string());
        return false;
    }

    m_qrc.baseDir = qrcPath.parent_path().string();

    std::ifstream file(qrcPath);
    if (!file.is_open()) {
        ReportError("Cannot open QRC file: " + qrcPath.string());
        return false;
    }

    std::string line;
    bool inResource = false;
    
    while (std::getline(file, line)) {
        size_t pos = 0;
        
        while ((pos = line.find('<', pos)) != std::string::npos) {
            if (line.find("<RCC", pos) == pos) {
                pos += 4;
            } else if (line.find("<qresource", pos) == pos) {
                size_t prefixPos = line.find("prefix=\"", pos);
                if (prefixPos != std::string::npos) {
                    prefixPos += 8;
                    size_t endPos = line.find('"', prefixPos);
                    if (endPos != std::string::npos) {
                        m_qrc.prefix = line.substr(prefixPos, endPos - prefixPos);
                    }
                }
                inResource = true;
                pos += 10;
            } else if (line.find("<file", pos) == pos && inResource) {
                ResourceEntry entry;
                
                size_t aliasPos = line.find("alias=\"", pos);
                if (aliasPos != std::string::npos) {
                    aliasPos += 7;
                    size_t endPos = line.find('"', aliasPos);
                    if (endPos != std::string::npos) {
                        entry.alias = line.substr(aliasPos, endPos - aliasPos);
                    }
                }
                
                size_t startPos = line.find('>', pos);
                if (startPos != std::string::npos) {
                    startPos += 1;
                    size_t endPos = line.find("</file>", startPos);
                    if (endPos != std::string::npos) {
                        entry.filePath = line.substr(startPos, endPos - startPos);
                    }
                }
                
                if (!entry.filePath.empty()) {
                    if (entry.alias.empty()) {
                        entry.alias = entry.filePath;
                    }
                    entry.resourcePath = m_qrc.prefix + "/" + entry.alias;
                    entry.compress = m_compressionEnabled;
                    m_qrc.entries.push_back(entry);
                }
                
                pos += 5;
            } else if (line.find("</qresource>", pos) == pos) {
                inResource = false;
                pos += 12;
            } else if (line.find("</RCC>", pos) == pos) {
                pos += 6;
            } else {
                pos += 1;
            }
        }
    }

    return !m_qrc.entries.empty();
}

bool ResourceCompiler::GenerateCppCode(const std::filesystem::path& outputPath, 
                                        const std::string& resourceName)
{
    if (m_qrc.entries.empty()) {
        ReportError("No resources to compile");
        return false;
    }

    std::ofstream out(outputPath);
    if (!out.is_open()) {
        ReportError("Cannot create output file: " + outputPath.string());
        return false;
    }

    std::string headerGuard = GenerateHeaderGuard(resourceName);

    out << "// Auto-generated resource file\n";
    out << "// Do not edit manually\n\n";
    out << "#ifndef " << headerGuard << "\n";
    out << "#define " << headerGuard << "\n\n";
    out << "#include <cstdint>\n\n";

    std::vector<std::vector<uint8_t>> fileContents;
    size_t totalDataSize = 0;

    for (const auto& entry : m_qrc.entries) {
        std::filesystem::path fullPath = std::filesystem::path(m_qrc.baseDir) / entry.filePath;
        
        std::vector<uint8_t> content;
        if (!ReadFileContent(fullPath, content)) {
            ReportWarning("Failed to read: " + entry.filePath);
            content.clear();
        }
        
        fileContents.push_back(content);
        totalDataSize += content.size();
    }

    size_t dataOffset = sizeof(uint32_t) * 3;
    for (const auto& entry : m_qrc.entries) {
        dataOffset += sizeof(uint32_t) * 3 + entry.resourcePath.length();
    }

    out << "namespace {\n\n";
    
    out << "const uint8_t k" << resourceName << "Data[] = {\n";
    
    out << "    // Header: magic, version, entry count\n";
    out << "    0x52, 0x49, 0x55, 0x44,  // Magic: 'DUIR'\n";
    out << "    0x01, 0x00, 0x00, 0x00,  // Version: 1\n";
    out << "    " << m_qrc.entries.size() << ", 0x00, 0x00, 0x00,  // Entry count\n";
    
    out << "\n    // Entry table\n";
    
    size_t currentDataOffset = dataOffset;
    
    for (size_t i = 0; i < m_qrc.entries.size(); ++i) {
        const auto& entry = m_qrc.entries[i];
        const auto& content = fileContents[i];
        
        out << "    // Entry " << i << ": " << entry.resourcePath << "\n";
        out << "    " << entry.resourcePath.length() << ", 0x00, 0x00, 0x00,  // Path length\n";
        out << "    " << currentDataOffset << ", 0x00, 0x00, 0x00,  // Data offset\n";
        out << "    " << content.size() << ", 0x00, 0x00, 0x00,  // Data size\n";
        
        out << "    ";
        for (char c : entry.resourcePath) {
            out << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                << (static_cast<unsigned char>(c) & 0xFF) << ", ";
        }
        out << std::dec << "  // \"" << EscapeString(entry.resourcePath) << "\"\n\n";
        
        currentDataOffset += content.size();
    }
    
    out << "    // Resource data\n";
    
    for (size_t i = 0; i < m_qrc.entries.size(); ++i) {
        const auto& entry = m_qrc.entries[i];
        const auto& content = fileContents[i];
        
        if (!content.empty()) {
            out << "    // " << entry.resourcePath << " (" << content.size() << " bytes)\n";
            out << "    ";
            
            for (size_t j = 0; j < content.size(); ++j) {
                out << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                    << (content[j] & 0xFF) << ", ";
                if ((j + 1) % 16 == 0 && j + 1 < content.size()) {
                    out << "\n    ";
                }
            }
            out << std::dec << "\n\n";
        }
    }
    
    out << "};\n\n";
    
    out << "} // namespace\n\n";
    
    out << "const void* Get" << resourceName << "Data() {\n";
    out << "    return k" << resourceName << "Data;\n";
    out << "}\n\n";
    
    out << "size_t Get" << resourceName << "DataSize() {\n";
    out << "    return sizeof(k" << resourceName << "Data);\n";
    out << "}\n\n";
    
    out << "#endif // " << headerGuard << "\n";

    return true;
}

void ResourceCompiler::SetCompressionThreshold(size_t threshold)
{
    m_compressionThreshold = threshold;
}

void ResourceCompiler::SetCompressionEnabled(bool enabled)
{
    m_compressionEnabled = enabled;
}

const std::vector<std::string>& ResourceCompiler::GetErrors() const
{
    return m_errors;
}

const std::vector<std::string>& ResourceCompiler::GetWarnings() const
{
    return m_warnings;
}

bool ResourceCompiler::ReadFileContent(const std::filesystem::path& path, 
                                        std::vector<uint8_t>& content)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    content.resize(size);
    file.read(reinterpret_cast<char*>(content.data()), size);
    
    return file.good();
}

std::string ResourceCompiler::GenerateHeaderGuard(const std::string& resourceName)
{
    std::string guard = "COMPILED_RESOURCES_" + resourceName + "_H_";
    std::transform(guard.begin(), guard.end(), guard.begin(), ::toupper);
    return guard;
}

std::string ResourceCompiler::EscapeString(const std::string& input)
{
    std::string result;
    for (char c : input) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (c >= 0x20 && c < 0x7F) {
                    result += c;
                } else {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\x%02x", static_cast<unsigned char>(c));
                    result += buf;
                }
        }
    }
    return result;
}

void ResourceCompiler::ReportError(const std::string& message)
{
    m_errors.push_back(message);
}

void ResourceCompiler::ReportWarning(const std::string& message)
{
    m_warnings.push_back(message);
}

}
}
