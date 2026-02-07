#include <gtest/gtest.h>

#include "../../creator/Core/ControlNameTagger.h"

#include <memory>
#include <string>

class ControlNameTaggerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_tagger = std::make_unique<creator::ControlNameTagger>();
    }

    std::unique_ptr<creator::ControlNameTagger> m_tagger;
    pugi::xml_document m_xmlDoc;
};

TEST_F(ControlNameTaggerTest, TagNodesAssignsUniqueIds)
{
    const std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Window>
    <VBox>
        <Button/>
        <Label/>
    </VBox>
</Window>)";

    ASSERT_TRUE(m_xmlDoc.load_string(xml.c_str()));
    pugi::xml_node root = m_xmlDoc.first_child();

    m_tagger->TagNodes(root);

    const auto& idMap = m_tagger->GetIdToNodeMap();
    // Window + VBox + Button + Label = 4 nodes
    EXPECT_EQ(idMap.size(), 4u);

    // 每个 ID 都是唯一的
    std::set<std::string> ids;
    for (const auto& pair : idMap) {
        EXPECT_TRUE(pair.first.find("__cr_") == 0);
        ids.insert(pair.first);
    }
    EXPECT_EQ(ids.size(), 4u);

    // 验证节点的 name 属性已被设置
    EXPECT_STREQ(root.attribute("name").as_string(), "__cr_0");
}

TEST_F(ControlNameTaggerTest, TagNodesPreservesExistingNames)
{
    const std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Window>
    <Button name="ok_btn"/>
</Window>)";

    ASSERT_TRUE(m_xmlDoc.load_string(xml.c_str()));
    pugi::xml_node root = m_xmlDoc.first_child();

    m_tagger->TagNodes(root);

    // Button 节点应该有 __cr_orig_name 备份
    pugi::xml_node button = root.first_child();
    EXPECT_FALSE(button.attribute("__cr_orig_name").empty());
    EXPECT_STREQ(button.attribute("__cr_orig_name").as_string(), "ok_btn");

    // name 应该被替换为 __cr_N
    std::string nameVal = button.attribute("name").as_string();
    EXPECT_TRUE(nameVal.find("__cr_") == 0);
}

TEST_F(ControlNameTaggerTest, UntagNodesRemovesCreatorIds)
{
    const std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Window>
    <Button name="ok_btn"/>
    <Label/>
</Window>)";

    ASSERT_TRUE(m_xmlDoc.load_string(xml.c_str()));
    pugi::xml_node root = m_xmlDoc.first_child();

    m_tagger->TagNodes(root);
    m_tagger->UntagNodes(root);

    // Button 的 name 应该恢复为 "ok_btn"
    pugi::xml_node button = root.first_child();
    EXPECT_STREQ(button.attribute("name").as_string(), "ok_btn");
    EXPECT_TRUE(button.attribute("__cr_orig_name").empty());

    // Label 不应该有 name 属性（原来没有）
    pugi::xml_node label = button.next_sibling();
    EXPECT_TRUE(label.attribute("name").empty());

    // Window 不应该有 name 属性（原来没有）
    EXPECT_TRUE(root.attribute("name").empty());
}

TEST_F(ControlNameTaggerTest, FindNodeByCreatorId)
{
    const std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Window>
    <VBox>
        <Button/>
    </VBox>
</Window>)";

    ASSERT_TRUE(m_xmlDoc.load_string(xml.c_str()));
    pugi::xml_node root = m_xmlDoc.first_child();

    m_tagger->TagNodes(root);

    // 查找 __cr_0 应该是 Window
    pugi::xml_node found0 = m_tagger->FindNodeByCreatorId(root, "__cr_0");
    EXPECT_FALSE(found0.empty());
    EXPECT_STREQ(found0.name(), "Window");

    // 查找 __cr_2 应该是 Button
    pugi::xml_node found2 = m_tagger->FindNodeByCreatorId(root, "__cr_2");
    EXPECT_FALSE(found2.empty());
    EXPECT_STREQ(found2.name(), "Button");

    // 查找不存在的 ID
    pugi::xml_node notFound = m_tagger->FindNodeByCreatorId(root, "__cr_999");
    EXPECT_TRUE(notFound.empty());
}

TEST_F(ControlNameTaggerTest, TagEmptyDocument)
{
    pugi::xml_node emptyNode;
    // 不应崩溃
    m_tagger->TagNodes(emptyNode);
    EXPECT_TRUE(m_tagger->GetIdToNodeMap().empty());
}

TEST_F(ControlNameTaggerTest, TagDeepNestedTree)
{
    const std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Window>
    <VBox>
        <HBox>
            <VBox>
                <Button/>
                <Label/>
                <CheckBox/>
            </VBox>
        </HBox>
    </VBox>
</Window>)";

    ASSERT_TRUE(m_xmlDoc.load_string(xml.c_str()));
    pugi::xml_node root = m_xmlDoc.first_child();

    m_tagger->TagNodes(root);

    const auto& idMap = m_tagger->GetIdToNodeMap();
    // Window + VBox + HBox + VBox + Button + Label + CheckBox = 7 nodes
    EXPECT_EQ(idMap.size(), 7u);

    // 验证所有节点都有 __cr_ 前缀的 name
    for (const auto& pair : idMap) {
        EXPECT_TRUE(pair.first.find("__cr_") == 0);
        EXPECT_FALSE(pair.second.empty());
    }
}
