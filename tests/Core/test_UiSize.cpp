#include <gtest/gtest.h>
#include "duilib/Core/UiSize.h"

using ui::UiSize;

TEST(UiSizeTest, DefaultConstructor)
{
    UiSize s;
    EXPECT_EQ(s.Width(), 0);
    EXPECT_EQ(s.Height(), 0);
}

TEST(UiSizeTest, ParameterizedConstructor)
{
    UiSize s(100, 200);
    EXPECT_EQ(s.Width(), 100);
    EXPECT_EQ(s.Height(), 200);
}

TEST(UiSizeTest, CopyConstructor)
{
    UiSize a(10, 20);
    UiSize b(a);
    EXPECT_EQ(b.Width(), 10);
    EXPECT_EQ(b.Height(), 20);
}

TEST(UiSizeTest, AssignmentOperator)
{
    UiSize a(10, 20);
    UiSize b;
    b = a;
    EXPECT_EQ(b.Width(), 10);
    EXPECT_EQ(b.Height(), 20);
}

TEST(UiSizeTest, SetWidthHeight)
{
    UiSize s;
    s.SetWidthHeight(50, 60);
    EXPECT_EQ(s.Width(), 50);
    EXPECT_EQ(s.Height(), 60);
}

TEST(UiSizeTest, SetWidth)
{
    UiSize s(1, 2);
    s.SetWidth(99);
    EXPECT_EQ(s.Width(), 99);
    EXPECT_EQ(s.Height(), 2);
}

TEST(UiSizeTest, SetHeight)
{
    UiSize s(1, 2);
    s.SetHeight(99);
    EXPECT_EQ(s.Width(), 1);
    EXPECT_EQ(s.Height(), 99);
}

TEST(UiSizeTest, Validate_NegativeValues)
{
    UiSize s(-10, -20);
    s.Validate();
    EXPECT_EQ(s.Width(), 0);
    EXPECT_EQ(s.Height(), 0);
}

TEST(UiSizeTest, Validate_PositiveValues)
{
    UiSize s(10, 20);
    s.Validate();
    EXPECT_EQ(s.Width(), 10);
    EXPECT_EQ(s.Height(), 20);
}

TEST(UiSizeTest, Validate_MixedValues)
{
    UiSize s(-5, 10);
    s.Validate();
    EXPECT_EQ(s.Width(), 0);
    EXPECT_EQ(s.Height(), 10);
}

TEST(UiSizeTest, IsValid)
{
    EXPECT_TRUE(UiSize(0, 0).IsValid());
    EXPECT_TRUE(UiSize(10, 20).IsValid());
    EXPECT_FALSE(UiSize(-1, 0).IsValid());
    EXPECT_FALSE(UiSize(0, -1).IsValid());
}

TEST(UiSizeTest, IsEmpty)
{
    EXPECT_TRUE(UiSize(0, 0).IsEmpty());
    EXPECT_FALSE(UiSize(1, 0).IsEmpty());
    EXPECT_FALSE(UiSize(0, 1).IsEmpty());
    EXPECT_FALSE(UiSize(1, 1).IsEmpty());
}

TEST(UiSizeTest, Clear)
{
    UiSize s(10, 20);
    s.Clear();
    EXPECT_EQ(s.Width(), 0);
    EXPECT_EQ(s.Height(), 0);
    EXPECT_TRUE(s.IsEmpty());
}

TEST(UiSizeTest, EqualsWithValues)
{
    UiSize s(10, 20);
    EXPECT_TRUE(s.Equals(10, 20));
    EXPECT_FALSE(s.Equals(10, 21));
}

TEST(UiSizeTest, EqualsWithSize)
{
    UiSize a(10, 20);
    UiSize b(10, 20);
    UiSize c(11, 20);
    EXPECT_TRUE(a.Equals(b));
    EXPECT_FALSE(a.Equals(c));
}

TEST(UiSizeTest, OperatorEqual)
{
    UiSize a(10, 20);
    UiSize b(10, 20);
    EXPECT_TRUE(a == b);
}

TEST(UiSizeTest, OperatorNotEqual)
{
    UiSize a(10, 20);
    UiSize b(10, 21);
    EXPECT_TRUE(a != b);
}
