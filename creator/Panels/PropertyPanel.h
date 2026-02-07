#ifndef CREATOR_PANELS_PROPERTY_PANEL_H_
#define CREATOR_PANELS_PROPERTY_PANEL_H_

#include "duilib/duilib.h"
#include "Core/Document.h"
#include "Core/Selection.h"
#include "Core/ControlMetadata.h"
#include <memory>

namespace creator {

/** 属性编辑面板
 *  根据选中的控件，动态展示和编辑其属性。
 *  使用 PropertyGrid 或手动构建的属性条目。
 */
class PropertyPanel
{
public:
    PropertyPanel();
    ~PropertyPanel();

    /** 初始化属性面板
     * @param container   属性面板所在的容器
     * @param document    文档模型
     * @param selection   选择管理器
     * @param metadata    控件元数据
     */
    void Initialize(ui::VScrollBox* container,
                    std::shared_ptr<Document> document,
                    std::shared_ptr<Selection> selection,
                    std::shared_ptr<ControlMetadata> metadata);

    /** 刷新属性面板（选择变更时调用） */
    void Refresh();

    /** 清空属性面板 */
    void Clear();

private:
    /** 选择变更回调 */
    void OnSelectionChanged(pugi::xml_node selectedNode);

    /** 填充属性列表 */
    void PopulateProperties(pugi::xml_node node);

    /** 创建一个属性分组标题 */
    void CreateGroupHeader(const std::string& groupName);

    /** 创建一个属性编辑行 */
    void CreatePropertyRow(const PropertyDef& propDef, const std::string& currentValue);

    /** 属性值变更处理 */
    void OnPropertyValueChanged(const std::string& propName, const std::string& newValue);

private:
    ui::VScrollBox* m_container = nullptr;
    std::shared_ptr<Document> m_document;
    std::shared_ptr<Selection> m_selection;
    std::shared_ptr<ControlMetadata> m_metadata;
};

} // namespace creator

#endif // CREATOR_PANELS_PROPERTY_PANEL_H_
