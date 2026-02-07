#ifndef UI_CORE_COMPILED_RESOURCE_LOADER_H_
#define UI_CORE_COMPILED_RESOURCE_LOADER_H_

#pragma once

#include "duilib/Utils/FilePath.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ui
{

class UILIB_API CompiledResourceLoader
{
public:
    CompiledResourceLoader();
    ~CompiledResourceLoader();
    CompiledResourceLoader(const CompiledResourceLoader&) = delete;
    CompiledResourceLoader& operator=(const CompiledResourceLoader&) = delete;

public:
    bool Initialize(const void* compiledData, size_t dataSize);

    bool Exists(const DString& path) const;

    bool GetResourceData(const DString& path, const uint8_t*& data, size_t& size) const;
    bool GetResourceData(const DString& path, std::vector<uint8_t>& output) const;

    DString GetResourceString(const DString& path) const;

    std::vector<DString> ListDirectory(const DString& dirPath) const;
    std::vector<DString> GetAllResourcePaths() const;

    void SetCaseSensitive(bool sensitive);
    bool IsCaseSensitive() const;

private:
    DString NormalizePath(const DString& path) const;
    void BuildIndex();

private:
    struct ResourceData {
        const uint8_t* data = nullptr;
        size_t size = 0;
        uint32_t flags = 0;
    };

    std::unordered_map<DString, ResourceData> m_resourceIndex;
    const uint8_t* m_compiledData;
    size_t m_dataSize;
    bool m_caseSensitive;
    bool m_initialized;
};

} // namespace ui

#endif // UI_CORE_COMPILED_RESOURCE_LOADER_H_
