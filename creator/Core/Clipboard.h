#ifndef CREATOR_CORE_CLIPBOARD_H_
#define CREATOR_CORE_CLIPBOARD_H_

#include "duilib/third_party/xml/pugixml.hpp"
#include <string>

namespace creator {

/** 控件剪贴板
 *  支持复制/剪切/粘贴 XML 节点。
 *  使用 XML 字符串作为内部存储格式。
 */
class Clipboard
{
public:
    Clipboard();
    ~Clipboard();

    /** 复制节点到剪贴板 */
    bool Copy(pugi::xml_node node);

    /** 是否有可粘贴的内容 */
    bool HasContent() const;

    /** 将剪贴板内容粘贴到目标节点下
     * @param targetParent 目标父节点
     * @return 新创建的节点（空节点表示失败）
     */
    pugi::xml_node Paste(pugi::xml_node targetParent);

    /** 获取剪贴板中的 XML 内容 */
    const std::string& GetContent() const { return m_xmlContent; }

    /** 清空剪贴板 */
    void Clear();

private:
    std::string m_xmlContent;
};

} // namespace creator

#endif // CREATOR_CORE_CLIPBOARD_H_
