#ifndef UI_UTILS_LOG_UTIL_H_
#define UI_UTILS_LOG_UTIL_H_

#include "duilib/duilib_defs.h"

namespace ui
{

class UILIB_API LogUtil
{
public:
    // Enable runtime logger backend (glog when compiled with DUILIB_WITH_GLOG).
    static bool InitRuntimeLogger(const DString& processName = _T("duilib"),
                                  const DString& logDir = DString(),
                                  bool alsoLogToStderr = true);

    // Disable runtime logger backend.
    static void ShutdownRuntimeLogger();

    // Query whether runtime logger backend is enabled.
    static bool IsRuntimeLoggerEnabled();

    // Output debug log text.
    static void Output(const DString& log, bool bPrintTime = true);

    // Output debug log text and append newline for non-glog backends.
    static void OutputLine(const DString& log, bool bPrintTime = true);

    // Output log only in Debug build.
    static void Debug(const DString& log, bool bPrintTime = true);

    // Output line log only in Debug build.
    static void DebugLine(const DString& log, bool bPrintTime = true);

private:
    static DString GetTimeStamp();
};

} // namespace ui

#endif // UI_UTILS_LOG_UTIL_H_
