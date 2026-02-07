#ifndef UI_LUA_CONTROL_BINDING_H_
#define UI_LUA_CONTROL_BINDING_H_

#ifdef DUILIB_BUILD_FOR_LUA

namespace sol { class state; }

namespace ui
{

namespace LuaControlBinding
{
    void Register(sol::state& lua);
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_LUA

#endif // UI_LUA_CONTROL_BINDING_H_
