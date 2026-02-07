#ifndef CREATOR_CORE_CONTROL_METADATA_H_
#define CREATOR_CORE_CONTROL_METADATA_H_

#include "duilib/duilib.h"
#include <string>
#include <vector>
#include <map>

namespace creator {

/** 属性类型 */
enum class PropertyType
{
    kString,   // 字符串
    kInt,      // 整数
    kFloat,    // 浮点数
    kBool,     // 布尔值
    kColor,    // 颜色值
    kCombo,    // 下拉选择
    kFont,     // 字体
    kImage,    // 图片路径
    kSize,     // 尺寸 (如 "100,200")
    kRect,     // 矩形 (如 "0,0,100,100")
    kMargin,   // 边距 (如 "4,4,4,4")
};

/** 属性定义 */
struct PropertyDef
{
    std::string name;          // 属性名 (如 "width")
    std::string displayName;   // 显示名 (如 "宽度")
    std::string group;         // 分组 (如 "布局")
    PropertyType type;         // 类型
    std::string defaultValue;  // 默认值
    std::vector<std::string> options; // 下拉选项（仅 kCombo 类型使用）
    std::string description;   // 属性描述

    PropertyDef() : type(PropertyType::kString) {}
    PropertyDef(const std::string& n, const std::string& dn, const std::string& g,
                PropertyType t, const std::string& dv = "",
                const std::vector<std::string>& opts = {},
                const std::string& desc = "")
        : name(n), displayName(dn), group(g), type(t),
          defaultValue(dv), options(opts), description(desc) {}
};

/** 控件分类 */
struct ControlCategory
{
    std::string categoryName;  // 分类名称
    std::vector<std::string> controlTypes; // 该分类下的控件类型名列表
};

/** 控件元数据注册表
 *  定义每个控件支持的属性、类型和默认值。
 *  连接画布和属性面板的桥梁。
 */
class ControlMetadata
{
public:
    ControlMetadata();
    ~ControlMetadata();

    /** 初始化：注册所有内置控件元数据 */
    void Initialize();

    /** 注册一个控件的属性列表 */
    void RegisterControl(const std::string& controlType,
                         const std::vector<PropertyDef>& properties);

    /** 获取指定控件的属性列表（包含继承自基类的属性） */
    std::vector<PropertyDef> GetProperties(const std::string& controlType) const;

    /** 检查控件类型是否已注册 */
    bool IsRegistered(const std::string& controlType) const;

    /** 获取所有注册的控件类型 */
    std::vector<std::string> GetAllControlTypes() const;

    /** 获取控件分类列表 */
    const std::vector<ControlCategory>& GetCategories() const { return m_categories; }

    /** 判断一个控件类型是否是容器 */
    bool IsContainer(const std::string& controlType) const;

private:
    /** 注册基础控件属性（所有控件共有） */
    void RegisterBaseProperties();

    /** 注册具体控件属性 */
    void RegisterBasicControls();
    void RegisterContainerControls();
    void RegisterInputControls();
    void RegisterListControls();
    void RegisterSpecialControls();

    /** 注册控件分类 */
    void RegisterCategories();

private:
    // 基础属性（所有控件都有）
    std::vector<PropertyDef> m_baseProperties;

    // 控件类型 -> 特有属性列表
    std::map<std::string, std::vector<PropertyDef>> m_registry;

    // 控件分类
    std::vector<ControlCategory> m_categories;

    // 容器类型集合
    std::vector<std::string> m_containerTypes;
};

} // namespace creator

#endif // CREATOR_CORE_CONTROL_METADATA_H_
