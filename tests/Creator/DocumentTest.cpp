#include <gtest/gtest.h>

#include "../../creator/Core/Document.h"

#include <memory>

class DocumentTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_doc = std::make_unique<creator::Document>();
    }

    std::unique_ptr<creator::Document> m_doc;
};

TEST_F(DocumentTest, Construction)
{
    EXPECT_FALSE(m_doc->IsModified());
    EXPECT_TRUE(m_doc->GetRootNode().empty());
}

TEST_F(DocumentTest, CreateNewDocument)
{
    m_doc->CreateNewDocument();

    EXPECT_FALSE(m_doc->IsModified());
    EXPECT_FALSE(m_doc->GetRootNode().empty());
    EXPECT_STREQ(m_doc->GetRootNode().name(), "Window");
}

TEST_F(DocumentTest, LoadFromString)
{
    const std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Window size="800,600">
    <VBox name="main">
        <Label text="Hello"/>
    </VBox>
</Window>)";

    EXPECT_TRUE(m_doc->LoadFromString(xml));
    EXPECT_FALSE(m_doc->GetRootNode().empty());
    EXPECT_STREQ(m_doc->GetRootNode().name(), "Window");
}

TEST_F(DocumentTest, ToXmlString)
{
    m_doc->CreateNewDocument();
    const std::string xml = m_doc->ToXmlString();

    EXPECT_FALSE(xml.empty());
    EXPECT_NE(xml.find("Window"), std::string::npos);
    EXPECT_NE(xml.find("size=\"800,600\""), std::string::npos);
}

TEST_F(DocumentTest, AddControl)
{
    m_doc->CreateNewDocument();
    pugi::xml_node root = m_doc->GetRootNode();

    pugi::xml_node button = m_doc->AddControl(root, _T("Button"));
    EXPECT_FALSE(button.empty());
    EXPECT_STREQ(button.name(), "Button");
    EXPECT_TRUE(m_doc->IsModified());
}

TEST_F(DocumentTest, RemoveControl)
{
    m_doc->CreateNewDocument();
    pugi::xml_node root = m_doc->GetRootNode();

    pugi::xml_node button = m_doc->AddControl(root, _T("Button"));
    ASSERT_FALSE(button.empty());

    const bool removed = m_doc->RemoveControl(button);
    EXPECT_TRUE(removed);
}

TEST_F(DocumentTest, SetAndGetAttribute)
{
    m_doc->CreateNewDocument();
    pugi::xml_node root = m_doc->GetRootNode();

    pugi::xml_node label = m_doc->AddControl(root, _T("Label"));
    m_doc->SetNodeAttribute(label, "text", "Hello World");

    const std::string value = m_doc->GetNodeAttribute(label, "text");
    EXPECT_EQ(value, "Hello World");
}

TEST_F(DocumentTest, RemoveAttribute)
{
    m_doc->CreateNewDocument();
    pugi::xml_node root = m_doc->GetRootNode();

    pugi::xml_node label = m_doc->AddControl(root, _T("Label"));
    m_doc->SetNodeAttribute(label, "text", "test");
    m_doc->RemoveNodeAttribute(label, "text");

    const std::string value = m_doc->GetNodeAttribute(label, "text");
    EXPECT_TRUE(value.empty());
}

TEST_F(DocumentTest, FindNodeByName)
{
    const std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Window>
    <VBox name="container">
        <Button name="ok_btn"/>
    </VBox>
</Window>)";

    ASSERT_TRUE(m_doc->LoadFromString(xml));
    pugi::xml_node found = m_doc->FindNodeByName(_T("ok_btn"));
    EXPECT_FALSE(found.empty());
    EXPECT_STREQ(found.name(), "Button");
}

TEST_F(DocumentTest, FindNodeByNameNotFound)
{
    m_doc->CreateNewDocument();
    pugi::xml_node found = m_doc->FindNodeByName(_T("nonexistent"));
    EXPECT_TRUE(found.empty());
}

TEST_F(DocumentTest, ObserverNotification)
{
    int notifyCount = 0;
    m_doc->AddObserver([&notifyCount](creator::ChangeType, pugi::xml_node) {
        ++notifyCount;
    });

    m_doc->CreateNewDocument();
    EXPECT_EQ(notifyCount, 1);

    pugi::xml_node root = m_doc->GetRootNode();
    m_doc->AddControl(root, _T("Button"));
    EXPECT_EQ(notifyCount, 2);
}

TEST_F(DocumentTest, EdgeCasesEmptyNode)
{
    pugi::xml_node emptyNode;

    pugi::xml_node result = m_doc->AddControl(emptyNode, _T("Label"));
    EXPECT_TRUE(result.empty());

    EXPECT_FALSE(m_doc->RemoveControl(emptyNode));

    m_doc->SetNodeAttribute(emptyNode, "key", "value");
    EXPECT_EQ(m_doc->GetNodeAttribute(emptyNode, "key"), "");
}

TEST_F(DocumentTest, LoadInvalidXml)
{
    EXPECT_FALSE(m_doc->LoadFromString("<invalid xml<<<"));
}
