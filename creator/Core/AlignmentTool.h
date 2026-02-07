#ifndef CREATOR_CORE_ALIGNMENT_TOOL_H_
#define CREATOR_CORE_ALIGNMENT_TOOL_H_

#include "duilib/duilib.h"
#include "Core/Document.h"
#include "Core/Selection.h"
#include <memory>
#include <vector>

namespace creator {

/** 对齐/分布工具
 *  为画布中的控件提供对齐和等距分布功能。
 */
class AlignmentTool
{
public:
    enum class AlignType {
        Left,       // 左对齐
        Right,      // 右对齐
        Top,        // 上对齐
        Bottom,     // 下对齐
        CenterH,    // 水平居中
        CenterV     // 垂直居中
    };

    enum class DistributeType {
        Horizontal, // 水平等距分布
        Vertical    // 垂直等距分布
    };

    AlignmentTool(std::shared_ptr<Document> document,
                  std::shared_ptr<Selection> selection);
    ~AlignmentTool();

    /** 对齐操作（需要 ≥2 个选中控件） */
    bool Align(AlignType type);

    /** 分布操作（需要 ≥3 个选中控件） */
    bool Distribute(DistributeType type);

    /** 检查是否可执行对齐 */
    bool CanAlign() const;

    /** 检查是否可执行分布 */
    bool CanDistribute() const;

private:
    /** 获取控件的矩形区域（从 margin 属性解析） */
    ui::UiRect GetControlRect(pugi::xml_node node) const;

    /** 设置控件位置（修改 margin 属性） */
    void SetControlPosition(pugi::xml_node node, int32_t x, int32_t y);

    /** 解析 margin 字符串 "left,top,right,bottom" */
    ui::UiMargin ParseMargin(const std::string& marginStr) const;

    /** 将 margin 转为字符串 */
    std::string MarginToString(const ui::UiMargin& margin) const;

    /** 解析 width/height 属性 */
    int32_t ParseDimension(const std::string& value, int32_t defaultVal) const;

private:
    std::shared_ptr<Document> m_document;
    std::shared_ptr<Selection> m_selection;
};

} // namespace creator

#endif // CREATOR_CORE_ALIGNMENT_TOOL_H_
