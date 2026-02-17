#include <gtest/gtest.h>
#include "duilib/Core/ControlFactory.h"
#include "duilib/Core/Control.h"

// Define a MockControl for testing
class MockControl : public ui::Control
{
public:
    MockControl(ui::Window* pWindow) : ui::Control(pWindow) {}
    virtual ui::DString GetType() const override { return _T("MockControl"); }
};

TEST(ControlFactoryTest, RegisterAndCreate)
{
    // Register the mock control
    ui::ControlFactory::Instance().Register<MockControl>("MockControl");

    // Verify it is registered
    EXPECT_TRUE(ui::ControlFactory::Instance().IsRegistered("MockControl"));

    // Create the control
    ui::Control* pControl = ui::ControlFactory::Instance().Create("MockControl", nullptr);
    ASSERT_NE(pControl, nullptr);

    // Verify the type
    EXPECT_EQ(pControl->GetType(), _T("MockControl"));

    delete pControl;
}

TEST(ControlFactoryTest, CreateUnregistered)
{
    // Try to create a non-existent control
    ui::Control* pControl = ui::ControlFactory::Instance().Create("NonExistentControl", nullptr);
    EXPECT_EQ(pControl, nullptr);
}

TEST(ControlFactoryTest, IsRegistered)
{
    EXPECT_FALSE(ui::ControlFactory::Instance().IsRegistered("AnotherUnregistered"));

    ui::ControlFactory::Instance().Register<MockControl>("AnotherRegistered");
    EXPECT_TRUE(ui::ControlFactory::Instance().IsRegistered("AnotherRegistered"));
}

TEST(ControlFactoryTest, GetRegisteredNames)
{
    ui::ControlFactory::Instance().Register<MockControl>("UniqueControlForList");

    std::vector<std::string> names = ui::ControlFactory::Instance().GetRegisteredNames();
    bool found = false;
    for (const auto& name : names) {
        if (name == "UniqueControlForList") {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}
