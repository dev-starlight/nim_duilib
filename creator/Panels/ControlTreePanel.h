#ifndef CREATOR_PANELS_CONTROL_TREE_PANEL_H_
#define CREATOR_PANELS_CONTROL_TREE_PANEL_H_

#include "duilib/duilib.h"
#include "Core/Document.h"
#include "Core/Selection.h"
#include "Core/Clipboard.h"
#include <memory>
#include <map>
#include <functional>

namespace creator {

/** 右键菜单操作回调 */
using TreeContextMenuCallback = std::function<void(const std::string& action, const std::string& param)>;

/** 控件层级树面板
 *  使用 TreeView 展示 XML 节点层级。
 *  支持点击选中、与画布联动、右键上下文菜单。
 */
class ControlTreePanel
{
public:
    ControlTreePanel();
    ~ControlTreePanel();

    /** 初始化层级树面板
     * @param treeView  TreeView 控件
     * @param document  文档模型
     * @param selection 选择管理器
     */
    void Initialize(ui::TreeView* treeView,
                    std::shared_ptr<Document> document,
                    std::shared_ptr<Selection> selection);

    /** 设置剪贴板引用 */
    void SetClipboard(Clipboard* clipboard) { m_clipboard = clipboard; }

    /** 设置右键菜单回调 */
    void SetContextMenuCallback(TreeContextMenuCallback callback) { m_contextMenuCallback = callback; }

    /** 刷新树（重建层级） */
    void Refresh();

    /** 清空树 */
    void Clear();

private:
    /** 文档变更回调 */
    void OnDocumentChanged(ChangeType type, pugi::xml_node node);

    /** 选择变更回调 */
    void OnSelectionChanged(pugi::xml_node selectedNode);

    /** 递归构建树节点 */
    void BuildTreeNode(ui::TreeNode* parentTreeNode, pugi::xml_node xmlNode);

    /** 生成节点显示文本 */
    DString GetNodeDisplayText(pugi::xml_node xmlNode) const;

    /** 显示右键菜单 */
    void ShowContextMenu(const ui::UiPoint& point);

    /** 处理右键菜单项选中 */
    void OnContextMenuItemClicked(const DString& itemName);

private:
    ui::TreeView* m_treeView = nullptr;
    std::shared_ptr<Document> m_document;
    std::shared_ptr<Selection> m_selection;
    Clipboard* m_clipboard = nullptr;

    // XML 节点 -> TreeNode 的映射（方便选择联动）
    std::map<std::string, ui::TreeNode*> m_nodeMap;
    int32_t m_nodeIdCounter = 0;

    // 右键菜单回调
    TreeContextMenuCallback m_contextMenuCallback;
};

} // namespace creator

#endif // CREATOR_PANELS_CONTROL_TREE_PANEL_H_
