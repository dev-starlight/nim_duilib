#ifndef CREATOR_PANELS_SELECTION_OVERLAY_H_
#define CREATOR_PANELS_SELECTION_OVERLAY_H_

#include "duilib/duilib.h"
#include <vector>

namespace creator {

/** 选择指示器控件
 *  在选中控件上方绘制蓝色边框和调整手柄。
 *  作为覆盖层放在渲染层之上。
 */
class SelectionOverlay : public ui::Control
{
public:
    explicit SelectionOverlay(ui::Window* pWindow);
    virtual ~SelectionOverlay() override;

    /** 设置选择矩形（单选） */
    void SetSelectionRect(const ui::UiRect& rect);

    /** 设置多个选择矩形（多选） */
    void SetSelectionRects(const std::vector<ui::UiRect>& rects);

    /** 清除选择 */
    void ClearSelection();

    /** 是否有选择 */
    bool HasSelection() const;

    /** 获取选择矩形列表 */
    const std::vector<ui::UiRect>& GetSelectionRects() const { return m_selectionRects; }

    /** 绘制 */
    virtual void Paint(ui::IRender* pRender, const ui::UiRect& rcPaint) override;

private:
    /** 绘制单个选择框 */
    void PaintSelectionRect(ui::IRender* pRender, const ui::UiRect& rect);

    /** 绘制调整手柄 */
    void PaintHandle(ui::IRender* pRender, int32_t cx, int32_t cy);

private:
    std::vector<ui::UiRect> m_selectionRects;

    static constexpr int32_t kBorderWidth = 2;
    static constexpr int32_t kHandleSize = 6;
};

} // namespace creator

#endif // CREATOR_PANELS_SELECTION_OVERLAY_H_
