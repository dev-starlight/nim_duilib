/***************************************************************************
 *   Copyright (C) 2026 by rhett                                           *
 *   rhett@qq.com                                                          *
 *                                                                         *
 *   智能指针控件管理器 - 替代原始指针管理                                   *
 *   提供自动内存管理和生命周期控制                                          *
 ***************************************************************************/

#ifndef UI_CORE_CONTROL_MANAGER_H_
#define UI_CORE_CONTROL_MANAGER_H_

#include "duilib/Core/Callback.h"
#include "duilib/Core/UiTypes.h"
#include <memory>
#include <vector>
#include <algorithm>

namespace ui 
{

class Control;
class Box;
class Window;

/** 
 * 智能指针控件包装类
 * 提供类似 std::shared_ptr 的行为，但针对 UI 控件优化
 * 自动处理控件生命周期和父容器关系
 */
template<typename T>
class ControlRef
{
public:
    ControlRef() : m_ptr(nullptr) {}
    
    explicit ControlRef(T* ptr) : m_ptr(ptr) {
        if (m_ptr) {
            m_weakFlag = m_ptr->GetWeakFlag();
        }
    }
    
    ControlRef(std::nullptr_t) : m_ptr(nullptr) {}
    
    // 从 shared_ptr 构造
    explicit ControlRef(const std::shared_ptr<T>& shared) 
        : m_ptr(shared.get()) {
        if (m_ptr) {
            m_weakFlag = m_ptr->GetWeakFlag();
        }
    }
    
    // 拷贝构造
    ControlRef(const ControlRef& other) 
        : m_ptr(other.m_ptr), m_weakFlag(other.m_weakFlag) {}
    
    // 移动构造
    ControlRef(ControlRef&& other) noexcept
        : m_ptr(other.m_ptr), m_weakFlag(std::move(other.m_weakFlag)) {
        other.m_ptr = nullptr;
    }
    
    // 析构
    ~ControlRef() = default;
    
    // 赋值操作
    ControlRef& operator=(const ControlRef& other) {
        if (this != &other) {
            m_ptr = other.m_ptr;
            m_weakFlag = other.m_weakFlag;
        }
        return *this;
    }
    
    ControlRef& operator=(ControlRef&& other) noexcept {
        if (this != &other) {
            m_ptr = other.m_ptr;
            m_weakFlag = std::move(other.m_weakFlag);
            other.m_ptr = nullptr;
        }
        return *this;
    }
    
    ControlRef& operator=(T* ptr) {
        m_ptr = ptr;
        if (m_ptr) {
            m_weakFlag = m_ptr->GetWeakFlag();
        } else {
            m_weakFlag.reset();
        }
        return *this;
    }
    
    ControlRef& operator=(std::nullptr_t) {
        m_ptr = nullptr;
        m_weakFlag.reset();
        return *this;
    }
    
    // 访问操作
    T* get() const {
        if (m_weakFlag.expired()) {
            return nullptr;
        }
        return m_ptr;
    }
    
    T* operator->() const {
        return get();
    }
    
    T& operator*() const {
        return *get();
    }
    
    // 检查有效性
    bool expired() const {
        return m_weakFlag.expired();
    }
    
    explicit operator bool() const {
        return get() != nullptr;
    }
    
    // 重置
    void reset() {
        m_ptr = nullptr;
        m_weakFlag.reset();
    }
    
    // 比较操作
    bool operator==(const ControlRef& other) const { return get() == other.get(); }
    bool operator!=(const ControlRef& other) const { return get() != other.get(); }
    bool operator==(std::nullptr_t) const { return get() == nullptr; }
    bool operator!=(std::nullptr_t) const { return get() != nullptr; }
    bool operator==(T* ptr) const { return get() == ptr; }
    bool operator!=(T* ptr) const { return get() != ptr; }
    
private:
    T* m_ptr;
    std::weak_ptr<WeakFlag> m_weakFlag;
};

/** 控件智能指针类型定义 */
using ControlRefPtr = ControlRef<Control>;
using BoxRefPtr = ControlRef<Box>;
using WindowRefPtr = ControlRef<Window>;

/**
 * 智能容器 - 使用智能指针管理子控件
 * 替代 Box 中的原始指针管理
 */
class SmartContainer
{
public:
    SmartContainer();
    ~SmartContainer();
    
    // 禁止拷贝
    SmartContainer(const SmartContainer&) = delete;
    SmartContainer& operator=(const SmartContainer&) = delete;
    
    // 允许移动
    SmartContainer(SmartContainer&&) noexcept;
    SmartContainer& operator=(SmartContainer&&) noexcept;
    
    /** 添加控件 */
    void AddItem(std::shared_ptr<Control> control);
    
    /** 在指定位置插入控件 */
    void AddItemAt(std::shared_ptr<Control> control, size_t index);
    
    /** 移除控件 */
    bool RemoveItem(Control* control);
    bool RemoveItemAt(size_t index);
    
    /** 移除所有控件 */
    void RemoveAllItems();
    
    /** 获取子控件数量 */
    size_t GetItemCount() const { return m_items.size(); }
    
    /** 获取指定索引的控件 */
    Control* GetItemAt(size_t index) const;
    
    /** 获取控件的索引 */
    size_t GetItemIndex(Control* control) const;
    
    /** 设置控件索引 */
    bool SetItemIndex(Control* control, size_t index);
    
    /** 查找控件 */
    Control* FindControl(const DString& name) const;
    
    /** 遍历所有控件 */
    template<typename Func>
    void ForEach(Func&& func) const {
        for (const auto& item : m_items) {
            if (item) {
                func(item.get());
            }
        }
    }
    
    /** 获取原始指针列表（用于兼容性） */
    std::vector<Control*> GetRawItems() const;
    
private:
    std::vector<std::shared_ptr<Control>> m_items;
};

/**
 * 控件生命周期管理器
 * 提供控件创建、销毁和跟踪功能
 */
class ControlLifecycleManager
{
public:
    static ControlLifecycleManager& Instance();
    
    /** 注册控件创建 */
    void RegisterControl(Control* control);
    
    /** 注销控件 */
    void UnregisterControl(Control* control);
    
    /** 获取活跃控件数量 */
    size_t GetActiveControlCount() const;
    
    /** 检查控件是否有效 */
    bool IsControlValid(Control* control) const;
    
    /** 获取所有活跃控件 */
    std::vector<Control*> GetActiveControls() const;
    
private:
    ControlLifecycleManager() = default;
    ~ControlLifecycleManager() = default;
    ControlLifecycleManager(const ControlLifecycleManager&) = delete;
    ControlLifecycleManager& operator=(const ControlLifecycleManager&) = delete;
    
    std::set<Control*> m_activeControls;
    mutable std::mutex m_mutex;
};

/**
 * 智能控件创建辅助函数
 */
template<typename T, typename... Args>
std::shared_ptr<T> MakeControl(Args&&... args) {
    auto control = std::make_shared<T>(std::forward<Args>(args)...);
    ControlLifecycleManager::Instance().RegisterControl(control.get());
    return control;
}

} // namespace ui

#endif // UI_CORE_CONTROL_MANAGER_H_
