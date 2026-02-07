#include "Document.h"
#include <sstream>

namespace creator {

Document::Document()
{
}

Document::~Document()
{
}

bool Document::LoadFromFile(const DString& filePath)
{
    // pugixml 支持 wchar_t 路径（Windows），可正确处理中文路径
#ifdef DUILIB_UNICODE
    pugi::xml_parse_result result = m_xmlDoc.load_file(filePath.c_str());
#else
    pugi::xml_parse_result result = m_xmlDoc.load_file(filePath.c_str());
#endif
    if (result) {
        m_modified = false;
        m_undoMgr.Clear();
        NotifyChanged(ChangeType::kDocumentLoaded, GetRootNode());
        return true;
    }
    return false;
}

bool Document::SaveToFile(const DString& filePath) const
{
#ifdef DUILIB_UNICODE
    return m_xmlDoc.save_file(filePath.c_str(), "    ", pugi::format_indent | pugi::format_save_file_text);
#else
    return m_xmlDoc.save_file(filePath.c_str(), "    ", pugi::format_indent | pugi::format_save_file_text);
#endif
}

bool Document::LoadFromString(const std::string& xmlContent)
{
    pugi::xml_parse_result result = m_xmlDoc.load_string(xmlContent.c_str());
    if (result) {
        m_modified = false;
        NotifyChanged(ChangeType::kDocumentLoaded, GetRootNode());
        return true;
    }
    return false;
}

std::string Document::ToXmlString() const
{
    std::ostringstream oss;
    m_xmlDoc.save(oss, "    ", pugi::format_indent);
    return oss.str();
}

void Document::CreateNewDocument()
{
    m_xmlDoc.reset();

    // 添加 XML 声明
    pugi::xml_node decl = m_xmlDoc.prepend_child(pugi::node_declaration);
    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "UTF-8";

    // 创建默认 Window 节点
    pugi::xml_node windowNode = m_xmlDoc.append_child("Window");
    windowNode.append_attribute("size") = "800,600";
    windowNode.append_attribute("caption") = "0,0,0,36";

    // 创建默认布局容器
    pugi::xml_node vbox = windowNode.append_child("VBox");
    vbox.append_attribute("bkcolor") = "bk_wnd_darkcolor";

    // 标题栏
    pugi::xml_node caption = vbox.append_child("HBox");
    caption.append_attribute("name") = "caption_bar";
    caption.append_attribute("width") = "stretch";
    caption.append_attribute("height") = "36";
    caption.append_attribute("bkcolor") = "bk_wnd_lightcolor";

    pugi::xml_node titleLabel = caption.append_child("Label");
    titleLabel.append_attribute("text") = "新窗口";
    titleLabel.append_attribute("width") = "stretch";
    titleLabel.append_attribute("height") = "stretch";
    titleLabel.append_attribute("text_align") = "vcenter";
    titleLabel.append_attribute("text_padding") = "8,0,0,0";

    // 工作区容器
    pugi::xml_node body = vbox.append_child("Box");
    body.append_attribute("name") = "body";
    body.append_attribute("width") = "stretch";
    body.append_attribute("height") = "stretch";

    m_modified = false;
    m_undoMgr.Clear();
    NotifyChanged(ChangeType::kDocumentLoaded, GetRootNode());
}

pugi::xml_node Document::GetRootNode() const
{
    // 跳过 XML 声明节点（node_declaration），找到第一个元素节点
    for (pugi::xml_node child = m_xmlDoc.first_child(); child; child = child.next_sibling()) {
        if (child.type() == pugi::node_element) {
            return child;
        }
    }
    return pugi::xml_node();
}

pugi::xml_node Document::AddControl(pugi::xml_node parent, const DString& controlType)
{
    if (parent.empty()) {
        return pugi::xml_node();
    }

    // 保存操作前的 XML 快照
    std::string beforeXml = ToXmlString();

#ifdef DUILIB_UNICODE
    std::wstring wtype(controlType);
    std::string type(wtype.begin(), wtype.end());
#else
    std::string type(controlType);
#endif

    pugi::xml_node newNode = parent.append_child(type.c_str());

    // 记录撤销命令
    std::string afterXml = ToXmlString();
    auto cmd = std::make_shared<XmlSnapshotCommand>(
        "添加控件: " + type,
        beforeXml,
        afterXml,
        [this](const std::string& xml) { this->LoadFromString(xml); }
    );
    m_undoMgr.RecordCommand(cmd);

    m_modified = true;
    NotifyChanged(ChangeType::kNodeAdded, newNode);
    return newNode;
}

bool Document::RemoveControl(pugi::xml_node node)
{
    if (node.empty()) {
        return false;
    }
    pugi::xml_node parent = node.parent();
    if (parent.empty()) {
        return false;
    }

    std::string beforeXml = ToXmlString();
    parent.remove_child(node);
    std::string afterXml = ToXmlString();

    auto cmd = std::make_shared<XmlSnapshotCommand>(
        "删除控件",
        beforeXml,
        afterXml,
        [this](const std::string& xml) { this->LoadFromString(xml); }
    );
    m_undoMgr.RecordCommand(cmd);

    m_modified = true;
    NotifyChanged(ChangeType::kNodeRemoved, parent);
    return true;
}

void Document::SetNodeAttribute(pugi::xml_node node, const std::string& name, const std::string& value)
{
    if (node.empty()) {
        return;
    }

    std::string beforeXml = ToXmlString();

    pugi::xml_attribute attr = node.attribute(name.c_str());
    if (attr.empty()) {
        attr = node.append_attribute(name.c_str());
    }
    attr.set_value(value.c_str());

    std::string afterXml = ToXmlString();
    auto cmd = std::make_shared<XmlSnapshotCommand>(
        "修改属性: " + name,
        beforeXml,
        afterXml,
        [this](const std::string& xml) { this->LoadFromString(xml); }
    );
    m_undoMgr.RecordCommand(cmd);

    m_modified = true;
    NotifyChanged(ChangeType::kAttributeChanged, node);
}

void Document::RemoveNodeAttribute(pugi::xml_node node, const std::string& name)
{
    if (node.empty()) {
        return;
    }
    pugi::xml_attribute attr = node.attribute(name.c_str());
    if (!attr.empty()) {
        std::string beforeXml = ToXmlString();
        node.remove_attribute(attr);
        std::string afterXml = ToXmlString();

        auto cmd = std::make_shared<XmlSnapshotCommand>(
            "删除属性: " + name,
            beforeXml,
            afterXml,
            [this](const std::string& xml) { this->LoadFromString(xml); }
        );
        m_undoMgr.RecordCommand(cmd);

        m_modified = true;
        NotifyChanged(ChangeType::kAttributeChanged, node);
    }
}

std::string Document::GetNodeAttribute(pugi::xml_node node, const std::string& name) const
{
    if (node.empty()) {
        return "";
    }
    pugi::xml_attribute attr = node.attribute(name.c_str());
    if (!attr.empty()) {
        return attr.as_string();
    }
    return "";
}

bool Document::MoveControl(pugi::xml_node node, pugi::xml_node newParent, int32_t index)
{
    if (node.empty() || newParent.empty()) {
        return false;
    }

    std::string beforeXml = ToXmlString();

    // 先从原来的位置移除
    pugi::xml_node oldParent = node.parent();
    if (oldParent.empty()) {
        return false;
    }

    // 移动节点
    if (index < 0) {
        newParent.append_move(node);
    }
    else {
        // 找到目标位置
        pugi::xml_node target = newParent.first_child();
        for (int32_t i = 0; i < index && !target.empty(); ++i) {
            target = target.next_sibling();
        }
        if (target.empty()) {
            newParent.append_move(node);
        }
        else {
            newParent.insert_move_before(node, target);
        }
    }

    std::string afterXml = ToXmlString();
    auto cmd = std::make_shared<XmlSnapshotCommand>(
        "移动控件",
        beforeXml,
        afterXml,
        [this](const std::string& xml) { this->LoadFromString(xml); }
    );
    m_undoMgr.RecordCommand(cmd);

    m_modified = true;
    NotifyChanged(ChangeType::kNodeMoved, node);
    return true;
}

pugi::xml_node Document::FindNodeByName(const DString& name) const
{
    return FindNodeByNameRecursive(GetRootNode(), name);
}

void Document::AddObserver(DocumentObserverCallback callback)
{
    m_observers.push_back(callback);
}

void Document::NotifyChanged(ChangeType type, pugi::xml_node node)
{
    for (auto& observer : m_observers) {
        observer(type, node);
    }
}

pugi::xml_node Document::FindNodeByNameRecursive(pugi::xml_node parent, const DString& name) const
{
    if (parent.empty()) {
        return pugi::xml_node();
    }

#ifdef DUILIB_UNICODE
    std::wstring wname(name);
    std::string sname(wname.begin(), wname.end());
#else
    std::string sname(name);
#endif

    pugi::xml_attribute attr = parent.attribute("name");
    if (!attr.empty() && std::string(attr.as_string()) == sname) {
        return parent;
    }

    for (pugi::xml_node child = parent.first_child(); child; child = child.next_sibling()) {
        pugi::xml_node found = FindNodeByNameRecursive(child, name);
        if (!found.empty()) {
            return found;
        }
    }
    return pugi::xml_node();
}

} // namespace creator
