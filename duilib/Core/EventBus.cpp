#include "EventBus.h"
#include <algorithm>
#include <cassert>

namespace ui
{

/////////////////////////////////////////////////////////////////////////////////////
// EventBus 瀹炵幇
/////////////////////////////////////////////////////////////////////////////////////

EventBus::EventBus()
    : m_nextId(0)
    , m_isDispatching(false)
{
}

EventBus::~EventBus()
{
}

EventBus& EventBus::Instance()
{
    static EventBus instance;
    return instance;
}

SubscriptionID EventBus::NextID()
{
    ++m_nextId;
    //璺宠繃鏃犳晥ID
    if (m_nextId == static_cast<uint64_t>(SubscriptionID::kInvalid)) {
        ++m_nextId;
    }
    return static_cast<SubscriptionID>(m_nextId);
}

SubscriptionID EventBus::Subscribe(EventType eventType,
                                   const BusCallback& callback,
                                   EventChannel channel)
{
    assert(!m_isDispatching && "EventBus::Subscribe 涓嶈兘鍦ㄤ簨浠跺垎鍙戣繃绋嬩腑璋冪敤");
    if (!callback) {
        return SubscriptionID::kInvalid;
    }

    SubscriptionID id = NextID();
    SubscriptionData data;
    data.id = id;
    data.eventType = eventType;
    data.channel = channel;
    data.callback = callback;
    data.hasWeakFlag = false;

    m_channelMap[channel][eventType].push_back(std::move(data));

    SubscriptionLocation loc;
    loc.channel = channel;
    loc.eventType = eventType;
    loc.isAllEvent = false;
    m_idLocationMap[static_cast<uint64_t>(id)] = loc;

    return id;
}

SubscriptionID EventBus::Subscribe(EventType eventType,
                                   const BusCallback& callback,
                                   const std::weak_ptr<WeakFlag>& weakFlag,
                                   EventChannel channel)
{
    assert(!m_isDispatching && "EventBus::Subscribe 涓嶈兘鍦ㄤ簨浠跺垎鍙戣繃绋嬩腑璋冪敤");
    if (!callback) {
        return SubscriptionID::kInvalid;
    }

    SubscriptionID id = NextID();
    SubscriptionData data;
    data.id = id;
    data.eventType = eventType;
    data.channel = channel;
    data.callback = callback;
    data.weakFlag = weakFlag;
    data.hasWeakFlag = true;

    m_channelMap[channel][eventType].push_back(std::move(data));

    SubscriptionLocation loc;
    loc.channel = channel;
    loc.eventType = eventType;
    loc.isAllEvent = false;
    m_idLocationMap[static_cast<uint64_t>(id)] = loc;

    return id;
}

SubscriptionID EventBus::SubscribeAll(const BusCallback& callback,
                                      EventChannel channel)
{
    assert(!m_isDispatching && "EventBus::SubscribeAll 涓嶈兘鍦ㄤ簨浠跺垎鍙戣繃绋嬩腑璋冪敤");
    if (!callback) {
        return SubscriptionID::kInvalid;
    }

    SubscriptionID id = NextID();
    SubscriptionData data;
    data.id = id;
    data.eventType = kEventAll;
    data.channel = channel;
    data.callback = callback;
    data.hasWeakFlag = false;

    m_allEventSubscribers[channel].push_back(std::move(data));

    SubscriptionLocation loc;
    loc.channel = channel;
    loc.eventType = kEventAll;
    loc.isAllEvent = true;
    m_idLocationMap[static_cast<uint64_t>(id)] = loc;

    return id;
}

void EventBus::Unsubscribe(SubscriptionID id)
{
    if (id == SubscriptionID::kInvalid) {
        return;
    }

    //濡傛灉姝ｅ湪鍒嗗彂浜嬩欢锛屽欢杩熷垹闄?
    if (m_isDispatching) {
        uint64_t idValue = static_cast<uint64_t>(id);
        if (m_pendingUnsubscribeIds.insert(idValue).second) {
            m_pendingUnsubscribes.push_back(id);
        }
        return;
    }

    uint64_t idValue = static_cast<uint64_t>(id);
    auto locIt = m_idLocationMap.find(idValue);
    if (locIt == m_idLocationMap.end()) {
        return;
    }

    const SubscriptionLocation& loc = locIt->second;

    if (loc.isAllEvent) {
        //浠?璁㈤槄鎵€鏈?鍒楄〃涓Щ闄?
        auto channelIt = m_allEventSubscribers.find(loc.channel);
        if (channelIt != m_allEventSubscribers.end()) {
            auto& list = channelIt->second;
            list.erase(
                std::remove_if(list.begin(), list.end(),
                    [id](const SubscriptionData& data) { return data.id == id; }),
                list.end()
            );
            if (list.empty()) {
                m_allEventSubscribers.erase(channelIt);
            }
        }
    }
    else {
        //浠庢寜浜嬩欢绫诲瀷鍒嗙粍鐨勫垪琛ㄤ腑绉婚櫎
        auto channelIt = m_channelMap.find(loc.channel);
        if (channelIt != m_channelMap.end()) {
            auto& eventMap = channelIt->second;
            auto eventIt = eventMap.find(loc.eventType);
            if (eventIt != eventMap.end()) {
                auto& list = eventIt->second;
                list.erase(
                    std::remove_if(list.begin(), list.end(),
                        [id](const SubscriptionData& data) { return data.id == id; }),
                    list.end()
                );
                if (list.empty()) {
                    eventMap.erase(eventIt);
                }
            }
            if (eventMap.empty()) {
                m_channelMap.erase(channelIt);
            }
        }
    }

    m_idLocationMap.erase(locIt);
}

void EventBus::UnsubscribeByChannel(EventChannel channel)
{
    assert(!m_isDispatching && "EventBus::UnsubscribeByChannel 涓嶈兘鍦ㄤ簨浠跺垎鍙戣繃绋嬩腑璋冪敤");

    //娓呯悊鎸囧畾棰戦亾鐨勬墍鏈夋寜浜嬩欢绫诲瀷鍒嗙粍鐨勮闃?
    auto channelIt = m_channelMap.find(channel);
    if (channelIt != m_channelMap.end()) {
        for (auto& [eventType, list] : channelIt->second) {
            for (const auto& data : list) {
                m_idLocationMap.erase(static_cast<uint64_t>(data.id));
            }
        }
        m_channelMap.erase(channelIt);
    }

    //娓呯悊鎸囧畾棰戦亾鐨?璁㈤槄鎵€鏈?鍒楄〃
    auto allIt = m_allEventSubscribers.find(channel);
    if (allIt != m_allEventSubscribers.end()) {
        for (const auto& data : allIt->second) {
            m_idLocationMap.erase(static_cast<uint64_t>(data.id));
        }
        m_allEventSubscribers.erase(allIt);
    }
}

void EventBus::Publish(EventType eventType,
                       WPARAM wParam,
                       LPARAM lParam,
                       EventChannel channel)
{
    DispatchToChannel(eventType, wParam, lParam, channel);
}

void EventBus::PublishToChannelAndGlobal(EventType eventType,
                                         WPARAM wParam,
                                         LPARAM lParam,
                                         EventChannel channel)
{
    //鍏堝垎鍙戝埌鎸囧畾棰戦亾
    if (channel != EventChannels::kGlobal) {
        DispatchToChannel(eventType, wParam, lParam, channel);
    }
    //鍐嶅垎鍙戝埌鍏ㄥ眬棰戦亾
    DispatchToChannel(eventType, wParam, lParam, EventChannels::kGlobal);
}

void EventBus::DispatchToChannel(EventType eventType, WPARAM wParam, LPARAM lParam, EventChannel channel)
{
    auto channelIt = m_channelMap.find(channel);
    const SubscriptionList* typedSubscribers = nullptr;
    if (channelIt != m_channelMap.end()) {
        auto eventIt = channelIt->second.find(eventType);
        if (eventIt != channelIt->second.end() && !eventIt->second.empty()) {
            typedSubscribers = &eventIt->second;
        }
    }

    auto allIt = m_allEventSubscribers.find(channel);
    const SubscriptionList* allSubscribers = nullptr;
    if (allIt != m_allEventSubscribers.end() && !allIt->second.empty()) {
        allSubscribers = &allIt->second;
    }

    if ((typedSubscribers == nullptr) && (allSubscribers == nullptr)) {
        return;
    }

    bool wasDispatching = m_isDispatching;
    m_isDispatching = true;

    if (typedSubscribers != nullptr) {
        for (const auto& data : *typedSubscribers) {
            if (data.hasWeakFlag && data.weakFlag.expired()) {
                continue;
            }
            data.callback(eventType, wParam, lParam);
        }
    }

    if (allSubscribers != nullptr) {
        for (const auto& data : *allSubscribers) {
            if (data.hasWeakFlag && data.weakFlag.expired()) {
                continue;
            }
            data.callback(eventType, wParam, lParam);
        }
    }

    m_isDispatching = wasDispatching;

    if (!wasDispatching && !m_pendingUnsubscribes.empty()) {
        std::vector<SubscriptionID> pending;
        pending.swap(m_pendingUnsubscribes);
        m_pendingUnsubscribeIds.clear();
        for (SubscriptionID pendingId : pending) {
            Unsubscribe(pendingId);
        }
    }
}

void EventBus::PurgeExpired()
{
    assert(!m_isDispatching && "EventBus::PurgeExpired 涓嶈兘鍦ㄤ簨浠跺垎鍙戣繃绋嬩腑璋冪敤");

    //娓呯悊鎸変簨浠剁被鍨嬪垎缁勭殑璁㈤槄
    for (auto channelIt = m_channelMap.begin(); channelIt != m_channelMap.end(); ) {
        auto& eventMap = channelIt->second;
        for (auto eventIt = eventMap.begin(); eventIt != eventMap.end(); ) {
            auto& list = eventIt->second;
            list.erase(
                std::remove_if(list.begin(), list.end(),
                    [this](const SubscriptionData& data) {
                        if (data.hasWeakFlag && data.weakFlag.expired()) {
                            m_idLocationMap.erase(static_cast<uint64_t>(data.id));
                            return true;
                        }
                        return false;
                    }),
                list.end()
            );
            if (list.empty()) {
                eventIt = eventMap.erase(eventIt);
            }
            else {
                ++eventIt;
            }
        }
        if (eventMap.empty()) {
            channelIt = m_channelMap.erase(channelIt);
        }
        else {
            ++channelIt;
        }
    }

    //娓呯悊"璁㈤槄鎵€鏈?鍒楄〃
    for (auto allIt = m_allEventSubscribers.begin(); allIt != m_allEventSubscribers.end(); ) {
        auto& list = allIt->second;
        list.erase(
            std::remove_if(list.begin(), list.end(),
                [this](const SubscriptionData& data) {
                    if (data.hasWeakFlag && data.weakFlag.expired()) {
                        m_idLocationMap.erase(static_cast<uint64_t>(data.id));
                        return true;
                    }
                    return false;
                }),
            list.end()
        );
        if (list.empty()) {
            allIt = m_allEventSubscribers.erase(allIt);
        }
        else {
            ++allIt;
        }
    }
}

void EventBus::Clear()
{
    assert(!m_isDispatching && "EventBus::Clear 涓嶈兘鍦ㄤ簨浠跺垎鍙戣繃绋嬩腑璋冪敤");
    m_channelMap.clear();
    m_allEventSubscribers.clear();
    m_idLocationMap.clear();
    m_pendingUnsubscribes.clear();
    m_pendingUnsubscribeIds.clear();
}

void EventBus::Clear(EventChannel channel)
{
    assert(!m_isDispatching && "EventBus::Clear(channel) 涓嶈兘鍦ㄤ簨浠跺垎鍙戣繃绋嬩腑璋冪敤");
    UnsubscribeByChannel(channel);
}

size_t EventBus::GetSubscriptionCount() const
{
    return m_idLocationMap.size();
}

size_t EventBus::GetSubscriptionCount(EventChannel channel) const
{
    size_t count = 0;

    auto channelIt = m_channelMap.find(channel);
    if (channelIt != m_channelMap.end()) {
        for (const auto& [eventType, list] : channelIt->second) {
            count += list.size();
        }
    }

    auto allIt = m_allEventSubscribers.find(channel);
    if (allIt != m_allEventSubscribers.end()) {
        count += allIt->second.size();
    }

    return count;
}

/////////////////////////////////////////////////////////////////////////////////////
// ScopedSubscription 瀹炵幇
/////////////////////////////////////////////////////////////////////////////////////

ScopedSubscription::ScopedSubscription()
    : m_id(SubscriptionID::kInvalid)
{
}

ScopedSubscription::ScopedSubscription(SubscriptionID id)
    : m_id(id)
{
}

ScopedSubscription::~ScopedSubscription()
{
    Reset();
}

ScopedSubscription::ScopedSubscription(ScopedSubscription&& other) noexcept
    : m_id(other.m_id)
{
    other.m_id = SubscriptionID::kInvalid;
}

ScopedSubscription& ScopedSubscription::operator=(ScopedSubscription&& other) noexcept
{
    if (this != &other) {
        Reset();
        m_id = other.m_id;
        other.m_id = SubscriptionID::kInvalid;
    }
    return *this;
}

void ScopedSubscription::Reset()
{
    if (m_id != SubscriptionID::kInvalid) {
        EventBus::Instance().Unsubscribe(m_id);
        m_id = SubscriptionID::kInvalid;
    }
}

SubscriptionID ScopedSubscription::Release()
{
    SubscriptionID id = m_id;
    m_id = SubscriptionID::kInvalid;
    return id;
}

SubscriptionID ScopedSubscription::GetID() const
{
    return m_id;
}

bool ScopedSubscription::IsValid() const
{
    return m_id != SubscriptionID::kInvalid;
}

ScopedSubscription::operator bool() const
{
    return IsValid();
}

/////////////////////////////////////////////////////////////////////////////////////
// SubscriptionGroup 瀹炵幇
/////////////////////////////////////////////////////////////////////////////////////

SubscriptionGroup::~SubscriptionGroup()
{
    UnsubscribeAll();
}

SubscriptionGroup::SubscriptionGroup(SubscriptionGroup&& other) noexcept
    : m_ids(std::move(other.m_ids))
{
}

SubscriptionGroup& SubscriptionGroup::operator=(SubscriptionGroup&& other) noexcept
{
    if (this != &other) {
        UnsubscribeAll();
        m_ids = std::move(other.m_ids);
    }
    return *this;
}

void SubscriptionGroup::Add(SubscriptionID id)
{
    if (id != SubscriptionID::kInvalid) {
        m_ids.push_back(id);
    }
}

void SubscriptionGroup::UnsubscribeAll()
{
    for (SubscriptionID id : m_ids) {
        EventBus::Instance().Unsubscribe(id);
    }
    m_ids.clear();
}

size_t SubscriptionGroup::GetCount() const
{
    return m_ids.size();
}

bool SubscriptionGroup::IsEmpty() const
{
    return m_ids.empty();
}

} // namespace ui
