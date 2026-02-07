#include "duilib/Core/XmlBinaryCache.h"
#include "duilib/Utils/FilePathUtil.h"
#include <fstream>
#include <cstring>
#include <filesystem>
#include <chrono>

// Forward declare pugixml types
namespace pugi {
class xml_document;
class xml_node;
class xml_attribute;
}

namespace ui {

static const uint32_t kXmlBinaryMagic = 0x584D4244; // 'XMBD'
static const uint32_t kXmlBinaryVersion = 1;
static const uint32_t kFlagCompressed = 0x01;

static std::filesystem::path ToFsPath(const FilePath& path)
{
#ifdef DUILIB_BUILD_FOR_WIN
    return std::filesystem::path(path.ToStringW());
#else
    return std::filesystem::path(path.ToStringA());
#endif
}

static int64_t GetLastWriteTimeValue(const FilePath& path)
{
    std::error_code ec;
    const std::filesystem::file_time_type ftime = std::filesystem::last_write_time(ToFsPath(path), ec);
    if (ec) {
        return 0;
    }
    return std::chrono::duration_cast<std::chrono::nanoseconds>(ftime.time_since_epoch()).count();
}

XmlBinaryCache::XmlBinaryCache()
    : m_compressionEnabled(false)
{
}

XmlBinaryCache::~XmlBinaryCache() = default;

FilePath XmlBinaryCache::GetCacheFilePath(const FilePath& xmlPath) {
    DString path = xmlPath.ToString();
    size_t dotPos = path.rfind(_T('.'));
    if (dotPos != DString::npos) {
        path = path.substr(0, dotPos);
    }
    path += _T(".xmc"); // Xml Memory Cache
    return FilePath(path);
}

void XmlBinaryCache::SetCacheDirectory(const FilePath& cacheDir) {
    m_cacheDirectory = cacheDir;
}

void XmlBinaryCache::SetCompressionEnabled(bool enabled) {
    m_compressionEnabled = enabled;
}

bool XmlBinaryCache::IsCacheValid(const FilePath& xmlPath) const {
    FilePath cachePath = GetCacheFilePath(xmlPath);
    
    if (!m_cacheDirectory.IsEmpty()) {
        DString cacheFileName = cachePath.GetFileName();
        cachePath = FilePathUtil::JoinFilePath(m_cacheDirectory, FilePath(cacheFileName));
    }
    
    if (!cachePath.IsExistsFile()) {
        return false;
    }
    
    // Check if XML file is newer than cache
    const int64_t xmlTime = GetLastWriteTimeValue(xmlPath);
    const int64_t cacheTime = GetLastWriteTimeValue(cachePath);
    
    if (xmlTime > cacheTime) {
        return false;
    }
    
    // Verify cache file header
    std::ifstream file(ToFsPath(cachePath), std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    BinaryHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    
    if (header.magic != kXmlBinaryMagic) {
        return false;
    }
    
    if (header.version != kXmlBinaryVersion) {
        return false;
    }
    
    return true;
}

bool XmlBinaryCache::SaveToCache(const FilePath& xmlPath, const pugi::xml_document& doc) {
    FilePath cachePath = GetCacheFilePath(xmlPath);
    
    if (!m_cacheDirectory.IsEmpty()) {
        DString cacheFileName = cachePath.GetFileName();
        cachePath = FilePathUtil::JoinFilePath(m_cacheDirectory, FilePath(cacheFileName));
    }
    
    std::vector<uint8_t> binaryData;
    if (!SerializeDocument(doc, binaryData)) {
        return false;
    }
    
    std::ofstream file(ToFsPath(cachePath), std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());
    return file.good();
}

bool XmlBinaryCache::LoadFromCache(const FilePath& xmlPath, pugi::xml_document& doc) {
    if (!IsCacheValid(xmlPath)) {
        return false;
    }
    
    FilePath cachePath = GetCacheFilePath(xmlPath);
    
    if (!m_cacheDirectory.IsEmpty()) {
        DString cacheFileName = cachePath.GetFileName();
        cachePath = FilePathUtil::JoinFilePath(m_cacheDirectory, FilePath(cacheFileName));
    }
    
    std::ifstream file(ToFsPath(cachePath), std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    if (!file.good()) {
        return false;
    }
    
    return DeserializeDocument(data, doc);
}

bool XmlBinaryCache::ClearCache(const FilePath& xmlPath) {
    FilePath cachePath = GetCacheFilePath(xmlPath);
    
    if (!m_cacheDirectory.IsEmpty()) {
        DString cacheFileName = cachePath.GetFileName();
        cachePath = FilePathUtil::JoinFilePath(m_cacheDirectory, FilePath(cacheFileName));
    }
    
    if (cachePath.IsExistsFile()) {
        std::error_code ec;
        const bool removed = std::filesystem::remove(ToFsPath(cachePath), ec);
        return removed && !ec;
    }
    
    return true;
}

int XmlBinaryCache::ClearAllCache() {
    // Implementation depends on filesystem utilities
    // For now, return 0 as placeholder
    return 0;
}

bool XmlBinaryCache::SerializeDocument(const pugi::xml_document& doc, 
                                        std::vector<uint8_t>& output) {
    UNUSED_VARIABLE(doc);
    // Placeholder implementation
    // Full implementation would traverse the DOM tree and serialize to binary format
    
    BinaryHeader header;
    header.magic = kXmlBinaryMagic;
    header.version = kXmlBinaryVersion;
    header.flags = m_compressionEnabled ? kFlagCompressed : 0;
    header.nodeCount = 0;
    header.attrCount = 0;
    header.stringPoolOffset = 0;
    header.stringPoolSize = 0;
    header.reserved = 0;
    
    output.resize(sizeof(header));
    std::memcpy(output.data(), &header, sizeof(header));
    
    return true;
}

bool XmlBinaryCache::DeserializeDocument(const std::vector<uint8_t>& input,
                                          pugi::xml_document& doc) {
    UNUSED_VARIABLE(doc);
    if (input.size() < sizeof(BinaryHeader)) {
        return false;
    }
    
    const BinaryHeader* header = reinterpret_cast<const BinaryHeader*>(input.data());
    
    if (header->magic != kXmlBinaryMagic) {
        return false;
    }
    
    if (header->version != kXmlBinaryVersion) {
        return false;
    }
    
    // Placeholder implementation
    // Full implementation would reconstruct the DOM tree from binary format
    
    return true;
}

bool XmlBinaryCache::CompressData(const std::vector<uint8_t>& input,
                                   std::vector<uint8_t>& output) {
    // Placeholder: integrate with LZ4 or similar compression library
    output = input;
    return true;
}

bool XmlBinaryCache::DecompressData(const std::vector<uint8_t>& input,
                                     std::vector<uint8_t>& output,
                                     size_t originalSize) {
    UNUSED_VARIABLE(originalSize);
    // Placeholder: integrate with LZ4 or similar compression library
    output = input;
    return true;
}

uint32_t XmlBinaryCache::CalculateFileCRC32(const FilePath& path) const {
    UNUSED_VARIABLE(path);
    // CRC32 implementation for cache validation
    // Placeholder: return 0
    return 0;
}

} // namespace ui
