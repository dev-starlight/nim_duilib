#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_LUA

#include "LuaBoxBinding.h"
#include "duilib/Core/Box.h"
#include "sol/sol.hpp"

namespace ui {

namespace LuaBoxBinding {

void Register(sol::state& lua) {
    lua.new_usertype<Box>("Box",
        sol::no_constructor,
        "add", &Box::AddItem,
        "remove", &Box::RemoveItem,
        "remove_all", &Box::RemoveAllItems,
        "get_count", &Box::GetItemCount,
        "get_child", &Box::GetItemAt
    );
}

}

}

#endif
