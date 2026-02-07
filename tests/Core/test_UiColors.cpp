#include <gtest/gtest.h>
#include <set>
#include <utility>
#include <vector>
#include "duilib/Core/UiColors.h"

TEST(UiColorsTest, GetUiColorsStringBasic)
{
    std::vector<std::pair<DString, int32_t>> colors;
    ui::UiColors::GetUiColorsString(colors);

    EXPECT_FALSE(colors.empty());
    EXPECT_GT(colors.size(), static_cast<size_t>(100));

    bool foundWhite = false;
    bool foundBlack = false;
    bool foundTransparent = false;
    for (const auto& kv : colors) {
        if (kv.first == _T("White")) {
            foundWhite = true;
            EXPECT_EQ(kv.second, ui::UiColors::White);
        }
        if (kv.first == _T("Black")) {
            foundBlack = true;
            EXPECT_EQ(kv.second, ui::UiColors::Black);
        }
        if (kv.first == _T("Transparent")) {
            foundTransparent = true;
            EXPECT_EQ(kv.second, ui::UiColors::Transparent);
        }
    }
    EXPECT_TRUE(foundWhite);
    EXPECT_TRUE(foundBlack);
    EXPECT_TRUE(foundTransparent);
}

TEST(UiColorsTest, NamesAreUnique)
{
    std::vector<std::pair<DString, int32_t>> colors;
    ui::UiColors::GetUiColorsString(colors);

    std::set<DString> uniqueNames;
    for (const auto& kv : colors) {
        uniqueNames.insert(kv.first);
    }
    EXPECT_EQ(uniqueNames.size(), colors.size());
}

TEST(UiColorsTest, KnownAliasesShareSameValue)
{
    EXPECT_EQ(ui::UiColors::Aqua, ui::UiColors::Cyan);
    EXPECT_EQ(ui::UiColors::Magenta, ui::UiColors::Fuchsia);
}
