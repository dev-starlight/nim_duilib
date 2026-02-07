#ifndef UI_CORE_EVENT_BUS_H_
#define UI_CORE_EVENT_BUS_H_

#include "duilib/duilib_defs.h"
#include "duilib/Core/Callback.h"
#include <functional>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <cassert>

namespace ui
{

class Control;
struct EventArgs;

/////////////////////////////////////////////////////////////////////////////////////
// SubscriptionID — 订阅句柄，强类型，防止误用
/////////////////////////////////////////////////////////////////////////////////////

/** 事件订阅的唯一标识
*/
enum class SubscriptionID : uint64_t
{
    kInvalid = 0    //无效ID
};

/////////////////////////////////////////////////////////////////////////////////////
// EventChannel — 事件频道，用于事件作用域隔离
/////////////////////////////////////////////////////////////////////////////////////

/** 事件频道标识
*   用于隔离不同作用域的事件，例如不同窗口的事件不会互相干扰
*/
using EventChannel = uint64_t;

/** 预定义的事件频道
*/
namespace EventChannels
{
    /** 全局频道：所有订阅者都可以收到
    */
    constexpr EventChannel kGlobal = 0;

    /** 根据指针计算窗口专属频道
    * @param [in] pWindow 窗口指针
    * @return 窗口专属的频道ID
    */
    inline EventChannel FromPointer(const void* pWindow)
    {
        return static_cast<EventChannel>(reinterpret_cast<uintptr_t>(pWindow));
    }
} // namespace EventChannels

/////////////////////////////////////////////////////////////////////////////////////
// EventBus — 事件总线核心类
/////////////////////////////////////////////////////////////////////////////////////

/** 事件总线 — 解耦的发布/订阅系统
*
*   设计定位：
*   - 用于跨组件、跨窗口的解耦通信
*   - 补充（而非替代）现有的 Control::AttachEvent / AttachBubbledEvent 系统
*   - 现有系统适合"知道对方是谁"的场景（直接监听某个控件）
*   - EventBus 适合"不关心来源"的场景（广播式通知）
*
*   使用场景：
*   1. 跨窗口通信：窗口A数据变化，通知窗口B刷新
*   2. 业务层解耦：数据模型变化，通知多个视图更新
*   3. 应用级事件：主题切换、语言切换、用户登录/登出等
*   4. 模块间通信：插件/模块之间的松耦合通信
*
*   线程要求：
*   - Subscribe / Unsubscribe / Publish 必须在 UI 线程调用
*   - 如果需要从后台线程发布事件，请通过 ThreadManager 投递到 UI 线程后再 Publish
*
*   生命周期管理（三种方式，按推荐顺序）：
*   1. ScopedSubscription  — RAII，对象析构时自动取消订阅
*   2. SubscriptionGroup   — 批量管理，一次性取消多个订阅
*   3. WeakFlag 绑定       — 控件/窗口销毁时自动跳过回调
*/
class UILIB_API EventBus
{
public:
    /** 获取全局单例
    */
    static EventBus& Instance();

    /** 事件回调函数原型：接收 EventType 和自定义数据指针
    * @param [in] eventType 事件类型
    * @param [in] wParam 参数1
    * @param [in] lParam 参数2
    */
    using BusCallback = std::function<void(EventType eventType, WPARAM wParam, LPARAM lParam)>;

public:
    ///@name 订阅接口
    ///@{

    /** 订阅指定事件类型
    * @param [in] eventType 事件类型，见 EventType 枚举
    * @param [in] callback 事件处理回调函数
    * @param [in] channel 事件频道，默认为全局频道
    * @return 订阅ID，用于取消订阅
    */
    SubscriptionID Subscribe(EventType eventType,
                             const BusCallback& callback,
                             EventChannel channel = EventChannels::kGlobal);

    /** 订阅指定事件类型（带生命周期绑定）
    *   当 weakFlag 过期时，回调函数自动失效（不会被调用，也不会崩溃）
    *   适合在控件或窗口中使用，控件销毁后自动安全
    * @param [in] eventType 事件类型
    * @param [in] callback 事件处理回调函数
    * @param [in] weakFlag 生命周期标志，通常从 SupportWeakCallback::GetWeakFlag() 获取
    * @param [in] channel 事件频道
    * @return 订阅ID
    */
    SubscriptionID Subscribe(EventType eventType,
                             const BusCallback& callback,
                             const std::weak_ptr<WeakFlag>& weakFlag,
                             EventChannel channel = EventChannels::kGlobal);

    /** 订阅所有事件类型（kEventAll）
    *   收到任何事件都会回调，通过 eventType 参数区分
    * @param [in] callback 事件处理回调函数
    * @param [in] channel 事件频道
    * @return 订阅ID
    */
    SubscriptionID SubscribeAll(const BusCallback& callback,
                                EventChannel channel = EventChannels::kGlobal);

    ///@}

    ///@name 取消订阅接口
    ///@{

    /** 取消订阅
    * @param [in] id 订阅ID
    */
    void Unsubscribe(SubscriptionID id);

    /** 取消指定频道的所有订阅
    *   适合在窗口关闭时清理该窗口频道的所有订阅
    * @param [in] channel 事件频道
    */
    void UnsubscribeByChannel(EventChannel channel);

    ///@}

    ///@name 发布接口
    ///@{

    /** 发布事件
    * @param [in] eventType 事件类型
    * @param [in] wParam 参数1，默认为0
    * @param [in] lParam 参数2，默认为0
    * @param [in] channel 事件频道，默认为全局频道
    */
    void Publish(EventType eventType,
                 WPARAM wParam = 0,
                 LPARAM lParam = 0,
                 EventChannel channel = EventChannels::kGlobal);

    /** 发布事件到全局频道和指定频道
    *   同时将事件分发到全局频道的订阅者和指定频道的订阅者
    * @param [in] eventType 事件类型
    * @param [in] wParam 参数1
    * @param [in] lParam 参数2
    * @param [in] channel 目标频道（全局频道总是会收到）
    */
    void PublishToChannelAndGlobal(EventType eventType,
                                  WPARAM wParam,
                                  LPARAM lParam,
                                  EventChannel channel);

    ///@}

    ///@name 维护接口
    ///@{

    /** 清理已过期的订阅（WeakFlag 已失效的订阅）
    *   可以定期调用以释放内存，非必需（过期订阅在触发时会自动跳过）
    */
    void PurgeExpired();

    /** 清空所有订阅
    */
    void Clear();

    /** 清空指定频道的所有订阅
    * @param [in] channel 事件频道
    */
    void Clear(EventChannel channel);

    /** 获取当前订阅总数（用于调试/监控）
    */
    size_t GetSubscriptionCount() const;

    /** 获取指定频道的订阅数量
    */
    size_t GetSubscriptionCount(EventChannel channel) const;

    ///@}

private:
    EventBus();
    ~EventBus();
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    /** 内部订阅数据
    */
    struct SubscriptionData
    {
        SubscriptionID id;                      //订阅ID
        EventType eventType;                    //事件类型
        EventChannel channel;                   //事件频道
        BusCallback callback;                   //回调函数
        std::weak_ptr<WeakFlag> weakFlag;       //生命周期标志（可选）
        bool hasWeakFlag;                       //是否绑定了生命周期标志
    };

    /** 分发事件到指定频道的订阅者
    */
    void DispatchToChannel(EventType eventType, WPARAM wParam, LPARAM lParam, EventChannel channel);

    /** 生成下一个订阅ID
    */
    SubscriptionID NextID();

private:
    /** 按频道分组的订阅列表
    *   key: EventChannel
    *   value: 按 EventType 分组的订阅列表
    */
    using SubscriptionList = std::vector<SubscriptionData>;
    using EventTypeMap = std::unordered_map<EventType, SubscriptionList>;
    std::unordered_map<EventChannel, EventTypeMap> m_channelMap;

    /** 全局频道的"订阅所有"列表（kEventAll 订阅者）
    *   key: EventChannel
    *   value: 订阅列表
    */
    std::unordered_map<EventChannel, SubscriptionList> m_allEventSubscribers;

    /** 订阅 ID 到频道的反向映射（用于 Unsubscribe 时快速定位）
    */
    struct SubscriptionLocation
    {
        EventChannel channel;
        EventType eventType;
        bool isAllEvent;        //是否是"订阅所有"类型
    };
    std::unordered_map<uint64_t, SubscriptionLocation> m_idLocationMap;

    /** ID 计数器
    */
    uint64_t m_nextId;

    /** 是否正在分发事件（防止在分发过程中修改容器）
    */
    bool m_isDispatching;

    /** 延迟删除的订阅ID列表（分发过程中取消的订阅）
    */
    std::vector<SubscriptionID> m_pendingUnsubscribes;
    std::unordered_set<uint64_t> m_pendingUnsubscribeIds;
};

/////////////////////////////////////////////////////////////////////////////////////
// ScopedSubscription — RAII 订阅管理
/////////////////////////////////////////////////////////////////////////////////////

/** 作用域订阅器 — RAII 方式管理订阅生命周期
*
*   用法示例：
*   @code
*   class MyWindow : public Window {
*       ScopedSubscription m_dataSub;
*
*       void Init() {
*           m_dataSub = ScopedSubscription(
*               EventBus::Instance().Subscribe(kEventValueChanged,
*                   [this](EventType, WPARAM, LPARAM) {
*                       RefreshUI();
*                   })
*           );
*       }
*       // 窗口析构时 m_dataSub 自动取消订阅
*   };
*   @endcode
*/
class UILIB_API ScopedSubscription
{
public:
    /** 默认构造（无效订阅）
    */
    ScopedSubscription();

    /** 从订阅ID构造
    * @param [in] id 从 EventBus::Subscribe 获得的订阅ID
    */
    explicit ScopedSubscription(SubscriptionID id);

    /** 析构时自动取消订阅
    */
    ~ScopedSubscription();

    /** 禁止拷贝
    */
    ScopedSubscription(const ScopedSubscription&) = delete;
    ScopedSubscription& operator=(const ScopedSubscription&) = delete;

    /** 支持移动
    */
    ScopedSubscription(ScopedSubscription&& other) noexcept;
    ScopedSubscription& operator=(ScopedSubscription&& other) noexcept;

    /** 手动取消订阅并重置
    */
    void Reset();

    /** 释放所有权，返回订阅ID但不取消订阅
    */
    SubscriptionID Release();

    /** 获取订阅ID
    */
    SubscriptionID GetID() const;

    /** 是否持有有效订阅
    */
    bool IsValid() const;

    /** bool 转换，方便 if 判断
    */
    explicit operator bool() const;

private:
    SubscriptionID m_id;
};

/////////////////////////////////////////////////////////////////////////////////////
// SubscriptionGroup — 批量订阅管理
/////////////////////////////////////////////////////////////////////////////////////

/** 订阅组 — 批量管理多个订阅的生命周期
*
*   用法示例：
*   @code
*   class MyController {
*       SubscriptionGroup m_subs;
*
*       void Init() {
*           m_subs.Add(EventBus::Instance().Subscribe(kEventClick, ...));
*           m_subs.Add(EventBus::Instance().Subscribe(kEventTextChanged, ...));
*           m_subs.Add(EventBus::Instance().Subscribe(kEventSelect, ...));
*       }
*
*       void Shutdown() {
*           m_subs.UnsubscribeAll();  // 一次性取消所有订阅
*       }
*       // 或者析构时自动取消
*   };
*   @endcode
*/
class UILIB_API SubscriptionGroup
{
public:
    SubscriptionGroup() = default;
    ~SubscriptionGroup();

    /** 禁止拷贝
    */
    SubscriptionGroup(const SubscriptionGroup&) = delete;
    SubscriptionGroup& operator=(const SubscriptionGroup&) = delete;

    /** 支持移动
    */
    SubscriptionGroup(SubscriptionGroup&& other) noexcept;
    SubscriptionGroup& operator=(SubscriptionGroup&& other) noexcept;

    /** 添加一个订阅
    * @param [in] id 订阅ID
    */
    void Add(SubscriptionID id);

    /** 取消所有订阅
    */
    void UnsubscribeAll();

    /** 获取订阅数量
    */
    size_t GetCount() const;

    /** 是否为空
    */
    bool IsEmpty() const;

private:
    std::vector<SubscriptionID> m_ids;
};

} // namespace ui

#endif // UI_CORE_EVENT_BUS_H_
