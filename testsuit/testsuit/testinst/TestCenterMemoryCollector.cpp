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
 * Date: 2023-07-02 14:34:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestCenterMemoryCollector.h>

class TestCenterMemroyAlloc
{
    POOL_CREATE_OBJ_DEFAULT(TestCenterMemroyAlloc);
public:
    BUFFER1024 _buffer;
};

#define TEST_CENTER_COLLECT_COUNT 10

static KERNEL_NS::Locker g_TestCenterMemCollectorGuard;
static std::vector<TestCenterMemroyAlloc *> g_Ptrs;

static void TestMultiThreadAlloc(KERNEL_NS::LibThread *t)
{
    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    auto timeMgr = poller->GetTimerMgr();

    poller->PrepareLoop();

    auto tlsOwner = KERNEL_NS::TlsUtil::GetTlsCompsOwner();
    auto memoryCleaner = tlsOwner->GetComp<KERNEL_NS::TlsMemoryCleanerComp>();
    memoryCleaner->SetIntervalMs(5000);

    while (!t->IsDestroy())
    {
        timeMgr->Drive();
        KERNEL_NS::SystemUtil::ThreadSleep(100);
        auto ptr = TestCenterMemroyAlloc::New_TestCenterMemroyAlloc();

        g_TestCenterMemCollectorGuard.Lock();
        g_Ptrs.push_back(ptr);
        g_TestCenterMemCollectorGuard.Unlock();
    }

    poller->OnLoopEnd();
}

static void TestMultiThreadAlloc2(KERNEL_NS::LibThread *t)
{
    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    auto timeMgr = poller->GetTimerMgr();
    poller->PrepareLoop();

    auto tlsOwner = KERNEL_NS::TlsUtil::GetTlsCompsOwner();
    auto memoryCleaner = tlsOwner->GetComp<KERNEL_NS::TlsMemoryCleanerComp>();
    memoryCleaner->SetIntervalMs(5000);

    while (!t->IsDestroy())
    {
        timeMgr->Drive();
        KERNEL_NS::SystemUtil::ThreadSleep(100);
        auto ptr = TestCenterMemroyAlloc::NewThreadLocal_TestCenterMemroyAlloc();

        g_TestCenterMemCollectorGuard.Lock();
        g_Ptrs.push_back(ptr);
        g_TestCenterMemCollectorGuard.Unlock();
    }

    poller->OnLoopEnd();
}

// 内存日志监控
static void TestMultiThreadAlloc3(KERNEL_NS::LibThread *t)
{
    auto delg = KERNEL_NS::MemoryMonitor::GetInstance()->MakeWorkTask();
    while (!t->IsDestroy())
    {
        KERNEL_NS::SystemUtil::ThreadSleep(5000);
        delg->Invoke();
    }
}

void TestCenterMemoryCollector::Run()
{
    // 1.测试CenterMemoryCollector 启动和正常退出 ok
    // 2.测试单线程下分配和释放ThreadLocal
    if(0)
    {
        // 同线程调用相同分配器
        std::vector<TestCenterMemroyAlloc *> ptrs;
        for(Int32 idx = 0; idx < TEST_CENTER_COLLECT_COUNT; ++idx)
        {
            auto ptr = TestCenterMemroyAlloc::NewThreadLocal_TestCenterMemroyAlloc();
            ptrs.push_back(ptr);
        }

        for(Int32 idx = 0; idx < TEST_CENTER_COLLECT_COUNT; ++idx)
        {
            auto ptr = ptrs[idx];
            TestCenterMemroyAlloc::DeleteThreadLocal_TestCenterMemroyAlloc(ptr);
        }
        ptrs.clear();

        // 同线程调用不同分配器
        for(Int32 idx = 0; idx < TEST_CENTER_COLLECT_COUNT; ++idx)
        {
            auto ptr = TestCenterMemroyAlloc::NewThreadLocal_TestCenterMemroyAlloc();
            ptrs.push_back(ptr);
        }

        for(Int32 idx = 0; idx < TEST_CENTER_COLLECT_COUNT; ++idx)
        {
            auto ptr = ptrs[idx];
            TestCenterMemroyAlloc::Delete_TestCenterMemroyAlloc(ptr);
        }
    }
    
    // 3.测试多线程的分配和释放, 重点看合并, 以及CenterMemoryCollector内部的逻辑
    {
        KERNEL_NS::CenterMemoryCollector::GetInstance()->SetBlockNumForPurgeLimit(5);
        KERNEL_NS::CenterMemoryCollector::GetInstance()->SetRecycleForPurgeLimit(5);
        // KERNEL_NS::CenterMemoryCollector::GetInstance()->SetAllThreadMemoryAllocUpperLimit(10);

        KERNEL_NS::SmartPtr<KERNEL_NS::LibThread> t1 = new KERNEL_NS::LibThread;
        t1->AddTask(TestMultiThreadAlloc);
        t1->Start();

        KERNEL_NS::SmartPtr<KERNEL_NS::LibThread> t2 = new KERNEL_NS::LibThread;
        t2->AddTask(TestMultiThreadAlloc2);
        t2->Start();

        KERNEL_NS::SmartPtr<KERNEL_NS::LibThread> t3 = new KERNEL_NS::LibThread;
        t3->AddTask(TestMultiThreadAlloc3);
        t3->Start();


        // 一次只释放一个 
        while (true)
        {
             g_TestCenterMemCollectorGuard.Lock();
             for(auto ptr : g_Ptrs)
             {
                TestCenterMemroyAlloc::DeleteThreadLocal_TestCenterMemroyAlloc(ptr);
             }
             g_Ptrs.clear();
             
             g_TestCenterMemCollectorGuard.Unlock(); 

            KERNEL_NS::SystemUtil::ThreadSleep(500);
        }
    }

    // 4.看CenterMemoryCollector 的内存监控信息
    {
        
    }
}