#include "ControlMetadata.h"

namespace creator {

ControlMetadata::ControlMetadata()
{
}

ControlMetadata::~ControlMetadata()
{
}

void ControlMetadata::Initialize()
{
    RegisterBaseProperties();
    RegisterBasicControls();
    RegisterContainerControls();
    RegisterInputControls();
    RegisterListControls();
    RegisterSpecialControls();
    RegisterCategories();
}

void ControlMetadata::RegisterControl(const std::string& controlType,
                                      const std::vector<PropertyDef>& properties)
{
    m_registry[controlType] = properties;
}

std::vector<PropertyDef> ControlMetadata::GetProperties(const std::string& controlType) const
{
    // 先添加基础属性
    std::vector<PropertyDef> result = m_baseProperties;

    // 再添加控件特有属性
    auto it = m_registry.find(controlType);
    if (it != m_registry.end()) {
        result.insert(result.end(), it->second.begin(), it->second.end());
    }

    return result;
}

bool ControlMetadata::IsRegistered(const std::string& controlType) const
{
    return m_registry.find(controlType) != m_registry.end();
}

std::vector<std::string> ControlMetadata::GetAllControlTypes() const
{
    std::vector<std::string> types;
    for (auto& pair : m_registry) {
        types.push_back(pair.first);
    }
    return types;
}

bool ControlMetadata::IsContainer(const std::string& controlType) const
{
    for (auto& t : m_containerTypes) {
        if (t == controlType) {
            return true;
        }
    }
    return false;
}

// ============================================================
// 基础属性（所有控件共有）
// ============================================================

void ControlMetadata::RegisterBaseProperties()
{
    m_baseProperties = {
        // 基本属性组
        {"name",        "名称",     "基本",   PropertyType::kString, "", {}, "控件的唯一标识名称"},
        {"class",       "样式类",   "基本",   PropertyType::kString, "", {}, "引用 global.xml 中定义的样式类"},
        {"visible",     "可见",     "基本",   PropertyType::kBool,   "true", {}, "控件是否可见"},
        {"enabled",     "启用",     "基本",   PropertyType::kBool,   "true", {}, "控件是否启用"},
        {"mouse_enabled", "鼠标响应", "基本", PropertyType::kBool,   "true", {}, "是否响应鼠标事件"},
        {"keyboard_enabled", "键盘响应", "基本", PropertyType::kBool, "true", {}, "是否响应键盘事件"},
        {"no_focus",    "禁止焦点", "基本",   PropertyType::kBool,   "false", {}, "控件是否不可获取焦点"},
        {"tooltip_text", "提示文本", "基本",  PropertyType::kString, "", {}, "鼠标悬停提示文本"},

        // 布局属性组
        {"width",       "宽度",     "布局",   PropertyType::kString, "stretch", {}, "控件宽度 (数值/stretch/auto/百分比)"},
        {"height",      "高度",     "布局",   PropertyType::kString, "stretch", {}, "控件高度 (数值/stretch/auto/百分比)"},
        {"min_width",   "最小宽度", "布局",   PropertyType::kInt,    "0", {}, "最小宽度"},
        {"min_height",  "最小高度", "布局",   PropertyType::kInt,    "0", {}, "最小高度"},
        {"max_width",   "最大宽度", "布局",   PropertyType::kInt,    "0", {}, "最大宽度（0表示不限制）"},
        {"max_height",  "最大高度", "布局",   PropertyType::kInt,    "0", {}, "最大高度（0表示不限制）"},
        {"halign",      "水平对齐", "布局",   PropertyType::kCombo,  "left",
            {"left", "center", "right"}, "水平对齐方式"},
        {"valign",      "垂直对齐", "布局",   PropertyType::kCombo,  "top",
            {"top", "center", "bottom"}, "垂直对齐方式"},
        {"margin",      "外边距",   "布局",   PropertyType::kMargin, "0,0,0,0", {}, "外边距 (左,上,右,下)"},
        {"padding",     "内边距",   "布局",   PropertyType::kMargin, "0,0,0,0", {}, "内边距 (左,上,右,下)"},
        {"float",       "浮动",     "布局",   PropertyType::kBool,   "false", {}, "是否使用绝对定位"},

        // 外观属性组
        {"bkcolor",     "背景色",   "外观",   PropertyType::kColor,  "", {}, "背景颜色"},
        {"bkimage",     "背景图",   "外观",   PropertyType::kImage,  "", {}, "背景图片"},
        {"border_size", "边框大小", "外观",   PropertyType::kString, "0", {}, "边框大小"},
        {"border_color", "边框颜色", "外观",  PropertyType::kColor,  "", {}, "边框颜色"},
        {"border_round", "圆角",    "外观",   PropertyType::kString, "0,0", {}, "圆角半径 (x,y)"},
        {"alpha",       "透明度",   "外观",   PropertyType::kInt,    "255", {}, "透明度 (0-255)"},
        {"cursor_type", "鼠标样式", "外观",   PropertyType::kCombo,  "arrow",
            {"arrow", "ibeam", "hand", "wait", "cross", "size_we", "size_ns", "size_nwse", "size_nesw"}, "鼠标光标样式"},
    };
}

// ============================================================
// 基础控件
// ============================================================

void ControlMetadata::RegisterBasicControls()
{
    // Control - 最基本的控件
    RegisterControl("Control", {});

    // Label - 标签
    RegisterControl("Label", {
        {"text",            "文本",     "标签",   PropertyType::kString, "", {}, "显示文本内容"},
        {"text_id",         "文本ID",   "标签",   PropertyType::kString, "", {}, "多语言文本ID"},
        {"font",            "字体",     "标签",   PropertyType::kFont,   "", {}, "字体名称"},
        {"text_align",      "文本对齐", "标签",   PropertyType::kCombo,  "left",
            {"left", "right", "hcenter", "top", "bottom", "vcenter",
             "left,vcenter", "hcenter,vcenter", "right,vcenter",
             "left,top", "hcenter,top", "right,top"}, "文本对齐方式"},
        {"text_padding",    "文本边距", "标签",   PropertyType::kMargin, "0,0,0,0", {}, "文本内边距"},
        {"normal_text_color", "文本颜色", "标签", PropertyType::kColor,  "", {}, "正常状态文本颜色"},
        {"single_line",     "单行",     "标签",   PropertyType::kBool,   "true", {}, "是否单行显示"},
        {"auto_tooltip",    "自动提示", "标签",   PropertyType::kBool,   "false", {}, "文本超出时自动显示提示"},
    });

    // Button - 按钮
    RegisterControl("Button", {
        {"text",            "文本",     "按钮",   PropertyType::kString, "", {}, "按钮文本"},
        {"font",            "字体",     "按钮",   PropertyType::kFont,   "", {}, "字体名称"},
        {"text_align",      "文本对齐", "按钮",   PropertyType::kCombo,  "hcenter,vcenter",
            {"left,vcenter", "hcenter,vcenter", "right,vcenter"}, "文本对齐方式"},
        {"text_padding",    "文本边距", "按钮",   PropertyType::kMargin, "0,0,0,0", {}, "文本内边距"},
        {"normal_text_color", "文本颜色", "按钮", PropertyType::kColor,  "", {}, "正常状态文本颜色"},
        {"hot_text_color",  "悬停文本色", "按钮", PropertyType::kColor,  "", {}, "鼠标悬停文本颜色"},
        {"pushed_text_color", "按下文本色", "按钮", PropertyType::kColor, "", {}, "按下状态文本颜色"},
        {"normal_color",    "正常背景", "按钮",   PropertyType::kColor,  "", {}, "正常状态背景色"},
        {"hot_color",       "悬停背景", "按钮",   PropertyType::kColor,  "", {}, "悬停状态背景色"},
        {"pushed_color",    "按下背景", "按钮",   PropertyType::kColor,  "", {}, "按下状态背景色"},
        {"normal_image",    "正常图片", "按钮",   PropertyType::kImage,  "", {}, "正常状态图片"},
        {"hot_image",       "悬停图片", "按钮",   PropertyType::kImage,  "", {}, "悬停状态图片"},
        {"pushed_image",    "按下图片", "按钮",   PropertyType::kImage,  "", {}, "按下状态图片"},
        {"disabled_image",  "禁用图片", "按钮",   PropertyType::kImage,  "", {}, "禁用状态图片"},
    });

    // CheckBox - 复选框
    RegisterControl("CheckBox", {
        {"text",            "文本",     "复选框", PropertyType::kString, "", {}, "复选框文本"},
        {"font",            "字体",     "复选框", PropertyType::kFont,   "", {}, "字体名称"},
        {"selected",        "选中",     "复选框", PropertyType::kBool,   "false", {}, "是否选中"},
        {"normal_image",    "未选图片", "复选框", PropertyType::kImage,  "", {}, "未选中状态图片"},
        {"selected_normal_image", "选中图片", "复选框", PropertyType::kImage, "", {}, "选中状态图片"},
    });

    // Option - 单选框
    RegisterControl("Option", {
        {"text",            "文本",     "单选框", PropertyType::kString, "", {}, "单选框文本"},
        {"font",            "字体",     "单选框", PropertyType::kFont,   "", {}, "字体名称"},
        {"group",           "分组",     "单选框", PropertyType::kString, "", {}, "单选组名称"},
        {"selected",        "选中",     "单选框", PropertyType::kBool,   "false", {}, "是否选中"},
        {"normal_image",    "未选图片", "单选框", PropertyType::kImage,  "", {}, "未选中状态图片"},
        {"selected_normal_image", "选中图片", "单选框", PropertyType::kImage, "", {}, "选中状态图片"},
    });

    // HyperLink - 超链接
    RegisterControl("HyperLink", {
        {"text",            "文本",     "超链接", PropertyType::kString, "", {}, "显示文本"},
        {"url",             "链接",     "超链接", PropertyType::kString, "", {}, "URL 链接"},
        {"font",            "字体",     "超链接", PropertyType::kFont,   "", {}, "字体名称"},
        {"normal_text_color", "文本颜色", "超链接", PropertyType::kColor, "", {}, "正常文本颜色"},
        {"hot_text_color",  "悬停颜色", "超链接", PropertyType::kColor,  "", {}, "悬停文本颜色"},
    });

    // Line - 分割线
    RegisterControl("Line", {
        {"line_type",       "方向",     "分割线", PropertyType::kCombo,  "horizontal",
            {"horizontal", "vertical"}, "线的方向"},
        {"line_color",      "线颜色",   "分割线", PropertyType::kColor,  "", {}, "线的颜色"},
        {"line_width",      "线宽",     "分割线", PropertyType::kInt,    "1", {}, "线的宽度"},
        {"dash_style",      "线型",     "分割线", PropertyType::kCombo,  "solid",
            {"solid", "dash", "dot", "dash_dot"}, "线的样式"},
    });

    // Progress - 进度条
    RegisterControl("Progress", {
        {"value",           "当前值",   "进度条", PropertyType::kInt,    "0", {}, "当前进度值"},
        {"min",             "最小值",   "进度条", PropertyType::kInt,    "0", {}, "最小值"},
        {"max",             "最大值",   "进度条", PropertyType::kInt,    "100", {}, "最大值"},
        {"horizontal",      "水平",     "进度条", PropertyType::kBool,   "true", {}, "是否水平方向"},
        {"progress_image",  "进度图片", "进度条", PropertyType::kImage,  "", {}, "进度条前景图片"},
        {"progress_color",  "进度颜色", "进度条", PropertyType::kColor,  "", {}, "进度条前景颜色"},
    });

    // Slider - 滑块
    RegisterControl("Slider", {
        {"value",           "当前值",   "滑块",   PropertyType::kInt,    "0", {}, "当前值"},
        {"min",             "最小值",   "滑块",   PropertyType::kInt,    "0", {}, "最小值"},
        {"max",             "最大值",   "滑块",   PropertyType::kInt,    "100", {}, "最大值"},
        {"thumb_size",      "滑块大小", "滑块",   PropertyType::kString, "10,20", {}, "滑块大小"},
        {"progress_color",  "进度颜色", "滑块",   PropertyType::kColor,  "", {}, "进度条颜色"},
    });
}

// ============================================================
// 容器控件
// ============================================================

void ControlMetadata::RegisterContainerControls()
{
    m_containerTypes = {
        "Box", "HBox", "VBox", "TabBox", "GridBox",
        "ScrollBox", "VScrollBox", "HScrollBox",
        "ListBox", "VListBox", "HListBox",
        "TileBox", "HTileBox", "VTileBox",
        "SplitBox", "GroupBox",
        "Window"
    };

    // Box - 基础容器
    RegisterControl("Box", {
        {"child_halign",    "子项水平对齐", "容器", PropertyType::kCombo, "left",
            {"left", "center", "right"}, "子控件水平对齐"},
        {"child_valign",    "子项垂直对齐", "容器", PropertyType::kCombo, "top",
            {"top", "center", "bottom"}, "子控件垂直对齐"},
    });

    // HBox - 水平容器
    RegisterControl("HBox", {
        {"child_margin_x",  "子项水平间距", "容器", PropertyType::kInt,   "0", {}, "子控件水平间距"},
        {"child_halign",    "子项水平对齐", "容器", PropertyType::kCombo, "left",
            {"left", "center", "right"}, "子控件水平对齐"},
        {"child_valign",    "子项垂直对齐", "容器", PropertyType::kCombo, "top",
            {"top", "center", "bottom"}, "子控件垂直对齐"},
    });

    // VBox - 垂直容器
    RegisterControl("VBox", {
        {"child_margin_y",  "子项垂直间距", "容器", PropertyType::kInt,   "0", {}, "子控件垂直间距"},
        {"child_halign",    "子项水平对齐", "容器", PropertyType::kCombo, "left",
            {"left", "center", "right"}, "子控件水平对齐"},
        {"child_valign",    "子项垂直对齐", "容器", PropertyType::kCombo, "top",
            {"top", "center", "bottom"}, "子控件垂直对齐"},
    });

    // TabBox - 标签页容器
    RegisterControl("TabBox", {
        {"selected_id",     "选中索引", "标签页", PropertyType::kInt,    "0", {}, "默认选中的页面索引"},
        {"fade_switch",     "切换动画", "标签页", PropertyType::kBool,   "false", {}, "切换时是否有动画"},
    });

    // GridBox - 网格容器
    RegisterControl("GridBox", {
        {"columns",         "列数",     "网格",   PropertyType::kInt,    "2", {}, "网格列数"},
        {"child_margin_x",  "子项水平间距", "网格", PropertyType::kInt,  "0", {}, "子控件水平间距"},
        {"child_margin_y",  "子项垂直间距", "网格", PropertyType::kInt,  "0", {}, "子控件垂直间距"},
    });

    // ScrollBox - 滚动容器
    RegisterControl("ScrollBox", {
        {"vscrollbar",      "垂直滚动条", "滚动",  PropertyType::kBool,  "false", {}, "是否显示垂直滚动条"},
        {"hscrollbar",      "水平滚动条", "滚动",  PropertyType::kBool,  "false", {}, "是否显示水平滚动条"},
    });

    // VScrollBox
    RegisterControl("VScrollBox", {
        {"vscrollbar",      "垂直滚动条", "滚动",  PropertyType::kBool,  "true", {}, "是否显示垂直滚动条"},
        {"hscrollbar",      "水平滚动条", "滚动",  PropertyType::kBool,  "false", {}, "是否显示水平滚动条"},
    });

    // HScrollBox
    RegisterControl("HScrollBox", {
        {"vscrollbar",      "垂直滚动条", "滚动",  PropertyType::kBool,  "false", {}, "是否显示垂直滚动条"},
        {"hscrollbar",      "水平滚动条", "滚动",  PropertyType::kBool,  "true", {}, "是否显示水平滚动条"},
    });
}

// ============================================================
// 输入控件
// ============================================================

void ControlMetadata::RegisterInputControls()
{
    // RichEdit - 富文本编辑框
    RegisterControl("RichEdit", {
        {"text",            "文本",     "编辑框", PropertyType::kString, "", {}, "编辑框文本"},
        {"font",            "字体",     "编辑框", PropertyType::kFont,   "", {}, "字体名称"},
        {"text_align",      "文本对齐", "编辑框", PropertyType::kCombo,  "left,vcenter",
            {"left,vcenter", "hcenter,vcenter", "right,vcenter", "left,top"}, "文本对齐"},
        {"text_padding",    "文本边距", "编辑框", PropertyType::kMargin, "0,0,0,0", {}, "文本内边距"},
        {"multi_line",      "多行",     "编辑框", PropertyType::kBool,   "true", {}, "是否多行编辑"},
        {"rich_text",       "富文本",   "编辑框", PropertyType::kBool,   "true", {}, "是否富文本模式"},
        {"readonly",        "只读",     "编辑框", PropertyType::kBool,   "false", {}, "是否只读"},
        {"password",        "密码模式", "编辑框", PropertyType::kBool,   "false", {}, "是否密码模式"},
        {"number_only",     "仅数字",   "编辑框", PropertyType::kBool,   "false", {}, "是否仅允许数字"},
        {"want_return",     "接收回车", "编辑框", PropertyType::kBool,   "true", {}, "是否接收回车键"},
        {"want_tab",        "接收Tab",  "编辑框", PropertyType::kBool,   "false", {}, "是否接收Tab键"},
        {"vscrollbar",      "垂直滚动条", "编辑框", PropertyType::kBool, "false", {}, "显示垂直滚动条"},
        {"hscrollbar",      "水平滚动条", "编辑框", PropertyType::kBool, "false", {}, "显示水平滚动条"},
        {"prompt_mode",     "提示模式", "编辑框", PropertyType::kBool,   "false", {}, "空时显示提示文字"},
        {"prompt_text",     "提示文字", "编辑框", PropertyType::kString, "", {}, "提示文字内容"},
        {"prompt_color",    "提示颜色", "编辑框", PropertyType::kColor,  "", {}, "提示文字颜色"},
        {"max_char",        "最大字符数", "编辑框", PropertyType::kInt,  "0", {}, "最大字符数（0无限制）"},
        {"normal_text_color", "文本颜色", "编辑框", PropertyType::kColor, "", {}, "文本颜色"},
    });

    // Combo - 下拉框
    RegisterControl("Combo", {
        {"font",            "字体",     "下拉框", PropertyType::kFont,   "", {}, "字体名称"},
        {"text_padding",    "文本边距", "下拉框", PropertyType::kMargin, "0,0,0,0", {}, "文本内边距"},
        {"drop_box_size",   "下拉尺寸", "下拉框", PropertyType::kString, "0,200", {}, "下拉框大小 (宽,高)"},
    });

    // FilterCombo - 过滤下拉框
    RegisterControl("FilterCombo", {
        {"font",            "字体",     "过滤下拉", PropertyType::kFont,  "", {}, "字体名称"},
        {"text_padding",    "文本边距", "过滤下拉", PropertyType::kMargin, "0,0,0,0", {}, "文本内边距"},
    });
}

// ============================================================
// 列表控件
// ============================================================

void ControlMetadata::RegisterListControls()
{
    // ListBox (abstract parent, concrete types below)
    RegisterControl("VListBox", {
        {"vscrollbar",      "垂直滚动条", "列表",  PropertyType::kBool,  "true", {}, "显示垂直滚动条"},
        {"child_margin_y",  "子项垂直间距", "列表", PropertyType::kInt,  "0", {}, "子项垂直间距"},
    });

    RegisterControl("HListBox", {
        {"hscrollbar",      "水平滚动条", "列表",  PropertyType::kBool,  "true", {}, "显示水平滚动条"},
        {"child_margin_x",  "子项水平间距", "列表", PropertyType::kInt,  "0", {}, "子项水平间距"},
    });

    // TreeView
    RegisterControl("TreeView", {
        {"indent",          "缩进",     "树视图", PropertyType::kInt,    "20", {}, "缩进宽度"},
        {"vscrollbar",      "垂直滚动条", "树视图", PropertyType::kBool, "true", {}, "显示垂直滚动条"},
    });

    // ListCtrl
    RegisterControl("ListCtrl", {
        {"vscrollbar",      "垂直滚动条", "列表控件", PropertyType::kBool, "true", {}, "显示垂直滚动条"},
        {"hscrollbar",      "水平滚动条", "列表控件", PropertyType::kBool, "true", {}, "显示水平滚动条"},
    });
}

// ============================================================
// 特殊控件
// ============================================================

void ControlMetadata::RegisterSpecialControls()
{
    // DateTime
    RegisterControl("DateTime", {
        {"format",          "格式",     "日期时间", PropertyType::kString, "", {}, "日期时间格式"},
    });

    // IPAddress
    RegisterControl("IPAddress", {});

    // HotKey
    RegisterControl("HotKey", {});

    // TabCtrl
    RegisterControl("TabCtrl", {
        {"tab_box_name",    "TabBox名称", "标签页", PropertyType::kString, "", {}, "关联的TabBox名称"},
    });

    // ColorPicker - 颜色选择器
    RegisterControl("ColorPicker", {
        {"select_color",    "选中颜色",  "颜色选择", PropertyType::kColor, "", {}, "当前选中的颜色"},
    });

    // PropertyGrid - 属性网格
    RegisterControl("PropertyGrid", {
        {"vscrollbar",      "垂直滚动条", "属性网格", PropertyType::kBool, "true", {}, "显示垂直滚动条"},
    });

    // CircleProgress - 圆形进度条
    RegisterControl("CircleProgress", {
        {"value",           "当前值",   "进度条", PropertyType::kInt,    "0", {}, "当前进度值"},
        {"min",             "最小值",   "进度条", PropertyType::kInt,    "0", {}, "最小值"},
        {"max",             "最大值",   "进度条", PropertyType::kInt,    "100", {}, "最大值"},
        {"circle_width",    "环宽",     "进度条", PropertyType::kInt,    "8", {}, "圆环宽度"},
        {"progress_color",  "进度颜色", "进度条", PropertyType::kColor,  "", {}, "进度条颜色"},
        {"bg_circle_color", "背景环颜色","进度条", PropertyType::kColor, "", {}, "背景圆环颜色"},
        {"clockwise",       "顺时针",   "进度条", PropertyType::kBool,   "true", {}, "是否顺时针方向"},
    });

    // RichText - 富文本显示
    RegisterControl("RichText", {
        {"text",            "文本",     "富文本", PropertyType::kString, "", {}, "富文本内容（支持简单HTML标签）"},
        {"font",            "字体",     "富文本", PropertyType::kFont,   "", {}, "默认字体"},
        {"normal_text_color","文本颜色", "富文本", PropertyType::kColor, "", {}, "默认文本颜色"},
        {"link_text_color",  "链接颜色", "富文本", PropertyType::kColor, "", {}, "链接文本颜色"},
    });

    // CheckCombo - 多选下拉框
    RegisterControl("CheckCombo", {
        {"font",            "字体",     "多选下拉", PropertyType::kFont,   "", {}, "字体名称"},
        {"text_padding",    "文本边距", "多选下拉", PropertyType::kMargin, "0,0,0,0", {}, "文本内边距"},
        {"drop_box_size",   "下拉尺寸", "多选下拉", PropertyType::kString, "0,200", {}, "下拉框大小 (宽,高)"},
    });

    // SplitBox - 分割容器
    RegisterControl("SplitBox", {
        {"split_bar_size",  "分割条大小", "分割", PropertyType::kInt, "4", {}, "分割条宽度"},
    });

    // TileBox - 瓦片容器（水平+垂直）
    RegisterControl("TileBox", {
        {"columns",         "列数",     "瓦片",   PropertyType::kInt,    "0", {}, "列数（0 为自动计算）"},
        {"item_size",       "项目大小", "瓦片",   PropertyType::kString, "80,80", {}, "每个瓦片的尺寸 (宽,高)"},
        {"child_margin_x",  "水平间距", "瓦片",   PropertyType::kInt,    "0", {}, "子项水平间距"},
        {"child_margin_y",  "垂直间距", "瓦片",   PropertyType::kInt,    "0", {}, "子项垂直间距"},
    });
    RegisterControl("VTileBox", {
        {"columns",         "列数",     "瓦片",   PropertyType::kInt,    "0", {}, "列数"},
        {"item_size",       "项目大小", "瓦片",   PropertyType::kString, "80,80", {}, "每个瓦片的尺寸"},
    });
    RegisterControl("HTileBox", {
        {"columns",         "列数",     "瓦片",   PropertyType::kInt,    "0", {}, "列数"},
        {"item_size",       "项目大小", "瓦片",   PropertyType::kString, "80,80", {}, "每个瓦片的尺寸"},
    });

    // Split - 分割线
    RegisterControl("Split", {
        {"is_sep_size",     "分割大小", "分割", PropertyType::kInt, "4", {}, "分割线大小"},
    });

    // GroupBox - 分组框
    RegisterControl("GroupBox", {
        {"text",            "标题",     "分组框", PropertyType::kString, "", {}, "分组框标题文本"},
        {"font",            "字体",     "分组框", PropertyType::kFont,   "", {}, "标题字体"},
    });
}

// ============================================================
// 控件分类
// ============================================================

void ControlMetadata::RegisterCategories()
{
    m_categories = {
        {"基础控件", {"Control", "Label", "Button", "CheckBox", "Option", "HyperLink", "Line", "Progress", "Slider", "CircleProgress", "RichText"}},
        {"容器控件", {"Box", "HBox", "VBox", "TabBox", "GridBox", "ScrollBox", "VScrollBox", "HScrollBox", "SplitBox", "GroupBox", "TileBox", "VTileBox", "HTileBox"}},
        {"输入控件", {"RichEdit", "Combo", "FilterCombo", "CheckCombo"}},
        {"列表控件", {"VListBox", "HListBox", "TreeView", "ListCtrl"}},
        {"特殊控件", {"DateTime", "IPAddress", "HotKey", "TabCtrl", "ColorPicker", "PropertyGrid", "Split"}},
    };
}

} // namespace creator
