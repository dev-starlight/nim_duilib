#ifndef UI_CORE_CONTROL_H_
#define UI_CORE_CONTROL_H_

#include "duilib/duilib_defs.h"

namespace ui
{
class Window;
class UILIB_API Control
{
public:
    explicit Control(Window* pWindow) {}
    virtual ~Control() {}
    virtual DString GetType() const { return _T("Control"); }
};
}
#endif
