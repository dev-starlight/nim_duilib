#include <gtest/gtest.h>
#include "duilib/Core/UiEstInt.h"

using ui::UiEstInt;

TEST(UiEstIntTest, DefaultAndInt32)
{
    UiEstInt value;
    EXPECT_TRUE(value.IsInt32());
    EXPECT_FALSE(value.IsStretch());
    EXPECT_TRUE(value.IsValid());
    EXPECT_EQ(value.GetInt32(), 0);

    value.SetInt32(55);
    EXPECT_TRUE(value.IsInt32());
    EXPECT_EQ(value.GetInt32(), 55);
}

TEST(UiEstIntTest, StretchValue)
{
    UiEstInt stretch = UiEstInt::MakeStretch(35);
    EXPECT_TRUE(stretch.IsStretch());
    EXPECT_EQ(stretch.GetStretchPercentValue(), 35);
    EXPECT_EQ(stretch.GetInt32(), 0);
}

TEST(UiEstIntTest, EqualityAndInvalidInt32)
{
    UiEstInt a(10);
    UiEstInt b(10);
    UiEstInt c = UiEstInt::MakeStretch();
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a != c);

    UiEstInt invalid(-1);
    EXPECT_FALSE(invalid.IsValid());
    EXPECT_EQ(invalid.GetInt32(), 0);
}
