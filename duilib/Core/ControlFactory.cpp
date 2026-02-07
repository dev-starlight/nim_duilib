#include "ControlFactory.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Window.h"

namespace ui 
{

ControlFactory& ControlFactory::Instance()
{
    static ControlFactory instance;
    return instance;
}

Control* ControlFactory::Create(const std::string& name, Window* window)
{
    auto it = m_creators.find(name);
    if (it != m_creators.end()) {
        return it->second(window);
    }
    return nullptr;
}

bool ControlFactory::IsRegistered(const std::string& name) const
{
    return m_creators.find(name) != m_creators.end();
}

std::vector<std::string> ControlFactory::GetRegisteredNames() const
{
    std::vector<std::string> names;
    for (const auto& pair : m_creators) {
        names.push_back(pair.first);
    }
    return names;
}

}
