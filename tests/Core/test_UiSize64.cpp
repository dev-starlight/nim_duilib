#include <gtest/gtest.h>
#include "duilib/Core/UiSize64.h"

using ui::UiSize64;

TEST(UiSize64Test, DefaultAndSetters)
{
    UiSize64 s;
    EXPECT_TRUE(s.IsEmpty());
    s.SetWidthHeight(123456789LL, 987654321LL);
    EXPECT_EQ(s.Width(), 123456789LL);
    EXPECT_EQ(s.Height(), 987654321LL);
    s.SetWidth(1LL);
    s.SetHeight(2LL);
    EXPECT_EQ(s.Width(), 1LL);
    EXPECT_EQ(s.Height(), 2LL);
}

TEST(UiSize64Test, ClearAndEquals)
{
    UiSize64 a(100LL, 200LL);
    UiSize64 b = a;
    EXPECT_TRUE(b.Equals(100LL, 200LL));
    EXPECT_TRUE(a == b);
    b.Clear();
    EXPECT_TRUE(b.IsEmpty());
    EXPECT_TRUE(a != b);
}
