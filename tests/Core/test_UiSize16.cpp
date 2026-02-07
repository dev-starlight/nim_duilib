#include <gtest/gtest.h>
#include "duilib/Core/UiSize16.h"

using ui::UiSize16;

TEST(UiSize16Test, DefaultAndSetters)
{
    UiSize16 s;
    EXPECT_TRUE(s.IsEmpty());
    s.SetWidthHeight(12, 34);
    EXPECT_EQ(s.Width(), 12);
    EXPECT_EQ(s.Height(), 34);
    s.SetWidth(56);
    s.SetHeight(78);
    EXPECT_EQ(s.Width(), 56);
    EXPECT_EQ(s.Height(), 78);
}

TEST(UiSize16Test, ValidateAndClear)
{
    UiSize16 s(-1, 10);
    EXPECT_FALSE(s.IsValid());
    s.Validate();
    EXPECT_TRUE(s.IsValid());
    EXPECT_EQ(s.Width(), 0);
    EXPECT_EQ(s.Height(), 10);
    s.Clear();
    EXPECT_TRUE(s.IsEmpty());
}

TEST(UiSize16Test, Equality)
{
    UiSize16 a(10, 20);
    UiSize16 b(10, 20);
    UiSize16 c(10, 21);
    EXPECT_TRUE(a.Equals(10, 20));
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a != c);
}
