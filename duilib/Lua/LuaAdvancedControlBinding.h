#ifndef UI_LUA_ADVANCED_CONTROL_BINDING_H_
#define UI_LUA_ADVANCED_CONTROL_BINDING_H_

#ifdef DUILIB_BUILD_FOR_LUA

namespace sol { class state; }

namespace ui
{

namespace LuaAdvancedControlBinding
{
    void Register(sol::state& lua);
}

}

#endif

#endif
