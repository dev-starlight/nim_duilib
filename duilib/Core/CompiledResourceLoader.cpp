#include "duilib/Core/CompiledResourceLoader.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"
#include <cstring>
#include <algorithm>

namespace ui 
{

CompiledResourceLoader::CompiledResourceLoader()
    : m_compiledData(nullptr)
    , m_dataSize(0)
    , m_caseSensitive(false)
    , m_initialized(false)
{
#if defined(DUILIB_BUILD_FOR_LINUX) || defined(DUILIB_BUILD_FOR_MACOS)
    m_caseSensitive = true;
#endif
}

CompiledResourceLoader::~CompiledResourceLoader() = default;

bool CompiledResourceLoader::Initialize(const void* compiledData, size_t dataSize)
{
    if (!compiledData || dataSize == 0) {
        return false;
    }

    m_compiledData = static_cast<const uint8_t*>(compiledData);
    m_dataSize = dataSize;
    m_initialized = true;

    BuildIndex();
    return true;
}

bool CompiledResourceLoader::Exists(const DString& path) const
{
    if (!m_initialized) {
        return false;
    }

    DString normalizedPath = NormalizePath(path);
    if (normalizedPath.empty()) {
        return false;
    }

    if (m_caseSensitive) {
        return m_resourceIndex.find(normalizedPath) != m_resourceIndex.end();
    } else {
        for (const auto& entry : m_resourceIndex) {
            if (_tcsicmp(entry.first.c_str(), normalizedPath.c_str()) == 0) {
                return true;
            }
        }
        return false;
    }
}

bool CompiledResourceLoader::GetResourceData(const DString& path, 
                                              const uint8_t*& data, 
                                              size_t& size) const
{
    if (!m_initialized) {
        return false;
    }

    DString normalizedPath = NormalizePath(path);
    if (normalizedPath.empty()) {
        return false;
    }

    auto it = m_resourceIndex.find(normalizedPath);
    if (it != m_resourceIndex.end()) {
        data = it->second.data;
        size = it->second.size;
        return true;
    }

    if (!m_caseSensitive) {
        for (const auto& entry : m_resourceIndex) {
            if (_tcsicmp(entry.first.c_str(), normalizedPath.c_str()) == 0) {
                data = entry.second.data;
                size = entry.second.size;
                return true;
            }
        }
    }

    return false;
}

bool CompiledResourceLoader::GetResourceData(const DString& path, 
                                              std::vector<uint8_t>& output) const
{
    const uint8_t* data = nullptr;
    size_t size = 0;

    if (!GetResourceData(path, data, size)) {
        return false;
    }

    output.resize(size);
    if (size > 0) {
        std::memcpy(output.data(), data, size);
    }
    return true;
}

DString CompiledResourceLoader::GetResourceString(const DString& path) const
{
    const uint8_t* data = nullptr;
    size_t size = 0;

    if (!GetResourceData(path, data, size)) {
        return DString();
    }

    return StringConvert::UTF8ToT(reinterpret_cast<const DUTF8Char*>(data), size);
}

std::vector<DString> CompiledResourceLoader::ListDirectory(const DString& dirPath) const
{
    std::vector<DString> result;

    if (!m_initialized) {
        return result;
    }

    DString normalizedDir = NormalizePath(dirPath);
    if (!normalizedDir.empty() && normalizedDir.back() != _T('/')) {
        normalizedDir += _T("/");
    }

    for (const auto& entry : m_resourceIndex) {
        const DString& resourcePath = entry.first;
        
        if (normalizedDir.empty()) {
            size_t slashPos = resourcePath.find(_T('/'));
            if (slashPos == DString::npos) {
                if (std::find(result.begin(), result.end(), resourcePath) == result.end()) {
                    result.push_back(resourcePath);
                }
            } else {
                DString dirName = resourcePath.substr(0, slashPos);
                if (std::find(result.begin(), result.end(), dirName) == result.end()) {
                    result.push_back(dirName);
                }
            }
        } else {
            if (resourcePath.find(normalizedDir) == 0) {
                DString remaining = resourcePath.substr(normalizedDir.length());
                size_t slashPos = remaining.find(_T('/'));
                
                DString itemName = (slashPos == DString::npos) ? remaining : remaining.substr(0, slashPos);

                if (!itemName.empty() && 
                    std::find(result.begin(), result.end(), itemName) == result.end()) {
                    result.push_back(itemName);
                }
            }
        }
    }

    return result;
}

std::vector<DString> CompiledResourceLoader::GetAllResourcePaths() const
{
    std::vector<DString> result;
    result.reserve(m_resourceIndex.size());

    for (const auto& entry : m_resourceIndex) {
        result.push_back(entry.first);
    }

    return result;
}

void CompiledResourceLoader::SetCaseSensitive(bool sensitive)
{
    if (m_caseSensitive == sensitive) {
        return;
    }
    m_caseSensitive = sensitive;
    if (m_initialized) {
        BuildIndex();
    }
}

bool CompiledResourceLoader::IsCaseSensitive() const
{
    return m_caseSensitive;
}

DString CompiledResourceLoader::NormalizePath(const DString& path) const
{
    DString result = path;

    if (result.length() >= 2 && result[0] == _T(':') && result[1] == _T('/')) {
        result = result.substr(2);
    }

    for (auto& ch : result) {
        if (ch == _T('\\')) {
            ch = _T('/');
        }
    }

    while (!result.empty() && result[0] == _T('/')) {
        result = result.substr(1);
    }

    if (!m_caseSensitive) {
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    }

    return result;
}

void CompiledResourceLoader::BuildIndex()
{
    m_resourceIndex.clear();

    if (!m_compiledData || m_dataSize < sizeof(uint32_t)) {
        return;
    }

    const uint8_t* ptr = m_compiledData;
    const uint8_t* end = m_compiledData + m_dataSize;

    uint32_t magic = *reinterpret_cast<const uint32_t*>(ptr);
    ptr += sizeof(uint32_t);

    if (magic != 0x44554952) {
        return;
    }

    if (static_cast<size_t>(end - ptr) < sizeof(uint32_t) * 2) {
        return;
    }

    uint32_t version = *reinterpret_cast<const uint32_t*>(ptr);
    ptr += sizeof(uint32_t);

    uint32_t entryCount = *reinterpret_cast<const uint32_t*>(ptr);
    ptr += sizeof(uint32_t);

    for (uint32_t i = 0; i < entryCount && ptr < end; ++i) {
        if (static_cast<size_t>(end - ptr) < sizeof(uint32_t) * 3) {
            break;
        }

        uint32_t pathLen = *reinterpret_cast<const uint32_t*>(ptr);
        ptr += sizeof(uint32_t);

        uint32_t dataOffset = *reinterpret_cast<const uint32_t*>(ptr);
        ptr += sizeof(uint32_t);

        uint32_t dataSize = *reinterpret_cast<const uint32_t*>(ptr);
        ptr += sizeof(uint32_t);

        if (static_cast<size_t>(end - ptr) < pathLen) {
            break;
        }

        DString path = StringConvert::UTF8ToT(reinterpret_cast<const DUTF8Char*>(ptr), pathLen);
        ptr += pathLen;

        if (dataOffset + dataSize <= m_dataSize) {
            ResourceData resData;
            resData.data = m_compiledData + dataOffset;
            resData.size = dataSize;
            resData.flags = 0;

            DString normalizedPath = NormalizePath(path);
            if (!normalizedPath.empty()) {
                m_resourceIndex[normalizedPath] = resData;
            }
        }
    }
}

} // namespace ui
