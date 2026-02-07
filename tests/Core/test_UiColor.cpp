#include <gtest/gtest.h>
#include "duilib/Core/UiColor.h"

using ui::UiColor;

// --- 构造函数 ---

TEST(UiColorTest, DefaultConstructor)
{
    UiColor c;
    EXPECT_EQ(c.GetARGB(), 0u);
    EXPECT_TRUE(c.IsEmpty());
}

TEST(UiColorTest, RGBConstructor)
{
    UiColor c(255, 128, 0);
    EXPECT_EQ(c.GetAlpha(), 255);
    EXPECT_EQ(c.GetRed(), 255);
    EXPECT_EQ(c.GetGreen(), 128);
    EXPECT_EQ(c.GetBlue(), 0);
}

TEST(UiColorTest, ARGBConstructor)
{
    UiColor c(128, 255, 128, 0);
    EXPECT_EQ(c.GetAlpha(), 128);
    EXPECT_EQ(c.GetRed(), 255);
    EXPECT_EQ(c.GetGreen(), 128);
    EXPECT_EQ(c.GetBlue(), 0);
}

TEST(UiColorTest, ExplicitARGBValueConstructor)
{
    UiColor c(UiColor::ARGB(0xFF804020));
    EXPECT_EQ(c.GetAlpha(), 0xFF);
    EXPECT_EQ(c.GetRed(), 0x80);
    EXPECT_EQ(c.GetGreen(), 0x40);
    EXPECT_EQ(c.GetBlue(), 0x20);
}

// --- Getters ---

TEST(UiColorTest, GetAlpha)
{
    UiColor c(100, 200, 150, 50);
    EXPECT_EQ(c.GetAlpha(), 100);
    EXPECT_EQ(c.GetA(), 100);
}

TEST(UiColorTest, GetRed)
{
    UiColor c(100, 200, 150, 50);
    EXPECT_EQ(c.GetRed(), 200);
    EXPECT_EQ(c.GetR(), 200);
}

TEST(UiColorTest, GetGreen)
{
    UiColor c(100, 200, 150, 50);
    EXPECT_EQ(c.GetGreen(), 150);
    EXPECT_EQ(c.GetG(), 150);
}

TEST(UiColorTest, GetBlue)
{
    UiColor c(100, 200, 150, 50);
    EXPECT_EQ(c.GetBlue(), 50);
    EXPECT_EQ(c.GetB(), 50);
}

TEST(UiColorTest, GetARGB)
{
    UiColor c(0xFF, 0x80, 0x40, 0x20);
    EXPECT_EQ(c.GetARGB(), 0xFF804020u);
}

// --- IsEmpty ---

TEST(UiColorTest, IsEmpty_True)
{
    UiColor c;
    EXPECT_TRUE(c.IsEmpty());
}

TEST(UiColorTest, IsEmpty_False)
{
    UiColor c(255, 0, 0);
    EXPECT_FALSE(c.IsEmpty());
}

// --- SetARGB ---

TEST(UiColorTest, SetARGB)
{
    UiColor c;
    c.SetARGB(0xAABBCCDD);
    EXPECT_EQ(c.GetAlpha(), 0xAA);
    EXPECT_EQ(c.GetRed(), 0xBB);
    EXPECT_EQ(c.GetGreen(), 0xCC);
    EXPECT_EQ(c.GetBlue(), 0xDD);
}

TEST(UiColorTest, SetFromCOLORREFAndToCOLORREF)
{
    UiColor c;
    c.SetFromCOLORREF(RGB(0x11, 0x22, 0x33));
    EXPECT_EQ(c.GetA(), 0xFF);
    EXPECT_EQ(c.GetR(), 0x11);
    EXPECT_EQ(c.GetG(), 0x22);
    EXPECT_EQ(c.GetB(), 0x33);
    EXPECT_EQ(c.ToCOLORREF(), RGB(0x11, 0x22, 0x33));
}

// --- MakeARGB ---

TEST(UiColorTest, MakeARGB)
{
    UiColor::ARGB argb = UiColor::MakeARGB(0xFF, 0x11, 0x22, 0x33);
    EXPECT_EQ(argb, 0xFF112233u);
}

TEST(UiColorTest, MakeARGB_Transparent)
{
    UiColor::ARGB argb = UiColor::MakeARGB(0, 0, 0, 0);
    EXPECT_EQ(argb, 0u);
}

TEST(UiColorTest, MakeARGB_White)
{
    UiColor::ARGB argb = UiColor::MakeARGB(255, 255, 255, 255);
    EXPECT_EQ(argb, 0xFFFFFFFFu);
}

// --- Equals / Operators ---

TEST(UiColorTest, Equals)
{
    UiColor a(255, 128, 0);
    UiColor b(255, 128, 0);
    UiColor c(255, 128, 1);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_FALSE(a.Equals(c));
}

TEST(UiColorTest, OperatorEqual)
{
    UiColor a(255, 128, 0);
    UiColor b(255, 128, 0);
    EXPECT_TRUE(a == b);
}

TEST(UiColorTest, OperatorNotEqual)
{
    UiColor a(255, 128, 0);
    UiColor b(255, 128, 1);
    EXPECT_TRUE(a != b);
}
