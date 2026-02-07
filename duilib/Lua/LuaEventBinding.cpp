#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_LUA

#include "LuaEventBinding.h"
#include "duilib/Core/EventArgs.h"
#include "duilib/Core/Control.h"
#include "sol/sol.hpp"
#include <unordered_map>

namespace ui {

namespace LuaEventBinding {

static std::unordered_map<std::string, EventType> s_eventNameMap = {
    {"click", kEventClick},
    {"mouse_enter", kEventMouseEnter},
    {"mouse_leave", kEventMouseLeave},
    {"mouse_move", kEventMouseMove},
    {"mouse_down", kEventMouseButtonDown},
    {"mouse_up", kEventMouseButtonUp},
    {"double_click", kEventMouseDoubleClick},
    {"key_down", kEventKeyDown},
    {"key_up", kEventKeyUp},
    {"char", kEventChar},
    {"set_focus", kEventSetFocus},
    {"kill_focus", kEventKillFocus},
    {"value_change", kEventValueChanged},
    {"select", kEventSelect},
    {"unselect", kEventUnSelect},
    {"text_change", kEventTextChanged},
    {"resize", kEventSizeChanged},
    {"visible_change", kEventVisibleChanged},
    {"scroll_change", kEventScrollPosChanged},
    {"window_init", kEventWindowCreate},
    {"window_close", kEventWindowClose}
};

EventType ParseEventType(const std::string& eventName) {
    auto it = s_eventNameMap.find(eventName);
    if (it != s_eventNameMap.end()) {
        return it->second;
    }
    return kEventNone;
}

void Register(sol::state& lua) {
    lua.new_usertype<EventArgs>("EventArgs",
        sol::no_constructor,
        "event_type", sol::property([](const EventArgs& args) { return static_cast<int>(args.eventType); }),
        "wParam", sol::property([](const EventArgs& args) { return static_cast<int64_t>(args.wParam); }),
        "lParam", sol::property([](const EventArgs& args) { return static_cast<int64_t>(args.lParam); }),
        "vk_code", sol::property([](const EventArgs& args) { return static_cast<int>(args.vkCode); }),
        "sender", sol::property([](const EventArgs& args) { return args.GetSender(); })
    );

    sol::table ui_table = lua["ui"].get_or_create<sol::table>();

    for (const auto& pair : s_eventNameMap) {
        ui_table[pair.first] = pair.second;
    }
}

}

}

#endif
