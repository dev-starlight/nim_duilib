#include <gtest/gtest.h>
#include "duilib/Core/ColorManager.h"
#include "duilib/Core/GlobalManager.h"

// --- GlobalManager Mock Implementation ---

namespace ui {

GlobalManager::GlobalManager()
{
}

GlobalManager::~GlobalManager()
{
}

GlobalManager& GlobalManager::Instance()
{
    static GlobalManager gm;
    return gm;
}

ColorManager& GlobalManager::Color()
{
    return m_colorManager;
}

void GlobalManager::Reset()
{
    m_colorManager = ColorManager();
}

}

using ui::ColorManager;
using ui::UiColor;
using ui::ColorMap;

// --- ColorMap Tests ---

TEST(ColorMapTest, AddAndGetColor_ARGB)
{
    ColorMap cm;
    UiColor red(255, 0, 0, 255);
    cm.AddColor(_T("test_red"), red);

    UiColor result = cm.GetColor(_T("test_red"));
    EXPECT_EQ(result.GetARGB(), red.GetARGB());

    UiColor missing = cm.GetColor(_T("missing"));
    EXPECT_TRUE(missing.IsEmpty());
}

TEST(ColorMapTest, AddAndGetColor_String)
{
    ColorMap cm;
    // Note: ColorMap::AddColor(string, string) calls ColorManager::ConvertToUiColor internally
    // which might rely on GlobalManager depending on implementation details of ConvertToUiColor.
    // However, if we pass hex strings, it should be fine without GlobalManager dependency logic.
    cm.AddColor(_T("test_blue"), _T("#FF0000FF"));

    UiColor result = cm.GetColor(_T("test_blue"));
    EXPECT_EQ(result.GetARGB(), 0xFF0000FF);
}

TEST(ColorMapTest, RemoveColor)
{
    ColorMap cm;
    UiColor green(0, 255, 0, 255);
    cm.AddColor(_T("green"), green);

    EXPECT_FALSE(cm.GetColor(_T("green")).IsEmpty());

    cm.RemoveColor(_T("green"));
    EXPECT_TRUE(cm.GetColor(_T("green")).IsEmpty());
}

TEST(ColorMapTest, RemoveAllColors)
{
    ColorMap cm;
    cm.AddColor(_T("c1"), UiColor(1, 1, 1, 1));
    cm.AddColor(_T("c2"), UiColor(2, 2, 2, 2));

    EXPECT_FALSE(cm.GetColor(_T("c1")).IsEmpty());
    EXPECT_FALSE(cm.GetColor(_T("c2")).IsEmpty());

    cm.RemoveAllColors();

    EXPECT_TRUE(cm.GetColor(_T("c1")).IsEmpty());
    EXPECT_TRUE(cm.GetColor(_T("c2")).IsEmpty());
}

// --- ColorManager Tests ---

class ColorManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset GlobalManager to ensure clean state (standard colors loaded, custom colors cleared)
        ui::GlobalManager::Instance().Reset();
    }

    void TearDown() override {
        ui::GlobalManager::Instance().Reset();
    }
};

TEST_F(ColorManagerTest, ConvertToUiColor_Hex)
{
    // Test direct hex conversion (no GlobalManager needed theoretically, but implementation might call it if logic fails)
    UiColor c1 = ColorManager::ConvertToUiColor(_T("#FFFF0000"));
    EXPECT_EQ(c1.GetARGB(), 0xFFFF0000);

    UiColor c2 = ColorManager::ConvertToUiColor(_T("#FF0000")); // No alpha
    EXPECT_EQ(c2.GetARGB(), 0xFFFF0000); // Should add FF alpha

    UiColor c3 = ColorManager::ConvertToUiColor(_T("#00FF00"));
    EXPECT_EQ(c3.GetARGB(), 0xFF00FF00);
}

TEST_F(ColorManagerTest, ConvertToUiColor_Named)
{
    // This relies on GlobalManager::Instance()
    UiColor red = ColorManager::ConvertToUiColor(_T("red"));
    EXPECT_EQ(red.GetARGB(), 0xFFFF0000); // Standard color red is usually FFFF0000

    UiColor blue = ColorManager::ConvertToUiColor(_T("blue"));
    EXPECT_EQ(blue.GetARGB(), 0xFF0000FF);
}

TEST_F(ColorManagerTest, ConvertToUiColor_Invalid)
{
    UiColor c = ColorManager::ConvertToUiColor(_T("invalid_color_name"));
    EXPECT_TRUE(c.IsEmpty());

    c = ColorManager::ConvertToUiColor(_T("#GGGGGG")); // Invalid hex
    EXPECT_TRUE(c.IsEmpty());
}

TEST_F(ColorManagerTest, AddAndGetColor)
{
    ColorManager cm;
    cm.AddColor(_T("custom_color"), UiColor(10, 20, 30, 40));

    UiColor c = cm.GetColor(_T("custom_color"));
    EXPECT_EQ(c.GetAlpha(), 10);
    EXPECT_EQ(c.GetRed(), 20);
    EXPECT_EQ(c.GetGreen(), 30);
    EXPECT_EQ(c.GetBlue(), 40);
}

TEST_F(ColorManagerTest, GetStandardColor)
{
    ColorManager cm;
    // Standard colors are initialized in constructor
    UiColor white = cm.GetStandardColor(_T("white"));
    EXPECT_EQ(white.GetARGB(), 0xFFFFFFFF);

    // Case insensitive
    UiColor white2 = cm.GetStandardColor(_T("WHITE"));
    EXPECT_EQ(white2.GetARGB(), 0xFFFFFFFF);
}

TEST_F(ColorManagerTest, DefaultTextColor)
{
    ColorManager cm;
    EXPECT_TRUE(cm.GetDefaultTextColor().empty());

    cm.SetDefaultTextColor(_T("#FF000000"));
    EXPECT_EQ(cm.GetDefaultTextColor(), _T("#FF000000"));
}

TEST_F(ColorManagerTest, DefaultDisabledTextColor)
{
    ColorManager cm;
    EXPECT_TRUE(cm.GetDefaultDisabledTextColor().empty());

    cm.SetDefaultDisabledTextColor(_T("#FF888888"));
    EXPECT_EQ(cm.GetDefaultDisabledTextColor(), _T("#FF888888"));
}

TEST_F(ColorManagerTest, Clear)
{
    ColorManager cm;
    // Check standard colors exist
    EXPECT_FALSE(cm.GetStandardColor(_T("white")).IsEmpty());

    cm.AddColor(_T("custom"), UiColor(1,2,3,4));
    EXPECT_FALSE(cm.GetColor(_T("custom")).IsEmpty());

    cm.Clear();

    // Clear should remove EVERYTHING including standard colors
    EXPECT_TRUE(cm.GetStandardColor(_T("white")).IsEmpty());
    EXPECT_TRUE(cm.GetColor(_T("custom")).IsEmpty());
}
