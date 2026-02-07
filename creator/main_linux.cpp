#if defined (__linux__) && !defined (__ANDROID__)

#include "CreatorApplication.h"

int main(int /*argc*/, char** /*argv*/)
{
    CreatorApplication app;
    app.Run();
    return 0;
}

#endif
