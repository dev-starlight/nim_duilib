#include <gtest/gtest.h>

#include "duilib/Core/WindowCreateAttributes.h"

using ui::WindowCreateAttributes;

TEST(WindowCreateAttributesTest, DefaultValues)
{
    WindowCreateAttributes attrs;

    EXPECT_FALSE(attrs.m_bUseSystemCaption);
    EXPECT_FALSE(attrs.m_bUseSystemCaptionDefined);

    EXPECT_TRUE(attrs.m_rcSizeBox.IsZero());
    EXPECT_FALSE(attrs.m_bSizeBoxDefined);

    EXPECT_TRUE(attrs.m_rcCaption.IsZero());
    EXPECT_FALSE(attrs.m_bCaptionDefined);

    EXPECT_TRUE(attrs.m_bShadowAttached);
    EXPECT_FALSE(attrs.m_bShadowAttachedDefined);
    EXPECT_TRUE(attrs.m_rcShadowCorner.IsEmpty());

    EXPECT_TRUE(attrs.m_bIsLayeredWindow);
    EXPECT_FALSE(attrs.m_bIsLayeredWindowDefined);

    EXPECT_EQ(attrs.m_nLayeredWindowAlpha, 255u);
    EXPECT_FALSE(attrs.m_bLayeredWindowAlphaDefined);

    EXPECT_EQ(attrs.m_nLayeredWindowOpacity, 255u);
    EXPECT_FALSE(attrs.m_bLayeredWindowOpacityDefined);

    EXPECT_TRUE(attrs.m_szInitSize.IsEmpty());
    EXPECT_FALSE(attrs.m_bInitSizeDefined);

    EXPECT_TRUE(attrs.m_sdlRenderName.empty());
}
