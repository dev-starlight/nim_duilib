#include "ControlTreePanel.h"
#include <sstream>

namespace creator {

ControlTreePanel::ControlTreePanel()
{
}

ControlTreePanel::~ControlTreePanel()
{
}

void ControlTreePanel::Initialize(ui::TreeView* treeView,
                                   std::shared_ptr<Document> document,
                                   std::shared_ptr<Selection> selection)
{
    m_treeView = treeView;
    m_document = document;
    m_selection = selection;

    if (!m_document) {
        return;
    }

    // 监听文档变更
    m_document->AddObserver([this](ChangeType type, pugi::xml_node node) {
        OnDocumentChanged(type, node);
    });

    // 监听选择变更
    if (m_selection) {
        m_selection->AddSelectionChangedCallback([this](pugi::xml_node node) {
            OnSelectionChanged(node);
        });
    }

    // 注册右键菜单事件
    if (m_treeView) {
        m_treeView->AttachRButtonUp([this](const ui::EventArgs& args) {
            // 获取鼠标位置（转换为屏幕坐标）
            ui::UiPoint pt = args.ptMouse;
            ui::Window* pWindow = m_treeView->GetWindow();
            if (pWindow) {
                pWindow->ClientToScreen(pt);
            }
            ShowContextMenu(pt);
            return true;
        });
    }
}

void ControlTreePanel::Refresh()
{
    if (!m_treeView || !m_document) {
        return;
    }

    Clear();

    pugi::xml_node root = m_document->GetRootNode();
    if (root.empty()) {
        return;
    }

    // 获取根 TreeNode
    ui::TreeNode* rootTreeNode = m_treeView->GetRootNode();
    if (!rootTreeNode) {
        return;
    }

    // 为 XML 根节点创建树节点
    ui::Window* pWindow = m_treeView->GetWindow();
    if (!pWindow) {
        return;
    }

    ui::TreeNode* xmlRootNode = new ui::TreeNode(pWindow);
    xmlRootNode->SetText(GetNodeDisplayText(root));
    xmlRootNode->SetFixedHeight(ui::UiFixedInt(24), true, true);
    xmlRootNode->SetFontId(_T("system_12"));
    xmlRootNode->SetStateTextColor(ui::kControlStateNormal, _T("#FFCCCCCC"));
    xmlRootNode->SetTextPadding(ui::UiPadding(4, 0, 0, 0), true);
    xmlRootNode->SetStateColor(ui::kControlStateHot, _T("#FF3E3E42"));
    xmlRootNode->SetStateColor(ui::kControlStatePushed, _T("#FF094771"));

    // 点击事件：选中根节点
    pugi::xml_node capturedRoot = root;
    xmlRootNode->AttachClick([this, capturedRoot](const ui::EventArgs& /*args*/) {
        if (m_selection) {
            m_selection->Select(capturedRoot);
        }
        return true;
    });

    rootTreeNode->AddChildNode(xmlRootNode);
    xmlRootNode->SetExpand(true);

    // 记录映射
    std::string rootKey = std::string(root.name()) + "_root";
    m_nodeMap[rootKey] = xmlRootNode;

    // 递归构建子节点
    for (pugi::xml_node child = root.first_child(); child; child = child.next_sibling()) {
        BuildTreeNode(xmlRootNode, child);
    }
}

void ControlTreePanel::Clear()
{
    if (m_treeView) {
        m_treeView->RemoveAllNodes();
    }
    m_nodeMap.clear();
    m_nodeIdCounter = 0;
}

void ControlTreePanel::OnDocumentChanged(ChangeType /*type*/, pugi::xml_node /*node*/)
{
    Refresh();
}

void ControlTreePanel::OnSelectionChanged(pugi::xml_node /*selectedNode*/)
{
    // TODO: 在树中高亮选中的节点
}

void ControlTreePanel::BuildTreeNode(ui::TreeNode* parentTreeNode, pugi::xml_node xmlNode)
{
    if (parentTreeNode == nullptr || xmlNode.empty()) {
        return;
    }

    ui::Window* pWindow = m_treeView->GetWindow();
    if (!pWindow) {
        return;
    }

    ui::TreeNode* treeNode = new ui::TreeNode(pWindow);
    treeNode->SetText(GetNodeDisplayText(xmlNode));
    treeNode->SetFixedHeight(ui::UiFixedInt(22), true, true);
    treeNode->SetFontId(_T("system_12"));
    treeNode->SetStateTextColor(ui::kControlStateNormal, _T("#FFCCCCCC"));
    treeNode->SetTextPadding(ui::UiPadding(4, 0, 0, 0), true);
    treeNode->SetStateColor(ui::kControlStateHot, _T("#FF3E3E42"));
    treeNode->SetStateColor(ui::kControlStatePushed, _T("#FF094771"));

    // 点击事件
    pugi::xml_node capturedNode = xmlNode;
    treeNode->AttachClick([this, capturedNode](const ui::EventArgs& /*args*/) {
        if (m_selection) {
            m_selection->Select(capturedNode);
        }
        return true;
    });

    parentTreeNode->AddChildNode(treeNode);

    // 记录映射
    std::string nodeKey = std::string(xmlNode.name()) + "_" + std::to_string(m_nodeIdCounter++);
    std::string nameAttr = xmlNode.attribute("name").as_string();
    if (!nameAttr.empty()) {
        nodeKey = nameAttr;
    }
    m_nodeMap[nodeKey] = treeNode;

    // 递归构建子节点
    bool hasChildren = false;
    for (pugi::xml_node child = xmlNode.first_child(); child; child = child.next_sibling()) {
        BuildTreeNode(treeNode, child);
        hasChildren = true;
    }

    // 如果有子节点则默认展开
    if (hasChildren) {
        treeNode->SetExpand(true);
    }
}

DString ControlTreePanel::GetNodeDisplayText(pugi::xml_node xmlNode) const
{
    if (xmlNode.empty()) {
        return _T("(空)");
    }

    std::string nodeName = xmlNode.name();
    std::string nameAttr = xmlNode.attribute("name").as_string();
    std::string textAttr = xmlNode.attribute("text").as_string();

    DString display = ui::StringUtil::MBCSToT(nodeName);

    if (!nameAttr.empty()) {
        display += _T(" [");
        display += ui::StringUtil::MBCSToT(nameAttr);
        display += _T("]");
    }
    else if (!textAttr.empty()) {
        // 截取前 20 个字符
        if (textAttr.length() > 20) {
            textAttr = textAttr.substr(0, 20) + "...";
        }
        display += _T(" \"");
        display += ui::StringUtil::MBCSToT(textAttr);
        display += _T("\"");
    }

    return display;
}

void ControlTreePanel::ShowContextMenu(const ui::UiPoint& point)
{
    if (!m_treeView) {
        return;
    }

    ui::Window* pParentWindow = m_treeView->GetWindow();
    if (!pParentWindow) {
        return;
    }

    ui::Menu* menu = new ui::Menu(pParentWindow);
    menu->SetSkinFolder(pParentWindow->GetResourcePath().ToString());
    DString xml(_T("menu/tree_context_menu.xml"));
    menu->ShowMenu(xml, point);

    // 注册菜单项点击回调
    menu->AttachMenuItemActivated([this](const DString& /*menuName*/, int32_t /*nMenuLevel*/,
                                         const DString& itemName, size_t /*nItemIndex*/) {
        OnContextMenuItemClicked(itemName);
    });
}

void ControlTreePanel::OnContextMenuItemClicked(const DString& itemName)
{
    std::string action = ui::StringUtil::TToMBCS(itemName);

    if (action == "menu_add_label") {
        if (m_contextMenuCallback) {
            m_contextMenuCallback("add", "Label");
        }
    }
    else if (action == "menu_add_button") {
        if (m_contextMenuCallback) {
            m_contextMenuCallback("add", "Button");
        }
    }
    else if (action == "menu_add_richedit") {
        if (m_contextMenuCallback) {
            m_contextMenuCallback("add", "RichEdit");
        }
    }
    else if (action == "menu_add_hbox") {
        if (m_contextMenuCallback) {
            m_contextMenuCallback("add", "HBox");
        }
    }
    else if (action == "menu_add_vbox") {
        if (m_contextMenuCallback) {
            m_contextMenuCallback("add", "VBox");
        }
    }
    else if (action == "menu_copy") {
        if (m_contextMenuCallback) {
            m_contextMenuCallback("copy", "");
        }
    }
    else if (action == "menu_cut") {
        if (m_contextMenuCallback) {
            m_contextMenuCallback("cut", "");
        }
    }
    else if (action == "menu_paste") {
        if (m_contextMenuCallback) {
            m_contextMenuCallback("paste", "");
        }
    }
    else if (action == "menu_delete") {
        if (m_contextMenuCallback) {
            m_contextMenuCallback("delete", "");
        }
    }
}

} // namespace creator
