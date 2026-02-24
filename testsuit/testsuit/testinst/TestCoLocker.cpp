// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-02-23 22:02:01
// Author: Eric Yonng
// Description:


#include <pch.h>
#include "TestCoLocker.h"

namespace TestCoLockerNs
{
    static std::atomic<Int64> s_GenQps;
    static std::atomic<Int64> s_comsumeQps;

    class EventLoopStartupCounter : public KERNEL_NS::IThreadStartUp
    {
    public:
        virtual void Run() override
        {
            CLOG_INFO_GLOBAL(EventLoopStartupCounter, "EventLoopStartupCounter start");
            
            auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
            timer->SetTimeOutHandler([](KERNEL_NS::LibTimer *t)
            {
                auto gen = s_GenQps.exchange(0, std::memory_order_acq_rel);
                auto comsume = s_comsumeQps.exchange(0, std::memory_order_acq_rel);
                // CLOG_INFO_GLOBAL(EventLoopStartupCounter, "gen:%lld, comsume:%lld", gen, comsume);
            });
            timer->Schedule(KERNEL_NS::TimeSlice::FromSeconds(1));
        }

        virtual void Release()
        {
            delete this;
        }
    };

    class EventLoopStartup : public KERNEL_NS::IThreadStartUp
    {
    public:
        EventLoopStartup(KERNEL_NS::CoLocker *locker, Int32 id)
            :_locker(locker)
        ,_id(id)
        {
            
        }
        virtual void Run()
        {
            // wait 这段代码在堆空间, 协程唤醒后执行不会在poller中执行
            auto newCoLockerInfo = KERNEL_NS::CoLockerInfo::New_CoLockerInfo();
            KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
            {
                // 在poller退出的时候必须唤醒locker,避免locker无法被释放
                auto poller = KERNEL_NS::TlsUtil::GetPoller();
                Int64 idCount = 0;
                while ((!poller->IsQuit()) && (co_await _locker->Wait() == Status::Success))
                {
                    CLOG_INFO("TestCoLocker-[%d]: wake up idCount:%lld.", _id, ++idCount);
                    s_comsumeQps.fetch_add(1, std::memory_order_release);
                }

                CLOG_INFO("TestCoLocker QUIT");
            });
        }
        virtual void Release()
        {
            delete this;
        }

        KERNEL_NS::CoLocker *_locker;
        const Int32 _id;
    };

    class EventLoopStartupSignal : public KERNEL_NS::IThreadStartUp
    {
    public:
        EventLoopStartupSignal(KERNEL_NS::CoLocker *locker)
            :_locker(locker)
        {
            
        }
        virtual void Run()
        {
            // KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
            // {
            //     auto poller = KERNEL_NS::TlsUtil::GetPoller();
            //     while (!poller->IsQuit())
            //     {
            //         _locker->Sinal();
            //         s_GenQps.fetch_add(1, std::memory_order_release);
            //     }
            //
            //     CLOG_INFO_GLOBAL(EventLoopStartupSignal, "signal quit");
            //     co_return;
            // });

            auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
            
            timer->SetTimeOutHandler([this](KERNEL_NS::LibTimer *t)
            {
                CLOG_INFO("testcolocker hello signal.");
                _locker->Broadcast();
            });
            timer->GetMgr()->TakeOverLifeTime(timer, [](KERNEL_NS::LibTimer *t)
            {
                KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
            });
            timer->Schedule(KERNEL_NS::TimeSlice::FromSeconds(1));
        }
        virtual void Release()
        {
            delete this;
        }

        KERNEL_NS::CoLocker *_locker;
    };
}
using namespace TestCoLockerNs;

void TestCoLocker::Run()
{
    KERNEL_NS::CoLocker locker;
    auto thread3 = new KERNEL_NS::LibEventLoopThread("EventLoopStartupCounter counter", new EventLoopStartupCounter());
    thread3->Start();

    // waiter
    std::vector<KERNEL_NS::LibEventLoopThread *> waiters;
    {
        auto thread = new KERNEL_NS::LibEventLoopThread("testcolocker waiter", new EventLoopStartup(&locker, 1));
        waiters.push_back(thread);
        thread->Start();

        thread = new KERNEL_NS::LibEventLoopThread("testcolocker waiter", new EventLoopStartup(&locker, 2));
        waiters.push_back(thread);
        thread->Start();
    }

    auto thread2 = new KERNEL_NS::LibEventLoopThread("testcolocker signaler", new EventLoopStartupSignal(&locker));
    thread2->Start();

    // while (true)
    // {
    //     KERNEL_NS::SystemUtil::ThreadSleep(1000);
    // }
    //
    getchar();

    locker.Quit();

    while(locker.HasWaiter())
    {
        locker.Broadcast();
        CLOG_INFO_GLOBAL(TestCoLocker, "has waiter...");
    }

    for (auto waiter : waiters)
        waiter->HalfClose();

    thread2->HalfClose();
    thread3->HalfClose();

    for (auto waiter : waiters)
        waiter->FinishClose();
    
    thread2->FinishClose();
    thread3->FinishClose();
}
