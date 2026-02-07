#include "Clipboard.h"
#include <sstream>

namespace creator {

Clipboard::Clipboard()
{
}

Clipboard::~Clipboard()
{
}

bool Clipboard::Copy(pugi::xml_node node)
{
    if (node.empty()) {
        return false;
    }

    // 将节点序列化为 XML 字符串
    std::ostringstream oss;
    node.print(oss, "    ", pugi::format_indent);
    m_xmlContent = oss.str();
    return !m_xmlContent.empty();
}

bool Clipboard::HasContent() const
{
    return !m_xmlContent.empty();
}

pugi::xml_node Clipboard::Paste(pugi::xml_node targetParent)
{
    if (targetParent.empty() || m_xmlContent.empty()) {
        return pugi::xml_node();
    }

    // 解析剪贴板内容
    pugi::xml_document tempDoc;
    pugi::xml_parse_result result = tempDoc.load_string(m_xmlContent.c_str());
    if (!result) {
        return pugi::xml_node();
    }

    // 将解析出的节点追加到目标父节点
    pugi::xml_node srcNode = tempDoc.first_child();
    if (srcNode.empty()) {
        return pugi::xml_node();
    }

    pugi::xml_node newNode = targetParent.append_copy(srcNode);
    return newNode;
}

void Clipboard::Clear()
{
    m_xmlContent.clear();
}

} // namespace creator
