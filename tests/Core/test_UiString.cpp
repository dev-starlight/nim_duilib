#include <gtest/gtest.h>
#include <string_view>
#include "duilib/Core/UiString.h"

using ui::UiString;

TEST(UiStringTest, DefaultAndEmpty)
{
    UiString text;
    EXPECT_TRUE(text.empty());
    EXPECT_STREQ(text.c_str(), _T(""));
}

TEST(UiStringTest, ConstructAssignAndEquals)
{
    UiString text(_T("hello"));
    EXPECT_FALSE(text.empty());
    EXPECT_TRUE(text.equals(_T("hello")));
    EXPECT_TRUE(text == _T("hello"));
    EXPECT_TRUE(text != _T("world"));

    text = _T("world");
    EXPECT_TRUE(text.equals(_T("world")));

    text = DString(_T("value"));
    EXPECT_TRUE(text.equals(_T("value")));

    text = std::basic_string_view<DString::value_type>(_T("view"));
    EXPECT_TRUE(text.equals(_T("view")));
}

TEST(UiStringTest, CopyAndClear)
{
    UiString a(_T("copy-source"));
    UiString b(a);
    EXPECT_TRUE(b.equals(a));

    UiString c;
    c = a;
    EXPECT_TRUE(c.equals(a));

    c.clear();
    EXPECT_TRUE(c.empty());
    EXPECT_TRUE(c.equals(_T("")));
}

TEST(UiStringTest, CompareWithDString)
{
    UiString text(_T("duilib"));
    const DString same = _T("duilib");
    const DString diff = _T("nim");

    EXPECT_TRUE(text == same);
    EXPECT_TRUE(same == text);
    EXPECT_TRUE(text != diff);
    EXPECT_TRUE(diff != text);
}
