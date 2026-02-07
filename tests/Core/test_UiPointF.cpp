#include <gtest/gtest.h>
#include "duilib/Core/UiPointF.h"

using ui::UiPointF;

TEST(UiPointFTest, DefaultConstructor)
{
    UiPointF pt;
    EXPECT_FLOAT_EQ(pt.X(), 0.0f);
    EXPECT_FLOAT_EQ(pt.Y(), 0.0f);
    EXPECT_TRUE(pt.IsZero());
}

TEST(UiPointFTest, SetAndOffset)
{
    UiPointF pt(1.5f, 2.5f);
    pt.SetXY(3.0f, 4.0f);
    pt.Offset(1.0f, -2.0f);
    EXPECT_FLOAT_EQ(pt.X(), 4.0f);
    EXPECT_FLOAT_EQ(pt.Y(), 2.0f);
}

TEST(UiPointFTest, OffsetByPoint)
{
    UiPointF pt(10.0f, 20.0f);
    pt.Offset(UiPointF(-5.0f, -7.5f));
    EXPECT_FLOAT_EQ(pt.X(), 5.0f);
    EXPECT_FLOAT_EQ(pt.Y(), 12.5f);
}

TEST(UiPointFTest, EqualityOperators)
{
    UiPointF a(1.0f, 2.0f);
    UiPointF b(1.0f, 2.0f);
    UiPointF c(1.0f, 2.5f);
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a != c);
    EXPECT_TRUE(a.Equals(1.0f, 2.0f));
}
