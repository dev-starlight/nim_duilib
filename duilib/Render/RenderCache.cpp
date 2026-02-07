#include "RenderCache.h"
#include "duilib/Core/Control.h"
#include "duilib/Render/IRender.h"
#include "duilib/Core/UiRect.h"

namespace ui 
{

RenderCache& RenderCache::Instance()
{
    static RenderCache instance;
    return instance;
}

RenderCache::RenderCache() : m_enabled(true), m_currentRender(nullptr) {}

RenderCache::~RenderCache() = default;

void RenderCache::Invalidate(Control* control)
{
    if (!m_enabled || !control) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_dirtyControls.insert(control);
    
    auto it = m_cache.find(control);
    if (it != m_cache.end()) {
        it->second.dirty = true;
    }
}

void RenderCache::InvalidateAll()
{
    if (!m_enabled) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& pair : m_cache) {
        pair.second.dirty = true;
        m_dirtyControls.insert(pair.first);
    }
}

bool RenderCache::IsDirty(Control* control) const
{
    if (!m_enabled || !control) return true;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_cache.find(control);
    if (it != m_cache.end()) {
        return it->second.dirty;
    }
    return true;
}

void RenderCache::BeginPaint(IRender* render, const UiRect& clipRect)
{
    m_currentRender = render;
}

void RenderCache::EndPaint()
{
    m_currentRender = nullptr;
}

bool RenderCache::CanUseCache(Control* control) const
{
    if (!m_enabled || !control) return false;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_cache.find(control);
    return it != m_cache.end() && !it->second.dirty && it->second.bitmap;
}

void RenderCache::UpdateCache(Control* control, std::shared_ptr<void> bitmap)
{
    if (!m_enabled || !control) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cache[control] = {bitmap, 0, false};
    m_dirtyControls.erase(control);
}

std::shared_ptr<void> RenderCache::GetCache(Control* control) const
{
    if (!m_enabled || !control) return nullptr;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_cache.find(control);
    if (it != m_cache.end() && !it->second.dirty) {
        return it->second.bitmap;
    }
    return nullptr;
}

void RenderCache::Clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cache.clear();
    m_dirtyControls.clear();
}

void RenderCache::SetCacheEnabled(bool enabled)
{
    m_enabled = enabled;
    if (!enabled) {
        Clear();
    }
}

bool RenderCache::IsCacheEnabled() const
{
    return m_enabled;
}

}
