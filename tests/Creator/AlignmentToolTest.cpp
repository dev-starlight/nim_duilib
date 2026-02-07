#include <gtest/gtest.h>

#include "../../creator/Core/AlignmentTool.h"
#include "../../creator/Core/Document.h"
#include "../../creator/Core/Selection.h"

#include <memory>

class AlignmentToolTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_doc = std::make_shared<creator::Document>();
        m_selection = std::make_shared<creator::Selection>();
        m_tool = std::make_unique<creator::AlignmentTool>(m_doc, m_selection);

        // 创建包含多个控件的文档
        const std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Window>
    <Box>
        <Button name="btn1" width="80" height="30" margin="10,10,0,0"/>
        <Button name="btn2" width="80" height="30" margin="100,50,0,0"/>
        <Button name="btn3" width="80" height="30" margin="200,100,0,0"/>
        <Label name="lbl1" width="60" height="20" margin="50,200,0,0"/>
    </Box>
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

    void SelectNodes(const std::vector<std::string>& names)
    {
        bool first = true;
        for (const auto& name : names) {
            pugi::xml_node node = FindNode(name);
            if (first) {
                m_selection->Select(node);
                first = false;
            }
            else {
                m_selection->AddToSelection(node);
            }
        }
    }

    std::string GetMargin(const std::string& name)
    {
        pugi::xml_node node = FindNode(name);
        return node.attribute("margin").as_string();
    }

    std::shared_ptr<creator::Document> m_doc;
    std::shared_ptr<creator::Selection> m_selection;
    std::unique_ptr<creator::AlignmentTool> m_tool;
};

TEST_F(AlignmentToolTest, CanAlignWithTwoNodes)
{
    SelectNodes({"btn1", "btn2"});
    EXPECT_TRUE(m_tool->CanAlign());
}

TEST_F(AlignmentToolTest, CannotAlignWithOneNode)
{
    SelectNodes({"btn1"});
    EXPECT_FALSE(m_tool->CanAlign());
}

TEST_F(AlignmentToolTest, AlignLeft)
{
    SelectNodes({"btn1", "btn2", "btn3"});
    EXPECT_TRUE(m_tool->Align(creator::AlignmentTool::AlignType::Left));

    // 所有控件的 left margin 应该是最小值 10
    EXPECT_EQ(GetMargin("btn1"), "10,10,0,0");
    EXPECT_EQ(GetMargin("btn2"), "10,50,0,0");
    EXPECT_EQ(GetMargin("btn3"), "10,100,0,0");
}

TEST_F(AlignmentToolTest, AlignRight)
{
    SelectNodes({"btn1", "btn2", "btn3"});
    EXPECT_TRUE(m_tool->Align(creator::AlignmentTool::AlignType::Right));

    // 所有控件的 right 边缘对齐到最大值 (200+80=280)
    // btn1: left = 280-80 = 200
    // btn2: left = 280-80 = 200
    // btn3: left = 280-80 = 200
    EXPECT_EQ(GetMargin("btn1"), "200,10,0,0");
    EXPECT_EQ(GetMargin("btn2"), "200,50,0,0");
    EXPECT_EQ(GetMargin("btn3"), "200,100,0,0");
}

TEST_F(AlignmentToolTest, AlignTop)
{
    SelectNodes({"btn1", "btn2", "btn3"});
    EXPECT_TRUE(m_tool->Align(creator::AlignmentTool::AlignType::Top));

    // 所有控件的 top margin 应该是最小值 10
    EXPECT_EQ(GetMargin("btn1"), "10,10,0,0");
    EXPECT_EQ(GetMargin("btn2"), "100,10,0,0");
    EXPECT_EQ(GetMargin("btn3"), "200,10,0,0");
}

TEST_F(AlignmentToolTest, AlignBottom)
{
    SelectNodes({"btn1", "btn2", "btn3"});
    EXPECT_TRUE(m_tool->Align(creator::AlignmentTool::AlignType::Bottom));

    // 所有控件的 bottom 边缘对齐到最大值 (100+30=130)
    // btn1: top = 130-30 = 100
    // btn2: top = 130-30 = 100
    // btn3: top = 130-30 = 100
    EXPECT_EQ(GetMargin("btn1"), "10,100,0,0");
    EXPECT_EQ(GetMargin("btn2"), "100,100,0,0");
    EXPECT_EQ(GetMargin("btn3"), "200,100,0,0");
}

TEST_F(AlignmentToolTest, AlignCenterH)
{
    SelectNodes({"btn1", "btn2", "btn3"});
    EXPECT_TRUE(m_tool->Align(creator::AlignmentTool::AlignType::CenterH));

    // 各控件中心 X: btn1=50, btn2=140, btn3=240
    // 平均中心 X = (50+140+240)/3 = 143
    // btn1: left = 143 - 40 = 103
    // btn2: left = 143 - 40 = 103
    // btn3: left = 143 - 40 = 103
    EXPECT_EQ(GetMargin("btn1"), "103,10,0,0");
    EXPECT_EQ(GetMargin("btn2"), "103,50,0,0");
    EXPECT_EQ(GetMargin("btn3"), "103,100,0,0");
}

TEST_F(AlignmentToolTest, AlignCenterV)
{
    SelectNodes({"btn1", "btn2", "btn3"});
    EXPECT_TRUE(m_tool->Align(creator::AlignmentTool::AlignType::CenterV));

    // 各控件中心 Y: btn1=25, btn2=65, btn3=115
    // 平均中心 Y = (25+65+115)/3 = 68
    // btn1: top = 68 - 15 = 53
    // btn2: top = 68 - 15 = 53
    // btn3: top = 68 - 15 = 53
    EXPECT_EQ(GetMargin("btn1"), "10,53,0,0");
    EXPECT_EQ(GetMargin("btn2"), "100,53,0,0");
    EXPECT_EQ(GetMargin("btn3"), "200,53,0,0");
}

TEST_F(AlignmentToolTest, DistributeHorizontal)
{
    SelectNodes({"btn1", "btn2", "btn3"});
    EXPECT_TRUE(m_tool->Distribute(creator::AlignmentTool::DistributeType::Horizontal));

    // 排序后: btn1(left=10), btn2(left=100), btn3(left=200)
    // 总范围: 10 到 280 (200+80)
    // 总宽度: 80+80+80 = 240
    // 总间距: (280-10) - 240 = 30
    // 每个间距: 30/2 = 15
    // btn1: left = 10
    // btn2: left = 10 + 80 + 15 = 105
    // btn3: left = 105 + 80 + 15 = 200
    EXPECT_EQ(GetMargin("btn1"), "10,10,0,0");
    EXPECT_EQ(GetMargin("btn2"), "105,50,0,0");
    EXPECT_EQ(GetMargin("btn3"), "200,100,0,0");
}

TEST_F(AlignmentToolTest, DistributeVertical)
{
    SelectNodes({"btn1", "btn2", "btn3"});
    EXPECT_TRUE(m_tool->Distribute(creator::AlignmentTool::DistributeType::Vertical));

    // 排序后: btn1(top=10), btn2(top=50), btn3(top=100)
    // 总范围: 10 到 130 (100+30)
    // 总高度: 30+30+30 = 90
    // 总间距: (130-10) - 90 = 30
    // 每个间距: 30/2 = 15
    // btn1: top = 10
    // btn2: top = 10 + 30 + 15 = 55
    // btn3: top = 55 + 30 + 15 = 100
    EXPECT_EQ(GetMargin("btn1"), "10,10,0,0");
    EXPECT_EQ(GetMargin("btn2"), "100,55,0,0");
    EXPECT_EQ(GetMargin("btn3"), "200,100,0,0");
}

TEST_F(AlignmentToolTest, DistributeNeedsThreeNodes)
{
    SelectNodes({"btn1", "btn2"});
    EXPECT_FALSE(m_tool->CanDistribute());
    EXPECT_FALSE(m_tool->Distribute(creator::AlignmentTool::DistributeType::Horizontal));
}

TEST_F(AlignmentToolTest, AlignWithUndo)
{
    SelectNodes({"btn1", "btn2"});

    std::string beforeMargin1 = GetMargin("btn1");
    std::string beforeMargin2 = GetMargin("btn2");

    EXPECT_TRUE(m_tool->Align(creator::AlignmentTool::AlignType::Left));

    // 验证对齐后 margin 已改变
    std::string afterMargin2 = GetMargin("btn2");
    EXPECT_NE(beforeMargin2, afterMargin2);

    // 撤销
    EXPECT_TRUE(m_doc->GetUndoManager().CanUndo());
    m_doc->GetUndoManager().Undo();

    // 验证恢复
    EXPECT_EQ(GetMargin("btn1"), beforeMargin1);
    EXPECT_EQ(GetMargin("btn2"), beforeMargin2);
}
