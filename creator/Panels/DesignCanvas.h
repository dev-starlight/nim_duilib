#ifndef CREATOR_PANELS_DESIGN_CANVAS_H_
#define CREATOR_PANELS_DESIGN_CANVAS_H_

#include "duilib/duilib.h"
#include "Core/Document.h"
#include "Core/Selection.h"
#include "Core/ControlMetadata.h"
#include "Core/ControlNameTagger.h"
#include <memory>
#include <functional>

namespace creator {

class SelectionOverlay;

/** 画布点击回调：在画布上点击选中了某个控件 */
using CanvasClickCallback = std::function<void(const std::string& controlName)>;

/** 设计画布
 *  双层架构：渲染层（m_renderBox）+ 选择覆盖层（m_overlay）
 *  使用 WindowBuilder 将 Document 的 XML 实时渲染为真实 duilib 控件。
 */
class DesignCanvas
{
public:
    DesignCanvas();
    ~DesignCanvas();

    /** 初始化画布
     * @param canvasBox 画布所在的 Box 容器
     * @param document  文档模型
     * @param selection 选择管理器
     * @param metadata  控件元数据
     */
    void Initialize(ui::Box* canvasBox,
                    std::shared_ptr<Document> document,
                    std::shared_ptr<Selection> selection,
                    std::shared_ptr<ControlMetadata> metadata);

    /** 刷新画布预览（重新加载 XML） */
    void RefreshPreview();

    /** 设置画布大小 */
    void SetCanvasSize(int32_t width, int32_t height);

    /** 获取画布容器 */
    ui::Box* GetCanvasBox() const { return m_canvasBox; }

    /** 获取渲染容器 */
    ui::Box* GetRenderBox() const { return m_renderBox; }

    /** 在选中节点下添加控件 */
    bool AddControlToSelected(const std::string& controlType);

    /** 设置是否启用 drop（Feature 2 使用） */
    void SetDropEnabled(bool enabled);

    /** 控件被拖放到画布时的回调（Feature 2 使用） */
    void OnControlDropped(ui::Box* targetContainer, size_t index,
                          const std::string& controlType);

private:
    /** 文档变更回调 */
    void OnDocumentChanged(ChangeType type, pugi::xml_node node);

    /** 处理画布上的鼠标点击，进行 hit-test */
    void OnCanvasClick(const ui::EventArgs& args);

    /** 重建渲染控件（WYSIWYG 核心） */
    void RebuildRenderedControls();

    /** 准备用于渲染的 XML（带 __cr_N 标签） */
    std::string PrepareRenderXml();

    /** 在渲染控件树中进行 hit-test，找到最深层包含该点的控件 */
    ui::Control* HitTestRenderedControl(const ui::UiPoint& pt);

    /** 递归 hit-test */
    ui::Control* HitTestRecursive(ui::Control* control, const ui::UiPoint& pt);

    /** 通过渲染控件的 name 属性映射回 xml_node */
    pugi::xml_node MapControlToXmlNode(ui::Control* control);

    /** 更新选择覆盖层 */
    void UpdateSelectionOverlay();

    /** 查找点击位置对应的控件名（兼容旧接口） */
    std::string HitTestControl(const ui::UiPoint& pt);

    /** 为渲染出的容器控件设置 DropInId（Feature 2 使用） */
    void SetupDropTargets(ui::Control* control);

private:
    ui::Box* m_canvasBox = nullptr;
    ui::Box* m_renderBox = nullptr;          // 渲染层
    SelectionOverlay* m_overlay = nullptr;   // 选择覆盖层
    std::shared_ptr<Document> m_document;
    std::shared_ptr<Selection> m_selection;
    std::shared_ptr<ControlMetadata> m_metadata;
    ControlNameTagger m_tagger;

    std::string m_lastRenderedXml;           // 上次渲染的 XML（避免无变化时重建）
    bool m_dropEnabled = false;
};

} // namespace creator

#endif // CREATOR_PANELS_DESIGN_CANVAS_H_
