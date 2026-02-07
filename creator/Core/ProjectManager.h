#ifndef CREATOR_CORE_PROJECT_MANAGER_H_
#define CREATOR_CORE_PROJECT_MANAGER_H_

#include "duilib/duilib.h"
#include <string>
#include <vector>
#include <functional>

namespace creator {

/** 项目资源文件信息 */
struct ResourceFileInfo
{
    DString filePath;         // 完整路径
    DString relativePath;     // 相对项目根目录的路径
    DString fileName;         // 文件名
    bool isXml = false;       // 是否为 XML 文件
    bool isImage = false;     // 是否为图片文件
};

/** 项目管理器
 *  管理项目根目录、资源文件列表、最近打开文件等。
 */
class ProjectManager
{
public:
    ProjectManager();
    ~ProjectManager();

    /** 设置项目根目录 */
    bool SetProjectRoot(const DString& rootPath);

    /** 获取项目根目录 */
    const DString& GetProjectRoot() const { return m_projectRoot; }

    /** 项目是否已打开 */
    bool IsProjectOpen() const { return !m_projectRoot.empty(); }

    /** 获取资源目录路径 */
    DString GetResourcePath() const;

    /** 获取主题目录路径 */
    DString GetThemePath() const;

    /** 扫描项目资源文件 */
    std::vector<ResourceFileInfo> ScanResources() const;

    /** 添加最近打开文件 */
    void AddRecentFile(const DString& filePath);

    /** 获取最近打开文件列表 */
    const std::vector<DString>& GetRecentFiles() const { return m_recentFiles; }

    /** 清除最近打开文件列表 */
    void ClearRecentFiles();

    /** 获取 skin 文件夹名（从项目根目录推断） */
    std::string GetSkinFolderName() const;

private:
    /** 扫描目录（递归） */
    void ScanDirectory(const DString& dirPath, const DString& basePath,
                       std::vector<ResourceFileInfo>& results) const;

    /** 判断文件扩展名 */
    static bool IsXmlExtension(const DString& fileName);
    static bool IsImageExtension(const DString& fileName);

private:
    DString m_projectRoot;
    std::vector<DString> m_recentFiles;
    static const size_t kMaxRecentFiles = 10;
};

} // namespace creator

#endif // CREATOR_CORE_PROJECT_MANAGER_H_
