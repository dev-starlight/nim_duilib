#include "MainForm.h"
#include "Panels/ToolboxPanel.h"
#include "Panels/DesignCanvas.h"
#include "Panels/PropertyPanel.h"
#include "Panels/ControlTreePanel.h"
#include "Panels/XmlEditorPanel.h"
#include "Panels/ResourcePanel.h"
#include "CodeGen/CodeGenerator.h"
#include "Core/ProjectManager.h"
#include <fstream>
#include <cctype>

MainForm::MainForm()
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("creator");
}

DString MainForm::GetSkinFile()
{
    return _T("creator_main.xml");
}

void MainForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    // 创建核心模型
    m_document = std::make_shared<creator::Document>();
    m_metadata = std::make_shared<creator::ControlMetadata>();
    m_selection = std::make_shared<creator::Selection>();
    m_clipboard = std::make_unique<creator::Clipboard>();
    m_codeGenerator = std::make_unique<creator::CodeGenerator>();
    m_projectManager = std::make_unique<creator::ProjectManager>();

    // 初始化控件元数据
    m_metadata->Initialize();

    // 初始化各面板
    InitPanels();

    // 初始化工具栏事件
    InitToolbarEvents();

    // 初始化底部 Tab 切换
    InitBottomTabs();

    // 监听选择变更
    m_selection->AddSelectionChangedCallback([this](pugi::xml_node node) {
        if (node.empty()) {
            UpdateSelectedLabel(_T("(无)"));
        }
        else {
            std::string typeName = node.name();
            std::string nameAttr = node.attribute("name").as_string();
            std::string display = typeName;
            if (!nameAttr.empty()) {
                display += " [" + nameAttr + "]";
            }
            UpdateSelectedLabel(ui::StringUtil::MBCSToT(display));
        }
    });

    // 监听文档变更 → 同步 XML 编辑器
    m_document->AddObserver([this](creator::ChangeType /*type*/, pugi::xml_node /*node*/) {
        SyncXmlToEditor();
        // 更新撤销/重做按钮状态
        ui::Button* undoBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_undo")));
        ui::Button* redoBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_redo")));
        ui::Button* deleteBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_delete_ctrl")));
        if (undoBtn) {
            undoBtn->SetEnabled(m_document->GetUndoManager().CanUndo());
        }
        if (redoBtn) {
            redoBtn->SetEnabled(m_document->GetUndoManager().CanRedo());
        }
        if (deleteBtn) {
            deleteBtn->SetEnabled(m_selection && m_selection->HasSelection());
        }
    });

    // 创建默认空文档
    OnNewFile();

    UpdateStatusBar(_T("就绪 - Duilib Creator v1.0"));
}

LRESULT MainForm::OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey,
                                const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    // 使用 duilib 跨平台 API 检查 Ctrl 键状态
    bool ctrlDown = ui::Keyboard::IsKeyDown(ui::kVK_CONTROL);

    if (ctrlDown) {
        if (HandleCtrlShortcut(vkCode)) {
            bHandled = true;
            return 0;
        }
    }

    // Delete 键 → 删除选中控件
    if (vkCode == ui::kVK_DELETE) {
        OnDeleteControl();
        bHandled = true;
        return 0;
    }

    return BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
}

bool MainForm::HandleCtrlShortcut(ui::VirtualKeyCode vkCode)
{
    switch (vkCode) {
    case ui::kVK_N:
        OnNewFile();
        UpdateStatusBar(_T("Ctrl+N: 新建文档"));
        return true;
    case ui::kVK_O:
        OnOpenFile();
        return true;
    case ui::kVK_S:
        OnSaveFile();
        return true;
    case ui::kVK_Z:
        OnUndo();
        UpdateStatusBar(_T("Ctrl+Z: 撤销"));
        return true;
    case ui::kVK_Y:
        OnRedo();
        UpdateStatusBar(_T("Ctrl+Y: 重做"));
        return true;
    case ui::kVK_C:
        OnCopyControl();
        return true;
    case ui::kVK_V:
        OnPasteControl();
        return true;
    case ui::kVK_X:
        OnCutControl();
        return true;
    default:
        return false;
    }
}

void MainForm::InitToolbarEvents()
{
    // 新建
    ui::Button* newBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_new")));
    if (newBtn) {
        newBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnNewFile();
            return true;
        });
    }

    // 打开
    ui::Button* openBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_open")));
    if (openBtn) {
        openBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnOpenFile();
            return true;
        });
    }

    // 保存
    ui::Button* saveBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_save")));
    if (saveBtn) {
        saveBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnSaveFile();
            return true;
        });
    }

    // 撤销
    ui::Button* undoBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_undo")));
    if (undoBtn) {
        undoBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnUndo();
            return true;
        });
    }

    // 重做
    ui::Button* redoBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_redo")));
    if (redoBtn) {
        redoBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnRedo();
            return true;
        });
    }

    // 预览
    ui::Button* previewBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_preview")));
    if (previewBtn) {
        previewBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnPreview();
            return true;
        });
    }

    // 删除控件
    ui::Button* deleteBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_delete_ctrl")));
    if (deleteBtn) {
        deleteBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnDeleteControl();
            return true;
        });
    }

    // 导出代码
    ui::Button* exportBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_export_code")));
    if (exportBtn) {
        exportBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnExportCode();
            return true;
        });
    }
}

void MainForm::InitBottomTabs()
{
    ui::TabBox* tabBox = dynamic_cast<ui::TabBox*>(FindControl(_T("bottom_tab_box")));
    if (!tabBox) {
        return;
    }

    // 默认选中第一个 Tab
    tabBox->SelectItem(0);

    // Tab 按钮
    ui::Button* tabTree = dynamic_cast<ui::Button*>(FindControl(_T("btn_tab_tree")));
    ui::Button* tabXml = dynamic_cast<ui::Button*>(FindControl(_T("btn_tab_xml")));
    ui::Button* tabResource = dynamic_cast<ui::Button*>(FindControl(_T("btn_tab_resource")));

    if (tabTree) {
        tabTree->AttachClick([this](const ui::EventArgs& /*args*/) {
            SwitchBottomTab(0);
            return true;
        });
    }
    if (tabXml) {
        tabXml->AttachClick([this](const ui::EventArgs& /*args*/) {
            SwitchBottomTab(1);
            return true;
        });
    }
    if (tabResource) {
        tabResource->AttachClick([this](const ui::EventArgs& /*args*/) {
            SwitchBottomTab(2);
            return true;
        });
    }

    // 初始高亮
    SwitchBottomTab(0);
}

void MainForm::InitPanels()
{
    // ===== 工具箱面板 =====
    ui::VScrollBox* toolboxContainer = dynamic_cast<ui::VScrollBox*>(FindControl(_T("toolbox_container")));
    if (toolboxContainer) {
        m_toolboxPanel = std::make_unique<creator::ToolboxPanel>();
        m_toolboxPanel->Initialize(toolboxContainer, m_metadata);
        m_toolboxPanel->SetSelectCallback([this](const std::string& controlType) {
            // 工具箱选中控件后，添加到当前选中位置
            if (m_designCanvas) {
                m_designCanvas->AddControlToSelected(controlType);
            }
            UpdateStatusBar(_T("已添加控件: ") + ui::StringUtil::MBCSToT(controlType));
        });
    }

    // ===== 设计画布 =====
    ui::Box* canvasBox = dynamic_cast<ui::Box*>(FindControl(_T("design_canvas")));
    if (canvasBox) {
        m_designCanvas = std::make_unique<creator::DesignCanvas>();
        m_designCanvas->Initialize(canvasBox, m_document, m_selection, m_metadata);
    }

    // ===== 属性面板 =====
    ui::VScrollBox* propertyContainer = dynamic_cast<ui::VScrollBox*>(FindControl(_T("property_container")));
    if (propertyContainer) {
        m_propertyPanel = std::make_unique<creator::PropertyPanel>();
        m_propertyPanel->Initialize(propertyContainer, m_document, m_selection, m_metadata);
    }

    // ===== 控件层级树 =====
    ui::TreeView* controlTree = dynamic_cast<ui::TreeView*>(FindControl(_T("control_tree")));
    if (controlTree) {
        m_controlTreePanel = std::make_unique<creator::ControlTreePanel>();
        m_controlTreePanel->Initialize(controlTree, m_document, m_selection);
        m_controlTreePanel->SetClipboard(m_clipboard.get());

        // 右键菜单回调
        m_controlTreePanel->SetContextMenuCallback([this](const std::string& action, const std::string& param) {
            if (action == "add" && m_designCanvas) {
                m_designCanvas->AddControlToSelected(param);
                UpdateStatusBar(_T("已添加控件: ") + ui::StringUtil::MBCSToT(param));
            }
            else if (action == "copy") {
                OnCopyControl();
            }
            else if (action == "cut") {
                OnCutControl();
            }
            else if (action == "paste") {
                OnPasteControl();
            }
            else if (action == "delete") {
                OnDeleteControl();
            }
        });
    }

    // ===== XML 编辑器 =====
    ui::RichEdit* xmlEditor = dynamic_cast<ui::RichEdit*>(FindControl(_T("xml_editor")));
    if (xmlEditor) {
        m_xmlEditorPanel = std::make_unique<creator::XmlEditorPanel>();
        m_xmlEditorPanel->Initialize(xmlEditor, m_document);
    }

    // ===== 资源管理器 =====
    ui::Box* resourceContainer = dynamic_cast<ui::Box*>(FindControl(_T("resource_panel")));
    if (resourceContainer) {
        m_resourcePanel = std::make_unique<creator::ResourcePanel>();
        m_resourcePanel->Initialize(resourceContainer);
    }
}

void MainForm::UpdateStatusBar(const DString& text)
{
    ui::Label* statusText = dynamic_cast<ui::Label*>(FindControl(_T("status_text")));
    if (statusText) {
        statusText->SetText(text);
    }
}

void MainForm::UpdateSelectedLabel(const DString& controlType)
{
    ui::Label* label = dynamic_cast<ui::Label*>(FindControl(_T("selected_control_label")));
    if (label) {
        label->SetText(controlType);
    }

    // 更新删除按钮状态
    ui::Button* deleteBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_delete_ctrl")));
    if (deleteBtn) {
        deleteBtn->SetEnabled(m_selection && m_selection->HasSelection());
    }
}

void MainForm::OnNewFile()
{
    m_currentFilePath.clear();
    m_document->CreateNewDocument();
    m_selection->ClearSelection();
    UpdateStatusBar(_T("新建文档"));
}

void MainForm::OnOpenFile()
{
    // 使用文件对话框选择 XML 文件
    ui::FileDialog fileDlg;
    std::vector<ui::FileDialog::FileType> fileTypes;
    fileTypes.push_back({_T("XML Files"), _T("*.xml")});
    fileTypes.push_back({_T("All Files"), _T("*.*")});

    ui::FilePath filePath;
    if (fileDlg.BrowseForFile(this, filePath, true, fileTypes, 0, _T("xml"))) {
        DString pathStr = filePath.ToString();
        if (m_document->LoadFromFile(pathStr)) {
            m_currentFilePath = pathStr;
            m_selection->ClearSelection();

            // 添加到最近打开文件
            if (m_projectManager) {
                m_projectManager->AddRecentFile(pathStr);
            }
            UpdateStatusBar(_T("已打开: ") + pathStr);
        }
        else {
            UpdateStatusBar(_T("打开文件失败"));
        }
    }
}

void MainForm::OnSaveFile()
{
    if (m_currentFilePath.empty()) {
        // 另存为
        ui::FileDialog fileDlg;
        std::vector<ui::FileDialog::FileType> fileTypes;
        fileTypes.push_back({_T("XML Files"), _T("*.xml")});
        fileTypes.push_back({_T("All Files"), _T("*.*")});

        ui::FilePath filePath;
        if (fileDlg.BrowseForFile(this, filePath, false, fileTypes, 0, _T("xml"))) {
            m_currentFilePath = filePath.ToString();
        }
        else {
            return;
        }
    }

    if (m_document->SaveToFile(m_currentFilePath)) {
        m_document->ClearModified();
        UpdateStatusBar(_T("已保存: ") + m_currentFilePath);
    }
    else {
        UpdateStatusBar(_T("保存文件失败"));
    }
}

void MainForm::OnUndo()
{
    if (m_document->GetUndoManager().CanUndo()) {
        m_document->GetUndoManager().Undo();
        UpdateStatusBar(_T("撤销操作"));
    }
}

void MainForm::OnRedo()
{
    if (m_document->GetUndoManager().CanRedo()) {
        m_document->GetUndoManager().Redo();
        UpdateStatusBar(_T("重做操作"));
    }
}

void MainForm::OnPreview()
{
    if (m_designCanvas) {
        m_designCanvas->RefreshPreview();
        UpdateStatusBar(_T("刷新预览"));
    }
}

void MainForm::OnDeleteControl()
{
    if (!m_selection || !m_selection->HasSelection()) {
        return;
    }

    pugi::xml_node node = m_selection->GetSelectedNode();
    if (node.empty()) {
        return;
    }

    // 不允许删除根节点
    pugi::xml_node root = m_document->GetRootNode();
    if (node == root) {
        UpdateStatusBar(_T("不能删除根节点"));
        return;
    }

    std::string typeName = node.name();
    m_selection->ClearSelection();
    m_document->RemoveControl(node);
    UpdateStatusBar(_T("已删除控件: ") + ui::StringUtil::MBCSToT(typeName));
}

void MainForm::OnCopyControl()
{
    if (!m_selection || !m_selection->HasSelection() || !m_clipboard) {
        return;
    }

    pugi::xml_node node = m_selection->GetSelectedNode();
    if (node.empty()) {
        return;
    }

    if (m_clipboard->Copy(node)) {
        UpdateStatusBar(_T("已复制控件: ") + ui::StringUtil::MBCSToT(std::string(node.name())));
    }
}

void MainForm::OnPasteControl()
{
    if (!m_clipboard || !m_clipboard->HasContent() || !m_document) {
        return;
    }

    // 确定粘贴目标
    pugi::xml_node targetParent;
    if (m_selection && m_selection->HasSelection()) {
        pugi::xml_node selected = m_selection->GetSelectedNode();
        if (m_metadata && m_metadata->IsContainer(selected.name())) {
            targetParent = selected;
        }
        else {
            targetParent = selected.parent();
        }
    }
    else {
        targetParent = m_document->GetRootNode();
    }

    if (targetParent.empty()) {
        return;
    }

    // 记录撤销快照
    std::string beforeXml = m_document->ToXmlString();

    pugi::xml_node newNode = m_clipboard->Paste(targetParent);
    if (!newNode.empty()) {
        // 记录撤销命令
        std::string afterXml = m_document->ToXmlString();
        auto cmd = std::make_shared<creator::XmlSnapshotCommand>(
            "粘贴控件",
            beforeXml,
            afterXml,
            [this](const std::string& xml) { m_document->LoadFromString(xml); }
        );
        m_document->GetUndoManager().RecordCommand(cmd);

        m_document->NotifyChanged(creator::ChangeType::kNodeAdded, newNode);
        m_selection->Select(newNode);
        UpdateStatusBar(_T("已粘贴控件"));
    }
}

void MainForm::OnCutControl()
{
    if (!m_selection || !m_selection->HasSelection() || !m_clipboard) {
        return;
    }

    pugi::xml_node node = m_selection->GetSelectedNode();
    if (node.empty()) {
        return;
    }

    // 不允许剪切根节点
    pugi::xml_node root = m_document->GetRootNode();
    if (node == root) {
        UpdateStatusBar(_T("不能剪切根节点"));
        return;
    }

    // 先复制再删除
    if (m_clipboard->Copy(node)) {
        std::string typeName = node.name();
        m_selection->ClearSelection();
        m_document->RemoveControl(node);
        UpdateStatusBar(_T("已剪切控件: ") + ui::StringUtil::MBCSToT(typeName));
    }
}

void MainForm::OnExportCode()
{
    if (!m_document || !m_codeGenerator) {
        return;
    }

    // 配置代码生成器
    std::string className = "MyForm";
    std::string xmlFile = "main.xml";
    std::string skinFolder = "my_app";

    if (!m_currentFilePath.empty()) {
        ui::FilePath fp(m_currentFilePath);
        DString fileName = fp.GetFileName();
        std::string fileNameStr = ui::StringUtil::TToMBCS(fileName);
        if (!fileNameStr.empty()) {
            xmlFile = fileNameStr;
            // 从文件名推导类名（去掉扩展名，首字母大写）
            size_t dotPos = fileNameStr.rfind('.');
            if (dotPos != std::string::npos) {
                className = fileNameStr.substr(0, dotPos);
            }
            else {
                className = fileNameStr;
            }
            // 首字母大写
            if (!className.empty()) {
                className[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(className[0])));
            }
            // 加 "Form" 后缀
            className += "Form";
        }
    }

    if (m_projectManager && m_projectManager->IsProjectOpen()) {
        skinFolder = m_projectManager->GetSkinFolderName();
    }

    m_codeGenerator->SetClassName(className);
    m_codeGenerator->SetXmlFile(xmlFile);
    m_codeGenerator->SetSkinFolder(skinFolder);

    auto code = m_codeGenerator->Generate(m_document);

    // 选择保存位置
    ui::FileDialog fileDlg;
    std::vector<ui::FileDialog::FileType> fileTypes;
    fileTypes.push_back({_T("C++ Header"), _T("*.h")});

    ui::FilePath savePath;
    if (fileDlg.BrowseForFile(this, savePath, false, fileTypes, 0, _T("h"))) {
        DString savePathStr = savePath.ToString();
        std::string savePathMBCS = ui::StringUtil::TToMBCS(savePathStr);

        // 保存 .h 文件
        {
            std::ofstream headerFile(savePathMBCS);
            if (headerFile.is_open()) {
                headerFile << code.headerContent;
                headerFile.close();
            }
        }

        // 保存 .cpp 文件（替换扩展名）
        std::string cppPath = savePathMBCS;
        size_t extPos = cppPath.rfind(".h");
        if (extPos != std::string::npos) {
            cppPath.replace(extPos, 2, ".cpp");
        }
        else {
            cppPath += ".cpp";
        }

        {
            std::ofstream implFile(cppPath);
            if (implFile.is_open()) {
                implFile << code.implContent;
                implFile.close();
            }
        }

        UpdateStatusBar(_T("代码已导出: ") + savePathStr);
    }
}

void MainForm::SwitchBottomTab(int32_t tabIndex)
{
    ui::TabBox* tabBox = dynamic_cast<ui::TabBox*>(FindControl(_T("bottom_tab_box")));
    if (tabBox) {
        tabBox->SelectItem(tabIndex);
    }

    // 更新 Tab 按钮样式
    ui::Button* tabBtns[] = {
        dynamic_cast<ui::Button*>(FindControl(_T("btn_tab_tree"))),
        dynamic_cast<ui::Button*>(FindControl(_T("btn_tab_xml"))),
        dynamic_cast<ui::Button*>(FindControl(_T("btn_tab_resource")))
    };

    for (int32_t i = 0; i < 3; ++i) {
        if (tabBtns[i]) {
            if (i == tabIndex) {
                tabBtns[i]->SetBkColor(_T("#FF007ACC"));
                tabBtns[i]->SetStateTextColor(ui::kControlStateNormal, _T("#FFFFFFFF"));
            }
            else {
                tabBtns[i]->SetBkColor(_T(""));
                tabBtns[i]->SetStateTextColor(ui::kControlStateNormal, _T("#FFCCCCCC"));
            }
        }
    }

    // 切换到 XML 编辑器时同步内容
    if (tabIndex == 1) {
        SyncXmlToEditor();
    }
}

void MainForm::SyncXmlToEditor()
{
    if (m_xmlEditorPanel) {
        m_xmlEditorPanel->SyncFromDocument();
    }
}

void MainForm::SyncXmlFromEditor()
{
    if (m_xmlEditorPanel) {
        m_xmlEditorPanel->SyncToDocument();
    }
}
