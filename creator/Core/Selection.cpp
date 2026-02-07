#include "Selection.h"

namespace creator {

Selection::Selection()
{
}

Selection::~Selection()
{
}

void Selection::Select(pugi::xml_node node)
{
    m_selectedNode = node;
    m_selectedNodes.clear();
    if (!node.empty()) {
        m_selectedNodes.push_back(node);
    }
    NotifySelectionChanged();
}

void Selection::AddToSelection(pugi::xml_node node)
{
    if (node.empty()) {
        return;
    }

    // 如果已经选中，则取消选中（toggle 行为）
    if (IsNodeSelected(node)) {
        RemoveFromSelection(node);
        return;
    }

    m_selectedNodes.push_back(node);
    // 主选中节点设为第一个
    if (m_selectedNodes.size() == 1) {
        m_selectedNode = node;
    }
    NotifySelectionChanged();
}

void Selection::RemoveFromSelection(pugi::xml_node node)
{
    if (node.empty()) {
        return;
    }

    auto it = std::find_if(m_selectedNodes.begin(), m_selectedNodes.end(),
                           [&node](const pugi::xml_node& n) { return n == node; });
    if (it != m_selectedNodes.end()) {
        m_selectedNodes.erase(it);
    }

    // 更新主选中节点
    if (m_selectedNodes.empty()) {
        m_selectedNode = pugi::xml_node();
    }
    else {
        m_selectedNode = m_selectedNodes.front();
    }
    NotifySelectionChanged();
}

void Selection::ClearSelection()
{
    m_selectedNode = pugi::xml_node();
    m_selectedNodes.clear();
    NotifySelectionChanged();
}

pugi::xml_node Selection::GetSelectedNode() const
{
    return m_selectedNode;
}

std::vector<pugi::xml_node> Selection::GetSelectedNodes() const
{
    return m_selectedNodes;
}

bool Selection::HasSelection() const
{
    return !m_selectedNode.empty();
}

size_t Selection::GetSelectionCount() const
{
    return m_selectedNodes.size();
}

bool Selection::IsMultiSelection() const
{
    return m_selectedNodes.size() > 1;
}

std::string Selection::GetSelectedTypeName() const
{
    if (m_selectedNode.empty()) {
        return "";
    }
    return m_selectedNode.name();
}

void Selection::AddSelectionChangedCallback(SelectionChangedCallback callback)
{
    m_callbacks.push_back(callback);
}

void Selection::NotifySelectionChanged()
{
    for (auto& cb : m_callbacks) {
        cb(m_selectedNode);
    }
}

bool Selection::IsNodeSelected(pugi::xml_node node) const
{
    return std::find_if(m_selectedNodes.begin(), m_selectedNodes.end(),
                        [&node](const pugi::xml_node& n) { return n == node; })
           != m_selectedNodes.end();
}

} // namespace creator
