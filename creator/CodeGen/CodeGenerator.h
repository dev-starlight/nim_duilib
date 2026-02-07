#ifndef CREATOR_CODEGEN_CODE_GENERATOR_H_
#define CREATOR_CODEGEN_CODE_GENERATOR_H_

#include "duilib/duilib.h"
#include "Core/Document.h"
#include <string>
#include <memory>

namespace creator {

/** C++ 代码生成器
 *  从 XML 文档模型生成 C++ 窗口类代码框架。
 *  生成的代码遵循 nim_duilib 的 WindowImplBase 模式。
 */
class CodeGenerator
{
public:
    CodeGenerator();
    ~CodeGenerator();

    /** 设置类名 */
    void SetClassName(const std::string& className) { m_className = className; }

    /** 设置 XML 文件路径（相对于 skin 目录） */
    void SetXmlFile(const std::string& xmlFile) { m_xmlFile = xmlFile; }

    /** 设置 skin 文件夹名 */
    void SetSkinFolder(const std::string& skinFolder) { m_skinFolder = skinFolder; }

    /** 从文档生成代码
     * @param document 文档模型
     * @return 生成的代码（header + implementation）
     */
    struct GeneratedCode
    {
        std::string headerContent;
        std::string implContent;
    };

    GeneratedCode Generate(const std::shared_ptr<Document>& document) const;

    /** 生成 header 文件内容 */
    std::string GenerateHeader(const std::shared_ptr<Document>& document) const;

    /** 生成 cpp 文件内容 */
    std::string GenerateImpl(const std::shared_ptr<Document>& document) const;

private:
    /** 收集所有有 name 属性的控件 */
    struct ControlInfo
    {
        std::string name;
        std::string type;
    };
    std::vector<ControlInfo> CollectNamedControls(pugi::xml_node node) const;

    /** 将控件类型映射为 C++ 类型 */
    std::string MapControlType(const std::string& xmlType) const;

    /** 生成 include guard */
    std::string MakeIncludeGuard() const;

private:
    std::string m_className = "MyForm";
    std::string m_xmlFile = "main.xml";
    std::string m_skinFolder = "my_app";
};

} // namespace creator

#endif // CREATOR_CODEGEN_CODE_GENERATOR_H_
