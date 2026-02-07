#include "LogUtil.h"

#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"

#include <atomic>
#include <chrono>
#include <string>

#ifdef DUILIB_BUILD_FOR_SDL
    #include <SDL3/SDL.h>
#endif

#if defined(DUILIB_WITH_GLOG)
    #include <glog/logging.h>
    #include <mutex>
#endif

namespace ui
{

static std::chrono::steady_clock::time_point s_startTime = std::chrono::steady_clock::now();
static std::atomic_bool s_runtimeLoggerEnabled(false);

#if defined(DUILIB_WITH_GLOG)
static std::mutex s_runtimeLoggerMutex;
static std::string s_runtimeProcessName("duilib");
static std::string s_runtimeLogDir;
static bool s_runtimeAlsoLogToStderr = true;
static std::atomic_bool s_runtimeLoggerAutoEnable(true);

static void ApplyRuntimeLoggerOptionsUnlocked()
{
    if (!s_runtimeLogDir.empty()) {
        FLAGS_log_dir = s_runtimeLogDir;
    }
    FLAGS_alsologtostderr = s_runtimeAlsoLogToStderr ? 1 : 0;
    FLAGS_colorlogtostderr = s_runtimeAlsoLogToStderr ? 1 : 0;
    FLAGS_logbufsecs = 0;
}

static bool InitializeRuntimeLoggerUnlocked()
{
    if (s_runtimeLoggerEnabled.load()) {
        return true;
    }
    if (!s_runtimeLoggerAutoEnable.load()) {
        return false;
    }

    if (s_runtimeProcessName.empty()) {
        s_runtimeProcessName = "duilib";
    }
    google::InitGoogleLogging(s_runtimeProcessName.c_str());
    ApplyRuntimeLoggerOptionsUnlocked();
    s_runtimeLoggerEnabled.store(true);
    return true;
}
#endif

bool LogUtil::InitRuntimeLogger(const DString& processName,
                                const DString& logDir,
                                bool alsoLogToStderr)
{
#if defined(DUILIB_WITH_GLOG)
    std::lock_guard<std::mutex> guard(s_runtimeLoggerMutex);
    s_runtimeProcessName = processName.empty() ? s_runtimeProcessName : StringConvert::TToUTF8(processName);
    if (s_runtimeProcessName.empty()) {
        s_runtimeProcessName = "duilib";
    }
    s_runtimeLogDir = logDir.empty() ? s_runtimeLogDir : StringConvert::TToUTF8(logDir);
    s_runtimeAlsoLogToStderr = alsoLogToStderr;
    s_runtimeLoggerAutoEnable.store(true);

    if (s_runtimeLoggerEnabled.load()) {
        ApplyRuntimeLoggerOptionsUnlocked();
        return true;
    }
    return InitializeRuntimeLoggerUnlocked();
#else
    UNUSED_VARIABLE(processName);
    UNUSED_VARIABLE(logDir);
    UNUSED_VARIABLE(alsoLogToStderr);
    return false;
#endif
}

void LogUtil::ShutdownRuntimeLogger()
{
#if defined(DUILIB_WITH_GLOG)
    std::lock_guard<std::mutex> guard(s_runtimeLoggerMutex);
    s_runtimeLoggerAutoEnable.store(false);
    if (!s_runtimeLoggerEnabled.load()) {
        return;
    }

    google::ShutdownGoogleLogging();
    s_runtimeLoggerEnabled.store(false);
#endif
}

bool LogUtil::IsRuntimeLoggerEnabled()
{
#if defined(DUILIB_WITH_GLOG)
    if (!s_runtimeLoggerEnabled.load() && s_runtimeLoggerAutoEnable.load()) {
        std::lock_guard<std::mutex> guard(s_runtimeLoggerMutex);
        if (!s_runtimeLoggerEnabled.load()) {
            InitializeRuntimeLoggerUnlocked();
        }
    }
#endif
    return s_runtimeLoggerEnabled.load();
}

DString LogUtil::GetTimeStamp()
{
    std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
    auto thisTime = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - s_startTime);
    uint64_t nTimeMs = static_cast<uint64_t>(thisTime.count());
    uint32_t nHH = static_cast<uint32_t>((nTimeMs / 1000) / 60 / 60);
    uint32_t nMM = static_cast<uint32_t>((nTimeMs / 1000) / 60);
    uint32_t nSS = static_cast<uint32_t>(nTimeMs / 1000);
    uint32_t nMS = static_cast<uint32_t>(nTimeMs % 1000);
    return StringUtil::Printf(_T("%02u:%02u:%02u.%03u "), nHH, nMM, nSS, nMS);
}

void LogUtil::Output(const DString& log, bool bPrintTime)
{
    DString logMsg;
    if (bPrintTime) {
        logMsg = GetTimeStamp() + log;
    }
    else {
        logMsg = log;
    }

#if defined(DUILIB_WITH_GLOG)
    if (IsRuntimeLoggerEnabled()) {
        LOG(INFO) << StringConvert::TToUTF8(logMsg);
        return;
    }
#endif

#ifdef DUILIB_BUILD_FOR_WIN
    ::OutputDebugString(logMsg.c_str());
#elif defined(DUILIB_BUILD_FOR_SDL)
    SDL_Log("%s", logMsg.c_str());
#else
    UNUSED_VARIABLE(logMsg);
#endif
}

void LogUtil::OutputLine(const DString& log, bool bPrintTime)
{
#if defined(DUILIB_WITH_GLOG)
    if (IsRuntimeLoggerEnabled()) {
        Output(log, bPrintTime);
        return;
    }
#endif

#ifdef DUILIB_BUILD_FOR_WIN
    Output(log + _T("\r\n"), bPrintTime);
#else
    Output(log + _T("\n"), bPrintTime);
#endif
}

void LogUtil::Debug(const DString& log, bool bPrintTime)
{
#ifdef _DEBUG
    Output(log, bPrintTime);
#else
    UNUSED_VARIABLE(log);
    UNUSED_VARIABLE(bPrintTime);
#endif
}

void LogUtil::DebugLine(const DString& log, bool bPrintTime)
{
#ifdef _DEBUG
    OutputLine(log, bPrintTime);
#else
    UNUSED_VARIABLE(log);
    UNUSED_VARIABLE(bPrintTime);
#endif
}

} // namespace ui
