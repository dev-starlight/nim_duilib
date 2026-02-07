#include "SelectionOverlay.h"

namespace creator {

SelectionOverlay::SelectionOverlay(ui::Window* pWindow)
    : ui::Control(pWindow)
{
    // 覆盖层不接收鼠标事件（由 DesignCanvas 统一处理 hit-test）
    SetMouseEnabled(false);
    // 透明背景
    SetBkColor(_T(""));
}

SelectionOverlay::~SelectionOverlay()
{
}

void SelectionOverlay::SetSelectionRect(const ui::UiRect& rect)
{
    m_selectionRects.clear();
    m_selectionRects.push_back(rect);
    Invalidate();
}

void SelectionOverlay::SetSelectionRects(const std::vector<ui::UiRect>& rects)
{
    m_selectionRects = rects;
    Invalidate();
}

void SelectionOverlay::ClearSelection()
{
    m_selectionRects.clear();
    Invalidate();
}

bool SelectionOverlay::HasSelection() const
{
    return !m_selectionRects.empty();
}

void SelectionOverlay::Paint(ui::IRender* pRender, const ui::UiRect& rcPaint)
{
    // 先绘制基类
    __super::Paint(pRender, rcPaint);

    if (m_selectionRects.empty() || pRender == nullptr) {
        return;
    }

    for (const auto& rect : m_selectionRects) {
        PaintSelectionRect(pRender, rect);
    }
}

void SelectionOverlay::PaintSelectionRect(ui::IRender* pRender, const ui::UiRect& rect)
{
    if (rect.IsEmpty()) {
        return;
    }

    // 绘制 2px 蓝色边框 (#FF007ACC)
    ui::UiColor borderColor = ui::UiColor(0xFF007ACC);
    ui::UiRect borderRect = rect;

    // 上边
    pRender->FillRect(ui::UiRect(borderRect.left, borderRect.top,
                                  borderRect.right, borderRect.top + kBorderWidth),
                      borderColor);
    // 下边
    pRender->FillRect(ui::UiRect(borderRect.left, borderRect.bottom - kBorderWidth,
                                  borderRect.right, borderRect.bottom),
                      borderColor);
    // 左边
    pRender->FillRect(ui::UiRect(borderRect.left, borderRect.top,
                                  borderRect.left + kBorderWidth, borderRect.bottom),
                      borderColor);
    // 右边
    pRender->FillRect(ui::UiRect(borderRect.right - kBorderWidth, borderRect.top,
                                  borderRect.right, borderRect.bottom),
                      borderColor);

    // 绘制 8 个白色调整手柄（四角 + 四边中点），6x6px
    int32_t cx = borderRect.CenterX();
    int32_t cy = borderRect.CenterY();

    // 四角
    PaintHandle(pRender, borderRect.left, borderRect.top);
    PaintHandle(pRender, borderRect.right, borderRect.top);
    PaintHandle(pRender, borderRect.left, borderRect.bottom);
    PaintHandle(pRender, borderRect.right, borderRect.bottom);

    // 四边中点
    PaintHandle(pRender, cx, borderRect.top);
    PaintHandle(pRender, cx, borderRect.bottom);
    PaintHandle(pRender, borderRect.left, cy);
    PaintHandle(pRender, borderRect.right, cy);
}

void SelectionOverlay::PaintHandle(ui::IRender* pRender, int32_t cx, int32_t cy)
{
    int32_t half = kHandleSize / 2;
    ui::UiRect handleRect(cx - half, cy - half, cx + half, cy + half);

    // 白色填充
    ui::UiColor fillColor = ui::UiColor(0xFFFFFFFF);
    pRender->FillRect(handleRect, fillColor);

    // 蓝色边框
    ui::UiColor borderColor = ui::UiColor(0xFF007ACC);
    pRender->FillRect(ui::UiRect(handleRect.left, handleRect.top,
                                  handleRect.right, handleRect.top + 1),
                      borderColor);
    pRender->FillRect(ui::UiRect(handleRect.left, handleRect.bottom - 1,
                                  handleRect.right, handleRect.bottom),
                      borderColor);
    pRender->FillRect(ui::UiRect(handleRect.left, handleRect.top,
                                  handleRect.left + 1, handleRect.bottom),
                      borderColor);
    pRender->FillRect(ui::UiRect(handleRect.right - 1, handleRect.top,
                                  handleRect.right, handleRect.bottom),
                      borderColor);
}

} // namespace creator
