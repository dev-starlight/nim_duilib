#ifndef UI_CORE_GLOBALMANAGER_H_
#define UI_CORE_GLOBALMANAGER_H_

#include "duilib/Core/ColorManager.h"

namespace ui
{

class GlobalManager
{
public:
    static GlobalManager& Instance();
    ColorManager& Color();

    // Helper for testing to reset state
    void Reset();

    // Constructor/Destructor needed because ColorManager is a member
    GlobalManager();
    ~GlobalManager();

private:
    ColorManager m_colorManager;
};

}

#endif // UI_CORE_GLOBALMANAGER_H_
