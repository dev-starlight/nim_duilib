#ifndef CREATOR_CORE_UNDO_MANAGER_H_
#define CREATOR_CORE_UNDO_MANAGER_H_

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace creator {

/** 撤销/重做命令基类 */
class UndoCommand
{
public:
    virtual ~UndoCommand() = default;

    /** 执行 / 重做 */
    virtual void Execute() = 0;

    /** 撤销 */
    virtual void Undo() = 0;

    /** 命令描述 */
    virtual std::string GetDescription() const = 0;
};

/** 基于 XML 快照的撤销命令
 *  记录执行前后的完整 XML 状态
 */
class XmlSnapshotCommand : public UndoCommand
{
public:
    XmlSnapshotCommand(const std::string& description,
                       const std::string& beforeXml,
                       const std::string& afterXml,
                       std::function<void(const std::string&)> applyFunc);

    void Execute() override;
    void Undo() override;
    std::string GetDescription() const override { return m_description; }

private:
    std::string m_description;
    std::string m_beforeXml;
    std::string m_afterXml;
    std::function<void(const std::string&)> m_applyFunc;
};

/** 撤销/重做管理器 (Command 模式) */
class UndoManager
{
public:
    UndoManager();
    ~UndoManager();

    /** 记录并执行一个命令 */
    void ExecuteCommand(std::shared_ptr<UndoCommand> cmd);

    /** 仅记录命令（命令已经执行过了） */
    void RecordCommand(std::shared_ptr<UndoCommand> cmd);

    /** 撤销 */
    bool Undo();

    /** 重做 */
    bool Redo();

    /** 是否可以撤销 */
    bool CanUndo() const;

    /** 是否可以重做 */
    bool CanRedo() const;

    /** 清空历史 */
    void Clear();

    /** 获取撤销描述 */
    std::string GetUndoDescription() const;

    /** 获取重做描述 */
    std::string GetRedoDescription() const;

private:
    std::vector<std::shared_ptr<UndoCommand>> m_undoStack;
    std::vector<std::shared_ptr<UndoCommand>> m_redoStack;
};

} // namespace creator

#endif // CREATOR_CORE_UNDO_MANAGER_H_
