#include <gtest/gtest.h>

#include "../../creator/Panels/DragDropManager.h"
#include "../../creator/Core/Document.h"
#include "../../creator/Core/ControlMetadata.h"

#include <memory>

class DragDropManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_doc = std::make_shared<creator::Document>();
        m_metadata = std::make_shared<creator::ControlMetadata>();
        m_metadata->Initialize();
        m_manager = std::make_unique<creator::DragDropManager>();

        m_doc->CreateNewDocument();
    }

    std::shared_ptr<creator::Document> m_doc;
    std::shared_ptr<creator::ControlMetadata> m_metadata;
    std::unique_ptr<creator::DragDropManager> m_manager;
};

TEST_F(DragDropManagerTest, Construction)
{
    EXPECT_FALSE(m_manager->IsDragging());
    EXPECT_TRUE(m_manager->GetDraggedType().empty());
}

TEST_F(DragDropManagerTest, OnDragStart)
{
    m_manager->OnDragStart("Button");
    EXPECT_TRUE(m_manager->IsDragging());
    EXPECT_EQ(m_manager->GetDraggedType(), "Button");
}

TEST_F(DragDropManagerTest, IsDragging)
{
    EXPECT_FALSE(m_manager->IsDragging());

    m_manager->OnDragStart("Label");
    EXPECT_TRUE(m_manager->IsDragging());

    m_manager->CancelDrag();
    EXPECT_FALSE(m_manager->IsDragging());
}

TEST_F(DragDropManagerTest, OnDropAddsControl)
{
    // 注意：完整的 OnDrop 测试需要 DesignCanvas 实例，
    // 这里测试 DragDropManager 的状态管理逻辑
    m_manager->OnDragStart("CheckBox");
    EXPECT_TRUE(m_manager->IsDragging());
    EXPECT_EQ(m_manager->GetDraggedType(), "CheckBox");

    // 模拟 drop（没有 canvas 时不会崩溃）
    m_manager->OnDrop(nullptr, 0);
    EXPECT_FALSE(m_manager->IsDragging());
    EXPECT_TRUE(m_manager->GetDraggedType().empty());
}

TEST_F(DragDropManagerTest, CancelDrag)
{
    m_manager->OnDragStart("Button");
    EXPECT_TRUE(m_manager->IsDragging());

    m_manager->CancelDrag();
    EXPECT_FALSE(m_manager->IsDragging());
    EXPECT_TRUE(m_manager->GetDraggedType().empty());
}

TEST_F(DragDropManagerTest, OnDropWithoutDragDoesNothing)
{
    // 没有开始拖拽就 drop，不应该有任何效果
    EXPECT_FALSE(m_manager->IsDragging());
    m_manager->OnDrop(nullptr, 0);
    EXPECT_FALSE(m_manager->IsDragging());
}
