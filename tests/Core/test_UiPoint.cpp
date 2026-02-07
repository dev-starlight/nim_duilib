#include <gtest/gtest.h>
#include "duilib/Core/UiPoint.h"

using ui::UiPoint;

TEST(UiPointTest, DefaultConstructor)
{
    UiPoint pt;
    EXPECT_EQ(pt.x, 0);
    EXPECT_EQ(pt.y, 0);
    EXPECT_EQ(pt.X(), 0);
    EXPECT_EQ(pt.Y(), 0);
}

TEST(UiPointTest, ParameterizedConstructor)
{
    UiPoint pt(10, 20);
    EXPECT_EQ(pt.X(), 10);
    EXPECT_EQ(pt.Y(), 20);
}

TEST(UiPointTest, NegativeValues)
{
    UiPoint pt(-5, -10);
    EXPECT_EQ(pt.X(), -5);
    EXPECT_EQ(pt.Y(), -10);
}

TEST(UiPointTest, SetXY)
{
    UiPoint pt;
    pt.SetXY(100, 200);
    EXPECT_EQ(pt.X(), 100);
    EXPECT_EQ(pt.Y(), 200);
}

TEST(UiPointTest, SetX)
{
    UiPoint pt(1, 2);
    pt.SetX(99);
    EXPECT_EQ(pt.X(), 99);
    EXPECT_EQ(pt.Y(), 2);
}

TEST(UiPointTest, SetY)
{
    UiPoint pt(1, 2);
    pt.SetY(99);
    EXPECT_EQ(pt.X(), 1);
    EXPECT_EQ(pt.Y(), 99);
}

TEST(UiPointTest, OffsetXY)
{
    UiPoint pt(10, 20);
    pt.Offset(5, -3);
    EXPECT_EQ(pt.X(), 15);
    EXPECT_EQ(pt.Y(), 17);
}

TEST(UiPointTest, OffsetByPoint)
{
    UiPoint pt(10, 20);
    UiPoint offset(-10, -20);
    pt.Offset(offset);
    EXPECT_EQ(pt.X(), 0);
    EXPECT_EQ(pt.Y(), 0);
}

TEST(UiPointTest, IsZero_True)
{
    UiPoint pt;
    EXPECT_TRUE(pt.IsZero());
}

TEST(UiPointTest, IsZero_False)
{
    UiPoint pt(1, 0);
    EXPECT_FALSE(pt.IsZero());

    UiPoint pt2(0, 1);
    EXPECT_FALSE(pt2.IsZero());
}

TEST(UiPointTest, EqualsWithValues)
{
    UiPoint pt(3, 4);
    EXPECT_TRUE(pt.Equals(3, 4));
    EXPECT_FALSE(pt.Equals(3, 5));
    EXPECT_FALSE(pt.Equals(4, 4));
}

TEST(UiPointTest, EqualsWithPoint)
{
    UiPoint a(3, 4);
    UiPoint b(3, 4);
    UiPoint c(5, 6);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_FALSE(a.Equals(c));
}

TEST(UiPointTest, OperatorEqual)
{
    UiPoint a(10, 20);
    UiPoint b(10, 20);
    EXPECT_TRUE(a == b);
}

TEST(UiPointTest, OperatorNotEqual)
{
    UiPoint a(10, 20);
    UiPoint b(10, 21);
    EXPECT_TRUE(a != b);
}
