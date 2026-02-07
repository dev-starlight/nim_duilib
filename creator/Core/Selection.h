#ifndef CREATOR_CORE_SELECTION_H_
#define CREATOR_CORE_SELECTION_H_

#include "duilib/duilib.h"
#include "duilib/third_party/xml/pugixml.hpp"
#include <functional>
#include <vector>
#include <algorithm>

namespace creator {

/** 选择变更回调 */
using SelectionChangedCallback = std::function<void(pugi::xml_node selectedNode)>;

/** 选择状态管理
 *  管理当前选中的控件节点，支持单选和多选，通知各面板联动更新。
 */
class Selection
{
public:
    Selection();
    ~Selection();

    /** 选中一个节点（清除之前的选择） */
    void Select(pugi::xml_node node);

    /** 追加选中一个节点（Ctrl+Click 多选） */
    void AddToSelection(pugi::xml_node node);

    /** 从选择中移除一个节点（Ctrl+Click 取消已选中的） */
    void RemoveFromSelection(pugi::xml_node node);

    /** 清除选择 */
    void ClearSelection();

    /** 获取当前选中的节点（单选时返回唯一选中节点，多选时返回第一个） */
    pugi::xml_node GetSelectedNode() const;

    /** 获取所有选中的节点 */
    std::vector<pugi::xml_node> GetSelectedNodes() const;

    /** 是否有选中节点 */
    bool HasSelection() const;

    /** 获取选中节点数量 */
    size_t GetSelectionCount() const;

    /** 是否为多选状态 */
    bool IsMultiSelection() const;

    /** 获取选中节点的类型名 */
    std::string GetSelectedTypeName() const;

    /** 注册选择变更回调 */
    void AddSelectionChangedCallback(SelectionChangedCallback callback);

private:
    /** 通知所有回调 */
    void NotifySelectionChanged();

    /** 检查节点是否已在选择列表中 */
    bool IsNodeSelected(pugi::xml_node node) const;

private:
    pugi::xml_node m_selectedNode;                  // 主选中节点（兼容单选）
    std::vector<pugi::xml_node> m_selectedNodes;    // 多选节点列表
    std::vector<SelectionChangedCallback> m_callbacks;
};

} // namespace creator

#endif // CREATOR_CORE_SELECTION_H_
