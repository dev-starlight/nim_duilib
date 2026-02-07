#include <gtest/gtest.h>
#include "duilib/Utils/LogUtil.h"

using ui::LogUtil;

TEST(LogUtilTest, RuntimeLoggerDefaultState)
{
#if defined(DUILIB_WITH_GLOG)
    EXPECT_TRUE(LogUtil::IsRuntimeLoggerEnabled());
#else
    EXPECT_FALSE(LogUtil::IsRuntimeLoggerEnabled());
#endif
}

TEST(LogUtilTest, RuntimeLoggerInitAndShutdown)
{
    const bool initResult = LogUtil::InitRuntimeLogger(_T("duilib_tests"), DString(), true);
#if defined(DUILIB_WITH_GLOG)
    EXPECT_TRUE(initResult);
    EXPECT_TRUE(LogUtil::IsRuntimeLoggerEnabled());
#else
    EXPECT_FALSE(initResult);
    EXPECT_FALSE(LogUtil::IsRuntimeLoggerEnabled());
#endif
    LogUtil::ShutdownRuntimeLogger();
    EXPECT_FALSE(LogUtil::IsRuntimeLoggerEnabled());
}

TEST(LogUtilTest, OutputDoesNotThrow)
{
    EXPECT_NO_THROW(LogUtil::Output(_T("log-util-output"), true));
    EXPECT_NO_THROW(LogUtil::OutputLine(_T("log-util-output-line"), false));
    EXPECT_NO_THROW(LogUtil::Debug(_T("log-util-debug"), true));
    EXPECT_NO_THROW(LogUtil::DebugLine(_T("log-util-debug-line"), false));
}
