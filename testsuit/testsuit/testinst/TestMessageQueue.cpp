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
 * Date: 2021-03-13 23:12:40
 * Author: Eric Yonng
 * Description: 
 *             1.8生产者,1消费者 1min测试：中位数:1035948 pps, 平均值: 1048130 pps, 总包数: 62865186
 *             2.8生产者,8消费者 1min测试: 
 *             3.linux 单线程生产极限30w/s
 * 原子操作一次在10ns左右，一次LibCpuCounter.Update也在10ns左右
*/

#include <pch.h>
#include <testsuit/testinst/TestMessageQueue.h>

#define TEST_MQ_MAX_CHANNEL 1
#define TEST_MQ_CONSUMER_COUNT 1

static std::atomic<UInt64> g_curMsgConsume{0};
static std::atomic<UInt64> g_curGenCount{ 0 };

static std::atomic<UInt64> g_TimerDriveTime {0};
static std::atomic<UInt64> g_TimerDriveCount {0};
static std::atomic<UInt64> g_FrameTime {0};
static std::atomic<UInt64> g_FrameCount {0};

static std::atomic<UInt64> g_MemoryAllocTime {0};
static std::atomic<UInt64> g_MemoryAllocCount {0};

static std::atomic<UInt64> g_pushTime {0};
static std::atomic<UInt64> g_pushTimeCount {0};

static std::atomic<UInt64> g_swapTime {0};
static std::atomic<UInt64> g_swapTimeCount {0};

static KERNEL_NS::MessageQueue<KERNEL_NS::PollerEvent *> *g_Queue = NULL;

struct TestMqBlock : public KERNEL_NS::PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, TestMqBlock);

public:
    TestMqBlock():KERNEL_NS::PollerEvent(1){}
    virtual ~TestMqBlock(){}

    virtual void Release()
    {
        TestMqBlock::Delete_TestMqBlock(this);
    }

public:
    BUFFER128 _buffer = {0};
};

POOL_CREATE_OBJ_DEFAULT_IMPL(TestMqBlock);

// mid:4247kqps, average:4246kqps
static void Generator(KERNEL_NS::LibThreadPool *t)
{
    while (!t->IsDestroy())
    {
        g_Queue->PushBack(TestMqBlock::New_TestMqBlock());
        ++g_curGenCount;
    }
}

// mid:3745kqps, average:3759kqps
static void Consumer(KERNEL_NS::LibThreadPool *t)
{
    TestMqBlock *newEv = NULL;
    while (!t->IsDestroy())
    {
        newEv = NULL;
        if(g_Queue->PopFront(newEv))
        {
            newEv->Release();
            ++g_curMsgConsume;
        }
    }
}

// mid:3840kqps, average:3837kqps
static void Generator2(KERNEL_NS::LibThreadPool *t)
{
    // 定时管理
    KERNEL_NS::SmartPtr<KERNEL_NS::TimerMgr, KERNEL_NS::AutoDelMethods::CustomDelete> timerMgr = KERNEL_NS::TimerMgr::New_TimerMgr();
    timerMgr.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<KERNEL_NS::TimerMgr *>(p);
        KERNEL_NS::TimerMgr::Delete_TimerMgr(ptr);
    });
    timerMgr->Launch(NULL);

    // 内存定时清理
    KERNEL_NS::SmartPtr<KERNEL_NS::TlsMemoryCleanerComp, KERNEL_NS::AutoDelMethods::CustomDelete> memoryCleaner = KERNEL_NS::TlsMemoryCleanerCompFactory::StaticCreate()->CastTo<KERNEL_NS::TlsMemoryCleanerComp>();
    memoryCleaner.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<KERNEL_NS::TlsMemoryCleanerComp *>(p);
        ptr->Release();
    });

    // 设置
    memoryCleaner->SetTimerMgr(timerMgr.AsSelf());

    // 启动内存清理
    do
    {
        auto err = memoryCleaner->Init();
        if(err != Status::Success)
        {
            CRYSTAL_TRACE("memory cleaner init fail err:%d", err);
            break;
        }

        err = memoryCleaner->Start();
        if(err != Status::Success)
        {
            CRYSTAL_TRACE("memory cleaner start fail err:%d", err);
            break;
        }
    } while (false);

    while (!t->IsDestroy())
    {
        g_Queue->PushBack(TestMqBlock::NewThreadLocal_TestMqBlock());
        ++g_curGenCount;

        timerMgr->Drive();
    }

    // 关闭
    memoryCleaner->WillClose();
    memoryCleaner->Close();
}

// mid:3661kqps, average:3655kqps
static void Consumer2(KERNEL_NS::LibThreadPool *t)
{
    TestMqBlock *newEv = NULL;
    while (!t->IsDestroy())
    {
        newEv = NULL;
        if(g_Queue->PopFront(newEv))
        {
            newEv->Release();
            ++g_curMsgConsume;
        }
    }
}


// mid:3602kqps, average:3603kqps
static void Generator3(KERNEL_NS::LibThreadPool *t)
{
    while (!t->IsDestroy())
    {
        g_Queue->PushBack(TestMqBlock::NewThreadLocal_TestMqBlock());
        ++g_curGenCount;
    }
}

// mid:3432kqps, average:3426kqps
static void Consumer3(KERNEL_NS::LibThreadPool *t)
{
    TestMqBlock *newEv = NULL;
    while (!t->IsDestroy())
    {
        newEv = NULL;
        if(g_Queue->PopFront(newEv))
        {
            newEv->Release();
            ++g_curMsgConsume;
        }
    }
}


// mid:5358kqps, average:5364kqps
static void Generator4(KERNEL_NS::LibThreadPool *t)
{
    while (!t->IsDestroy())
    {
        g_Queue->PushBack(TestMqBlock::NewThreadLocal_TestMqBlock());
        ++g_curGenCount;
    }
}

// mid:4654kqps, average:4633kqps
static void Consumer4(KERNEL_NS::LibThreadPool *t)
{
    KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *lis = KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT>::New_LibList();
    TestMqBlock *newEv = NULL;
    while (!t->IsDestroy())
    {
        newEv = NULL;
        g_Queue->SwapQueue(lis);
        for(auto iter = lis->Begin(); iter;)
        {
            auto data = iter->_data;
            data->Release();
            iter = lis->Erase(iter);
            ++g_curMsgConsume;
        }
    }
}

/// 最优解
// mid:5278kqps, average:5296kqps
static void Generator5(KERNEL_NS::LibThreadPool *t)
{
    // 定时管理
    KERNEL_NS::SmartPtr<KERNEL_NS::TimerMgr, KERNEL_NS::AutoDelMethods::CustomDelete> timerMgr = KERNEL_NS::TimerMgr::New_TimerMgr();
    timerMgr.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<KERNEL_NS::TimerMgr *>(p);
        KERNEL_NS::TimerMgr::Delete_TimerMgr(ptr);
    });
    timerMgr->Launch(NULL);

    // 内存定时清理
    KERNEL_NS::SmartPtr<KERNEL_NS::TlsMemoryCleanerComp, KERNEL_NS::AutoDelMethods::CustomDelete> memoryCleaner = KERNEL_NS::TlsMemoryCleanerCompFactory::StaticCreate()->CastTo<KERNEL_NS::TlsMemoryCleanerComp>();
    memoryCleaner.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<KERNEL_NS::TlsMemoryCleanerComp *>(p);
        ptr->Release();
    });

    // 设置
    memoryCleaner->SetTimerMgr(timerMgr.AsSelf());

    // 启动内存清理
    do
    {
        auto err = memoryCleaner->Init();
        if(err != Status::Success)
        {
            CRYSTAL_TRACE("memory cleaner init fail err:%d", err);
            break;
        }

        err = memoryCleaner->Start();
        if(err != Status::Success)
        {
            CRYSTAL_TRACE("memory cleaner start fail err:%d", err);
            break;
        }
    } while (false);

    while (!t->IsDestroy())
    {
        g_Queue->PushBack(TestMqBlock::NewThreadLocal_TestMqBlock());
        ++g_curGenCount;

        timerMgr->Drive();
    }

    memoryCleaner->WillClose();
    memoryCleaner->Close();
}

/// 最优解
// mid:4804kqps, average:4796kqps
static void Consumer5(KERNEL_NS::LibThreadPool *t)
{
    KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *lis = KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT>::New_LibList();
    TestMqBlock *newEv = NULL;
    while (!t->IsDestroy())
    {
        newEv = NULL;
        g_Queue->SwapQueue(lis);
        for(auto iter = lis->Begin(); iter;)
        {
            auto data = iter->_data;
            data->Release();
            iter = lis->Erase(iter);
            ++g_curMsgConsume;
        }
    }
}


// >= 1000w qps
static void Generator6(KERNEL_NS::LibThreadPool *t)
{
 // 定时管理
    KERNEL_NS::SmartPtr<KERNEL_NS::TimerMgr, KERNEL_NS::AutoDelMethods::CustomDelete> timerMgr = KERNEL_NS::TimerMgr::New_TimerMgr();
    timerMgr.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<KERNEL_NS::TimerMgr *>(p);
        KERNEL_NS::TimerMgr::Delete_TimerMgr(ptr);
    });
    timerMgr->Launch(NULL);

    // 内存定时清理
    KERNEL_NS::SmartPtr<KERNEL_NS::TlsMemoryCleanerComp, KERNEL_NS::AutoDelMethods::CustomDelete> memoryCleaner = KERNEL_NS::TlsMemoryCleanerCompFactory::StaticCreate()->CastTo<KERNEL_NS::TlsMemoryCleanerComp>();
    memoryCleaner.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<KERNEL_NS::TlsMemoryCleanerComp *>(p);
        ptr->Release();
    });

    // 设置
    memoryCleaner->SetTimerMgr(timerMgr.AsSelf());

    auto timerMgrSelf = timerMgr.AsSelf();

    // 启动内存清理
    do
    {
        auto err = memoryCleaner->Init();
        if(err != Status::Success)
        {
            CRYSTAL_TRACE("memory cleaner init fail err:%d", err);
            break;
        }

        err = memoryCleaner->Start();
        if(err != Status::Success)
        {
            CRYSTAL_TRACE("memory cleaner start fail err:%d", err);
            break;
        }
    } while (false);

    KERNEL_NS::LibCpuCounter startCounter;
    KERNEL_NS::LibCpuCounter startFrame;
    KERNEL_NS::LibCpuCounter endCounter;
    KERNEL_NS::LibCpuCounter endFrame;
    KERNEL_NS::LibCpuCounter newCounterStart;
    KERNEL_NS::LibCpuCounter newCounterEnd;

    KERNEL_NS::LibCpuCounter pushCounterStart;
    KERNEL_NS::LibCpuCounter pushCounterEnd;

    while (!t->IsDestroy())
    {
        // startFrame.Update();
        // newCounterStart.Update();
        auto newEv = TestMqBlock::New_TestMqBlock();
        // g_MemoryAllocTime += newCounterEnd.Update().ElapseNanoseconds(newCounterStart);
        // ++g_MemoryAllocCount;

        // pushCounterStart.Update();
        g_Queue->PushBack(newEv);
        // g_pushTime.fetch_add(pushCounterEnd.Update().ElapseNanoseconds(pushCounterStart), std::memory_order_release);
        // g_pushTimeCount.fetch_add(1, std::memory_order_release);
        // pushCounterEnd.Update().ElapseNanoseconds(pushCounterStart);

        ++g_curGenCount;

        startCounter.Update();
        timerMgr->Drive();
        g_TimerDriveTime += endCounter.Update().ElapseNanoseconds(startCounter);
        ++g_TimerDriveCount;

        // g_FrameTime += endFrame.Update().ElapseNanoseconds(startFrame);
        // ++g_FrameCount;
    }

    memoryCleaner->WillClose();
    memoryCleaner->Close();
}

// mid:4204kqps, average:4213kqps
static void Consumer6(KERNEL_NS::LibThreadPool *t)
{
    KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *lis = KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT>::New_LibList();
    TestMqBlock *newEv = NULL;

    KERNEL_NS::LibCpuCounter swapCounterStart;
    KERNEL_NS::LibCpuCounter swapCounterEnd;

    while (!t->IsDestroy())
    {
        newEv = NULL;
        swapCounterStart.Update();
        // 在双核平台上由于冲突严重SwapQueue会达到100ns以上, 导致内存暴涨（消费速度<生产速度）
        g_Queue->SwapQueue(lis);
        g_swapTime += swapCounterEnd.Update().ElapseNanoseconds(swapCounterStart);
        ++g_swapTimeCount;

        for(auto iter = lis->Begin(); iter;)
        {
            auto data = iter->_data;
            data->Release();
            iter = lis->Erase(iter);
            ++g_curMsgConsume;
        }
    }
}

static void MonitorTask(KERNEL_NS::LibThreadPool *t)
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMessageQueue, "MonitorTask"));

    KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void>, KERNEL_NS::AutoDelMethods::Release> memoryMonitorWork = KERNEL_NS::MemoryMonitor::GetInstance()->MakeWorkTask();

    while (!t->IsDestroy())
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        const Int64 genNum = g_curGenCount;
        const Int64 comsumNum = g_curMsgConsume;
        g_curGenCount -= genNum;
        g_curMsgConsume -= comsumNum;

        const UInt64 totalDriveTime = g_TimerDriveTime;
        const UInt64 totalDriveCount = g_TimerDriveCount;
        g_TimerDriveTime -= totalDriveTime;
        g_TimerDriveCount -= totalDriveCount;
        const UInt64 driveTimeAverage = (totalDriveCount > 0) ? (totalDriveTime/totalDriveCount) : 0;
        
        const UInt64 frameTotalTime = g_FrameTime;
        const UInt64 frameTotalCount = g_FrameCount;
        g_FrameTime -= frameTotalTime;
        g_FrameCount -= frameTotalCount;
        const UInt64 frameTimeAverage = (frameTotalCount > 0) ? (frameTotalTime/frameTotalCount) : 0;

        const UInt64 allocTotalTime = g_MemoryAllocTime;
        const UInt64 allocTotalCount = g_MemoryAllocCount;
        g_MemoryAllocTime -= allocTotalTime;
        g_MemoryAllocCount -= allocTotalCount;
        const UInt64 allcAverage = (allocTotalCount > 0) ? (allocTotalTime/allocTotalCount) : 0;

        const UInt64 pushTotalTime = g_pushTime.load(std::memory_order_acquire);
        const UInt64 pushTotalCount = g_pushTimeCount.load(std::memory_order_acquire);
        g_pushTime.fetch_sub(pushTotalTime, std::memory_order_release);
        g_pushTimeCount.fetch_sub(pushTotalCount, std::memory_order_relaxed);
        const UInt64 pushAverage = (pushTotalCount>0) ? (pushTotalTime/pushTotalCount) : 0;

        const UInt64 swapTotalTime = g_swapTime;
        const UInt64 swapTotalCount = g_swapTimeCount;
        g_swapTime -= swapTotalTime;
        g_swapTimeCount -= swapTotalCount;
        const UInt64 swapAverage = (swapTotalCount>0) ? (swapTotalTime/swapTotalCount) : 0;

        const Int64 backlogNum = static_cast<Int64>(g_Queue->GetAmount());
        g_Log->Custom("Monitor:[gen:%lld, consum:%lld, backlog:%lld], driveTimeAverage:%llu ns, frameTimeAverage:%llu ns, allcAverage:%llu ns, pushAverage:%llu ns, swapAverage:%llu ns"
        , genNum, comsumNum, backlogNum, driveTimeAverage, frameTimeAverage, allcAverage, pushAverage, swapAverage);

        if(KERNEL_NS::SignalHandleUtil::ExchangeSignoTriggerFlag(KERNEL_NS::SignoList::MEMORY_LOG_SIGNO, false))
            memoryMonitorWork->Invoke();
    }
}

void TestMessageQueue::Run() 
{
  // 忽略内存日志信号(保证收到信号的时候不会退出)
    KERNEL_NS::SignalHandleUtil::SetSignoIgnore(KERNEL_NS::SignoList::MEMORY_LOG_SIGNO);
    g_Queue = KERNEL_NS::MessageQueue<KERNEL_NS::PollerEvent *>::New_MessageQueue();

    KERNEL_NS::LibThreadPool *pool = new KERNEL_NS::LibThreadPool;
    pool->Init(0, TEST_MQ_CONSUMER_COUNT + TEST_MQ_MAX_CHANNEL + 1);

    const Int32 count = TEST_MQ_MAX_CHANNEL;
    for(Int32 idx = 1; idx <= count; ++idx)
        pool->AddTask(&Generator6);

    // pool->AddTask2(ComsumerTask2, NULL, false, 0);
    // pool->AddTask2(ComsumerTask, NULL, false, 0);
    pool->AddTask(&Consumer6);
    pool->AddTask(&MonitorTask);

    pool->Start(true, TEST_MQ_CONSUMER_COUNT + TEST_MQ_MAX_CHANNEL + 1);

    getchar();

    if(pool->HalfClose())
        pool->FinishClose();

    KERNEL_NS::MessageQueue<KERNEL_NS::PollerEvent *>::Delete_MessageQueue(g_Queue);
    g_Queue = NULL;

    delete pool;
}
