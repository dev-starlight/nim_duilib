#include "CreatorApplication.h"
#include "MainThread.h"

CreatorApplication::CreatorApplication()
{
}

CreatorApplication::~CreatorApplication()
{
}

void CreatorApplication::Run()
{
    // 创建主线程
    MainThread thread;

    // 执行主线程循环
    thread.RunMessageLoop();
}
