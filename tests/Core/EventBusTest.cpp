#include <gtest/gtest.h>

#include "duilib/Core/EventBus.h"

#include <string>
#include <vector>

namespace ui {
namespace test {

// 测试用辅助类：模拟具有 WeakFlag 生命周期的对象
class MockSubscriber : public SupportWeakCallback
{
public:
    int callCount = 0;
    EventType lastEventType = kEventNone;
    WPARAM lastWParam = 0;
    LPARAM lastLParam = 0;

    void OnEvent(EventType eventType, WPARAM wParam, LPARAM lParam)
    {
        ++callCount;
        lastEventType = eventType;
        lastWParam = wParam;
        lastLParam = lParam;
    }

    void Reset()
    {
        callCount = 0;
        lastEventType = kEventNone;
        lastWParam = 0;
        lastLParam = 0;
    }
};

/////////////////////////////////////////////////////////////////////////////////////
// EventBus 基础功能测试
/////////////////////////////////////////////////////////////////////////////////////

class EventBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventBus::Instance().Clear();
    }

    void TearDown() override
    {
        EventBus::Instance().Clear();
    }
};

// 1. 构造/初始状态测试
TEST_F(EventBusTest, InitialState)
{
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 0u);
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(EventChannels::kGlobal), 0u);
}

// 2. 基本订阅与发布
TEST_F(EventBusTest, SubscribeAndPublish)
{
    int callCount = 0;
    EventType receivedType = kEventNone;

    auto id = EventBus::Instance().Subscribe(kEventClick,
        [&](EventType et, WPARAM, LPARAM) {
            callCount++;
            receivedType = et;
        });

    EXPECT_NE(id, SubscriptionID::kInvalid);
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 1u);

    // 发布事件
    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(receivedType, kEventClick);

    // 再次发布
    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 2);
}

// 3. 不同事件类型不互相干扰
TEST_F(EventBusTest, DifferentEventTypes)
{
    int clickCount = 0;
    int selectCount = 0;

    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { clickCount++; });

    EventBus::Instance().Subscribe(kEventSelect,
        [&](EventType, WPARAM, LPARAM) { selectCount++; });

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(clickCount, 1);
    EXPECT_EQ(selectCount, 0);

    EventBus::Instance().Publish(kEventSelect);
    EXPECT_EQ(clickCount, 1);
    EXPECT_EQ(selectCount, 1);
}

// 4. 参数传递
TEST_F(EventBusTest, ParameterPassing)
{
    WPARAM receivedW = 0;
    LPARAM receivedL = 0;

    EventBus::Instance().Subscribe(kEventValueChanged,
        [&](EventType, WPARAM w, LPARAM l) {
            receivedW = w;
            receivedL = l;
        });

    EventBus::Instance().Publish(kEventValueChanged, 42, 100);
    EXPECT_EQ(receivedW, 42u);
    EXPECT_EQ(receivedL, 100);
}

// 5. 多个订阅者
TEST_F(EventBusTest, MultipleSubscribers)
{
    int count1 = 0, count2 = 0, count3 = 0;

    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { count1++; });
    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { count2++; });
    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { count3++; });

    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 3u);

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(count1, 1);
    EXPECT_EQ(count2, 1);
    EXPECT_EQ(count3, 1);
}

// 6. 取消订阅
TEST_F(EventBusTest, Unsubscribe)
{
    int callCount = 0;

    auto id = EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { callCount++; });

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1);

    EventBus::Instance().Unsubscribe(id);
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 0u);

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1); // 不再增长
}

// 7. 取消无效ID不崩溃
TEST_F(EventBusTest, UnsubscribeInvalidID)
{
    EventBus::Instance().Unsubscribe(SubscriptionID::kInvalid);
    EventBus::Instance().Unsubscribe(static_cast<SubscriptionID>(99999));
    // 不崩溃即通过
}

// 8. 无回调的订阅返回无效ID
TEST_F(EventBusTest, SubscribeNullCallback)
{
    EventBus::BusCallback nullCb;
    auto id = EventBus::Instance().Subscribe(kEventClick, nullCb);
    EXPECT_EQ(id, SubscriptionID::kInvalid);
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 0u);
}

// 9. 无匹配订阅时发布不崩溃
TEST_F(EventBusTest, PublishWithNoSubscribers)
{
    EventBus::Instance().Publish(kEventClick, 1, 2);
    // 不崩溃即通过
}

/////////////////////////////////////////////////////////////////////////////////////
// 频道（Channel）测试
/////////////////////////////////////////////////////////////////////////////////////

TEST_F(EventBusTest, ChannelIsolation)
{
    int globalCount = 0;
    int windowCount = 0;
    EventChannel windowChannel = EventChannels::FromPointer(reinterpret_cast<void*>(0x1234));

    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { globalCount++; },
        EventChannels::kGlobal);

    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { windowCount++; },
        windowChannel);

    // 发布到全局频道
    EventBus::Instance().Publish(kEventClick, 0, 0, EventChannels::kGlobal);
    EXPECT_EQ(globalCount, 1);
    EXPECT_EQ(windowCount, 0); // 窗口频道不受影响

    // 发布到窗口频道
    EventBus::Instance().Publish(kEventClick, 0, 0, windowChannel);
    EXPECT_EQ(globalCount, 1); // 全局频道不受影响
    EXPECT_EQ(windowCount, 1);
}

TEST_F(EventBusTest, PublishToChannelAndGlobal)
{
    int globalCount = 0;
    int windowCount = 0;
    EventChannel windowChannel = EventChannels::FromPointer(reinterpret_cast<void*>(0x5678));

    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { globalCount++; },
        EventChannels::kGlobal);

    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { windowCount++; },
        windowChannel);

    // 同时发布到窗口频道和全局频道
    EventBus::Instance().PublishToChannelAndGlobal(kEventClick, 0, 0, windowChannel);
    EXPECT_EQ(globalCount, 1);
    EXPECT_EQ(windowCount, 1);
}

TEST_F(EventBusTest, UnsubscribeByChannel)
{
    int globalCount = 0;
    int windowCount = 0;
    EventChannel windowChannel = EventChannels::FromPointer(reinterpret_cast<void*>(0xABCD));

    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { globalCount++; },
        EventChannels::kGlobal);

    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { windowCount++; },
        windowChannel);

    EventBus::Instance().Subscribe(kEventSelect,
        [&](EventType, WPARAM, LPARAM) { windowCount++; },
        windowChannel);

    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 3u);

    // 清理窗口频道
    EventBus::Instance().UnsubscribeByChannel(windowChannel);
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 1u);

    // 窗口频道订阅已被清理
    EventBus::Instance().Publish(kEventClick, 0, 0, windowChannel);
    EXPECT_EQ(windowCount, 0);

    // 全局频道不受影响
    EventBus::Instance().Publish(kEventClick, 0, 0, EventChannels::kGlobal);
    EXPECT_EQ(globalCount, 1);
}

TEST_F(EventBusTest, ChannelSubscriptionCount)
{
    EventChannel ch1 = EventChannels::FromPointer(reinterpret_cast<void*>(0x1));
    EventChannel ch2 = EventChannels::FromPointer(reinterpret_cast<void*>(0x2));

    EventBus::Instance().Subscribe(kEventClick,
        [](EventType, WPARAM, LPARAM) {}, ch1);
    EventBus::Instance().Subscribe(kEventSelect,
        [](EventType, WPARAM, LPARAM) {}, ch1);
    EventBus::Instance().Subscribe(kEventClick,
        [](EventType, WPARAM, LPARAM) {}, ch2);

    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(ch1), 2u);
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(ch2), 1u);
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 3u);
}

/////////////////////////////////////////////////////////////////////////////////////
// SubscribeAll 测试
/////////////////////////////////////////////////////////////////////////////////////

TEST_F(EventBusTest, SubscribeAllReceivesAllEvents)
{
    std::vector<EventType> receivedEvents;

    EventBus::Instance().SubscribeAll(
        [&](EventType et, WPARAM, LPARAM) {
            receivedEvents.push_back(et);
        });

    EventBus::Instance().Publish(kEventClick);
    EventBus::Instance().Publish(kEventSelect);
    EventBus::Instance().Publish(kEventTextChanged);

    ASSERT_EQ(receivedEvents.size(), 3u);
    EXPECT_EQ(receivedEvents[0], kEventClick);
    EXPECT_EQ(receivedEvents[1], kEventSelect);
    EXPECT_EQ(receivedEvents[2], kEventTextChanged);
}

TEST_F(EventBusTest, SubscribeAllWithSpecific)
{
    int allCount = 0;
    int clickCount = 0;

    EventBus::Instance().SubscribeAll(
        [&](EventType, WPARAM, LPARAM) { allCount++; });

    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { clickCount++; });

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(allCount, 1);
    EXPECT_EQ(clickCount, 1);

    EventBus::Instance().Publish(kEventSelect);
    EXPECT_EQ(allCount, 2);
    EXPECT_EQ(clickCount, 1); // 只监听 kEventClick
}

/////////////////////////////////////////////////////////////////////////////////////
// WeakFlag 生命周期管理测试
/////////////////////////////////////////////////////////////////////////////////////

TEST_F(EventBusTest, WeakFlagAutoExpire)
{
    int callCount = 0;

    {
        MockSubscriber subscriber;
        EventBus::Instance().Subscribe(kEventClick,
            [&](EventType, WPARAM, LPARAM) { callCount++; },
            subscriber.GetWeakFlag());

        EventBus::Instance().Publish(kEventClick);
        EXPECT_EQ(callCount, 1); // subscriber 存活，正常回调
    }
    // subscriber 已析构，WeakFlag 已过期

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1); // 不再回调
}

TEST_F(EventBusTest, WeakFlagWithCancel)
{
    int callCount = 0;
    WeakCallbackFlag flag;

    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { callCount++; },
        flag.GetWeakFlag());

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1);

    // 手动取消
    flag.Cancel();

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1); // flag 已取消，不再回调
}

TEST_F(EventBusTest, PurgeExpiredSubscriptions)
{
    {
        MockSubscriber subscriber;
        EventBus::Instance().Subscribe(kEventClick,
            [](EventType, WPARAM, LPARAM) {},
            subscriber.GetWeakFlag());

        EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 1u);
    }
    // subscriber 已析构

    // 订阅还在（只是回调会跳过）
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 1u);

    // 清理过期订阅
    EventBus::Instance().PurgeExpired();
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 0u);
}

/////////////////////////////////////////////////////////////////////////////////////
// ScopedSubscription 测试
/////////////////////////////////////////////////////////////////////////////////////

TEST_F(EventBusTest, ScopedSubscriptionAutoUnsubscribe)
{
    int callCount = 0;

    {
        ScopedSubscription sub(
            EventBus::Instance().Subscribe(kEventClick,
                [&](EventType, WPARAM, LPARAM) { callCount++; })
        );

        EXPECT_TRUE(sub.IsValid());
        EXPECT_NE(sub.GetID(), SubscriptionID::kInvalid);

        EventBus::Instance().Publish(kEventClick);
        EXPECT_EQ(callCount, 1);
    }
    // sub 析构，自动取消订阅

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1); // 不再增长
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 0u);
}

TEST_F(EventBusTest, ScopedSubscriptionDefault)
{
    ScopedSubscription sub;
    EXPECT_FALSE(sub.IsValid());
    EXPECT_EQ(sub.GetID(), SubscriptionID::kInvalid);
    EXPECT_FALSE(static_cast<bool>(sub));
}

TEST_F(EventBusTest, ScopedSubscriptionMove)
{
    int callCount = 0;

    ScopedSubscription sub1(
        EventBus::Instance().Subscribe(kEventClick,
            [&](EventType, WPARAM, LPARAM) { callCount++; })
    );

    // 移动构造
    ScopedSubscription sub2(std::move(sub1));
    EXPECT_FALSE(sub1.IsValid());
    EXPECT_TRUE(sub2.IsValid());

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1);

    // 移动赋值
    ScopedSubscription sub3;
    sub3 = std::move(sub2);
    EXPECT_FALSE(sub2.IsValid());
    EXPECT_TRUE(sub3.IsValid());

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 2);
}

TEST_F(EventBusTest, ScopedSubscriptionReset)
{
    int callCount = 0;

    ScopedSubscription sub(
        EventBus::Instance().Subscribe(kEventClick,
            [&](EventType, WPARAM, LPARAM) { callCount++; })
    );

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1);

    sub.Reset();
    EXPECT_FALSE(sub.IsValid());

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1);
}

TEST_F(EventBusTest, ScopedSubscriptionRelease)
{
    int callCount = 0;

    SubscriptionID releasedId;
    {
        ScopedSubscription sub(
            EventBus::Instance().Subscribe(kEventClick,
                [&](EventType, WPARAM, LPARAM) { callCount++; })
        );

        releasedId = sub.Release();
        EXPECT_FALSE(sub.IsValid());
    }
    // sub 析构但不取消订阅（因为已 Release）

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1); // 仍然有效

    // 手动取消
    EventBus::Instance().Unsubscribe(releasedId);
    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1);
}

/////////////////////////////////////////////////////////////////////////////////////
// SubscriptionGroup 测试
/////////////////////////////////////////////////////////////////////////////////////

TEST_F(EventBusTest, SubscriptionGroupBatchManage)
{
    int count1 = 0, count2 = 0, count3 = 0;

    SubscriptionGroup group;
    group.Add(EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { count1++; }));
    group.Add(EventBus::Instance().Subscribe(kEventSelect,
        [&](EventType, WPARAM, LPARAM) { count2++; }));
    group.Add(EventBus::Instance().Subscribe(kEventTextChanged,
        [&](EventType, WPARAM, LPARAM) { count3++; }));

    EXPECT_EQ(group.GetCount(), 3u);
    EXPECT_FALSE(group.IsEmpty());

    EventBus::Instance().Publish(kEventClick);
    EventBus::Instance().Publish(kEventSelect);
    EXPECT_EQ(count1, 1);
    EXPECT_EQ(count2, 1);
    EXPECT_EQ(count3, 0);

    group.UnsubscribeAll();
    EXPECT_EQ(group.GetCount(), 0u);
    EXPECT_TRUE(group.IsEmpty());
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 0u);

    // 取消后不再回调
    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(count1, 1);
}

TEST_F(EventBusTest, SubscriptionGroupAutoCleanup)
{
    int callCount = 0;

    {
        SubscriptionGroup group;
        group.Add(EventBus::Instance().Subscribe(kEventClick,
            [&](EventType, WPARAM, LPARAM) { callCount++; }));

        EventBus::Instance().Publish(kEventClick);
        EXPECT_EQ(callCount, 1);
    }
    // group 析构，自动取消所有订阅

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 0u);
}

TEST_F(EventBusTest, SubscriptionGroupMove)
{
    int callCount = 0;

    SubscriptionGroup group1;
    group1.Add(EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) { callCount++; }));

    SubscriptionGroup group2(std::move(group1));
    EXPECT_EQ(group1.GetCount(), 0u);
    EXPECT_EQ(group2.GetCount(), 1u);

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1);

    group2.UnsubscribeAll();
    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(callCount, 1);
}

/////////////////////////////////////////////////////////////////////////////////////
// 分发期间取消订阅（延迟删除）测试
/////////////////////////////////////////////////////////////////////////////////////

TEST_F(EventBusTest, UnsubscribeDuringDispatch)
{
    int count1 = 0, count2 = 0;
    SubscriptionID id2 = SubscriptionID::kInvalid;

    // 订阅者1：在回调中取消订阅者2
    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) {
            count1++;
            EventBus::Instance().Unsubscribe(id2); // 在分发中取消订阅
        });

    id2 = EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) {
            count2++;
        });

    // 第一次发布：两个订阅者都应该被调用（延迟删除）
    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(count1, 1);
    EXPECT_EQ(count2, 1);

    // 第二次发布：订阅者2已被删除
    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(count1, 2);
    EXPECT_EQ(count2, 1); // 不再增长
}

/////////////////////////////////////////////////////////////////////////////////////
// Clear 测试
/////////////////////////////////////////////////////////////////////////////////////

TEST_F(EventBusTest, DuplicateUnsubscribeDuringDispatch)
{
    int count1 = 0, count2 = 0;
    SubscriptionID id2 = SubscriptionID::kInvalid;

    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) {
            count1++;
            EventBus::Instance().Unsubscribe(id2);
            EventBus::Instance().Unsubscribe(id2);
            EventBus::Instance().Unsubscribe(id2);
        });

    id2 = EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) {
            count2++;
        });

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(count1, 1);
    EXPECT_EQ(count2, 1);
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 1u);

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(count1, 2);
    EXPECT_EQ(count2, 1);
}

TEST_F(EventBusTest, NestedPublishNoSubscriberDoesNotApplyPendingUnsubscribeEarly)
{
    int count1 = 0, count2 = 0;
    SubscriptionID id2 = SubscriptionID::kInvalid;

    EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) {
            count1++;
            EventBus::Instance().Unsubscribe(id2);
            EventBus::Instance().Publish(kEventSelect);
        });

    id2 = EventBus::Instance().Subscribe(kEventClick,
        [&](EventType, WPARAM, LPARAM) {
            count2++;
        });

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(count1, 1);
    EXPECT_EQ(count2, 1);
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 1u);

    EventBus::Instance().Publish(kEventClick);
    EXPECT_EQ(count1, 2);
    EXPECT_EQ(count2, 1);
}

TEST_F(EventBusTest, ClearAll)
{
    EventBus::Instance().Subscribe(kEventClick,
        [](EventType, WPARAM, LPARAM) {});
    EventBus::Instance().Subscribe(kEventSelect,
        [](EventType, WPARAM, LPARAM) {});
    EventBus::Instance().SubscribeAll(
        [](EventType, WPARAM, LPARAM) {});

    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 3u);

    EventBus::Instance().Clear();
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 0u);
}

TEST_F(EventBusTest, ClearByChannel)
{
    EventChannel ch = EventChannels::FromPointer(reinterpret_cast<void*>(0xFF));

    EventBus::Instance().Subscribe(kEventClick,
        [](EventType, WPARAM, LPARAM) {}, EventChannels::kGlobal);
    EventBus::Instance().Subscribe(kEventClick,
        [](EventType, WPARAM, LPARAM) {}, ch);
    EventBus::Instance().Subscribe(kEventSelect,
        [](EventType, WPARAM, LPARAM) {}, ch);

    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 3u);

    EventBus::Instance().Clear(ch);
    EXPECT_EQ(EventBus::Instance().GetSubscriptionCount(), 1u);
}

/////////////////////////////////////////////////////////////////////////////////////
// SubscriptionID 唯一性测试
/////////////////////////////////////////////////////////////////////////////////////

TEST_F(EventBusTest, UniqueSubscriptionIDs)
{
    std::vector<SubscriptionID> ids;
    for (int i = 0; i < 100; ++i) {
        auto id = EventBus::Instance().Subscribe(kEventClick,
            [](EventType, WPARAM, LPARAM) {});
        EXPECT_NE(id, SubscriptionID::kInvalid);
        // 检查唯一性
        for (const auto& existingId : ids) {
            EXPECT_NE(id, existingId);
        }
        ids.push_back(id);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// EventChannels::FromPointer 测试
/////////////////////////////////////////////////////////////////////////////////////

TEST_F(EventBusTest, EventChannelFromPointer)
{
    int a = 0, b = 0;
    EventChannel chA = EventChannels::FromPointer(&a);
    EventChannel chB = EventChannels::FromPointer(&b);

    // 不同指针产生不同频道
    EXPECT_NE(chA, chB);

    // 相同指针产生相同频道
    EXPECT_EQ(chA, EventChannels::FromPointer(&a));

    // 不等于全局频道
    EXPECT_NE(chA, EventChannels::kGlobal);
}

} // namespace test
} // namespace ui
