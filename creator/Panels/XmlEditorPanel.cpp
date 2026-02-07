#include "XmlEditorPanel.h"

namespace creator {

XmlEditorPanel::XmlEditorPanel()
{
}

XmlEditorPanel::~XmlEditorPanel()
{
}

void XmlEditorPanel::Initialize(ui::RichEdit* editor, std::shared_ptr<Document> document)
{
    m_editor = editor;
    m_document = document;

    if (!m_editor || !m_document) {
        return;
    }

    // 监听文档变更，自动同步到编辑器
    m_document->AddObserver([this](ChangeType /*type*/, pugi::xml_node /*node*/) {
        if (!m_syncing) {
            SyncFromDocument();
        }
    });
}

void XmlEditorPanel::SyncFromDocument()
{
    if (!m_editor || !m_document) {
        return;
    }

    m_syncing = true;
    std::string xml = m_document->ToXmlString();
    m_editor->SetText(ui::StringUtil::MBCSToT(xml));
    m_syncing = false;
}

bool XmlEditorPanel::SyncToDocument()
{
    if (!m_editor || !m_document) {
        return false;
    }

    m_syncing = true;
    DString text = m_editor->GetText();
    std::string xml = ui::StringUtil::TToMBCS(text);
    bool success = m_document->LoadFromString(xml);
    m_syncing = false;
    return success;
}

DString XmlEditorPanel::GetContent() const
{
    if (m_editor) {
        return m_editor->GetText();
    }
    return _T("");
}

void XmlEditorPanel::SetContent(const DString& content)
{
    if (m_editor) {
        m_editor->SetText(content);
    }
}

} // namespace creator
