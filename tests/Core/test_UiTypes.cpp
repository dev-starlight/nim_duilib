#include <gtest/gtest.h>
#include <cstdint>
#include <limits>
#include "duilib/Core/UiTypes.h"

using ui::UiEstInt;
using ui::UiEstSize;
using ui::UiFixedInt;
using ui::UiFixedSize;
using ui::UiSize;

TEST(UiTypesTest, MakeEstInt)
{
    UiFixedInt fixedInt = UiFixedInt::MakeInt(42);
    UiEstInt est = ui::MakeEstInt(fixedInt);
    EXPECT_TRUE(est.IsInt32());
    EXPECT_EQ(est.GetInt32(), 42);

    fixedInt = UiFixedInt::MakeStretch(25);
    est = ui::MakeEstInt(fixedInt);
    EXPECT_TRUE(est.IsStretch());
    EXPECT_EQ(est.GetStretchPercentValue(), 25);

    fixedInt = UiFixedInt::MakeAuto();
    est = ui::MakeEstInt(fixedInt);
    EXPECT_TRUE(est.IsInt32());
    EXPECT_EQ(est.GetInt32(), 0);
}

TEST(UiTypesTest, StretchAndSizeHelpers)
{
    UiEstInt stretch = UiEstInt::MakeStretch(25);
    EXPECT_EQ(ui::CalcStretchValue(stretch, 200), 50);

    UiFixedInt fixedStretch = UiFixedInt::MakeStretch(50);
    EXPECT_EQ(ui::CalcStretchValue(fixedStretch, 80), 40);

    UiFixedSize fixedSize;
    fixedSize.cx = UiFixedInt::MakeInt(100);
    fixedSize.cy = UiFixedInt::MakeStretch(50);
    UiEstSize estSize = ui::MakeEstSize(fixedSize);
    EXPECT_TRUE(estSize.cx.IsInt32());
    EXPECT_TRUE(estSize.cy.IsStretch());

    UiSize size(9, 7);
    UiEstSize fromSize = ui::MakeEstSize(size);
    EXPECT_EQ(fromSize.cx.GetInt32(), 9);
    EXPECT_EQ(fromSize.cy.GetInt32(), 7);

    UiSize back = ui::MakeSize(fromSize);
    EXPECT_EQ(back.cx, 9);
    EXPECT_EQ(back.cy, 7);
}

TEST(UiTypesTest, TruncateHelpers)
{
    EXPECT_EQ(ui::TruncateToInt32(static_cast<int64_t>(std::numeric_limits<int32_t>::max())), std::numeric_limits<int32_t>::max());
    EXPECT_EQ(ui::TruncateToInt32(static_cast<int64_t>(std::numeric_limits<int32_t>::min())), std::numeric_limits<int32_t>::min());
    EXPECT_EQ(ui::TruncateToUInt16(static_cast<uint32_t>(65535)), static_cast<uint16_t>(65535));
    EXPECT_EQ(ui::TruncateToUInt16(123), static_cast<uint16_t>(123));
    EXPECT_EQ(ui::TruncateToInt16(32767), static_cast<int16_t>(32767));
    EXPECT_EQ(ui::TruncateToInt8(-128), static_cast<int8_t>(-128));
    EXPECT_EQ(ui::TruncateToInt8(127), static_cast<int8_t>(127));
    EXPECT_EQ(ui::TruncateToUInt8(255), static_cast<uint8_t>(255));
}

TEST(UiTypesTest, MathHelpers)
{
    EXPECT_FLOAT_EQ(ui::CEILF(1.2f), 2.0f);
    EXPECT_FLOAT_EQ(ui::CEILF(-1.2f), -2.0f);
    EXPECT_TRUE(ui::IsFloatEqual(1.0f, 1.0f + 0.000001f));
    EXPECT_FALSE(ui::IsFloatEqual(1.0f, 1.01f));
}
