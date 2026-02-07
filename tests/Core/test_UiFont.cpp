#include <gtest/gtest.h>
#include "duilib/Core/UiFont.h"

using ui::UiFont;
using ui::UiFontEx;

TEST(UiFontTest, DefaultValues)
{
    UiFont font;
    EXPECT_TRUE(font.m_fontName.empty());
    EXPECT_EQ(font.m_fontSize, 0);
    EXPECT_FALSE(font.m_bBold);
    EXPECT_FALSE(font.m_bUnderline);
    EXPECT_FALSE(font.m_bItalic);
    EXPECT_FALSE(font.m_bStrikeOut);
}

TEST(UiFontTest, EqualityOperators)
{
    UiFont a;
    a.m_fontName = _T("Segoe UI");
    a.m_fontSize = 14;
    a.m_bBold = true;

    UiFont b = a;
    UiFont c = a;
    c.m_bItalic = true;

    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a != c);
}

TEST(UiFontTest, FontExCopyFrom)
{
    UiFont src;
    src.m_fontName = _T("Arial");
    src.m_fontSize = 16;
    src.m_bBold = true;
    src.m_bUnderline = true;
    src.m_bItalic = false;
    src.m_bStrikeOut = true;

    UiFontEx dst;
    dst.CopyFrom(src);
    UiFontEx expected;
    expected.CopyFrom(src);

    EXPECT_TRUE(dst == expected);
    EXPECT_EQ(dst.m_fontName, src.m_fontName);
    EXPECT_EQ(dst.m_fontSize, src.m_fontSize);
    EXPECT_EQ(dst.m_bBold, src.m_bBold);
    EXPECT_EQ(dst.m_bUnderline, src.m_bUnderline);
    EXPECT_EQ(dst.m_bItalic, src.m_bItalic);
    EXPECT_EQ(dst.m_bStrikeOut, src.m_bStrikeOut);
}
