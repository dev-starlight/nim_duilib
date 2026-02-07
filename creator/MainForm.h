#ifndef CREATOR_MAIN_FORM_H_
#define CREATOR_MAIN_FORM_H_

#include "duilib/duilib.h"
#include "Core/Document.h"
#include "Core/ControlMetadata.h"
#include "Core/Selection.h"
#include "Core/Clipboard.h"

// 前向声明
namespace creator {
class ToolboxPanel;
class DesignCanvas;
class PropertyPanel;
class ControlTreePanel;
class XmlEditorPanel;
class ResourcePanel;
class CodeGenerator;
class ProjectManager;
}

/** Creator 主窗口
*/
class MainForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    MainForm();
    virtual ~MainForm() override;

    /** 资源相关接口 */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数 */
    virtual void OnInitWindow() override;

protected:
    /** 键盘按下事件（快捷键处理）*/
    virtual LRESULT OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey,
                                 const ui::NativeMsg& nativeMsg, bool& bHandled) override;

private:
    /** 初始化工具栏按钮事件 */
    void InitToolbarEvents();

    /** 初始化底部 Tab 切换 */
    void InitBottomTabs();

    /** 初始化面板 */
    void InitPanels();

    /** 初始化键盘快捷键 */
    void InitKeyboardShortcuts();

    /** 更新状态栏 */
    void UpdateStatusBar(const DString& text);

    /** 更新选中控件标签 */
    void UpdateSelectedLabel(const DString& controlType);

    /** 文件操作 */
    void OnNewFile();
    void OnOpenFile();
    void OnSaveFile();

    /** 编辑操作 */
    void OnUndo();
    void OnRedo();
    void OnPreview();
    void OnDeleteControl();
    void OnCopyControl();
    void OnPasteControl();
    void OnCutControl();

    /** 代码生成 */
    void OnExportCode();

    /** 底部 Tab 切换 */
    void SwitchBottomTab(int32_t tabIndex);

    /** 同步 XML 到编辑器 */
    void SyncXmlToEditor();

    /** 从编辑器同步 XML */
    void SyncXmlFromEditor();

    /** 处理 Ctrl+快捷键 */
    bool HandleCtrlShortcut(ui::VirtualKeyCode vkCode);

private:
    /** 文档模型 */
    std::shared_ptr<creator::Document> m_document;

    /** 控件元数据 */
    std::shared_ptr<creator::ControlMetadata> m_metadata;

    /** 选择管理器 */
    std::shared_ptr<creator::Selection> m_selection;

    /** 剪贴板 */
    std::unique_ptr<creator::Clipboard> m_clipboard;

    /** 代码生成器 */
    std::unique_ptr<creator::CodeGenerator> m_codeGenerator;

    /** 项目管理器 */
    std::unique_ptr<creator::ProjectManager> m_projectManager;

    /** 各面板 */
    std::unique_ptr<creator::ToolboxPanel> m_toolboxPanel;
    std::unique_ptr<creator::DesignCanvas> m_designCanvas;
    std::unique_ptr<creator::PropertyPanel> m_propertyPanel;
    std::unique_ptr<creator::ControlTreePanel> m_controlTreePanel;
    std::unique_ptr<creator::XmlEditorPanel> m_xmlEditorPanel;
    std::unique_ptr<creator::ResourcePanel> m_resourcePanel;

    /** 当前文件路径 */
    DString m_currentFilePath;
};

#endif // CREATOR_MAIN_FORM_H_
