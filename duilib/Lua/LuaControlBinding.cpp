#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_LUA

#include "LuaControlBinding.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Window.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Utils/StringConvert.h"
#include "sol/sol.hpp"

namespace ui {

namespace LuaControlBinding {

void Register(sol::state& lua) {
    lua.new_usertype<Control>("Control",
        sol::no_constructor,
        "name", sol::property(
            [](Control& control) { return StringConvert::TToUTF8(control.GetName()); },
            [](Control& control, const std::string& name) { control.SetName(StringConvert::UTF8ToT(name)); }
        ),
        "text", sol::property(
            [](Control& control) -> std::string {
                RichEdit* richEdit = dynamic_cast<RichEdit*>(&control);
                if (richEdit != nullptr) {
                    return StringConvert::TToUTF8(richEdit->GetText());
                }
                Label* label = dynamic_cast<Label*>(&control);
                if (label != nullptr) {
                    return StringConvert::TToUTF8(label->GetText());
                }
                return std::string();
            },
            [](Control& control, const std::string& text) {
                RichEdit* richEdit = dynamic_cast<RichEdit*>(&control);
                if (richEdit != nullptr) {
                    richEdit->SetText(StringConvert::UTF8ToT(text));
                    return;
                }
                Label* label = dynamic_cast<Label*>(&control);
                if (label != nullptr) {
                    label->SetText(StringConvert::UTF8ToT(text));
                }
            }
        ),
        "visible", sol::property(&Control::IsVisible, &Control::SetVisible),
        "enabled", sol::property(&Control::IsEnabled, &Control::SetEnabled),
        "get_window", [](Control& control) { return control.GetWindow(); }
    );
}

} // namespace LuaControlBinding

} // namespace ui

#endif // DUILIB_BUILD_FOR_LUA
