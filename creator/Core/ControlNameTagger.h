#ifndef CREATOR_CORE_CONTROL_NAME_TAGGER_H_
#define CREATOR_CORE_CONTROL_NAME_TAGGER_H_

#include "duilib/third_party/xml/pugixml.hpp"
#include <string>
#include <map>

namespace creator {

/** XML 节点 ↔ 渲染控件映射
 *  给 XML 节点添加临时 __cr_N name 属性，渲染后通过 FindControl 反向查找。
 */
class ControlNameTagger
{
public:
    ControlNameTagger();
    ~ControlNameTagger();

    /** 递归遍历 XML 树，给每个节点设置 name="__cr_N" 属性
     *  已有 name 的节点用 __cr_orig_name 备份原始值
     */
    void TagNodes(pugi::xml_node root);

    /** 移除所有 __cr_ 前缀的 name 属性，恢复原始 name */
    void UntagNodes(pugi::xml_node root);

    /** 通过 creator_id 查找对应 xml_node */
    pugi::xml_node FindNodeByCreatorId(pugi::xml_node root, const std::string& creatorId) const;

    /** 获取 ID → Node 映射表 */
    const std::map<std::string, pugi::xml_node>& GetIdToNodeMap() const { return m_idToNode; }

    /** 重置计数器和映射表 */
    void Reset();

    /** 获取下一个 creator id（不递增计数器） */
    std::string PeekNextId() const;

private:
    void TagNodeRecursive(pugi::xml_node node);
    void UntagNodeRecursive(pugi::xml_node node);

    std::map<std::string, pugi::xml_node> m_idToNode;
    int32_t m_nextId = 0;
};

} // namespace creator

#endif // CREATOR_CORE_CONTROL_NAME_TAGGER_H_
