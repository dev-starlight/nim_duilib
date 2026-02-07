#include <gtest/gtest.h>
#include "duilib/Core/UiPointF.h"
#include "duilib/Core/UiRectF.h"

using ui::UiMargin;
using ui::UiPadding;
using ui::UiPoint;
using ui::UiPointF;
using ui::UiRectF;

TEST(UiRectFTest, BasicProperties)
{
    UiRectF r(10.0f, 20.0f, 110.0f, 220.0f);
    EXPECT_FLOAT_EQ(r.Width(), 100.0f);
    EXPECT_FLOAT_EQ(r.Height(), 200.0f);
    EXPECT_FLOAT_EQ(r.CenterX(), 60.0f);
    EXPECT_FLOAT_EQ(r.CenterY(), 120.0f);
    UiPointF c = r.Center();
    EXPECT_FLOAT_EQ(c.X(), 60.0f);
    EXPECT_FLOAT_EQ(c.Y(), 120.0f);
}

TEST(UiRectFTest, ValidateAndClear)
{
    UiRectF r(10.0f, 20.0f, 5.0f, 10.0f);
    EXPECT_TRUE(r.IsEmpty());
    r.Validate();
    EXPECT_FLOAT_EQ(r.right, 10.0f);
    EXPECT_FLOAT_EQ(r.bottom, 20.0f);
    r.Clear();
    EXPECT_TRUE(r.IsZero());
}

TEST(UiRectFTest, OffsetInflateDeflate)
{
    UiRectF r(10.0f, 20.0f, 30.0f, 40.0f);
    r.Offset(5.0f, -10.0f);
    EXPECT_FLOAT_EQ(r.left, 15.0f);
    EXPECT_FLOAT_EQ(r.top, 10.0f);

    r.Offset(UiPoint(5, 5));
    EXPECT_FLOAT_EQ(r.left, 20.0f);
    EXPECT_FLOAT_EQ(r.top, 15.0f);

    r.Inflate(UiMargin(1, 2, 3, 4));
    EXPECT_FLOAT_EQ(r.left, 19.0f);
    EXPECT_FLOAT_EQ(r.top, 13.0f);
    EXPECT_FLOAT_EQ(r.right, 43.0f);
    EXPECT_FLOAT_EQ(r.bottom, 39.0f);

    r.Deflate(UiPadding(1, 2, 3, 4));
    EXPECT_FLOAT_EQ(r.left, 20.0f);
    EXPECT_FLOAT_EQ(r.top, 15.0f);
    EXPECT_FLOAT_EQ(r.right, 40.0f);
    EXPECT_FLOAT_EQ(r.bottom, 35.0f);
}

TEST(UiRectFTest, UnionAndIntersect)
{
    UiRectF a(10.0f, 10.0f, 50.0f, 50.0f);
    UiRectF b(30.0f, 30.0f, 80.0f, 80.0f);
    EXPECT_TRUE(a.Union(b));
    EXPECT_TRUE(a.Equals(UiRectF(10.0f, 10.0f, 80.0f, 80.0f)));

    UiRectF c(10.0f, 10.0f, 50.0f, 50.0f);
    EXPECT_TRUE(c.Intersect(b));
    EXPECT_TRUE(c.Equals(UiRectF(30.0f, 30.0f, 50.0f, 50.0f)));

    UiRectF d;
    EXPECT_TRUE(UiRectF::Intersect(d, UiRectF(0.0f, 0.0f, 10.0f, 10.0f), UiRectF(5.0f, 5.0f, 20.0f, 20.0f)));
    EXPECT_TRUE(d.Equals(UiRectF(5.0f, 5.0f, 10.0f, 10.0f)));
}

TEST(UiRectFTest, ContainsChecks)
{
    UiRectF r(0.0f, 0.0f, 100.0f, 100.0f);
    EXPECT_TRUE(r.ContainsPt(50.0f, 50.0f));
    EXPECT_TRUE(r.ContainsPt(UiPoint(0, 0)));
    EXPECT_FALSE(r.ContainsPt(100.0f, 100.0f));
    EXPECT_TRUE(r.ContainsRect(UiRectF(10.0f, 10.0f, 20.0f, 20.0f)));
    EXPECT_FALSE(r.ContainsRect(UiRectF(10.0f, 10.0f, 120.0f, 20.0f)));
}
