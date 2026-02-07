#include <gtest/gtest.h>
#include "duilib/Core/UiFixedInt.h"

using ui::UiFixedInt;

TEST(UiFixedIntTest, DefaultAndInt32)
{
    UiFixedInt value;
    EXPECT_TRUE(value.IsInt32());
    EXPECT_FALSE(value.IsStretch());
    EXPECT_FALSE(value.IsAuto());
    EXPECT_TRUE(value.IsValid());
    EXPECT_EQ(value.GetInt32(), 0);

    value.SetInt32(42);
    EXPECT_TRUE(value.IsInt32());
    EXPECT_EQ(value.GetInt32(), 42);
}

TEST(UiFixedIntTest, StretchAndAuto)
{
    UiFixedInt stretch = UiFixedInt::MakeStretch(25);
    EXPECT_TRUE(stretch.IsStretch());
    EXPECT_EQ(stretch.GetStretchPercentValue(), 25);
    EXPECT_EQ(stretch.GetInt32(), 0);

    UiFixedInt autoValue = UiFixedInt::MakeAuto();
    EXPECT_TRUE(autoValue.IsAuto());
    EXPECT_TRUE(autoValue.IsValid());
    EXPECT_EQ(autoValue.GetInt32(), 0);
}

TEST(UiFixedIntTest, EqualityAndInvalidInt32)
{
    UiFixedInt a = UiFixedInt::MakeInt(10);
    UiFixedInt b = UiFixedInt::MakeInt(10);
    UiFixedInt c = UiFixedInt::MakeStretch();
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a != c);

    UiFixedInt invalid(-1);
    EXPECT_FALSE(invalid.IsValid());
    EXPECT_EQ(invalid.GetInt32(), 0);
}
