#include <gtest/gtest.h>

#include "../../creator/Core/UndoManager.h"

#include <memory>

class UndoManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_undoMgr = std::make_unique<creator::UndoManager>();
    }

    std::unique_ptr<creator::UndoManager> m_undoMgr;
};

class TestCommand : public creator::UndoCommand
{
public:
    TestCommand(int& value, int newVal, const std::string& desc)
        : m_value(value), m_oldVal(value), m_newVal(newVal), m_desc(desc)
    {
    }

    void Execute() override { m_value = m_newVal; }
    void Undo() override { m_value = m_oldVal; }
    std::string GetDescription() const override { return m_desc; }

private:
    int& m_value;
    int m_oldVal;
    int m_newVal;
    std::string m_desc;
};

TEST_F(UndoManagerTest, Construction)
{
    EXPECT_FALSE(m_undoMgr->CanUndo());
    EXPECT_FALSE(m_undoMgr->CanRedo());
}

TEST_F(UndoManagerTest, ExecuteCommand)
{
    int value = 0;
    auto cmd = std::make_shared<TestCommand>(value, 42, "Set to 42");
    m_undoMgr->ExecuteCommand(cmd);

    EXPECT_EQ(value, 42);
    EXPECT_TRUE(m_undoMgr->CanUndo());
    EXPECT_FALSE(m_undoMgr->CanRedo());
}

TEST_F(UndoManagerTest, Undo)
{
    int value = 0;
    auto cmd = std::make_shared<TestCommand>(value, 42, "Set to 42");
    m_undoMgr->ExecuteCommand(cmd);

    EXPECT_TRUE(m_undoMgr->Undo());
    EXPECT_EQ(value, 0);
    EXPECT_FALSE(m_undoMgr->CanUndo());
    EXPECT_TRUE(m_undoMgr->CanRedo());
}

TEST_F(UndoManagerTest, Redo)
{
    int value = 0;
    auto cmd = std::make_shared<TestCommand>(value, 42, "Set to 42");
    m_undoMgr->ExecuteCommand(cmd);
    m_undoMgr->Undo();

    EXPECT_TRUE(m_undoMgr->Redo());
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(m_undoMgr->CanUndo());
    EXPECT_FALSE(m_undoMgr->CanRedo());
}

TEST_F(UndoManagerTest, MultipleUndoRedo)
{
    int value = 0;
    m_undoMgr->ExecuteCommand(std::make_shared<TestCommand>(value, 10, "Step 1"));
    m_undoMgr->ExecuteCommand(std::make_shared<TestCommand>(value, 20, "Step 2"));
    m_undoMgr->ExecuteCommand(std::make_shared<TestCommand>(value, 30, "Step 3"));

    EXPECT_EQ(value, 30);

    m_undoMgr->Undo();
    EXPECT_EQ(value, 20);

    m_undoMgr->Undo();
    EXPECT_EQ(value, 10);

    m_undoMgr->Redo();
    EXPECT_EQ(value, 20);
}

TEST_F(UndoManagerTest, NewCommandClearsRedoStack)
{
    int value = 0;
    m_undoMgr->ExecuteCommand(std::make_shared<TestCommand>(value, 10, "Step 1"));
    m_undoMgr->ExecuteCommand(std::make_shared<TestCommand>(value, 20, "Step 2"));

    m_undoMgr->Undo();
    EXPECT_TRUE(m_undoMgr->CanRedo());

    m_undoMgr->ExecuteCommand(std::make_shared<TestCommand>(value, 50, "New Step"));
    EXPECT_FALSE(m_undoMgr->CanRedo());
    EXPECT_EQ(value, 50);
}

TEST_F(UndoManagerTest, Clear)
{
    int value = 0;
    m_undoMgr->ExecuteCommand(std::make_shared<TestCommand>(value, 42, "Test"));
    m_undoMgr->Clear();

    EXPECT_FALSE(m_undoMgr->CanUndo());
    EXPECT_FALSE(m_undoMgr->CanRedo());
}

TEST_F(UndoManagerTest, Descriptions)
{
    int value = 0;
    m_undoMgr->ExecuteCommand(std::make_shared<TestCommand>(value, 42, "My Command"));

    EXPECT_EQ(m_undoMgr->GetUndoDescription(), "My Command");
    EXPECT_TRUE(m_undoMgr->GetRedoDescription().empty());

    m_undoMgr->Undo();
    EXPECT_TRUE(m_undoMgr->GetUndoDescription().empty());
    EXPECT_EQ(m_undoMgr->GetRedoDescription(), "My Command");
}

TEST_F(UndoManagerTest, EdgeCaseUndoEmpty)
{
    EXPECT_FALSE(m_undoMgr->Undo());
    EXPECT_FALSE(m_undoMgr->Redo());
}

TEST_F(UndoManagerTest, RecordCommand)
{
    int value = 0;
    auto cmd = std::make_shared<TestCommand>(value, 42, "Recorded");
    m_undoMgr->RecordCommand(cmd);

    EXPECT_EQ(value, 0);
    EXPECT_TRUE(m_undoMgr->CanUndo());

    m_undoMgr->Undo();
    EXPECT_EQ(value, 0);
}
