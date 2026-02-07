#include "ControlNameTagger.h"
#include <sstream>

namespace creator {

ControlNameTagger::ControlNameTagger()
{
}

ControlNameTagger::~ControlNameTagger()
{
}

void ControlNameTagger::TagNodes(pugi::xml_node root)
{
    Reset();
    if (root.empty()) {
        return;
    }
    TagNodeRecursive(root);
}

void ControlNameTagger::UntagNodes(pugi::xml_node root)
{
    if (root.empty()) {
        return;
    }
    UntagNodeRecursive(root);
    m_idToNode.clear();
}

pugi::xml_node ControlNameTagger::FindNodeByCreatorId(pugi::xml_node /*root*/,
                                                       const std::string& creatorId) const
{
    auto it = m_idToNode.find(creatorId);
    if (it != m_idToNode.end()) {
        return it->second;
    }
    return pugi::xml_node();
}

void ControlNameTagger::Reset()
{
    m_idToNode.clear();
    m_nextId = 0;
}

std::string ControlNameTagger::PeekNextId() const
{
    std::ostringstream oss;
    oss << "__cr_" << m_nextId;
    return oss.str();
}

void ControlNameTagger::TagNodeRecursive(pugi::xml_node node)
{
    if (node.empty()) {
        return;
    }

    // 生成唯一 ID
    std::ostringstream oss;
    oss << "__cr_" << m_nextId;
    std::string creatorId = oss.str();
    ++m_nextId;

    // 备份已有的 name 属性
    pugi::xml_attribute nameAttr = node.attribute("name");
    if (nameAttr) {
        std::string origName = nameAttr.as_string();
        if (!origName.empty() && origName.find("__cr_") != 0) {
            node.append_attribute("__cr_orig_name").set_value(origName.c_str());
        }
    }

    // 设置 creator id 作为 name
    if (nameAttr) {
        nameAttr.set_value(creatorId.c_str());
    }
    else {
        node.append_attribute("name").set_value(creatorId.c_str());
    }

    // 记录映射
    m_idToNode[creatorId] = node;

    // 递归处理子节点
    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        TagNodeRecursive(child);
    }
}

void ControlNameTagger::UntagNodeRecursive(pugi::xml_node node)
{
    if (node.empty()) {
        return;
    }

    // 先递归处理子节点
    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        UntagNodeRecursive(child);
    }

    // 检查是否有备份的原始 name
    pugi::xml_attribute origNameAttr = node.attribute("__cr_orig_name");
    pugi::xml_attribute nameAttr = node.attribute("name");

    if (origNameAttr) {
        // 恢复原始 name
        if (nameAttr) {
            nameAttr.set_value(origNameAttr.as_string());
        }
        node.remove_attribute(origNameAttr);
    }
    else if (nameAttr) {
        // 如果 name 是 __cr_ 开头的，说明是我们添加的，移除
        std::string nameVal = nameAttr.as_string();
        if (nameVal.find("__cr_") == 0) {
            node.remove_attribute(nameAttr);
        }
    }
}

} // namespace creator
