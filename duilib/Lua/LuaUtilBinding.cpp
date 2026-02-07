#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_LUA

#include "LuaUtilBinding.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Utils/LogUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"
#include "sol/sol.hpp"
#include <functional>
#include <chrono>

namespace ui {

namespace LuaUtilBinding {

void Register(sol::state& lua) {
    sol::table ui_table = lua["ui"].get_or_create<sol::table>();

    ui_table["log"] = [](const std::string& msg) {
        LogUtil::OutputLine(StringUtil::Printf(_T("[lua] %s"), StringConvert::UTF8ToT(msg).c_str()));
    };

    ui_table["set_timer"] = [](int delay_ms, sol::function callback) {
        return GlobalManager::Instance().Timer().AddTimer(std::weak_ptr<WeakFlag>(), [callback]() mutable {
            if (callback.valid()) {
                callback();
            }
        }, static_cast<uint32_t>(delay_ms), 1);
    };

    ui_table["clear_timer"] = [](int timer_id) {
        GlobalManager::Instance().Timer().RemoveTimer(static_cast<size_t>(timer_id));
    };

    ui_table["format"] = [](const std::string& fmt, sol::variadic_args args) -> std::string {
        char buffer[1024];
        std::vector<const char*> arg_list;
        arg_list.push_back(fmt.c_str());
        for (auto arg : args) {
            if (arg.is<std::string>()) {
                arg_list.push_back(arg.as<std::string>().c_str());
            }
            else if (arg.is<int>()) {
                static thread_local char int_buf[32];
                snprintf(int_buf, sizeof(int_buf), "%d", arg.as<int>());
                arg_list.push_back(int_buf);
            }
            else if (arg.is<double>()) {
                static thread_local char double_buf[32];
                snprintf(double_buf, sizeof(double_buf), "%f", arg.as<double>());
                arg_list.push_back(double_buf);
            }
            else {
                arg_list.push_back("?");
            }
        }
        snprintf(buffer, sizeof(buffer), fmt.c_str(), arg_list[1], arg_list[2], arg_list[3], arg_list[4], arg_list[5]);
        return std::string(buffer);
    };

    ui_table.set_function("find", sol::overload(
        [](const std::string& name) -> Control* {
            DString ctrlName = StringConvert::UTF8ToT(name);
            auto windowList = GlobalManager::Instance().Windows().GetAllWindowList();
            for (auto it = windowList.rbegin(); it != windowList.rend(); ++it) {
                if (*it != nullptr) {
                    Control* control = (*it)->FindControl(ctrlName);
                    if (control != nullptr) {
                        return control;
                    }
                }
            }
            return nullptr;
        },
        [](Window* window, const std::string& name) -> Control* {
            if (window == nullptr) {
                return nullptr;
            }
            return window->FindControl(StringConvert::UTF8ToT(name));
        }
    ));
}

}

}

#endif
