#include "DragDropManager.h"
#include "ToolboxPanel.h"
#include "DesignCanvas.h"

namespace creator {

DragDropManager::DragDropManager()
{
}

DragDropManager::~DragDropManager()
{
}

void DragDropManager::Initialize(ToolboxPanel* toolbox, DesignCanvas* canvas,
                                  std::shared_ptr<Document> document,
                                  std::shared_ptr<ControlMetadata> metadata)
{
    m_toolbox = toolbox;
    m_canvas = canvas;
    m_document = document;
    m_metadata = metadata;

    // 启用画布的 drop 功能
    if (m_canvas) {
        m_canvas->SetDropEnabled(true);
    }
}

void DragDropManager::OnDragStart(const std::string& controlType)
{
    m_draggedType = controlType;
    m_isDragging = true;
}

void DragDropManager::OnDrop(ui::Box* targetContainer, size_t index)
{
    if (!m_isDragging || m_draggedType.empty()) {
        return;
    }

    if (m_canvas && !m_draggedType.empty()) {
        m_canvas->OnControlDropped(targetContainer, index, m_draggedType);
    }

    // 重置拖拽状态
    m_draggedType.clear();
    m_isDragging = false;
}

void DragDropManager::CancelDrag()
{
    m_draggedType.clear();
    m_isDragging = false;
}

} // namespace creator
