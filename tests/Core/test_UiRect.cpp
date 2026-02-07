#include <gtest/gtest.h>
#include "duilib/Core/UiRect.h"

using ui::UiRect;
using ui::UiPoint;
using ui::UiSize;
using ui::UiMargin;
using ui::UiPadding;

// --- 构造函数 ---

TEST(UiRectTest, DefaultConstructor)
{
    UiRect r;
    EXPECT_EQ(r.left, 0);
    EXPECT_EQ(r.top, 0);
    EXPECT_EQ(r.right, 0);
    EXPECT_EQ(r.bottom, 0);
}

TEST(UiRectTest, ParameterizedConstructor)
{
    UiRect r(10, 20, 110, 220);
    EXPECT_EQ(r.left, 10);
    EXPECT_EQ(r.top, 20);
    EXPECT_EQ(r.right, 110);
    EXPECT_EQ(r.bottom, 220);
}

// --- 基本属性 ---

TEST(UiRectTest, WidthHeight)
{
    UiRect r(10, 20, 110, 220);
    EXPECT_EQ(r.Width(), 100);
    EXPECT_EQ(r.Height(), 200);
}

TEST(UiRectTest, Accessors)
{
    UiRect r(1, 2, 3, 4);
    EXPECT_EQ(r.Left(), 1);
    EXPECT_EQ(r.Top(), 2);
    EXPECT_EQ(r.Right(), 3);
    EXPECT_EQ(r.Bottom(), 4);
}

TEST(UiRectTest, CenterXY)
{
    UiRect r(0, 0, 100, 200);
    EXPECT_EQ(r.CenterX(), 50);
    EXPECT_EQ(r.CenterY(), 100);
}

TEST(UiRectTest, Center)
{
    UiRect r(10, 20, 110, 220);
    UiPoint c = r.Center();
    EXPECT_EQ(c.X(), 60);
    EXPECT_EQ(c.Y(), 120);
}

// --- 状态判断 ---

TEST(UiRectTest, IsZero)
{
    EXPECT_TRUE(UiRect().IsZero());
    EXPECT_TRUE(UiRect(0, 0, 0, 0).IsZero());
    EXPECT_FALSE(UiRect(1, 0, 0, 0).IsZero());
}

TEST(UiRectTest, IsEmpty)
{
    EXPECT_TRUE(UiRect(0, 0, 0, 0).IsEmpty());
    EXPECT_TRUE(UiRect(10, 10, 10, 10).IsEmpty());   // width=0
    EXPECT_TRUE(UiRect(10, 10, 5, 20).IsEmpty());     // width<0
    EXPECT_FALSE(UiRect(0, 0, 10, 10).IsEmpty());
}

TEST(UiRectTest, Clear)
{
    UiRect r(10, 20, 30, 40);
    r.Clear();
    EXPECT_TRUE(r.IsZero());
}

TEST(UiRectTest, Validate)
{
    UiRect r(10, 20, 5, 10);  // right < left, bottom < top
    r.Validate();
    EXPECT_EQ(r.left, 10);
    EXPECT_EQ(r.right, 10);   // corrected to left
    EXPECT_EQ(r.top, 20);
    EXPECT_EQ(r.bottom, 20);  // corrected to top
}

TEST(UiRectTest, Validate_AlreadyValid)
{
    UiRect r(10, 20, 30, 40);
    r.Validate();
    EXPECT_EQ(r.left, 10);
    EXPECT_EQ(r.right, 30);
}

// --- 偏移 ---

TEST(UiRectTest, OffsetXY)
{
    UiRect r(10, 20, 30, 40);
    r.Offset(5, 10);
    EXPECT_EQ(r.left, 15);
    EXPECT_EQ(r.top, 30);
    EXPECT_EQ(r.right, 35);
    EXPECT_EQ(r.bottom, 50);
}

TEST(UiRectTest, OffsetByPoint)
{
    UiRect r(10, 20, 30, 40);
    r.Offset(UiPoint(-10, -20));
    EXPECT_EQ(r.left, 0);
    EXPECT_EQ(r.top, 0);
    EXPECT_EQ(r.right, 20);
    EXPECT_EQ(r.bottom, 20);
}

// --- Inflate ---

TEST(UiRectTest, Inflate_DxDy)
{
    UiRect r(10, 20, 110, 120);
    r.Inflate(5, 10);
    EXPECT_EQ(r.left, 5);
    EXPECT_EQ(r.top, 10);
    EXPECT_EQ(r.right, 115);
    EXPECT_EQ(r.bottom, 130);
}

TEST(UiRectTest, Inflate_Margin)
{
    UiRect r(10, 20, 110, 120);
    UiMargin m(1, 2, 3, 4);
    r.Inflate(m);
    EXPECT_EQ(r.left, 9);
    EXPECT_EQ(r.top, 18);
    EXPECT_EQ(r.right, 113);
    EXPECT_EQ(r.bottom, 124);
}

TEST(UiRectTest, Inflate_Padding)
{
    UiRect r(10, 20, 110, 120);
    UiPadding p(1, 2, 3, 4);
    r.Inflate(p);
    EXPECT_EQ(r.left, 9);
    EXPECT_EQ(r.top, 18);
    EXPECT_EQ(r.right, 113);
    EXPECT_EQ(r.bottom, 124);
}

TEST(UiRectTest, Inflate_FourValues)
{
    UiRect r(10, 20, 110, 120);
    r.Inflate(1, 2, 3, 4);
    EXPECT_EQ(r.left, 9);
    EXPECT_EQ(r.top, 18);
    EXPECT_EQ(r.right, 113);
    EXPECT_EQ(r.bottom, 124);
}

// --- Deflate ---

TEST(UiRectTest, Deflate_DxDy)
{
    UiRect r(10, 20, 110, 120);
    r.Deflate(5, 10);
    EXPECT_EQ(r.left, 15);
    EXPECT_EQ(r.top, 30);
    EXPECT_EQ(r.right, 105);
    EXPECT_EQ(r.bottom, 110);
}

TEST(UiRectTest, Deflate_Padding)
{
    UiRect r(10, 20, 110, 120);
    UiPadding p(1, 2, 3, 4);
    r.Deflate(p);
    EXPECT_EQ(r.left, 11);
    EXPECT_EQ(r.top, 22);
    EXPECT_EQ(r.right, 107);
    EXPECT_EQ(r.bottom, 116);
}

TEST(UiRectTest, Deflate_Margin)
{
    UiRect r(10, 20, 110, 120);
    UiMargin m(1, 2, 3, 4);
    r.Deflate(m);
    EXPECT_EQ(r.left, 11);
    EXPECT_EQ(r.top, 22);
    EXPECT_EQ(r.right, 107);
    EXPECT_EQ(r.bottom, 116);
}

TEST(UiRectTest, Deflate_FourValues)
{
    UiRect r(10, 20, 110, 120);
    r.Deflate(1, 2, 3, 4);
    EXPECT_EQ(r.left, 11);
    EXPECT_EQ(r.top, 22);
    EXPECT_EQ(r.right, 107);
    EXPECT_EQ(r.bottom, 116);
}

// --- Union ---

TEST(UiRectTest, Union_BothNonEmpty)
{
    UiRect a(10, 10, 50, 50);
    UiRect b(30, 30, 80, 80);
    EXPECT_TRUE(a.Union(b));
    EXPECT_EQ(a.left, 10);
    EXPECT_EQ(a.top, 10);
    EXPECT_EQ(a.right, 80);
    EXPECT_EQ(a.bottom, 80);
}

TEST(UiRectTest, Union_OtherEmpty)
{
    UiRect a(10, 10, 50, 50);
    UiRect empty(0, 0, 0, 0);
    EXPECT_TRUE(a.Union(empty));
    // a should remain unchanged
    EXPECT_EQ(a.left, 10);
    EXPECT_EQ(a.top, 10);
    EXPECT_EQ(a.right, 50);
    EXPECT_EQ(a.bottom, 50);
}

TEST(UiRectTest, Union_SelfEmpty)
{
    UiRect a(0, 0, 0, 0);
    UiRect b(10, 10, 50, 50);
    EXPECT_TRUE(a.Union(b));
    // a should become b
    EXPECT_EQ(a.left, 10);
    EXPECT_EQ(a.top, 10);
    EXPECT_EQ(a.right, 50);
    EXPECT_EQ(a.bottom, 50);
}

TEST(UiRectTest, Union_BothEmpty)
{
    UiRect a(0, 0, 0, 0);
    UiRect b(0, 0, 0, 0);
    EXPECT_FALSE(a.Union(b));
}

// --- Intersect ---

TEST(UiRectTest, Intersect_HasOverlap)
{
    UiRect a(10, 10, 50, 50);
    UiRect b(30, 30, 80, 80);
    EXPECT_TRUE(a.Intersect(b));
    EXPECT_EQ(a.left, 30);
    EXPECT_EQ(a.top, 30);
    EXPECT_EQ(a.right, 50);
    EXPECT_EQ(a.bottom, 50);
}

TEST(UiRectTest, Intersect_NoOverlap)
{
    UiRect a(10, 10, 20, 20);
    UiRect b(30, 30, 40, 40);
    EXPECT_FALSE(a.Intersect(b));
}

TEST(UiRectTest, Intersect_Static)
{
    UiRect a(10, 10, 50, 50);
    UiRect b(30, 30, 80, 80);
    UiRect c;
    EXPECT_TRUE(UiRect::Intersect(c, a, b));
    EXPECT_EQ(c.left, 30);
    EXPECT_EQ(c.top, 30);
    EXPECT_EQ(c.right, 50);
    EXPECT_EQ(c.bottom, 50);
}

TEST(UiRectTest, Intersect_Static_NoOverlap)
{
    UiRect a(10, 10, 20, 20);
    UiRect b(30, 30, 40, 40);
    UiRect c;
    EXPECT_FALSE(UiRect::Intersect(c, a, b));
}

// --- ContainsPt ---

TEST(UiRectTest, ContainsPt_Inside)
{
    UiRect r(0, 0, 100, 100);
    EXPECT_TRUE(r.ContainsPt(UiPoint(50, 50)));
    EXPECT_TRUE(r.ContainsPt(50, 50));
}

TEST(UiRectTest, ContainsPt_OnLeftTopBorder)
{
    UiRect r(0, 0, 100, 100);
    // left and top borders are inclusive
    EXPECT_TRUE(r.ContainsPt(UiPoint(0, 0)));
}

TEST(UiRectTest, ContainsPt_OnRightBottomBorder)
{
    UiRect r(0, 0, 100, 100);
    // right and bottom borders are exclusive
    EXPECT_FALSE(r.ContainsPt(UiPoint(100, 100)));
    EXPECT_FALSE(r.ContainsPt(UiPoint(100, 50)));
    EXPECT_FALSE(r.ContainsPt(UiPoint(50, 100)));
}

TEST(UiRectTest, ContainsPt_Outside)
{
    UiRect r(10, 10, 50, 50);
    EXPECT_FALSE(r.ContainsPt(UiPoint(0, 0)));
    EXPECT_FALSE(r.ContainsPt(UiPoint(60, 60)));
}

// --- ContainsRect ---

TEST(UiRectTest, ContainsRect_FullyContained)
{
    UiRect outer(0, 0, 100, 100);
    UiRect inner(10, 10, 50, 50);
    EXPECT_TRUE(outer.ContainsRect(inner));
}

TEST(UiRectTest, ContainsRect_Same)
{
    UiRect r(10, 10, 50, 50);
    EXPECT_TRUE(r.ContainsRect(r));
}

TEST(UiRectTest, ContainsRect_PartialOverlap)
{
    UiRect a(0, 0, 50, 50);
    UiRect b(25, 25, 75, 75);
    EXPECT_FALSE(a.ContainsRect(b));
}

TEST(UiRectTest, ContainsRect_EmptyRect)
{
    UiRect outer(0, 0, 100, 100);
    UiRect empty(0, 0, 0, 0);
    EXPECT_FALSE(outer.ContainsRect(empty));
}

// --- Equals / Operators ---

TEST(UiRectTest, Equals)
{
    UiRect a(1, 2, 3, 4);
    UiRect b(1, 2, 3, 4);
    UiRect c(1, 2, 3, 5);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_FALSE(a.Equals(c));
}

TEST(UiRectTest, OperatorEqual)
{
    UiRect a(1, 2, 3, 4);
    UiRect b(1, 2, 3, 4);
    EXPECT_TRUE(a == b);
}

TEST(UiRectTest, OperatorNotEqual)
{
    UiRect a(1, 2, 3, 4);
    UiRect b(1, 2, 3, 5);
    EXPECT_TRUE(a != b);
}
