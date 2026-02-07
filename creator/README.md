# Duilib Creator

nim_duilib 的可视化 GUI 设计器，使用 nim_duilib 自身构建（dogfooding）。

## 功能概述

### 已实现
- **设计画布** - 可视化展示 XML 控件结构，点击选中控件
- **控件工具箱** - 分 5 类展示 40+ 控件，点击即可添加到画布
- **属性面板** - 动态展示选中控件的属性，支持实时编辑（文本/布尔/下拉）
- **控件层级树** - TreeView 展示完整 XML 节点层级，支持右键上下文菜单
- **XML 源码编辑器** - 查看和编辑原始 XML，双向同步
- **文件操作** - 新建 / 打开 / 保存 XML 文件
- **撤销/重做** - 基于 Command 模式的完整撤销重做
- **代码生成** - 从 XML 自动生成 C++ WindowImplBase 窗口类代码框架（.h + .cpp）
- **键盘快捷键** - Ctrl+N/O/S/Z/Y/C/V/X、Delete 等全套快捷键
- **剪贴板操作** - 复制/剪切/粘贴控件节点（含子节点）
- **右键菜单** - 控件层级树支持右键添加/复制/剪切/粘贴/删除
- **项目管理** - 项目根目录管理、资源文件扫描、最近打开文件

### 规划中
- 拖拽排序（从工具箱拖拽到画布）
- 真正的 WYSIWYG 渲染（基于 XmlBox 的实时渲染）
- 多文件 Tab 编辑
- 新建项目向导
- 全局样式/Class 编辑器
- 多 DPI 预览
- 控件模板/收藏
- 对齐/分布工具

## 快捷键

| 快捷键 | 功能 |
|--------|------|
| Ctrl+N | 新建文档 |
| Ctrl+O | 打开文件 |
| Ctrl+S | 保存文件 |
| Ctrl+Z | 撤销 |
| Ctrl+Y | 重做 |
| Ctrl+C | 复制控件 |
| Ctrl+X | 剪切控件 |
| Ctrl+V | 粘贴控件 |
| Delete | 删除选中控件 |

## 构建方法

```bash
# 从仓库根目录构建
cmake -S creator -B build_creator -DCMAKE_BUILD_TYPE=Release
cmake --build build_creator
```

## 项目结构

```
creator/
├── CMakeLists.txt            # 构建配置
├── main_windows.cpp          # Windows 入口
├── main_linux.cpp            # Linux 入口
├── main_macos.mm             # macOS 入口
├── CreatorApplication.h/cpp  # 应用程序封装
├── MainThread.h/cpp          # 主线程
├── MainForm.h/cpp            # 主窗口
│
├── Core/                     # 核心数据模型
│   ├── Document.h/cpp        # 文档模型 (pugixml XML DOM)
│   ├── UndoManager.h/cpp     # 撤销/重做 (Command 模式)
│   ├── ControlMetadata.h/cpp # 控件属性元数据注册表 (40+ 控件)
│   ├── Selection.h/cpp       # 选择状态管理
│   ├── Clipboard.h/cpp       # 控件剪贴板 (XML 节点复制粘贴)
│   └── ProjectManager.h/cpp  # 项目管理器 (资源扫描/最近文件)
│
├── Panels/                   # UI 面板
│   ├── DesignCanvas.h/cpp     # 设计画布
│   ├── ToolboxPanel.h/cpp     # 控件工具箱 (5 类分组)
│   ├── PropertyPanel.h/cpp    # 属性编辑面板 (动态 PropertyGrid)
│   ├── ControlTreePanel.h/cpp # 控件层级树 (右键菜单)
│   ├── XmlEditorPanel.h/cpp   # XML 源码编辑器
│   └── ResourcePanel.h/cpp    # 资源管理器
│
└── CodeGen/                  # 代码生成
    └── CodeGenerator.h/cpp   # C++ 代码生成器
```

## 架构设计

```
                    ┌──────────────┐
                    │   Document   │ ← 核心数据模型 (pugixml)
                    │  (观察者模式) │
                    └──────┬───────┘
                           │ 通知变更
        ┌──────────────────┼──────────────────┐
        │                  │                  │
   ┌────▼─────┐    ┌───────▼──────┐   ┌───────▼──────┐
   │DesignCanvas│   │PropertyPanel │   │ControlTree   │
   │ (画布)    │   │ (属性面板)   │   │ (层级树)     │
   └────┬──────┘   └──────┬───────┘   └───────┬──────┘
        │                 │                    │
        └─────────────────┼────────────────────┘
                          │
                   ┌──────▼──────┐
                   │  Selection  │ ← 选择状态管理
                   └──────┬──────┘
                          │
                   ┌──────▼──────┐
                   │  Clipboard  │ ← 控件剪贴板
                   └─────────────┘
```

所有面板通过 Document 模型和 Selection 管理器进行联动。
Document 使用观察者模式通知所有面板刷新，UndoManager 基于 XML 快照实现撤销重做。
