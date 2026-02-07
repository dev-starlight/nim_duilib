#include <gtest/gtest.h>
#include "duilib/Core/UiMargin.h"

using ui::UiMargin;

TEST(UiMarginTest, DefaultConstructor)
{
    UiMargin m;
    EXPECT_EQ(m.left, 0);
    EXPECT_EQ(m.top, 0);
    EXPECT_EQ(m.right, 0);
    EXPECT_EQ(m.bottom, 0);
}

TEST(UiMarginTest, ParameterizedConstructor)
{
    UiMargin m(1, 2, 3, 4);
    EXPECT_EQ(m.left, 1);
    EXPECT_EQ(m.top, 2);
    EXPECT_EQ(m.right, 3);
    EXPECT_EQ(m.bottom, 4);
}

TEST(UiMarginTest, Clear)
{
    UiMargin m(1, 2, 3, 4);
    m.Clear();
    EXPECT_EQ(m.left, 0);
    EXPECT_EQ(m.top, 0);
    EXPECT_EQ(m.right, 0);
    EXPECT_EQ(m.bottom, 0);
}

TEST(UiMarginTest, IsEmpty_True)
{
    UiMargin m;
    EXPECT_TRUE(m.IsEmpty());
}

TEST(UiMarginTest, IsEmpty_False)
{
    UiMargin m(1, 0, 0, 0);
    EXPECT_FALSE(m.IsEmpty());
}

TEST(UiMarginTest, Validate_NegativeValues)
{
    UiMargin m(-1, -2, -3, -4);
    m.Validate();
    EXPECT_EQ(m.left, 0);
    EXPECT_EQ(m.top, 0);
    EXPECT_EQ(m.right, 0);
    EXPECT_EQ(m.bottom, 0);
}

TEST(UiMarginTest, Validate_PositiveValues)
{
    UiMargin m(1, 2, 3, 4);
    m.Validate();
    EXPECT_EQ(m.left, 1);
    EXPECT_EQ(m.top, 2);
    EXPECT_EQ(m.right, 3);
    EXPECT_EQ(m.bottom, 4);
}

TEST(UiMarginTest, Validate_MixedValues)
{
    UiMargin m(-1, 2, -3, 4);
    m.Validate();
    EXPECT_EQ(m.left, 0);
    EXPECT_EQ(m.top, 2);
    EXPECT_EQ(m.right, 0);
    EXPECT_EQ(m.bottom, 4);
}

TEST(UiMarginTest, Equals)
{
    UiMargin a(1, 2, 3, 4);
    UiMargin b(1, 2, 3, 4);
    UiMargin c(1, 2, 3, 5);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_FALSE(a.Equals(c));
}

TEST(UiMarginTest, OperatorEqual)
{
    UiMargin a(1, 2, 3, 4);
    UiMargin b(1, 2, 3, 4);
    EXPECT_TRUE(a == b);
}

TEST(UiMarginTest, OperatorNotEqual)
{
    UiMargin a(1, 2, 3, 4);
    UiMargin b(1, 2, 3, 5);
    EXPECT_TRUE(a != b);
}
