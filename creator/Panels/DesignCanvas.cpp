#include "DesignCanvas.h"
#include "SelectionOverlay.h"
#include <sstream>

namespace creator {

DesignCanvas::DesignCanvas()
{
}

DesignCanvas::~DesignCanvas()
{
}

void DesignCanvas::Initialize(ui::Box* canvasBox,
                               std::shared_ptr<Document> document,
                               std::shared_ptr<Selection> selection,
                               std::shared_ptr<ControlMetadata> metadata)
{
    m_canvasBox = canvasBox;
    m_document = document;
    m_selection = selection;
    m_metadata = metadata;

    if (!m_canvasBox || !m_document) {
        return;
    }

    ui::Window* pWindow = m_canvasBox->GetWindow();
    if (!pWindow) {
        return;
    }

    // 创建双层架构
    // 渲染层：承载 WindowBuilder 创建的真实控件
    m_renderBox = new ui::Box(pWindow);
    m_renderBox->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
    m_renderBox->SetFixedHeight(ui::UiFixedInt::MakeStretch(), true, false);
    m_canvasBox->AddItem(m_renderBox);

    // 选择覆盖层：绘制选择指示器
    m_overlay = new SelectionOverlay(pWindow);
    m_overlay->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
    m_overlay->SetFixedHeight(ui::UiFixedInt::MakeStretch(), true, false);
    m_canvasBox->AddItem(m_overlay);

    // 注册文档变更监听
    m_document->AddObserver([this](ChangeType type, pugi::xml_node node) {
        OnDocumentChanged(type, node);
    });

    // 为覆盖层注册点击事件（统一 hit-test）
    m_overlay->SetMouseEnabled(true);
    m_overlay->AttachButtonDown([this](const ui::EventArgs& args) {
        OnCanvasClick(args);
        return true;
    });

    // 监听选择变更以更新覆盖层
    if (m_selection) {
        m_selection->AddSelectionChangedCallback([this](pugi::xml_node /*node*/) {
            UpdateSelectionOverlay();
        });
    }
}

void DesignCanvas::RefreshPreview()
{
    RebuildRenderedControls();
}

void DesignCanvas::RebuildRenderedControls()
{
    if (!m_canvasBox || !m_document || !m_renderBox) {
        return;
    }

    ui::Window* pWindow = m_canvasBox->GetWindow();
    if (!pWindow) {
        return;
    }

    // 准备带标签的 XML
    std::string renderXml = PrepareRenderXml();

    // 与上次渲染的 XML 比较，避免无变化时重建
    if (renderXml == m_lastRenderedXml && !renderXml.empty()) {
        UpdateSelectionOverlay();
        return;
    }
    m_lastRenderedXml = renderXml;

    // 清空渲染层
    m_renderBox->RemoveAllItems();

    if (renderXml.empty()) {
        // 空文档提示
        ui::Label* emptyLabel = new ui::Label(pWindow);
        emptyLabel->SetText(_T("(空文档 - 从工具箱添加控件)"));
        emptyLabel->SetFixedHeight(ui::UiFixedInt(40), true, true);
        emptyLabel->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
        emptyLabel->SetTextStyle(ui::TEXT_HCENTER | ui::TEXT_VCENTER, true);
        emptyLabel->SetFontId(_T("system_14"));
        emptyLabel->SetStateTextColor(ui::kControlStateNormal, _T("#FF888888"));
        m_renderBox->AddItem(emptyLabel);
        return;
    }

    // 使用 WindowBuilder 解析 XML 并创建控件
    ui::WindowBuilder builder;
    if (builder.ParseXmlData(ui::StringUtil::MBCSToT(renderXml))) {
        ui::Control* pRoot = builder.CreateControls(pWindow, ui::WindowBuilder::CreateControlCallback(),
                                                     m_renderBox, m_renderBox);
        if (pRoot) {
            // 如果 CreateControls 返回的控件不是 m_renderBox 本身，
            // 说明创建了一个新的根控件，需要添加到 m_renderBox
            if (pRoot != m_renderBox && pRoot->GetParent() != m_renderBox) {
                m_renderBox->AddItem(pRoot);
            }
        }
    }
    else {
        // XML 解析失败，显示错误提示
        ui::Label* errorLabel = new ui::Label(pWindow);
        errorLabel->SetText(_T("XML 渲染失败"));
        errorLabel->SetFixedHeight(ui::UiFixedInt(40), true, true);
        errorLabel->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
        errorLabel->SetTextStyle(ui::TEXT_HCENTER | ui::TEXT_VCENTER, true);
        errorLabel->SetFontId(_T("system_14"));
        errorLabel->SetStateTextColor(ui::kControlStateNormal, _T("#FFFF4444"));
        m_renderBox->AddItem(errorLabel);
    }

    // 如果启用了 drop，设置容器控件的 DropInId
    if (m_dropEnabled) {
        SetupDropTargets(m_renderBox);
    }

    // 更新选择覆盖层
    UpdateSelectionOverlay();
}

std::string DesignCanvas::PrepareRenderXml()
{
    pugi::xml_node root = m_document->GetRootNode();
    if (root.empty()) {
        return "";
    }

    // 在文档的副本上操作，避免修改原始文档
    // pugi::xml_document 不支持拷贝，通过序列化/反序列化创建副本
    std::string originalXml = m_document->ToXmlString();
    pugi::xml_document tempDoc;
    if (!tempDoc.load_string(originalXml.c_str())) {
        return "";
    }

    // 找到副本中的根元素节点（跳过 XML 声明）
    pugi::xml_node tempRoot;
    for (pugi::xml_node child = tempDoc.first_child(); child; child = child.next_sibling()) {
        if (child.type() == pugi::node_element) {
            tempRoot = child;
            break;
        }
    }
    if (tempRoot.empty()) {
        return "";
    }

    // 给副本的节点添加 __cr_N 标签
    m_tagger.TagNodes(tempRoot);

    // 导出为 XML 字符串
    std::ostringstream oss;
    tempDoc.save(oss, "  ", pugi::format_default, pugi::encoding_utf8);
    return oss.str();
}

ui::Control* DesignCanvas::HitTestRenderedControl(const ui::UiPoint& pt)
{
    if (!m_renderBox) {
        return nullptr;
    }
    return HitTestRecursive(m_renderBox, pt);
}

ui::Control* DesignCanvas::HitTestRecursive(ui::Control* control, const ui::UiPoint& pt)
{
    if (!control || !control->IsVisible()) {
        return nullptr;
    }

    ui::UiRect rc = control->GetPos();
    if (!rc.ContainsPt(pt)) {
        return nullptr;
    }

    // 如果是容器，递归检查子控件（从后往前，后绘制的在上层）
    ui::Box* box = dynamic_cast<ui::Box*>(control);
    if (box) {
        size_t count = box->GetItemCount();
        for (size_t i = count; i > 0; --i) {
            ui::Control* child = box->GetItemAt(i - 1);
            ui::Control* hit = HitTestRecursive(child, pt);
            if (hit) {
                return hit;
            }
        }
    }

    // 如果没有子控件命中，返回当前控件（但跳过 m_renderBox 本身）
    if (control == m_renderBox) {
        return nullptr;
    }

    return control;
}

pugi::xml_node DesignCanvas::MapControlToXmlNode(ui::Control* control)
{
    if (!control || !m_document) {
        return pugi::xml_node();
    }

    DString name = control->GetName();
    std::string nameStr = ui::StringUtil::TToMBCS(name);

    // 检查是否是 __cr_ 前缀的名称
    if (nameStr.find("__cr_") != 0) {
        return pugi::xml_node();
    }

    // 通过 tagger 的映射表查找对应的 xml_node
    // 注意：tagger 的映射表指向临时文档的节点，
    // 我们需要通过 ID 编号在原始文档中查找对应节点。
    // 策略：从 __cr_N 中提取 N，然后在原始文档中按相同的遍历顺序找到第 N 个节点。
    pugi::xml_node tempNode = m_tagger.FindNodeByCreatorId(pugi::xml_node(), nameStr);
    if (tempNode.empty()) {
        return pugi::xml_node();
    }

    // 通过节点路径在原始文档中查找
    // 简化方案：使用 __cr_N 中的序号，按深度优先遍历原始文档找到对应节点
    std::string idStr = nameStr.substr(5); // 去掉 "__cr_" 前缀
    int32_t targetIndex = 0;
    try {
        targetIndex = std::stoi(idStr);
    }
    catch (...) {
        return pugi::xml_node();
    }

    // 深度优先遍历原始文档，找到第 targetIndex 个节点
    struct NodeFinder {
        int32_t currentIndex = 0;
        int32_t targetIndex = 0;
        pugi::xml_node result;

        bool Find(pugi::xml_node node) {
            if (node.empty()) return false;
            if (currentIndex == targetIndex) {
                result = node;
                return true;
            }
            ++currentIndex;
            for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
                if (Find(child)) return true;
            }
            return false;
        }
    };

    NodeFinder finder;
    finder.targetIndex = targetIndex;
    finder.Find(m_document->GetRootNode());
    return finder.result;
}

void DesignCanvas::UpdateSelectionOverlay()
{
    if (!m_overlay || !m_selection || !m_renderBox) {
        return;
    }

    if (!m_selection->HasSelection()) {
        m_overlay->ClearSelection();
        return;
    }

    // 获取选中节点对应的渲染控件
    pugi::xml_node selectedNode = m_selection->GetSelectedNode();
    if (selectedNode.empty()) {
        m_overlay->ClearSelection();
        return;
    }

    // 多选支持（Feature 3）
    std::vector<ui::UiRect> selectionRects;

    auto findControlForNode = [this](pugi::xml_node node) -> ui::Control* {
        // 计算节点在深度优先遍历中的序号
        struct IndexFinder {
            int32_t currentIndex = 0;
            pugi::xml_node target;
            int32_t result = -1;

            bool Find(pugi::xml_node node) {
                if (node.empty()) return false;
                if (node == target) {
                    result = currentIndex;
                    return true;
                }
                ++currentIndex;
                for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
                    if (Find(child)) return true;
                }
                return false;
            }
        };

        IndexFinder finder;
        finder.target = node;
        finder.Find(m_document->GetRootNode());

        if (finder.result < 0) {
            return nullptr;
        }

        // 构造 __cr_N 名称
        std::ostringstream oss;
        oss << "__cr_" << finder.result;
        DString creatorName = ui::StringUtil::MBCSToT(oss.str());

        // 在渲染控件树中查找
        return m_renderBox->FindSubControl(creatorName);
    };

    // 单选
    ui::Control* selectedControl = findControlForNode(selectedNode);
    if (selectedControl) {
        ui::UiRect controlRect = selectedControl->GetPos();
        // 转换为相对于 overlay 的坐标
        ui::UiRect overlayRect = m_overlay->GetPos();
        controlRect.Offset(-overlayRect.left, -overlayRect.top);
        selectionRects.push_back(controlRect);
    }

    // 多选节点（如果 Selection 支持多选）
    auto selectedNodes = m_selection->GetSelectedNodes();
    if (selectedNodes.size() > 1) {
        selectionRects.clear();
        for (const auto& node : selectedNodes) {
            ui::Control* ctrl = findControlForNode(node);
            if (ctrl) {
                ui::UiRect controlRect = ctrl->GetPos();
                ui::UiRect overlayRect = m_overlay->GetPos();
                controlRect.Offset(-overlayRect.left, -overlayRect.top);
                selectionRects.push_back(controlRect);
            }
        }
    }

    if (!selectionRects.empty()) {
        m_overlay->SetSelectionRects(selectionRects);
    }
    else {
        m_overlay->ClearSelection();
    }
}

void DesignCanvas::SetCanvasSize(int32_t width, int32_t height)
{
    if (m_canvasBox) {
        m_canvasBox->SetFixedWidth(ui::UiFixedInt(width), true, true);
        m_canvasBox->SetFixedHeight(ui::UiFixedInt(height), true, true);
    }
}

bool DesignCanvas::AddControlToSelected(const std::string& controlType)
{
    if (!m_document || !m_selection) {
        return false;
    }

    pugi::xml_node parent;
    if (m_selection->HasSelection()) {
        pugi::xml_node selected = m_selection->GetSelectedNode();
        // 如果选中的是容器类型，添加到选中节点
        if (m_metadata && m_metadata->IsContainer(selected.name())) {
            parent = selected;
        }
        else {
            // 否则添加到选中节点的父节点
            parent = selected.parent();
        }
    }
    else {
        // 没有选中节点，添加到根节点的第一个容器子节点
        pugi::xml_node root = m_document->GetRootNode();
        if (!root.empty()) {
            // 尝试找到根节点下的第一个容器
            for (pugi::xml_node child = root.first_child(); child; child = child.next_sibling()) {
                if (m_metadata && m_metadata->IsContainer(child.name())) {
                    parent = child;
                    break;
                }
            }
            if (parent.empty()) {
                parent = root;
            }
        }
    }

    if (parent.empty()) {
        return false;
    }

    DString typeStr = ui::StringUtil::MBCSToT(controlType);
    pugi::xml_node newNode = m_document->AddControl(parent, typeStr);
    if (!newNode.empty()) {
        m_selection->Select(newNode);
        return true;
    }
    return false;
}

void DesignCanvas::OnDocumentChanged(ChangeType /*type*/, pugi::xml_node /*node*/)
{
    // 文档变更时，清除缓存的 XML 以强制重建
    m_lastRenderedXml.clear();
    RefreshPreview();
}

void DesignCanvas::OnCanvasClick(const ui::EventArgs& args)
{
    if (!m_selection) {
        return;
    }

    ui::UiPoint pt(args.ptMouse);

    // 在渲染控件树中进行 hit-test
    ui::Control* hitControl = HitTestRenderedControl(pt);
    if (hitControl) {
        pugi::xml_node xmlNode = MapControlToXmlNode(hitControl);
        if (!xmlNode.empty()) {
            // 检查是否按住 Ctrl 键（多选支持，Feature 3）
            bool ctrlDown = ui::Keyboard::IsKeyDown(ui::kVK_CONTROL);
            if (ctrlDown && m_selection->HasSelection()) {
                m_selection->AddToSelection(xmlNode);
            }
            else {
                m_selection->Select(xmlNode);
            }
            return;
        }
    }

    // 点击空白处清除选择
    m_selection->ClearSelection();
}

std::string DesignCanvas::HitTestControl(const ui::UiPoint& pt)
{
    ui::Control* hit = HitTestRenderedControl(pt);
    if (hit) {
        return ui::StringUtil::TToMBCS(hit->GetName());
    }
    return "";
}

void DesignCanvas::SetDropEnabled(bool enabled)
{
    m_dropEnabled = enabled;
    if (m_renderBox && enabled) {
        SetupDropTargets(m_renderBox);
    }
}

void DesignCanvas::OnControlDropped(ui::Box* targetContainer, size_t /*index*/,
                                     const std::string& controlType)
{
    if (!m_document || !targetContainer) {
        return;
    }

    // 通过 hit-test 映射回 xml_node
    pugi::xml_node targetNode = MapControlToXmlNode(targetContainer);
    if (targetNode.empty()) {
        // 如果映射失败，使用根节点
        targetNode = m_document->GetRootNode();
    }

    DString typeStr = ui::StringUtil::MBCSToT(controlType);
    pugi::xml_node newNode = m_document->AddControl(targetNode, typeStr);
    if (!newNode.empty() && m_selection) {
        m_selection->Select(newNode);
    }
}

void DesignCanvas::SetupDropTargets(ui::Control* control)
{
    if (!control) {
        return;
    }

    ui::Box* box = dynamic_cast<ui::Box*>(control);
    if (box) {
        box->SetDropInId(1);

        // 递归设置子容器
        size_t count = box->GetItemCount();
        for (size_t i = 0; i < count; ++i) {
            SetupDropTargets(box->GetItemAt(i));
        }
    }
}

} // namespace creator
