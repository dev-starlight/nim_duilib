#ifndef CREATOR_PANELS_XML_EDITOR_PANEL_H_
#define CREATOR_PANELS_XML_EDITOR_PANEL_H_

#include "duilib/duilib.h"
#include "Core/Document.h"
#include <memory>

namespace creator {

/** XML 编辑器面板
 *  使用 RichEdit 实现 XML 源码编辑，支持双向同步。
 *  Phase 3 完整实现（语法高亮），当前提供基础文本编辑。
 */
class XmlEditorPanel
{
public:
    XmlEditorPanel();
    ~XmlEditorPanel();

    /** 初始化 XML 编辑器 */
    void Initialize(ui::RichEdit* editor, std::shared_ptr<Document> document);

    /** 从文档同步到编辑器 */
    void SyncFromDocument();

    /** 从编辑器同步到文档（手动触发） */
    bool SyncToDocument();

    /** 获取编辑器内容 */
    DString GetContent() const;

    /** 设置编辑器内容 */
    void SetContent(const DString& content);

private:
    ui::RichEdit* m_editor = nullptr;
    std::shared_ptr<Document> m_document;
    bool m_syncing = false; // 防止循环同步
};

} // namespace creator

#endif // CREATOR_PANELS_XML_EDITOR_PANEL_H_
