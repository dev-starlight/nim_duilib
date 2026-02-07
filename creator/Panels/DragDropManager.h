#ifndef CREATOR_PANELS_DRAG_DROP_MANAGER_H_
#define CREATOR_PANELS_DRAG_DROP_MANAGER_H_

#include "duilib/duilib.h"
#include "Core/Document.h"
#include "Core/ControlMetadata.h"
#include <memory>
#include <string>

namespace creator {

class ToolboxPanel;
class DesignCanvas;

/** 拖拽管理器
 *  协调工具箱→画布的拖拽逻辑。
 *  利用 duilib 内置的 DragOutId/DropInId 机制。
 */
class DragDropManager
{
public:
    DragDropManager();
    ~DragDropManager();

    /** 初始化拖拽管理器 */
    void Initialize(ToolboxPanel* toolbox, DesignCanvas* canvas,
                    std::shared_ptr<Document> document,
                    std::shared_ptr<ControlMetadata> metadata);

    /** 工具箱开始拖拽时调用 */
    void OnDragStart(const std::string& controlType);

    /** 画布上放下时调用 */
    void OnDrop(ui::Box* targetContainer, size_t index);

    /** 获取当前拖拽的控件类型 */
    const std::string& GetDraggedType() const { return m_draggedType; }

    /** 是否正在拖拽 */
    bool IsDragging() const { return m_isDragging; }

    /** 取消拖拽 */
    void CancelDrag();

private:
    ToolboxPanel* m_toolbox = nullptr;
    DesignCanvas* m_canvas = nullptr;
    std::shared_ptr<Document> m_document;
    std::shared_ptr<ControlMetadata> m_metadata;

    std::string m_draggedType;
    bool m_isDragging = false;
};

} // namespace creator

#endif // CREATOR_PANELS_DRAG_DROP_MANAGER_H_
