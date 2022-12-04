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

static void ThreadPoolRun2(KERNEL_NS::LibThreadPool *pool)
{
    auto tlsDef = KERNEL_NS::TlsUtil::GetDefTls();
    KERNEL_NS::LibString info;
    info.AppendFormat("ThreadPoolRun2 thread id[%llu] pool[%s]\n"
        , tlsDef->_threadId, pool->ToString().c_str());

    KERNEL_NS::SystemUtil::LockConsole();
    KERNEL_NS::SystemUtil::OutputToConsole(info);
    KERNEL_NS::SystemUtil::UnlockConsole();
}

static void ThreadPoolRun3(KERNEL_NS::LibThreadPool *pool)
{
    auto tlsDef = KERNEL_NS::TlsUtil::GetDefTls();
    KERNEL_NS::LibString info;
    info.AppendFormat("ThreadPoolRun3 thread id[%llu]\n", tlsDef->_threadId);

    KERNEL_NS::SystemUtil::LockConsole();
    KERNEL_NS::SystemUtil::OutputToConsole(info);
    KERNEL_NS::SystemUtil::UnlockConsole();
}

static void ThreadPoolRun4(KERNEL_NS::LibThreadPool *pool)
{
    auto tlsDef = KERNEL_NS::TlsUtil::GetDefTls();
    KERNEL_NS::LibString info;
    info.AppendFormat("ThreadPoolRun4 thread id[%llu] pool[%s]\n"
        , tlsDef->_threadId, pool->ToString().c_str());

    KERNEL_NS::SystemUtil::LockConsole();
    KERNEL_NS::SystemUtil::OutputToConsole(info);
    KERNEL_NS::SystemUtil::UnlockConsole();
}

static void ThreadPoolRun5(KERNEL_NS::LibThreadPool *pool)
{
    auto tlsDef = KERNEL_NS::TlsUtil::GetDefTls();
    KERNEL_NS::LibString info;
    info.AppendFormat("ThreadPoolRun5 thread id[%llu] pool[%s]\n"
        , tlsDef->_threadId, pool->ToString().c_str());

    KERNEL_NS::SystemUtil::LockConsole();
    KERNEL_NS::SystemUtil::OutputToConsole(info);
    KERNEL_NS::SystemUtil::UnlockConsole();
}

void TestThread::Run()
{
    KERNEL_NS::LibThread libThead;
    KERNEL_NS::LibThread libThead2;
    KERNEL_NS::LibThread libThead3;
    KERNEL_NS::LibThread libThead4;
    TestThreadTask testTask;
    libThead.AddTask(&testTask, &TestThreadTask::Run);

    libThead2.AddTask(KERNEL_NS::DelegateFactory::Create(&TestThreadTask2::Run));
    libThead3.AddTask(KERNEL_NS::DelegateFactory::Create(&TestThreadTask2::Run));
    libThead4.AddTask(KERNEL_NS::DelegateFactory::Create(&TestThreadTask2::Run));

    libThead.Start();
    libThead2.Start();
    libThead3.Start();
    libThead4.Start();

    getchar();
    libThead.HalfClose();
    libThead2.HalfClose();
    libThead3.HalfClose();
    libThead4.HalfClose();
    libThead.FinishClose();
    libThead2.FinishClose();
    libThead3.FinishClose();
    libThead4.FinishClose();
    std::cout << "end thread test" << std::endl;

    // 线程池
    {
        KERNEL_NS::LibThreadPool pool;

        // 此时不会启动线程
        pool.Init(0, 12);
        pool.Start();

        ThreadPoolTask1 *task1 = new ThreadPoolTask1;

        // 只添加任务不启动线程工作 队列是有序的
        pool.AddTask(task1, true, 0);
        pool.AddTask(&ThreadPoolRun2, true, 0);
        pool.AddTask(&ThreadPoolRun3, true, 0);
        pool.AddTask(&ThreadPoolRun4, true, 0);
        pool.AddTask(&ThreadPoolRun5, true, 0);

        // 添加5个工作线程 多线程是乱序执行的,所以打印的顺序不能保证
        pool.AddThreads(5);

        // 只唤醒
        pool.AddTask(&ThreadPoolRun2, false, 0);
        pool.AddTask(&ThreadPoolRun3, false, 0);
        pool.AddTask(&ThreadPoolRun4, false, 0);
        pool.AddTask(&ThreadPoolRun5, false, 0);
        getchar();

        if(pool.HalfClose())
            pool.FinishClose();
    }
}