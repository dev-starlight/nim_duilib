#ifndef UI_CORE_CONTROL_FACTORY_H_
#define UI_CORE_CORE_CONTROL_FACTORY_H_

#include <map>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ui 
{

class Control;
class Window;

using ControlCreator = std::function<Control* (Window*)>;

class ControlFactory
{
public:
    static ControlFactory& Instance();

    template<typename T>
    void Register(const std::string& name)
    {
        m_creators[name] = [](Window* w) { return new T(w); };
    }

    Control* Create(const std::string& name, Window* window);
    
    bool IsRegistered(const std::string& name) const;
    
    std::vector<std::string> GetRegisteredNames() const;

private:
    ControlFactory() = default;
    ~ControlFactory() = default;
    ControlFactory(const ControlFactory&) = delete;
    ControlFactory& operator=(const ControlFactory&) = delete;

    std::map<std::string, ControlCreator> m_creators;
};

#define REGISTER_CONTROL(ClassName, ControlType) \
    struct ClassName##Registrar { \
        ClassName##Registrar() { \
            ui::ControlFactory::Instance().Register<ClassName>(#ControlType); \
        } \
    } static g_##ClassName##Registrar;

}

#endif
