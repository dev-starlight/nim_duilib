#include "CodeGenerator.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace creator {

CodeGenerator::CodeGenerator()
{
}

CodeGenerator::~CodeGenerator()
{
}

CodeGenerator::GeneratedCode CodeGenerator::Generate(const std::shared_ptr<Document>& document) const
{
    GeneratedCode code;
    code.headerContent = GenerateHeader(document);
    code.implContent = GenerateImpl(document);
    return code;
}

std::string CodeGenerator::GenerateHeader(const std::shared_ptr<Document>& document) const
{
    std::ostringstream oss;
    std::string guard = MakeIncludeGuard();
    auto controls = CollectNamedControls(document->GetRootNode());

    oss << "#ifndef " << guard << "\n";
    oss << "#define " << guard << "\n";
    oss << "\n";
    oss << "// duilib\n";
    oss << "#include \"duilib/duilib.h\"\n";
    oss << "\n";
    oss << "/** " << m_className << " 窗口\n";
    oss << " *  由 Duilib Creator 自动生成\n";
    oss << " */\n";
    oss << "class " << m_className << " : public ui::WindowImplBase\n";
    oss << "{\n";
    oss << "    typedef ui::WindowImplBase BaseClass;\n";
    oss << "public:\n";
    oss << "    " << m_className << "();\n";
    oss << "    virtual ~" << m_className << "() override;\n";
    oss << "\n";
    oss << "    /** 资源相关接口 */\n";
    oss << "    virtual DString GetSkinFolder() override;\n";
    oss << "    virtual DString GetSkinFile() override;\n";
    oss << "\n";
    oss << "    /** 窗口初始化完成后调用 */\n";
    oss << "    virtual void OnInitWindow() override;\n";
    oss << "\n";
    oss << "private:\n";
    oss << "    /** 初始化控件引用 */\n";
    oss << "    void InitControls();\n";
    oss << "\n";
    oss << "    /** 初始化事件处理 */\n";
    oss << "    void InitEvents();\n";
    oss << "\n";

    // 控件成员变量
    if (!controls.empty()) {
        oss << "private:\n";
        oss << "    // 控件引用（由 XML 中 name 属性对应）\n";
        for (const auto& ctrl : controls) {
            std::string cppType = MapControlType(ctrl.type);
            oss << "    ui::" << cppType << "* m_" << ctrl.name << " = nullptr;\n";
        }
    }

    oss << "};\n";
    oss << "\n";
    oss << "#endif // " << guard << "\n";

    return oss.str();
}

std::string CodeGenerator::GenerateImpl(const std::shared_ptr<Document>& document) const
{
    std::ostringstream oss;
    auto controls = CollectNamedControls(document->GetRootNode());

    oss << "#include \"" << m_className << ".h\"\n";
    oss << "\n";
    oss << m_className << "::" << m_className << "()\n";
    oss << "{\n";
    oss << "}\n";
    oss << "\n";
    oss << m_className << "::~" << m_className << "()\n";
    oss << "{\n";
    oss << "}\n";
    oss << "\n";
    oss << "DString " << m_className << "::GetSkinFolder()\n";
    oss << "{\n";
    oss << "    return _T(\"" << m_skinFolder << "\");\n";
    oss << "}\n";
    oss << "\n";
    oss << "DString " << m_className << "::GetSkinFile()\n";
    oss << "{\n";
    oss << "    return _T(\"" << m_xmlFile << "\");\n";
    oss << "}\n";
    oss << "\n";
    oss << "void " << m_className << "::OnInitWindow()\n";
    oss << "{\n";
    oss << "    BaseClass::OnInitWindow();\n";
    oss << "    InitControls();\n";
    oss << "    InitEvents();\n";
    oss << "}\n";
    oss << "\n";

    // InitControls
    oss << "void " << m_className << "::InitControls()\n";
    oss << "{\n";
    for (const auto& ctrl : controls) {
        std::string cppType = MapControlType(ctrl.type);
        oss << "    m_" << ctrl.name << " = dynamic_cast<ui::" << cppType
            << "*>(FindControl(_T(\"" << ctrl.name << "\")));\n";
    }
    oss << "}\n";
    oss << "\n";

    // InitEvents
    oss << "void " << m_className << "::InitEvents()\n";
    oss << "{\n";
    // 为按钮控件自动生成点击事件
    for (const auto& ctrl : controls) {
        if (ctrl.type == "Button") {
            oss << "    if (m_" << ctrl.name << ") {\n";
            oss << "        m_" << ctrl.name << "->AttachClick([this](const ui::EventArgs& /*args*/) {\n";
            oss << "            // TODO: 处理 " << ctrl.name << " 的点击事件\n";
            oss << "            return true;\n";
            oss << "        });\n";
            oss << "    }\n";
        }
    }
    oss << "}\n";

    return oss.str();
}

std::vector<CodeGenerator::ControlInfo> CodeGenerator::CollectNamedControls(pugi::xml_node node) const
{
    std::vector<ControlInfo> result;
    if (node.empty()) {
        return result;
    }

    std::string name = node.attribute("name").as_string();
    if (!name.empty()) {
        ControlInfo info;
        info.name = name;
        info.type = node.name();
        result.push_back(info);
    }

    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
        auto childControls = CollectNamedControls(child);
        result.insert(result.end(), childControls.begin(), childControls.end());
    }

    return result;
}

std::string CodeGenerator::MapControlType(const std::string& xmlType) const
{
    // 大部分类型名直接对应
    static const std::map<std::string, std::string> typeMap = {
        {"Window",          "Window"},
        {"Control",         "Control"},
        {"Label",           "Label"},
        {"Button",          "Button"},
        {"CheckBox",        "CheckBox"},
        {"Option",          "Option"},
        {"HyperLink",       "HyperLink"},
        {"Line",            "Line"},
        {"Progress",        "Progress"},
        {"CircleProgress",  "CircleProgress"},
        {"Slider",          "Slider"},
        {"RichEdit",        "RichEdit"},
        {"RichText",        "RichText"},
        {"Combo",           "Combo"},
        {"FilterCombo",     "FilterCombo"},
        {"CheckCombo",      "CheckCombo"},
        {"Box",             "Box"},
        {"HBox",            "HBox"},
        {"VBox",            "VBox"},
        {"TabBox",          "TabBox"},
        {"GridBox",         "GridBox"},
        {"ScrollBox",       "ScrollBox"},
        {"VScrollBox",      "VScrollBox"},
        {"HScrollBox",      "HScrollBox"},
        {"VListBox",        "VListBox"},
        {"HListBox",        "HListBox"},
        {"ListCtrl",        "ListCtrl"},
        {"TreeView",        "TreeView"},
        {"TabCtrl",         "TabCtrl"},
        {"DateTime",        "DateTime"},
        {"IPAddress",       "IPAddress"},
        {"HotKey",          "HotKey"},
        {"ColorPicker",     "ColorPicker"},
        {"PropertyGrid",    "PropertyGrid"},
        {"SplitBox",        "SplitBox"},
        {"TileBox",         "TileBox"},
        {"VTileBox",        "VTileBox"},
        {"HTileBox",        "HTileBox"},
        {"GroupBox",        "GroupBox"},
        {"Split",           "Split"},
    };

    auto it = typeMap.find(xmlType);
    if (it != typeMap.end()) {
        return it->second;
    }
    return "Control"; // 默认
}

std::string CodeGenerator::MakeIncludeGuard() const
{
    std::string guard = m_className;
    // 转为大写并加前后缀
    std::transform(guard.begin(), guard.end(), guard.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return guard + "_H_";
}

} // namespace creator
