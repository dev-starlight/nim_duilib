#include <gtest/gtest.h>
#include "duilib/Utils/FileTime.h"

using ui::FileTime;

TEST(FileTimeTest, DefaultValue)
{
    FileTime ft;
    EXPECT_EQ(ft.GetValue(), static_cast<uint64_t>(0));
}

TEST(FileTimeTest, RoundTripAndToString)
{
    FileTime ft;
#ifdef DUILIB_BUILD_FOR_WIN
    FILETIME input{};
    input.dwLowDateTime = 123456789;
    input.dwHighDateTime = 42;
    ft.FromFileTime(input);
    FILETIME output = ft.ToFileTime();
    EXPECT_EQ(output.dwLowDateTime, input.dwLowDateTime);
    EXPECT_EQ(output.dwHighDateTime, input.dwHighDateTime);
#else
    const uint64_t seconds = 1700000000ULL;
    ft.FromSecondsSinceEpoch(seconds);
    EXPECT_EQ(ft.ToSecondsSinceEpoch(), seconds);
#endif
    const DString text = ft.ToString();
    EXPECT_EQ(text.size(), static_cast<size_t>(19));
    EXPECT_EQ(text[4], _T('-'));
    EXPECT_EQ(text[7], _T('-'));
    EXPECT_EQ(text[10], _T(' '));
    EXPECT_EQ(text[13], _T(':'));
    EXPECT_EQ(text[16], _T(':'));
}
