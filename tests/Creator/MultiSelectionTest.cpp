#include <gtest/gtest.h>

#include "../../creator/Core/Document.h"
#include "../../creator/Core/Selection.h"

#include <memory>

class MultiSelectionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_selection = std::make_unique<creator::Selection>();
        m_doc = std::make_unique<creator::Document>();

        const std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Window>
    <VBox>
        <Button name="btn1"/>
        <Button name="btn2"/>
        <Button name="btn3"/>
        <Label name="lbl1"/>
    </VBox>
</Window>)";
        m_doc->LoadFromString(xml);
    }

    pugi::xml_node FindNode(const std::string& name)
    {
        return m_doc->FindNodeByName(
#ifdef DUILIB_UNICODE
            std::wstring(name.begin(), name.end())
#else
            name
#endif
        );
    }

    std::unique_ptr<creator::Selection> m_selection;
    std::unique_ptr<creator::Document> m_doc;
};

TEST_F(MultiSelectionTest, SingleSelection)
{
    pugi::xml_node btn1 = FindNode("btn1");
    ASSERT_FALSE(btn1.empty());

    m_selection->Select(btn1);
    EXPECT_TRUE(m_selection->HasSelection());
    EXPECT_EQ(m_selection->GetSelectionCount(), 1u);
    EXPECT_FALSE(m_selection->IsMultiSelection());
    EXPECT_EQ(m_selection->GetSelectedNode(), btn1);
}

TEST_F(MultiSelectionTest, AddToSelection)
{
    pugi::xml_node btn1 = FindNode("btn1");
    pugi::xml_node btn2 = FindNode("btn2");
    ASSERT_FALSE(btn1.empty());
    ASSERT_FALSE(btn2.empty());

    m_selection->Select(btn1);
    m_selection->AddToSelection(btn2);

    EXPECT_EQ(m_selection->GetSelectionCount(), 2u);
    EXPECT_TRUE(m_selection->IsMultiSelection());

    auto nodes = m_selection->GetSelectedNodes();
    EXPECT_EQ(nodes.size(), 2u);
}

TEST_F(MultiSelectionTest, RemoveFromSelection)
{
    pugi::xml_node btn1 = FindNode("btn1");
    pugi::xml_node btn2 = FindNode("btn2");

    m_selection->Select(btn1);
    m_selection->AddToSelection(btn2);
    EXPECT_EQ(m_selection->GetSelectionCount(), 2u);

    // AddToSelection 对已选中的节点执行 toggle（取消选中）
    m_selection->AddToSelection(btn1);
    EXPECT_EQ(m_selection->GetSelectionCount(), 1u);
    EXPECT_FALSE(m_selection->IsMultiSelection());
    EXPECT_EQ(m_selection->GetSelectedNode(), btn2);
}

TEST_F(MultiSelectionTest, ClearSelection)
{
    pugi::xml_node btn1 = FindNode("btn1");
    pugi::xml_node btn2 = FindNode("btn2");

    m_selection->Select(btn1);
    m_selection->AddToSelection(btn2);
    EXPECT_EQ(m_selection->GetSelectionCount(), 2u);

    m_selection->ClearSelection();
    EXPECT_FALSE(m_selection->HasSelection());
    EXPECT_EQ(m_selection->GetSelectionCount(), 0u);
    EXPECT_TRUE(m_selection->GetSelectedNodes().empty());
}

TEST_F(MultiSelectionTest, GetSelectionCount)
{
    pugi::xml_node btn1 = FindNode("btn1");
    pugi::xml_node btn2 = FindNode("btn2");
    pugi::xml_node btn3 = FindNode("btn3");

    EXPECT_EQ(m_selection->GetSelectionCount(), 0u);

    m_selection->Select(btn1);
    EXPECT_EQ(m_selection->GetSelectionCount(), 1u);

    m_selection->AddToSelection(btn2);
    EXPECT_EQ(m_selection->GetSelectionCount(), 2u);

    m_selection->AddToSelection(btn3);
    EXPECT_EQ(m_selection->GetSelectionCount(), 3u);
}

TEST_F(MultiSelectionTest, SelectionChangedCallback)
{
    int callbackCount = 0;
    m_selection->AddSelectionChangedCallback([&callbackCount](pugi::xml_node) {
        ++callbackCount;
    });

    pugi::xml_node btn1 = FindNode("btn1");
    pugi::xml_node btn2 = FindNode("btn2");

    m_selection->Select(btn1);
    EXPECT_EQ(callbackCount, 1);

    m_selection->AddToSelection(btn2);
    EXPECT_EQ(callbackCount, 2);

    m_selection->ClearSelection();
    EXPECT_EQ(callbackCount, 3);
}

TEST_F(MultiSelectionTest, SelectClearsMultiSelection)
{
    pugi::xml_node btn1 = FindNode("btn1");
    pugi::xml_node btn2 = FindNode("btn2");
    pugi::xml_node btn3 = FindNode("btn3");

    m_selection->Select(btn1);
    m_selection->AddToSelection(btn2);
    m_selection->AddToSelection(btn3);
    EXPECT_EQ(m_selection->GetSelectionCount(), 3u);

    // Select 应该清除多选，只保留新选中的
    m_selection->Select(btn2);
    EXPECT_EQ(m_selection->GetSelectionCount(), 1u);
    EXPECT_EQ(m_selection->GetSelectedNode(), btn2);
}
