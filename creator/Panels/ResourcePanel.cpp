#include "ResourcePanel.h"

namespace creator {

ResourcePanel::ResourcePanel()
{
}

ResourcePanel::~ResourcePanel()
{
}

void ResourcePanel::Initialize(ui::Box* container)
{
    m_container = container;
}

void ResourcePanel::SetProjectRoot(const DString& rootPath)
{
    m_projectRoot = rootPath;
    Refresh();
}

void ResourcePanel::SetOpenCallback(ResourceOpenCallback callback)
{
    m_openCallback = callback;
}

void ResourcePanel::Refresh()
{
    // TODO: Phase 3 - 实现文件列表浏览
}

} // namespace creator
