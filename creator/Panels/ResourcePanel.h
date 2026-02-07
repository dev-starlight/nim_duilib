#ifndef CREATOR_PANELS_RESOURCE_PANEL_H_
#define CREATOR_PANELS_RESOURCE_PANEL_H_

#include "duilib/duilib.h"
#include <functional>
#include <string>

namespace creator {

/** 资源文件打开回调 */
using ResourceOpenCallback = std::function<void(const DString& filePath)>;

/** 资源管理面板
 *  浏览项目资源目录，支持打开 XML 文件。
 *  Phase 3 完整实现，当前提供基础框架。
 */
class ResourcePanel
{
public:
    ResourcePanel();
    ~ResourcePanel();

    /** 初始化资源面板 */
    void Initialize(ui::Box* container);

    /** 设置项目根目录 */
    void SetProjectRoot(const DString& rootPath);

    /** 设置文件打开回调 */
    void SetOpenCallback(ResourceOpenCallback callback);

    /** 刷新文件列表 */
    void Refresh();

private:
    ui::Box* m_container = nullptr;
    DString m_projectRoot;
    ResourceOpenCallback m_openCallback;
};

} // namespace creator

#endif // CREATOR_PANELS_RESOURCE_PANEL_H_
