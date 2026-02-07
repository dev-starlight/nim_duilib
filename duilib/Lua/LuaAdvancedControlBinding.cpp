#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_LUA

#include "LuaAdvancedControlBinding.h"
#include "duilib/Control/Button.h"
#include "duilib/Control/RichEdit.h"
#include "sol/sol.hpp"

namespace ui {

namespace LuaAdvancedControlBinding {

void Register(sol::state& lua) {
    lua.new_usertype<Button>("Button",
        sol::no_constructor,
        "type", []() { return "Button"; }
    );

    lua.new_usertype<RichEdit>("RichEdit",
        sol::no_constructor,
        "type", []() { return "RichEdit"; }
    );
}

}

}

#endif
