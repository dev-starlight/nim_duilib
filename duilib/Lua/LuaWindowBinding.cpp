#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_LUA

#include "LuaWindowBinding.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Utils/StringConvert.h"
#include "sol/sol.hpp"

namespace ui {

namespace LuaWindowBinding {

void Register(sol::state& lua) {
    lua.new_usertype<Window>("Window",
        sol::no_constructor,
        "type", []() { return "Window"; },
        "find", [](Window& window, const std::string& name) {
            return window.FindControl(StringConvert::UTF8ToT(name));
        },
        "close", &Window::Close
    );
}

}

}

#endif
