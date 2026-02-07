#ifndef CREATOR_PANELS_TOOLBOX_PANEL_H_
#define CREATOR_PANELS_TOOLBOX_PANEL_H_

#include "duilib/duilib.h"
#include "Core/ControlMetadata.h"
#include <memory>
#include <functional>

namespace creator {

/** 控件选择回调：用户从工具箱选中了一个控件类型 */
using ToolboxSelectCallback = std::function<void(const std::string& controlType)>;

/** 拖拽开始回调 */
using DragStartCallback = std::function<void(const std::string& controlType)>;

/** 控件工具箱面板
 *  分类展示所有可用控件，支持点击选中并添加到画布。
 *  支持拖拽控件到画布（Feature 2）。
 */
class ToolboxPanel
{
public:
    ToolboxPanel();
    ~ToolboxPanel();

    /** 初始化工具箱
     * @param container 工具箱所在的滚动容器
     * @param metadata  控件元数据
     */
    void Initialize(ui::VScrollBox* container,
                    std::shared_ptr<ControlMetadata> metadata);

    /** 设置控件选择回调 */
    void SetSelectCallback(ToolboxSelectCallback callback);

    /** 设置拖拽开始回调 */
    void SetDragStartCallback(DragStartCallback callback);

    /** 设置是否启用拖拽 */
    void SetDragEnabled(bool enabled);

    /** 获取当前选中的控件类型（空表示无选中） */
    const std::string& GetSelectedType() const { return m_selectedType; }

    /** 清除选中状态 */
    void ClearSelection();

private:
    /** 创建一个分类组 */
    void CreateCategoryGroup(ui::VScrollBox* container,
                             const ControlCategory& category);

    /** 创建一个控件条目 */
    void CreateControlItem(ui::Box* parent,
                           const std::string& controlType);

    /** 控件条目点击事件 */
    void OnControlItemClick(const std::string& controlType);

private:
    ui::VScrollBox* m_container = nullptr;
    std::shared_ptr<ControlMetadata> m_metadata;
    ToolboxSelectCallback m_selectCallback;
    DragStartCallback m_dragStartCallback;
    std::string m_selectedType;
    bool m_dragEnabled = false;

    // 记录当前高亮的按钮
    ui::Button* m_highlightedButton = nullptr;
};

} // namespace creator

#endif // CREATOR_PANELS_TOOLBOX_PANEL_H_
