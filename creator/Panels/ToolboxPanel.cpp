#include "ToolboxPanel.h"

namespace creator {

ToolboxPanel::ToolboxPanel()
{
}

ToolboxPanel::~ToolboxPanel()
{
}

void ToolboxPanel::Initialize(ui::VScrollBox* container,
                               std::shared_ptr<ControlMetadata> metadata)
{
    m_container = container;
    m_metadata = metadata;

    if (!m_container || !m_metadata) {
        return;
    }

    // 移除现有内容
    m_container->RemoveAllItems();

    // 按分类创建工具箱内容
    const auto& categories = m_metadata->GetCategories();
    for (const auto& category : categories) {
        CreateCategoryGroup(m_container, category);
    }
}

void ToolboxPanel::SetSelectCallback(ToolboxSelectCallback callback)
{
    m_selectCallback = callback;
}

void ToolboxPanel::SetDragStartCallback(DragStartCallback callback)
{
    m_dragStartCallback = callback;
}

void ToolboxPanel::SetDragEnabled(bool enabled)
{
    m_dragEnabled = enabled;
}

void ToolboxPanel::ClearSelection()
{
    m_selectedType.clear();
    if (m_highlightedButton) {
        m_highlightedButton->SetBkColor(_T(""));
        m_highlightedButton = nullptr;
    }
}

void ToolboxPanel::CreateCategoryGroup(ui::VScrollBox* container,
                                        const ControlCategory& category)
{
    ui::Window* pWindow = container->GetWindow();
    if (!pWindow) {
        return;
    }

    // 分类标题
    ui::Label* titleLabel = new ui::Label(pWindow);
    titleLabel->SetText(ui::StringUtil::MBCSToT(category.categoryName));
    titleLabel->SetFixedHeight(ui::UiFixedInt(24), true, true);
    titleLabel->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
    titleLabel->SetTextStyle(ui::TEXT_LEFT | ui::TEXT_VCENTER, true);
    titleLabel->SetFontId(_T("system_bold_12"));
    titleLabel->SetStateTextColor(ui::kControlStateNormal, _T("#FF888888"));
    titleLabel->SetTextPadding(ui::UiPadding(4, 0, 0, 0), true);
    container->AddItem(titleLabel);

    // 分类下的控件按钮容器
    ui::VBox* itemsBox = new ui::VBox(pWindow);
    itemsBox->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
    itemsBox->SetFixedHeight(ui::UiFixedInt::MakeAuto(), true, false);
    itemsBox->SetMargin(ui::UiMargin(8, 0, 4, 8), true);
    container->AddItem(itemsBox);

    // 创建每个控件按钮
    for (const auto& controlType : category.controlTypes) {
        CreateControlItem(itemsBox, controlType);
    }
}

void ToolboxPanel::CreateControlItem(ui::Box* parent,
                                      const std::string& controlType)
{
    ui::Window* pWindow = parent->GetWindow();
    if (!pWindow) {
        return;
    }

    ui::Button* btn = new ui::Button(pWindow);
    btn->SetText(ui::StringUtil::MBCSToT(controlType));
    btn->SetFixedHeight(ui::UiFixedInt(24), true, true);
    btn->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
    btn->SetTextStyle(ui::TEXT_LEFT | ui::TEXT_VCENTER, true);
    btn->SetFontId(_T("system_12"));
    btn->SetStateTextColor(ui::kControlStateNormal, _T("#FFCCCCCC"));
    btn->SetStateTextColor(ui::kControlStateHot, _T("#FFFFFFFF"));
    btn->SetTextPadding(ui::UiPadding(20, 0, 0, 0), true);
    btn->SetStateColor(ui::kControlStateHot, _T("#FF3E3E42"));
    btn->SetStateColor(ui::kControlStatePushed, _T("#FF505054"));

    // 点击事件
    std::string type = controlType; // 捕获副本
    btn->AttachClick([this, type, btn](const ui::EventArgs& /*args*/) {
        OnControlItemClick(type);
        // 高亮当前选中
        if (m_highlightedButton && m_highlightedButton != btn) {
            m_highlightedButton->SetBkColor(_T(""));
        }
        btn->SetBkColor(_T("#FF094771"));
        m_highlightedButton = btn;
        return true;
    });

    parent->AddItem(btn);
}

void ToolboxPanel::OnControlItemClick(const std::string& controlType)
{
    m_selectedType = controlType;
    if (m_selectCallback) {
        m_selectCallback(controlType);
    }
}

} // namespace creator
