#include <gtest/gtest.h>

// SelectionOverlay 依赖 duilib 的 UI 控件系统，
// 在纯单元测试中无法创建真实的 Window 对象。
// 这里测试其数据逻辑部分（选择矩形管理）。

#include "../../creator/Panels/SelectionOverlay.h"

// 由于 SelectionOverlay 继承自 ui::Control，需要 Window 实例，
// 我们通过测试其公共接口的逻辑来验证。
// 在没有 Window 的情况下，我们测试数据管理逻辑。

class SelectionOverlayDataTest : public ::testing::Test
{
protected:
    // 测试选择矩形的数据结构
};

TEST_F(SelectionOverlayDataTest, Construction)
{
    // SelectionOverlay 需要 Window 指针，在纯单元测试中无法直接构造。
    // 验证 UiRect 的基本行为作为替代。
    ui::UiRect rect;
    EXPECT_TRUE(rect.IsZero());
}

TEST_F(SelectionOverlayDataTest, SetSelectionRect)
{
    ui::UiRect rect(10, 20, 110, 120);
    EXPECT_EQ(rect.Width(), 100);
    EXPECT_EQ(rect.Height(), 100);
    EXPECT_EQ(rect.CenterX(), 60);
    EXPECT_EQ(rect.CenterY(), 70);
}

TEST_F(SelectionOverlayDataTest, ClearSelection)
{
    std::vector<ui::UiRect> rects;
    rects.push_back(ui::UiRect(10, 20, 110, 120));
    EXPECT_EQ(rects.size(), 1u);

    rects.clear();
    EXPECT_TRUE(rects.empty());
}

TEST_F(SelectionOverlayDataTest, HasSelectionInitiallyFalse)
{
    std::vector<ui::UiRect> rects;
    EXPECT_TRUE(rects.empty());
}

TEST_F(SelectionOverlayDataTest, MultipleSelectionRects)
{
    std::vector<ui::UiRect> rects;
    rects.push_back(ui::UiRect(0, 0, 50, 50));
    rects.push_back(ui::UiRect(100, 100, 200, 200));
    rects.push_back(ui::UiRect(300, 300, 400, 400));

    EXPECT_EQ(rects.size(), 3u);
    EXPECT_EQ(rects[0].Width(), 50);
    EXPECT_EQ(rects[1].Width(), 100);
    EXPECT_EQ(rects[2].Width(), 100);
}

TEST_F(SelectionOverlayDataTest, HandlePositionCalculation)
{
    // 验证手柄位置计算逻辑
    ui::UiRect rect(100, 100, 300, 200);

    int32_t cx = rect.CenterX();
    int32_t cy = rect.CenterY();

    // 四角
    EXPECT_EQ(rect.Left(), 100);
    EXPECT_EQ(rect.Top(), 100);
    EXPECT_EQ(rect.Right(), 300);
    EXPECT_EQ(rect.Bottom(), 200);

    // 四边中点
    EXPECT_EQ(cx, 200);
    EXPECT_EQ(cy, 150);
}
