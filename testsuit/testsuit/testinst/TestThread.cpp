/*!
 *  MIT License
 *  
 *  Copyright (c) 2020 ericyonng<120453674@qq.com>
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to Fpermit persons to whom the Software is
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
 * Date: 2020-12-07 23:54:07
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestThread.h>

static KERNEL_NS::Locker g_lck;
static UInt64 g_Num = 0;

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
#define Sleep(...)
#endif

class TestThreadTask
{
public:
    TestThreadTask() {}
    void Run(KERNEL_NS::LibThread *libThread)
    {
        while (!libThread->IsDestroy())
        {
            g_lck.Lock();
            //KERNEL_NS::SystemUtil::Sleep(100);
            ++g_Num;
            std::cout << "id = " << libThread->GetId() << " g_num = " << g_Num << std::endl;
            g_lck.Unlock();

            //Sleep(1000);
            
        }

        std::cout << "id = " << libThread->GetId() << " quit." << std::endl;
    }
};

class TestThreadTask3 : public KERNEL_NS::ITask
{
    POOL_CREATE_OBJ_DEFAULT_P1(ITask, TestThreadTask3);

public:
    TestThreadTask3() {}
    virtual void Release()
    {
        TestThreadTask3::Delete_TestThreadTask3(this);
    }
    virtual bool CanReDo() {return true; }

    void Run()
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestThreadTask3, "test redo TestThreadTask3"));
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
    }
};


class TestThreadTask2
{
public:
    static void Run(KERNEL_NS::LibThread *libThread)
    {
        while (!libThread->IsDestroy())
        {
            g_lck.Lock();
            //KERNEL_NS::SystemUtil::Sleep(100);
            ++g_Num;
            std::cout << "id = " << libThread->GetId() << " g_num = " << g_Num << std::endl;
            g_lck.Unlock();

            //Sleep(1000);

        }
        std::cout << "id = " << libThread->GetId() << " quit." << std::endl;
    }
};

class ThreadPoolTask1 : public KERNEL_NS::ITask
{
public:
    virtual void Run()
    {
        auto tlsDef = KERNEL_NS::TlsUtil::GetDefTls();
        KERNEL_NS::LibString info;
        info.AppendFormat("ThreadPoolTask1 thread id[%llu]\n", tlsDef->_threadId);

        KERNEL_NS::SystemUtil::LockConsole();
        KERNEL_NS::SystemUtil::OutputToConsole(info);
        KERNEL_NS::SystemUtil::UnlockConsole();
    }
    virtual void Release()
    {
        CRYSTAL_DELETE(this);
    }
};

class TestThreadTPoolask3 : public KERNEL_NS::ITask
{
    POOL_CREATE_OBJ_DEFAULT_P1(ITask, TestThreadTPoolask3);

public:
    TestThreadTPoolask3() {}

    virtual void Release()
    {
        TestThreadTPoolask3::Delete_TestThreadTPoolask3(this);
    }
    virtual bool CanReDo() {return true; }

    void Run()
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestThreadTask3, "test redo pool TestThreadTPoolask3"));
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
    }
};


// static void ThreadPoolRun2(KERNEL_NS::LibThreadPool *pool)
// {
//     auto tlsDef = KERNEL_NS::TlsUtil::GetDefTls();
//     KERNEL_NS::LibString info;
//     info.AppendFormat("ThreadPoolRun2 thread id[%llu] pool[%s]\n"
//         , tlsDef->_threadId, pool->ToString().c_str());

//     KERNEL_NS::SystemUtil::LockConsole();
//     KERNEL_NS::SystemUtil::OutputToConsole(info);
//     KERNEL_NS::SystemUtil::UnlockConsole();
// }

// static void ThreadPoolRun3(KERNEL_NS::LibThreadPool *pool)
// {
//     auto tlsDef = KERNEL_NS::TlsUtil::GetDefTls();
//     KERNEL_NS::LibString info;
//     info.AppendFormat("ThreadPoolRun3 thread id[%llu]\n", tlsDef->_threadId);

//     KERNEL_NS::SystemUtil::LockConsole();
//     KERNEL_NS::SystemUtil::OutputToConsole(info);
//     KERNEL_NS::SystemUtil::UnlockConsole();
// }

// static void ThreadPoolRun4(KERNEL_NS::LibThreadPool *pool)
// {
//     auto tlsDef = KERNEL_NS::TlsUtil::GetDefTls();
//     KERNEL_NS::LibString info;
//     info.AppendFormat("ThreadPoolRun4 thread id[%llu] pool[%s]\n"
//         , tlsDef->_threadId, pool->ToString().c_str());

//     KERNEL_NS::SystemUtil::LockConsole();
//     KERNEL_NS::SystemUtil::OutputToConsole(info);
//     KERNEL_NS::SystemUtil::UnlockConsole();
// }

// static void ThreadPoolRun5(KERNEL_NS::LibThreadPool *pool)
// {
//     auto tlsDef = KERNEL_NS::TlsUtil::GetDefTls();
//     KERNEL_NS::LibString info;
//     info.AppendFormat("ThreadPoolRun5 thread id[%llu] pool[%s]\n"
//         , tlsDef->_threadId, pool->ToString().c_str());

//     KERNEL_NS::SystemUtil::LockConsole();
//     KERNEL_NS::SystemUtil::OutputToConsole(info);
//     KERNEL_NS::SystemUtil::UnlockConsole();
// }
//
// void TestThread::Run()
// {
//     KERNEL_NS::LibThread libThead;
//     KERNEL_NS::LibThread libThead2;
//     KERNEL_NS::LibThread libThead3;
//     KERNEL_NS::LibThread libThead4;
//     TestThreadTask testTask;
//     libThead.AddTask(&testTask, &TestThreadTask::Run);
//
//     libThead2.AddTask(KERNEL_NS::DelegateFactory::Create(&TestThreadTask2::Run));
//     libThead3.AddTask(KERNEL_NS::DelegateFactory::Create(&TestThreadTask2::Run));
//     libThead4.AddTask(KERNEL_NS::DelegateFactory::Create(&TestThreadTask2::Run));
//
//     KERNEL_NS::LibThread thread5;
//     TestThreadTask3 *task3 = TestThreadTask3::New_TestThreadTask3();
//     thread5.AddTask(task3);
//
//
//     // libThead.Start();
//     // libThead2.Start();
//     // libThead3.Start();
//     // libThead4.Start();
//     thread5.Start();
//
//     getchar();
//     libThead.HalfClose();
//     libThead2.HalfClose();
//     libThead3.HalfClose();
//     libThead4.HalfClose();
//     thread5.HalfClose();
//     libThead.FinishClose();
//     libThead2.FinishClose();
//     libThead3.FinishClose();
//     libThead4.FinishClose();
//     thread5.FinishClose();
//     std::cout << "end thread test" << std::endl;
//
//     // 线程池
//     {
//         KERNEL_NS::LibThreadPool pool;
//
//         // 此时不会启动线程
//         pool.Init(0, 12);
//         pool.Start();
//
//         // ThreadPoolTask1 *task1 = new ThreadPoolTask1;
//         TestThreadTPoolask3 *task2 = TestThreadTPoolask3::New_TestThreadTPoolask3();
//
//         // 只添加任务不启动线程工作 队列是有序的
//         // pool.AddTask(task1, true, 0);
//         // pool.AddTask(&ThreadPoolRun2, true, 0);
//         // pool.AddTask(&ThreadPoolRun3, true, 0);
//         // pool.AddTask(&ThreadPoolRun4, true, 0);
//         // pool.AddTask(&ThreadPoolRun5, true, 0);
//         pool.AddTask(task2);
//
//         // 添加5个工作线程 多线程是乱序执行的,所以打印的顺序不能保证
//         pool.AddThreads(5);
//
//         // 只唤醒
//         // pool.AddTask(&ThreadPoolRun2, false, 0);
//         // pool.AddTask(&ThreadPoolRun3, false, 0);
//         // pool.AddTask(&ThreadPoolRun4, false, 0);
//         // pool.AddTask(&ThreadPoolRun5, false, 0);
//         getchar();
//
//         if(pool.HalfClose())
//             pool.FinishClose();
//     }
// }

#ifdef CRYSTAL_NET_CPP20
static  KERNEL_NS::CoTask<KERNEL_NS::LibString> GetStr()
{
    auto str = KERNEL_NS::LibString("ni hao");

    co_return str;
}
#endif

void TestThread::Run()
{
    auto poller = KERNEL_NS::TlsUtil::GetPoller();

    KERNEL_NS::SmartPtr<KERNEL_NS::LibEventLoopThreadPool, KERNEL_NS::AutoDelMethods::Release> pool = new KERNEL_NS::LibEventLoopThreadPool(1, 16);
    pool->Start();

#ifdef CRYSTAL_NET_CPP20
    KERNEL_NS::PostCaller([poller, pool]() mutable ->KERNEL_NS::CoTask<>
    {
        KERNEL_NS::LibString info = "nihao ha";
        KERNEL_NS::RunRightNow([]()-> KERNEL_NS::CoTask<>
        {
            auto str = co_await GetStr();
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestThread, "hello run right now str:%s"), str.c_str());
        });
        
        pool->Send([]()
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestThread, "hello thread pool thread id:%llu"), KERNEL_NS::SystemUtil::GetCurrentThreadId());
        });

        pool->Send([]()
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestThread, "hello thread pool thread id:%llu"), KERNEL_NS::SystemUtil::GetCurrentThreadId());
        });

        // 监听退出
        pool->Send([poller]()
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestThread, "app listen quit signal..."));
            getchar();
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestThread, "will quit app"));
        
            poller->QuitLoop();
        }, true);

        co_await GetStr();

        // auto poolRes = co_await pool->SendAsync<KERNEL_NS::LibString>([]()->KERNEL_NS::LibString
        // {
        //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestThread, "pool excute lambda and will return result thread id:%llu"), KERNEL_NS::SystemUtil::GetCurrentThreadId());
        //
        //     return KERNEL_NS::LibString().AppendFormat("pool excute result thread id:%llu", KERNEL_NS::SystemUtil::GetCurrentThreadId());
        // });

        pool->Send([]()
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestThread, "hello thread pool thread id 3:%llu"), KERNEL_NS::SystemUtil::GetCurrentThreadId());
        });

        pool->Send([]()
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestThread, "hello thread pool thread id 4:%llu"), KERNEL_NS::SystemUtil::GetCurrentThreadId());
        });

        info = "buhaole ba";
        
        // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestThread, "current thread id:%llu, pool result:%s"), KERNEL_NS::SystemUtil::GetCurrentThreadId(), poolRes->c_str());
    });

#endif

    poller->PrepareLoop();
    poller->EventLoop();
    poller->OnLoopEnd();

    pool->Close();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestThread, "TestThread finish."));
}
