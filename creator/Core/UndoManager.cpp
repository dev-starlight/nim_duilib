#include "UndoManager.h"

namespace creator {

// ============================================================
// XmlSnapshotCommand
// ============================================================

XmlSnapshotCommand::XmlSnapshotCommand(const std::string& description,
                                       const std::string& beforeXml,
                                       const std::string& afterXml,
                                       std::function<void(const std::string&)> applyFunc)
    : m_description(description)
    , m_beforeXml(beforeXml)
    , m_afterXml(afterXml)
    , m_applyFunc(applyFunc)
{
}

void XmlSnapshotCommand::Execute()
{
    if (m_applyFunc) {
        m_applyFunc(m_afterXml);
    }
}

void XmlSnapshotCommand::Undo()
{
    if (m_applyFunc) {
        m_applyFunc(m_beforeXml);
    }
}

// ============================================================
// UndoManager
// ============================================================

UndoManager::UndoManager()
{
}

UndoManager::~UndoManager()
{
}

void UndoManager::ExecuteCommand(std::shared_ptr<UndoCommand> cmd)
{
    if (!cmd) {
        return;
    }
    cmd->Execute();
    m_undoStack.push_back(cmd);
    m_redoStack.clear(); // 新操作清空重做栈
}

void UndoManager::RecordCommand(std::shared_ptr<UndoCommand> cmd)
{
    if (!cmd) {
        return;
    }
    m_undoStack.push_back(cmd);
    m_redoStack.clear();
}

bool UndoManager::Undo()
{
    if (!CanUndo()) {
        return false;
    }
    auto cmd = m_undoStack.back();
    m_undoStack.pop_back();
    cmd->Undo();
    m_redoStack.push_back(cmd);
    return true;
}

bool UndoManager::Redo()
{
    if (!CanRedo()) {
        return false;
    }
    auto cmd = m_redoStack.back();
    m_redoStack.pop_back();
    cmd->Execute();
    m_undoStack.push_back(cmd);
    return true;
}

bool UndoManager::CanUndo() const
{
    return !m_undoStack.empty();
}

bool UndoManager::CanRedo() const
{
    return !m_redoStack.empty();
}

void UndoManager::Clear()
{
    m_undoStack.clear();
    m_redoStack.clear();
}

std::string UndoManager::GetUndoDescription() const
{
    if (CanUndo()) {
        return m_undoStack.back()->GetDescription();
    }
    return "";
}

std::string UndoManager::GetRedoDescription() const
{
    if (CanRedo()) {
        return m_redoStack.back()->GetDescription();
    }
    return "";
}

} // namespace creator
