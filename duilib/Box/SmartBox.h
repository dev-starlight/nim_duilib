#ifndef UI_CORE_SMART_BOX_H_
#define UI_CORE_SMART_BOX_H_

#include "duilib/Core/Box.h"
#include <memory>
#include <vector>

namespace ui 
{

class SmartBox : public Box
{
public:
    explicit SmartBox(Window* pWindow);
    virtual ~SmartBox() override;

    void AddItem(std::shared_ptr<Control> control);
    std::shared_ptr<Control> RemoveItem(size_t index);
    void RemoveAllItems();
    
    std::shared_ptr<Control> GetItem(size_t index) const;
    size_t GetItemCount() const { return m_items.size(); }
    
    template<typename T>
    std::shared_ptr<T> FindControl(const DString& name) {
        for (const auto& item : m_items) {
            if (item->GetName() == name) {
                return std::dynamic_pointer_cast<T>(item);
            }
        }
        return nullptr;
    }

private:
    std::vector<std::shared_ptr<Control>> m_items;
};

}

#endif
