#include "PropertyPanel.h"

namespace creator {

PropertyPanel::PropertyPanel()
{
}

PropertyPanel::~PropertyPanel()
{
}

void PropertyPanel::Initialize(ui::VScrollBox* container,
                                std::shared_ptr<Document> document,
                                std::shared_ptr<Selection> selection,
                                std::shared_ptr<ControlMetadata> metadata)
{
    m_container = container;
    m_document = document;
    m_selection = selection;
    m_metadata = metadata;

    if (!m_selection) {
        return;
    }

    // 监听选择变更
    m_selection->AddSelectionChangedCallback([this](pugi::xml_node node) {
        OnSelectionChanged(node);
    });
}

void PropertyPanel::Refresh()
{
    if (m_selection && m_selection->HasSelection()) {
        PopulateProperties(m_selection->GetSelectedNode());
    }
    else {
        Clear();
    }
}

void PropertyPanel::Clear()
{
    if (m_container) {
        m_container->RemoveAllItems();
    }
}

void PropertyPanel::OnSelectionChanged(pugi::xml_node selectedNode)
{
    if (selectedNode.empty()) {
        Clear();
    }
    else {
        PopulateProperties(selectedNode);
    }
}

void PropertyPanel::PopulateProperties(pugi::xml_node node)
{
    if (!m_container || !m_metadata || node.empty()) {
        return;
    }

    m_container->RemoveAllItems();

    std::string controlType = node.name();
    auto properties = m_metadata->GetProperties(controlType);

    if (properties.empty()) {
        // 未注册的控件类型，至少显示节点已有的属性
        for (pugi::xml_attribute attr = node.first_attribute(); attr; attr = attr.next_attribute()) {
            PropertyDef def;
            def.name = attr.name();
            def.displayName = attr.name();
            def.group = "属性";
            def.type = PropertyType::kString;
            CreatePropertyRow(def, attr.as_string());
        }
        return;
    }

    // 按分组组织属性
    std::string currentGroup;
    for (const auto& propDef : properties) {
        if (propDef.group != currentGroup) {
            currentGroup = propDef.group;
            CreateGroupHeader(currentGroup);
        }

        // 获取当前值
        std::string currentValue = node.attribute(propDef.name.c_str()).as_string();
        CreatePropertyRow(propDef, currentValue);
    }
}

void PropertyPanel::CreateGroupHeader(const std::string& groupName)
{
    ui::Window* pWindow = m_container->GetWindow();
    if (!pWindow) {
        return;
    }

    ui::Label* header = new ui::Label(pWindow);
    header->SetText(ui::StringUtil::MBCSToT(groupName));
    header->SetFixedHeight(ui::UiFixedInt(24), true, true);
    header->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
    header->SetTextStyle(ui::TEXT_LEFT | ui::TEXT_VCENTER, true);
    header->SetFontId(_T("system_bold_12"));
    header->SetStateTextColor(ui::kControlStateNormal, _T("#FF888888"));
    header->SetBkColor(_T("#FF333337"));
    header->SetTextPadding(ui::UiPadding(8, 0, 0, 0), true);
    header->SetMargin(ui::UiMargin(0, 2, 0, 0), true);
    m_container->AddItem(header);
}

void PropertyPanel::CreatePropertyRow(const PropertyDef& propDef, const std::string& currentValue)
{
    ui::Window* pWindow = m_container->GetWindow();
    if (!pWindow) {
        return;
    }

    // 一行：属性名 | 属性值编辑框
    ui::HBox* row = new ui::HBox(pWindow);
    row->SetFixedHeight(ui::UiFixedInt(26), true, true);
    row->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
    row->SetMargin(ui::UiMargin(0, 1, 0, 0), true);

    // 属性名称标签
    ui::Label* nameLabel = new ui::Label(pWindow);
    nameLabel->SetText(ui::StringUtil::MBCSToT(propDef.displayName));
    nameLabel->SetFixedWidth(ui::UiFixedInt(100), true, true);
    nameLabel->SetFixedHeight(ui::UiFixedInt::MakeStretch(), true, false);
    nameLabel->SetTextStyle(ui::TEXT_LEFT | ui::TEXT_VCENTER, true);
    nameLabel->SetFontId(_T("system_12"));
    nameLabel->SetStateTextColor(ui::kControlStateNormal, _T("#FFAAAAAA"));
    nameLabel->SetTextPadding(ui::UiPadding(8, 0, 0, 0), true);
    nameLabel->SetToolTipText(ui::StringUtil::MBCSToT(propDef.description));
    row->AddItem(nameLabel);

    // 属性值编辑
    if (propDef.type == PropertyType::kBool) {
        // 布尔值用 CheckBox
        ui::CheckBox* checkBox = new ui::CheckBox(pWindow);
        checkBox->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
        checkBox->SetFixedHeight(ui::UiFixedInt::MakeStretch(), true, false);
        checkBox->SetMargin(ui::UiMargin(4, 4, 4, 4), true);
        bool isChecked = (currentValue == "true" || currentValue == "1");
        checkBox->SetSelected(isChecked);

        std::string propName = propDef.name;
        checkBox->AttachSelect([this, propName](const ui::EventArgs& args) {
            ui::CheckBox* cb = dynamic_cast<ui::CheckBox*>(args.GetSender());
            if (cb) {
                OnPropertyValueChanged(propName, cb->IsSelected() ? "true" : "false");
            }
            return true;
        });
        row->AddItem(checkBox);
    }
    else if (propDef.type == PropertyType::kCombo && !propDef.options.empty()) {
        // 下拉选择用 Combo（这里简化为 Button + 循环选项）
        ui::Button* comboBtn = new ui::Button(pWindow);
        DString displayValue = currentValue.empty()
            ? ui::StringUtil::MBCSToT(propDef.defaultValue)
            : ui::StringUtil::MBCSToT(currentValue);
        comboBtn->SetText(displayValue);
        comboBtn->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
        comboBtn->SetFixedHeight(ui::UiFixedInt(22), true, true);
        comboBtn->SetMargin(ui::UiMargin(2, 2, 4, 2), true);
        comboBtn->SetTextStyle(ui::TEXT_LEFT | ui::TEXT_VCENTER, true);
        comboBtn->SetTextPadding(ui::UiPadding(4, 0, 0, 0), true);
        comboBtn->SetFontId(_T("system_12"));
        comboBtn->SetStateTextColor(ui::kControlStateNormal, _T("#FFCCCCCC"));
        comboBtn->SetBkColor(_T("#FF3E3E42"));
        comboBtn->SetStateColor(ui::kControlStateHot, _T("#FF505054"));

        // 点击循环切换选项
        std::string propName = propDef.name;
        std::vector<std::string> options = propDef.options;
        comboBtn->AttachClick([this, propName, options, comboBtn](const ui::EventArgs& /*args*/) {
            DString current = comboBtn->GetText();
            std::string currentStr = ui::StringUtil::TToMBCS(current);
            // 找到当前选项的索引
            int nextIndex = 0;
            for (size_t i = 0; i < options.size(); ++i) {
                if (options[i] == currentStr) {
                    nextIndex = static_cast<int>((i + 1) % options.size());
                    break;
                }
            }
            std::string newValue = options[nextIndex];
            comboBtn->SetText(ui::StringUtil::MBCSToT(newValue));
            OnPropertyValueChanged(propName, newValue);
            return true;
        });
        row->AddItem(comboBtn);
    }
    else {
        // 其他类型用文本编辑
        ui::RichEdit* edit = new ui::RichEdit(pWindow);
        edit->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, false);
        edit->SetFixedHeight(ui::UiFixedInt(22), true, true);
        edit->SetMargin(ui::UiMargin(2, 2, 4, 2), true);
        edit->SetAttribute(_T("multi_line"), _T("false"));
        edit->SetAttribute(_T("rich_text"), _T("false"));
        edit->SetAttribute(_T("want_return"), _T("false"));
        edit->SetAttribute(_T("auto_hscroll"), _T("true"));
        edit->SetFontId(_T("system_12"));
        edit->SetTextPadding(ui::UiPadding(4, 2, 4, 2), true);
        edit->SetBkColor(_T("#FF3E3E42"));
        edit->SetStateTextColor(ui::kControlStateNormal, _T("#FFCCCCCC"));
        edit->SetText(ui::StringUtil::MBCSToT(currentValue));

        // 失焦时提交更改
        std::string propName = propDef.name;
        edit->AttachKillFocus([this, propName, edit](const ui::EventArgs& /*args*/) {
            DString newText = edit->GetText();
            std::string newValue = ui::StringUtil::TToMBCS(newText);
            OnPropertyValueChanged(propName, newValue);
            return true;
        });
        row->AddItem(edit);
    }

    m_container->AddItem(row);
}

void PropertyPanel::OnPropertyValueChanged(const std::string& propName, const std::string& newValue)
{
    if (!m_document || !m_selection || !m_selection->HasSelection()) {
        return;
    }

    pugi::xml_node node = m_selection->GetSelectedNode();
    if (node.empty()) {
        return;
    }

    if (newValue.empty()) {
        m_document->RemoveNodeAttribute(node, propName);
    }
    else {
        m_document->SetNodeAttribute(node, propName, newValue);
    }
}

} // namespace creator
