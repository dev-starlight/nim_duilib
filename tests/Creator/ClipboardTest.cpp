#include <gtest/gtest.h>
#include "../../creator/Core/Clipboard.h"
#include "../../creator/Core/Clipboard.cpp"

class ClipboardTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_clipboard = std::make_unique<creator::Clipboard>();
    }

    void TearDown() override
    {
        m_clipboard.reset();
    }

    std::unique_ptr<creator::Clipboard> m_clipboard;
};

// 1. 构造测试
TEST_F(ClipboardTest, Construction)
{
    EXPECT_FALSE(m_clipboard->HasContent());
    EXPECT_TRUE(m_clipboard->GetContent().empty());
}

// 2. 复制节点
TEST_F(ClipboardTest, CopyNode)
{
    pugi::xml_document doc;
    doc.load_string("<Window><Button name=\"btn1\" text=\"OK\"/></Window>");
    pugi::xml_node btn = doc.child("Window").child("Button");

    EXPECT_TRUE(m_clipboard->Copy(btn));
    EXPECT_TRUE(m_clipboard->HasContent());
    EXPECT_FALSE(m_clipboard->GetContent().empty());

    // 验证内容包含 Button 信息
    std::string content = m_clipboard->GetContent();
    EXPECT_NE(content.find("Button"), std::string::npos);
    EXPECT_NE(content.find("btn1"), std::string::npos);
}

// 3. 复制空节点
TEST_F(ClipboardTest, CopyEmptyNode)
{
    pugi::xml_node emptyNode;
    EXPECT_FALSE(m_clipboard->Copy(emptyNode));
    EXPECT_FALSE(m_clipboard->HasContent());
}

// 4. 粘贴到目标节点
TEST_F(ClipboardTest, PasteToTarget)
{
    // 先复制一个节点
    pugi::xml_document srcDoc;
    srcDoc.load_string("<Window><Label text=\"Hello\"/></Window>");
    pugi::xml_node label = srcDoc.child("Window").child("Label");
    EXPECT_TRUE(m_clipboard->Copy(label));

    // 粘贴到目标
    pugi::xml_document targetDoc;
    targetDoc.load_string("<Window><VBox name=\"body\"/></Window>");
    pugi::xml_node body = targetDoc.child("Window").child("VBox");

    pugi::xml_node pasted = m_clipboard->Paste(body);
    EXPECT_FALSE(pasted.empty());
    EXPECT_STREQ(pasted.name(), "Label");
    EXPECT_STREQ(pasted.attribute("text").as_string(), "Hello");
}

// 5. 粘贴到空目标
TEST_F(ClipboardTest, PasteToEmptyTarget)
{
    m_clipboard->Copy(pugi::xml_node()); // 复制失败
    pugi::xml_node emptyNode;
    pugi::xml_node result = m_clipboard->Paste(emptyNode);
    EXPECT_TRUE(result.empty());
}

// 6. 清空剪贴板
TEST_F(ClipboardTest, Clear)
{
    pugi::xml_document doc;
    doc.load_string("<Window><Button/></Window>");
    m_clipboard->Copy(doc.child("Window").child("Button"));
    EXPECT_TRUE(m_clipboard->HasContent());

    m_clipboard->Clear();
    EXPECT_FALSE(m_clipboard->HasContent());
    EXPECT_TRUE(m_clipboard->GetContent().empty());
}

// 7. 多次复制覆盖
TEST_F(ClipboardTest, CopyOverwrite)
{
    pugi::xml_document doc;
    doc.load_string("<Window><Button name=\"a\"/><Label name=\"b\"/></Window>");

    m_clipboard->Copy(doc.child("Window").child("Button"));
    EXPECT_NE(m_clipboard->GetContent().find("Button"), std::string::npos);

    m_clipboard->Copy(doc.child("Window").child("Label"));
    EXPECT_NE(m_clipboard->GetContent().find("Label"), std::string::npos);
    // Button 不应该再在内容中
    EXPECT_EQ(m_clipboard->GetContent().find("Button"), std::string::npos);
}

// 8. 粘贴带子节点的控件
TEST_F(ClipboardTest, PasteWithChildren)
{
    pugi::xml_document srcDoc;
    srcDoc.load_string("<Window><VBox name=\"container\"><Label text=\"A\"/><Button text=\"B\"/></VBox></Window>");
    pugi::xml_node vbox = srcDoc.child("Window").child("VBox");

    EXPECT_TRUE(m_clipboard->Copy(vbox));

    pugi::xml_document targetDoc;
    targetDoc.load_string("<Window/>");
    pugi::xml_node root = targetDoc.child("Window");

    pugi::xml_node pasted = m_clipboard->Paste(root);
    EXPECT_FALSE(pasted.empty());
    EXPECT_STREQ(pasted.name(), "VBox");

    // 验证子节点也被粘贴
    int childCount = 0;
    for (pugi::xml_node child = pasted.first_child(); child; child = child.next_sibling()) {
        childCount++;
    }
    EXPECT_EQ(childCount, 2);
}
