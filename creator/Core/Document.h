#ifndef CREATOR_CORE_DOCUMENT_H_
#define CREATOR_CORE_DOCUMENT_H_

#include "duilib/duilib.h"
#include "Core/UndoManager.h"
#include <functional>
#include <vector>
#include <string>

// 使用 pugixml（duilib 第三方依赖已包含）
#include "duilib/third_party/xml/pugixml.hpp"

namespace creator {

/** 文档变更类型 */
enum class ChangeType
{
    kNodeAdded,        // 节点被添加
    kNodeRemoved,      // 节点被删除
    kNodeMoved,        // 节点被移动
    kAttributeChanged, // 属性被修改
    kDocumentLoaded,   // 文档被加载
    kDocumentCleared,  // 文档被清空
};

/** 文档观察者回调 */
using DocumentObserverCallback = std::function<void(ChangeType type, pugi::xml_node node)>;

/** 文档模型
 *  核心数据层，管理 XML DOM 树，所有面板通过它交互。
 *  采用观察者模式通知变更。
 */
class Document
{
public:
    Document();
    ~Document();

    /** 从文件加载 XML */
    bool LoadFromFile(const DString& filePath);

    /** 保存到文件 */
    bool SaveToFile(const DString& filePath) const;

    /** 从 XML 字符串加载 */
    bool LoadFromString(const std::string& xmlContent);

    /** 导出为 XML 字符串 */
    std::string ToXmlString() const;

    /** 创建新的空白文档（默认 Window 模板） */
    void CreateNewDocument();

    /** 获取根节点 */
    pugi::xml_node GetRootNode() const;

    /** 添加控件节点 */
    pugi::xml_node AddControl(pugi::xml_node parent, const DString& controlType);

    /** 删除控件节点 */
    bool RemoveControl(pugi::xml_node node);

    /** 设置节点属性 */
    void SetNodeAttribute(pugi::xml_node node, const std::string& name, const std::string& value);

    /** 移除节点属性 */
    void RemoveNodeAttribute(pugi::xml_node node, const std::string& name);

    /** 获取节点属性 */
    std::string GetNodeAttribute(pugi::xml_node node, const std::string& name) const;

    /** 移动节点到新的父节点 */
    bool MoveControl(pugi::xml_node node, pugi::xml_node newParent, int32_t index = -1);

    /** 通过 name 属性查找节点 */
    pugi::xml_node FindNodeByName(const DString& name) const;

    /** 文档是否已修改 */
    bool IsModified() const { return m_modified; }

    /** 标记为未修改 */
    void ClearModified() { m_modified = false; }

    /** 注册观察者 */
    void AddObserver(DocumentObserverCallback callback);

    /** 获取撤销管理器 */
    UndoManager& GetUndoManager() { return m_undoMgr; }

    /** 获取底层 xml_document（用于创建副本） */
    const pugi::xml_document& GetXmlDocument() const { return m_xmlDoc; }

    /** 通知所有观察者（允许外部模块在特殊操作后触发通知） */
    void NotifyChanged(ChangeType type, pugi::xml_node node);

private:

    /** 递归查找节点 */
    pugi::xml_node FindNodeByNameRecursive(pugi::xml_node parent, const DString& name) const;

private:
    pugi::xml_document m_xmlDoc;
    UndoManager m_undoMgr;
    std::vector<DocumentObserverCallback> m_observers;
    bool m_modified = false;
};

} // namespace creator

#endif // CREATOR_CORE_DOCUMENT_H_
