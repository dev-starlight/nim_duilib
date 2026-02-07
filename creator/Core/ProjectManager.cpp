#include "ProjectManager.h"
#include "duilib/Utils/FilePath.h"
#include <algorithm>
#include <filesystem>

namespace creator {

ProjectManager::ProjectManager()
{
}

ProjectManager::~ProjectManager()
{
}

bool ProjectManager::SetProjectRoot(const DString& rootPath)
{
    if (rootPath.empty()) {
        return false;
    }

    // 验证路径是否存在
    ui::FilePath fp(rootPath);
    if (!fp.IsExistsPath()) {
        return false;
    }

    m_projectRoot = rootPath;
    return true;
}

DString ProjectManager::GetResourcePath() const
{
    if (m_projectRoot.empty()) {
        return _T("");
    }
    return m_projectRoot + _T("/resources");
}

DString ProjectManager::GetThemePath() const
{
    if (m_projectRoot.empty()) {
        return _T("");
    }
    return m_projectRoot + _T("/resources/themes/default");
}

std::vector<ResourceFileInfo> ProjectManager::ScanResources() const
{
    std::vector<ResourceFileInfo> results;
    if (m_projectRoot.empty()) {
        return results;
    }

    DString resourcePath = GetResourcePath();
    ScanDirectory(resourcePath, resourcePath, results);
    return results;
}

void ProjectManager::AddRecentFile(const DString& filePath)
{
    // 移除已存在的相同路径
    auto it = std::find(m_recentFiles.begin(), m_recentFiles.end(), filePath);
    if (it != m_recentFiles.end()) {
        m_recentFiles.erase(it);
    }

    // 添加到头部
    m_recentFiles.insert(m_recentFiles.begin(), filePath);

    // 限制数量
    if (m_recentFiles.size() > kMaxRecentFiles) {
        m_recentFiles.resize(kMaxRecentFiles);
    }
}

void ProjectManager::ClearRecentFiles()
{
    m_recentFiles.clear();
}

std::string ProjectManager::GetSkinFolderName() const
{
    if (m_projectRoot.empty()) {
        return "my_app";
    }

    // 取项目根目录最后一级目录名
    ui::FilePath fp(m_projectRoot);
    DString name = fp.GetFileName();
    if (name.empty()) {
        return "my_app";
    }

#ifdef DUILIB_UNICODE
    // 转换为 std::string
    std::wstring wname(name);
    return std::string(wname.begin(), wname.end());
#else
    return name;
#endif
}

void ProjectManager::ScanDirectory(const DString& dirPath, const DString& basePath,
                                    std::vector<ResourceFileInfo>& results) const
{
    try {
#ifdef DUILIB_UNICODE
        std::filesystem::path fsPath(dirPath);
#else
        std::filesystem::path fsPath(dirPath);
#endif
        if (!std::filesystem::exists(fsPath) || !std::filesystem::is_directory(fsPath)) {
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(fsPath)) {
            if (entry.is_directory()) {
#ifdef DUILIB_UNICODE
                ScanDirectory(entry.path().wstring(), basePath, results);
#else
                ScanDirectory(entry.path().string(), basePath, results);
#endif
            }
            else if (entry.is_regular_file()) {
                ResourceFileInfo info;
#ifdef DUILIB_UNICODE
                info.filePath = entry.path().wstring();
                info.fileName = entry.path().filename().wstring();

                // 计算相对路径
                std::filesystem::path relPath = std::filesystem::relative(entry.path(), std::filesystem::path(basePath));
                info.relativePath = relPath.wstring();
#else
                info.filePath = entry.path().string();
                info.fileName = entry.path().filename().string();

                std::filesystem::path relPath = std::filesystem::relative(entry.path(), std::filesystem::path(basePath));
                info.relativePath = relPath.string();
#endif
                info.isXml = IsXmlExtension(info.fileName);
                info.isImage = IsImageExtension(info.fileName);
                results.push_back(info);
            }
        }
    }
    catch (const std::filesystem::filesystem_error&) {
        // 忽略文件系统异常
    }
}

bool ProjectManager::IsXmlExtension(const DString& fileName)
{
    if (fileName.size() < 4) {
        return false;
    }
    DString lower = fileName;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](auto c) { return static_cast<decltype(c)>(std::tolower(static_cast<int>(c))); });

    return lower.size() >= 4 &&
           lower.substr(lower.size() - 4) == _T(".xml");
}

bool ProjectManager::IsImageExtension(const DString& fileName)
{
    if (fileName.size() < 4) {
        return false;
    }
    DString lower = fileName;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](auto c) { return static_cast<decltype(c)>(std::tolower(static_cast<int>(c))); });

    if (lower.size() >= 4) {
        DString ext4 = lower.substr(lower.size() - 4);
        if (ext4 == _T(".png") || ext4 == _T(".jpg") || ext4 == _T(".bmp") || ext4 == _T(".gif") || ext4 == _T(".svg")) {
            return true;
        }
    }
    if (lower.size() >= 5) {
        DString ext5 = lower.substr(lower.size() - 5);
        if (ext5 == _T(".jpeg") || ext5 == _T(".webp")) {
            return true;
        }
    }
    return false;
}

} // namespace creator
