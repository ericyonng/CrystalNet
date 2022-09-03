/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2020-10-11 23:13:25
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst.h>

#include <testsuit/testinst/TestMemoryAlloctor.h>
#include <testsuit/testinst/TestMemoryPool.h>
#include <testsuit/testinst/TestObjAlloctor.h>
#include <testsuit/testinst/TestThread.h>
#include <testsuit/testinst/TestGarbageCollector.h>
#include <testsuit/testinst/TestLibString.h>
#include <testsuit/testinst/TestString.h>
#include <testsuit/testinst/TestTime.h>
#include <testsuit/testinst/TestFileUtil.h>
#include <testsuit/testinst/TestGuid.h>
#include <testsuit/testinst/TestTemplateObjPool.h>
#include <testsuit/testinst/TestBinaryArray.h>
#include <testsuit/testinst/TestTls.h>
#include <testsuit/testinst/TestRandom.h>
#include <testsuit/testinst/TestCypher.h>
#include <testsuit/testinst/TestStatic.h>
#include <testsuit/testinst/TestCountUtil.h>
#include <testsuit/testinst/TestRttiUtil.h>
#include <testsuit/testinst/TestContainerUtil.h>
#include <testsuit/testinst/TestSystemUtil.h>
#include <testsuit/testinst/TestEncrypt.h>
#include <testsuit/testinst/TestFile.h>
#include <testsuit/testinst/TestLog.h>
#include <testsuit/testinst/TestStream.h>
#include <testsuit/testinst/TestLocker.h>
#include <testsuit/testinst/TestMessageQueue.h>
#include <testsuit/testinst/TestVariant.h>
#include <testsuit/testinst/TestTimer.h>
#include <testsuit/testinst/TestCpuInfo.h>
#include <testsuit/testinst/TestBackTrace.h>
#include <testsuit/testinst/TestDaemon.h>
#include <testsuit/testinst/TestEvent.h>
#include <testsuit/testinst/TestConcurrentPriorityQueue.h>

// void *operator new(size_t bytes)
// {
//     g_TotalBytes.fetch_add(bytes);

//     printf("\noperator new bytes:%llu, \n", bytes);
    
//     return ::malloc(bytes);
// }

// void *operator new[](size_t bytes)
// {
//     g_TotalBytes.fetch_add(bytes);

//     printf("\noperator new[] bytes:%llu, \n", bytes);

//     return ::malloc(bytes);
// }

static inline void DebugLogHookBefore(KERNEL_NS::LogData *logData)
{
    logData->_logInfo.AppendFormat("\nDebugLogHookBefore .");
}

class LibTestLog : public KERNEL_NS::LibLog
{
public:
    LibTestLog() {}
    ~LibTestLog() {}
};

class LogFactory : public KERNEL_NS::ILogFactory
{
public:
    virtual KERNEL_NS::ILog *Create()
    {
        return new LibTestLog();
    }
};

void TestInst::Run(int argc, char const *argv[])
{
    printf("/*********************************************/!\n");
    printf("/*           Hello Crystal Net!              */\n");
    printf("/*********************************************/!\n\n");

    LogFactory logFactory;
    Int32 err = KERNEL_NS::KernelUtil::Init(&logFactory, "LogCfg.ini", "./ini/");
    if(err != Status::Success)
    {
        CRYSTAL_TRACE("kernel init fail err:%d", err);
        return;
    }

    // 安装loghook
    g_Log->InstallBeforeLogHookFunc(KERNEL_NS::LogLevel::Debug, &DebugLogHookBefore);

    KERNEL_NS::KernelUtil::Start();

    // // TODO:analyze argv参数 使用lua分析

    CRYSTAL_TRACE("test started.\n");

    getchar();

    // // TestMemoryAlloctor::Run();
    // //TestMemoryPool::Run();
    // // TestObjAlloctor::Run();
    // // TestThread::Run();

    // //TestGarbageCollector::Run();
    // // TestLibString::Run();
    // //TestString::Run();
    // //TestTime::Run();
    // //TestFileUtil::Run();
    // // TestGuid::Run();
    // // TestTemplateObjPool::Run();
    // // TestBinaryArray::Run();
    // // TestTls::Run();
    // // TestRandom::Run();
    // // TestCypher::Run();
    // //TestStatic::Run();
    // //TestCountUtil::Run();
    // // TestTls::Run();
    // //TestRttiUtil::Run();
    // //TestContainerUtil::Run();
    // // TestSystemUtil::Run();
    // //TestEncrypt::Run();
    // // TestFile::Run();
    // // TestLog::Run();
    // // TestStream::Run();
    // // TestLocker::Run();
    // // TestMessageQueue::Run();
    // // TestVariant::Run();
    // // TestTimer::Run();
    // // TestCpuInfo::Run();
    // // TestBackTrace::Run();
    // //TestDaemon::Run();
    // // TestEvent::Run();
    // TestConcurrentPriorityQueue::Run();
    // CRYSTAL_TRACE("test inst finish.");

    KERNEL_NS::KernelUtil::Destroy();

    getchar();
}