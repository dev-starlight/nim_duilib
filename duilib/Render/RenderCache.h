#ifndef UI_RENDER_RENDER_CACHE_H_
#define UI_RENDER_RENDER_CACHE_H_

#include <map>
#include <set>
#include <memory>
#include <mutex>

namespace ui 
{

class Control;
class IRender;
class UiRect;

struct CacheEntry
{
    std::shared_ptr<void> bitmap;
    uint64_t timestamp;
    bool dirty;
};

class RenderCache
{
public:
    static RenderCache& Instance();

    void Invalidate(Control* control);
    
    void InvalidateAll();
    
    bool IsDirty(Control* control) const;
    
    void BeginPaint(IRender* render, const UiRect& clipRect);
    
    void EndPaint();
    
    bool CanUseCache(Control* control) const;
    
    void UpdateCache(Control* control, std::shared_ptr<void> bitmap);
    
    std::shared_ptr<void> GetCache(Control* control) const;
    
    void Clear();
    
    void SetCacheEnabled(bool enabled);
    
    bool IsCacheEnabled() const;

private:
    RenderCache();
    ~RenderCache();
    RenderCache(const RenderCache&) = delete;
    RenderCache& operator=(const RenderCache&) = delete;

    std::map<Control*, CacheEntry> m_cache;
    std::set<Control*> m_dirtyControls;
    bool m_enabled;
    mutable std::mutex m_mutex;
    IRender* m_currentRender;
};

}

#endif
