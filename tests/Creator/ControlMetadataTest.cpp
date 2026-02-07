#include <gtest/gtest.h>

#include "../../creator/Core/ControlMetadata.h"

#include <memory>

class ControlMetadataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_metadata = std::make_unique<creator::ControlMetadata>();
        m_metadata->Initialize();
    }

    std::unique_ptr<creator::ControlMetadata> m_metadata;
};

TEST_F(ControlMetadataTest, Construction)
{
    const auto types = m_metadata->GetAllControlTypes();
    EXPECT_GT(types.size(), 0u);
}

TEST_F(ControlMetadataTest, BasicControlsRegistered)
{
    EXPECT_TRUE(m_metadata->IsRegistered("Label"));
    EXPECT_TRUE(m_metadata->IsRegistered("Button"));
    EXPECT_TRUE(m_metadata->IsRegistered("CheckBox"));
    EXPECT_TRUE(m_metadata->IsRegistered("Control"));
}

TEST_F(ControlMetadataTest, ContainerControlsRegistered)
{
    EXPECT_TRUE(m_metadata->IsRegistered("Box"));
    EXPECT_TRUE(m_metadata->IsRegistered("HBox"));
    EXPECT_TRUE(m_metadata->IsRegistered("VBox"));
    EXPECT_TRUE(m_metadata->IsRegistered("TabBox"));
}

TEST_F(ControlMetadataTest, GetPropertiesIncludesBase)
{
    const auto props = m_metadata->GetProperties("Label");
    EXPECT_GT(props.size(), 5u);

    bool hasName = false;
    for (const auto& p : props) {
        if (p.name == "name") {
            hasName = true;
            break;
        }
    }
    EXPECT_TRUE(hasName);
}

TEST_F(ControlMetadataTest, LabelSpecificProperties)
{
    const auto props = m_metadata->GetProperties("Label");

    bool hasText = false;
    for (const auto& p : props) {
        if (p.name == "text") {
            hasText = true;
            break;
        }
    }
    EXPECT_TRUE(hasText);
}

TEST_F(ControlMetadataTest, UnregisteredControlReturnsBaseOnly)
{
    const auto baseProps = m_metadata->GetProperties("Control");
    const auto unknownProps = m_metadata->GetProperties("NonexistentControl");
    EXPECT_EQ(unknownProps.size(), baseProps.size());
    EXPECT_GT(unknownProps.size(), 0u);
}

TEST_F(ControlMetadataTest, IsContainer)
{
    EXPECT_TRUE(m_metadata->IsContainer("Box"));
    EXPECT_TRUE(m_metadata->IsContainer("HBox"));
    EXPECT_TRUE(m_metadata->IsContainer("VBox"));
    EXPECT_FALSE(m_metadata->IsContainer("Label"));
    EXPECT_FALSE(m_metadata->IsContainer("Button"));
}

TEST_F(ControlMetadataTest, CategoriesContainLabelType)
{
    const auto& categories = m_metadata->GetCategories();
    EXPECT_GT(categories.size(), 0u);

    bool hasLabel = false;
    for (const auto& cat : categories) {
        for (const auto& t : cat.controlTypes) {
            if (t == "Label") {
                hasLabel = true;
                break;
            }
        }
        if (hasLabel) {
            break;
        }
    }
    EXPECT_TRUE(hasLabel);
}

TEST_F(ControlMetadataTest, ComboPropertyHasOptions)
{
    const auto props = m_metadata->GetProperties("Control");
    bool found = false;
    for (const auto& p : props) {
        if (p.name == "halign") {
            found = true;
            EXPECT_EQ(p.type, creator::PropertyType::kCombo);
            EXPECT_GT(p.options.size(), 0u);
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(ControlMetadataTest, RegisterCustomControl)
{
    std::vector<creator::PropertyDef> props = {
        {"custom_prop", "Custom Property", "Custom", creator::PropertyType::kString, ""}
    };
    m_metadata->RegisterControl("MyCustomControl", props);

    EXPECT_TRUE(m_metadata->IsRegistered("MyCustomControl"));
    const auto allProps = m_metadata->GetProperties("MyCustomControl");

    bool hasCustom = false;
    for (const auto& p : allProps) {
        if (p.name == "custom_prop") {
            hasCustom = true;
            break;
        }
    }
    EXPECT_TRUE(hasCustom);
}

TEST_F(ControlMetadataTest, ControlTypeCount)
{
    const auto types = m_metadata->GetAllControlTypes();
    EXPECT_GE(types.size(), 15u);
}
