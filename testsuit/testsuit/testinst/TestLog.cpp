/*!
 *  MIT License
 *  
 *  Copyright (c) 2020 ericyonng<120453674@qq.com>
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 * 
 * Date: 2021-02-20 23:13:06
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestLog.h>

class DebugHook
{
    static void AfterRun(const KERNEL_NS::LogData *logData)
    {

    }

    static void BeforeRun(KERNEL_NS::LogData *logData)
    {

    }
};

class Thread1
{
public:
    static void Run(KERNEL_NS::LibThread *o)
    {
        while(!o->IsDestroy())
            g_Log->Custom("thread1 run thread id=[%llu]", o->GetTheadId());
    }
};

class Thread2
{
public:
    static void Run(KERNEL_NS::LibThread *o)
    {
        while (!o->IsDestroy())
            g_Log->Custom("thread2 run thread id=[%llu]", o->GetTheadId());
    }
};

class LogEx : public KERNEL_NS::LibLog
{
public:
    enum LevelId
    {
        TestExpand = 0, // 禁用
    };

public:
    static LogEx *GetInstance()
    {
        static KERNEL_NS::SmartPtr<LogEx> s_Log = new LogEx;

        return s_Log.AsSelf();
    }

    void TestExpandLog(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        auto finalSize = KERNEL_NS::LibString::CheckFormatSize(fmt, va);
        va_end(va);

        va_start(va, fmt);
        _Common1(tag, LevelId::TestExpand, fileName, funcName, codeLine, fmt, va, finalSize);
        va_end(va);
    }

};

class TestLogClass
{
public:
    void Run()
    {
        // 可以在非类实例中使用
        CLOG_DEBUG_GLOBAL(TestLog, "hello world %s", KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
        CLOG_INFO_GLOBAL(TestLog, "hello world %s", KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
        CLOG_WARN_GLOBAL(TestLog, "hello world %s", KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
        CLOG_ERROR_GLOBAL(TestLog, "hello world %s", KERNEL_NS::SystemUtil::GetCurProgramName().c_str());

        // 在类实例中使用(需要有this)
        CLOG_DEBUG("hello world %s", KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
        CLOG_INFO("hello world %s", KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
        CLOG_WARN("hello world %s", KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
        CLOG_ERROR("hello world %s", KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
    }
};

void TestLog::Run() 
{
    auto myLog = g_Log;
    myLog->Debug(LOGFMT_NON_OBJ_TAG(TestLog, "hello world %s")
    , KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
    myLog->Info(LOGFMT_NON_OBJ_TAG(TestLog, "hello world %s")
    , KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
    myLog->Warn(LOGFMT_NON_OBJ_TAG(TestLog, "hello world %s")
    , KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
    myLog->Error(LOGFMT_NON_OBJ_TAG(TestLog, "hello world %s")
    , KERNEL_NS::SystemUtil::GetCurProgramName().c_str());

    myLog->Crash("hello world %s"
    , KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
    myLog->Net(LOG_NON_OBJ_TAG(TestLog), "hello world %s"
    , KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
    myLog->Custom("hello world %s"
    , KERNEL_NS::SystemUtil::GetCurProgramName().c_str());

    CLOG_INFO_GLOBAL(TestLog, "hello world %s", KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
    CLOG_WARN_GLOBAL(TestLog, "hello world %s", KERNEL_NS::SystemUtil::GetCurProgramName().c_str());

    getchar();
    
    // // 测试满1MB
    // // for(Int32 i = 0; i < 102400; ++i)
    // // {
    // //     myLog->Info(LOGFMT_OBJ_TAG("test pass 1MB %s")
    // //     , KERNEL_NS::SystemUtil::GetCurProgramName().c_str());
    // // }

    // // 多线程并发测试
    // {
    //     KERNEL_NS::LibThread thread1;
    //     KERNEL_NS::LibThread thread2;
    //     KERNEL_NS::LibThread thread3;
    //     thread1.AddTask(&Thread1::Run);
    //     thread2.AddTask(&Thread1::Run);
    //     thread3.AddTask(&Thread2::Run);

    //     thread1.Start();
    //     thread2.Start();
    //     thread3.Start();

    //     // getchar();
    //     thread1.HalfClose();
    //     thread2.HalfClose();
    //     thread3.HalfClose();

    //     thread1.FinishClose();
    //     thread2.FinishClose();
    //     thread3.FinishClose();

    // }

    // {
    //     g_Log->Close();
    //     g_Log = NULL;
    //     auto logEx = LogEx::GetInstance();
    //     logEx->Init(g_LogIniName.c_str(), g_LogIniRootPath.c_str());
    //     logEx->Start();
    //     logEx->TestExpandLog(LOGFMT_NON_OBJ_TAG(TestLog, "test log ex expand."));
    //     logEx->Close();
    // }
}
