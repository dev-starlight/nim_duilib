#include <gtest/gtest.h>

#include "duilib/Core/WindowCreateParam.h"

using ui::WindowCreateParam;

TEST(WindowCreateParamTest, DefaultConstructorValues)
{
    WindowCreateParam param;

    EXPECT_EQ(param.m_platformData, nullptr);
    EXPECT_EQ(param.m_className, _T("duilib_window"));
    EXPECT_EQ(param.m_dwClassStyle, static_cast<uint32_t>(ui::kCS_VREDRAW | ui::kCS_HREDRAW | ui::kCS_DBLCLKS));
    EXPECT_EQ(param.m_dwStyle, 0u);
    EXPECT_EQ(param.m_dwExStyle, 0u);
    EXPECT_TRUE(param.m_windowTitle.empty());
    EXPECT_TRUE(param.m_windowId.empty());
    EXPECT_EQ(param.m_nX, ui::kCW_USEDEFAULT);
    EXPECT_EQ(param.m_nY, ui::kCW_USEDEFAULT);
    EXPECT_EQ(param.m_nWidth, ui::kCW_USEDEFAULT);
    EXPECT_EQ(param.m_nHeight, ui::kCW_USEDEFAULT);
    EXPECT_FALSE(param.m_bCenterWindow);
}

TEST(WindowCreateParamTest, ConstructorWithTitleAndIdNonCenter)
{
    WindowCreateParam param(_T("demo-title"), false, _T("window-01"));

    EXPECT_EQ(param.m_windowTitle, _T("demo-title"));
    EXPECT_EQ(param.m_windowId, _T("window-01"));
    EXPECT_FALSE(param.m_bCenterWindow);
}

TEST(WindowCreateParamTest, ConstructorWithCenterFlag)
{
    WindowCreateParam centered(_T("centered"), true, _T("window-center"));
    EXPECT_EQ(centered.m_windowTitle, _T("centered"));
    EXPECT_EQ(centered.m_windowId, _T("window-center"));
    EXPECT_TRUE(centered.m_bCenterWindow);

    WindowCreateParam notCentered(_T("normal"), false, _T("window-normal"));
    EXPECT_EQ(notCentered.m_windowTitle, _T("normal"));
    EXPECT_EQ(notCentered.m_windowId, _T("window-normal"));
    EXPECT_FALSE(notCentered.m_bCenterWindow);
}
