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
 * Date: 2021-03-13 21:27:40
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestLocker.h>

static UInt64 g_curThreadId = 0;
static KERNEL_NS::ConditionLocker g_lck;

class TestLockerThread
{
public:
    static void Run(KERNEL_NS::LibThread *t)
    {
        g_lck.Lock();
        g_curThreadId = t->GetTheadId();
        g_lck.Unlock();
        g_lck.Sinal();

        t->AddTask(&TestLockerThread::Run);
    }
};

class TestLockerThreadWait
{
public:
    static void Run(KERNEL_NS::LibThread *t)
    {
        while (!t->IsDestroy())
        {
            g_lck.Lock();
            g_lck.Wait();
            g_Log->Custom("cur thread id=[%llu]", g_curThreadId);
            g_lck.Unlock();       
        }
    }
};

KERNEL_NS::LockWrap<KERNEL_NS::_Build::TL> g_SpinLock;
std::atomic<UInt64> g_Num{0};
KERNEL_NS::SmartPtr<std::list<Int64>, KERNEL_NS::_Build::TL> g_List = new std::list<Int64>;
std::atomic_bool g_StartTest{false};
std::atomic<Int64> g_StartTime{0};
std::atomic<Int64> g_EndTime{0};

#define TEST_SPIN_PUSH_NUM_AMOUNT   10000000LL

class TestSpinLock
{
public:
    static void Run(KERNEL_NS::LibThread *t)
    {
        if(!g_StartTest.exchange(true))
            g_StartTime = KERNEL_NS::TimeUtil::GetMicroTimestamp();

        while(!t->IsDestroy())
        {
            g_SpinLock.Lock();
            g_List->push_back(1024);
            const Int64 curNum = ++g_Num;
            g_SpinLock.Unlock();

            if(curNum >= TEST_SPIN_PUSH_NUM_AMOUNT)
                break;
        }

        if(g_StartTest.exchange(false))
        {
            g_EndTime = KERNEL_NS::TimeUtil::GetMicroTimestamp();
            g_Log->Custom("cost %lld micro seconds of pushing %lld numbers into list cur list amount=[%lld]"
            , g_EndTime-g_StartTime, TEST_SPIN_PUSH_NUM_AMOUNT, g_Num.load());
        }
    }
};

void TestLocker::Run() 
{
    // KERNEL_NS::LibThread thread1;
    // KERNEL_NS::LibThread thread2;
    // KERNEL_NS::LibThread thread3;
    // KERNEL_NS::LibThread thread4;

    // thread1.AddTask(&TestLockerThread::Run);
    // thread2.AddTask(&TestLockerThread::Run);
    // thread3.AddTask(&TestLockerThread::Run);
    // thread4.AddTask(&TestLockerThreadWait::Run);

    // thread4.Start();
    // thread1.Start();
    // thread2.Start();
    // thread3.Start();

    // getchar();
    // g_Log->Custom("test finish.");
    // thread4.HalfClose();
    // thread1.HalfClose();
    // thread2.HalfClose();
    // thread3.HalfClose();
    // g_Log->Custom("test finish halfclose.");
    // g_lck.Broadcast();

    // thread4.FinishClose();
    // thread1.FinishClose();
    // thread2.FinishClose();
    // thread3.FinishClose();

    KERNEL_NS::LibThread thread1;
    KERNEL_NS::LibThread thread2;

    thread1.AddTask(&TestSpinLock::Run);
    thread2.AddTask(&TestSpinLock::Run);

    thread1.Start();
    thread2.Start();

    getchar();
    g_Log->Custom("finish test spin lock.");

    thread1.HalfClose();
    thread2.HalfClose();

    thread1.FinishClose();
    thread2.FinishClose();
}
