#include <gtest/gtest.h>
#include "duilib/Core/UiPadding.h"

using ui::UiPadding;

TEST(UiPaddingTest, DefaultConstructor)
{
    UiPadding p;
    EXPECT_EQ(p.left, 0);
    EXPECT_EQ(p.top, 0);
    EXPECT_EQ(p.right, 0);
    EXPECT_EQ(p.bottom, 0);
}

TEST(UiPaddingTest, ParameterizedConstructor)
{
    UiPadding p(1, 2, 3, 4);
    EXPECT_EQ(p.left, 1);
    EXPECT_EQ(p.top, 2);
    EXPECT_EQ(p.right, 3);
    EXPECT_EQ(p.bottom, 4);
}

TEST(UiPaddingTest, Clear)
{
    UiPadding p(1, 2, 3, 4);
    p.Clear();
    EXPECT_EQ(p.left, 0);
    EXPECT_EQ(p.top, 0);
    EXPECT_EQ(p.right, 0);
    EXPECT_EQ(p.bottom, 0);
}

TEST(UiPaddingTest, IsEmpty_True)
{
    UiPadding p;
    EXPECT_TRUE(p.IsEmpty());
}

TEST(UiPaddingTest, IsEmpty_False)
{
    UiPadding p(0, 0, 0, 1);
    EXPECT_FALSE(p.IsEmpty());
}

TEST(UiPaddingTest, Validate_NegativeValues)
{
    UiPadding p(-1, -2, -3, -4);
    p.Validate();
    EXPECT_EQ(p.left, 0);
    EXPECT_EQ(p.top, 0);
    EXPECT_EQ(p.right, 0);
    EXPECT_EQ(p.bottom, 0);
}

TEST(UiPaddingTest, Validate_PositiveValues)
{
    UiPadding p(1, 2, 3, 4);
    p.Validate();
    EXPECT_EQ(p.left, 1);
    EXPECT_EQ(p.top, 2);
    EXPECT_EQ(p.right, 3);
    EXPECT_EQ(p.bottom, 4);
}

TEST(UiPaddingTest, Validate_MixedValues)
{
    UiPadding p(-1, 2, -3, 4);
    p.Validate();
    EXPECT_EQ(p.left, 0);
    EXPECT_EQ(p.top, 2);
    EXPECT_EQ(p.right, 0);
    EXPECT_EQ(p.bottom, 4);
}

TEST(UiPaddingTest, Equals)
{
    UiPadding a(1, 2, 3, 4);
    UiPadding b(1, 2, 3, 4);
    UiPadding c(1, 2, 3, 5);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_FALSE(a.Equals(c));
}

TEST(UiPaddingTest, OperatorEqual)
{
    UiPadding a(1, 2, 3, 4);
    UiPadding b(1, 2, 3, 4);
    EXPECT_TRUE(a == b);
}

TEST(UiPaddingTest, OperatorNotEqual)
{
    UiPadding a(1, 2, 3, 4);
    UiPadding b(1, 2, 3, 5);
    EXPECT_TRUE(a != b);
}
