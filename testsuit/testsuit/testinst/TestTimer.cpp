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
 * Author: Eric Yonng
 * Date: 2021-03-17 20:22:05
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestTimer.h>

#define TEST_TIMER_TIMER1_DESC "TIME1"
#define TEST_TIMER_TIMER3_DESC "TIME3"
#define TEST_TIMER_TIMER2_DESC "TIME2"

static Int32 g_countTimeOut1 = 0;
static void TimerOut1(KERNEL_NS::LibTimer *timer)
{
    g_Log->Custom("TimerOut1 %s 1 seconds", timer->ToString().c_str());
}

static void TimerOut2(KERNEL_NS::LibTimer *timer)
{
    static bool isSwitch = false;
    if(isSwitch)
    {
        g_Log->Custom("TimerOut2 10 seconds cur time=[%lld]", KERNEL_NS::TimeUtil::GetMicroTimestamp());
        const auto now = KERNEL_NS::LibTime::Now();
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestTimer, "TimerOut2 10 seconds cur time=[%lld] ms, [%lld] us, [%lld] ns, util ms:%lld")
        , now.GetMilliTimestamp(), now.GetMicroTimestamp(), now.GetNanoTimestamp()
        ,  KERNEL_NS::TimeUtil::GetMicroTimestamp());
    }
    else
    {
        g_Log->Custom("TimerOut2 5 seconds cur time=[%lld]", KERNEL_NS::TimeUtil::GetMicroTimestamp());
    }

    // 关闭定时器1
    if (g_countTimeOut1 < 20)
    {
        timer->GetParams()[TEST_TIMER_TIMER1_DESC].AsPtr<KERNEL_NS::LibTimer>()->Cancel();
        timer->GetParams()[TEST_TIMER_TIMER3_DESC].AsPtr<KERNEL_NS::LibTimer>()->Schedule(2000);

        timer->Schedule(10000);
        timer->Schedule(20000);
        timer->Cancel();
    }

    isSwitch = true;
}

static void TimerOut3(KERNEL_NS::LibTimer *timer)
{
    g_Log->Custom("TimerOut3 2 seconds cur time=[%lld]", KERNEL_NS::TimeUtil::GetMicroTimestamp());
    g_countTimeOut1++;
    if (g_countTimeOut1 > 20)
    {
        timer->Cancel();
        timer->GetParams()[TEST_TIMER_TIMER2_DESC].AsPtr<KERNEL_NS::LibTimer>()->Schedule(1000);
        g_Log->Custom("delete timer 3");
        CRYSTAL_DELETE_SAFE(timer);
    }
}

void TestTimer::Run() 
{
    // 时间管理器
    // const Int64 resolutionMicroSec = TIME_WHEEL_RESOLUTION_DEF * KERNEL_NS::TimeDefs::MICRO_SECOND_PER_MILLI_SECOND;
    KERNEL_NS::TimerMgr timerMgr;

    KERNEL_NS::LibTimer timer1(&timerMgr);
    KERNEL_NS::LibTimer timer2(&timerMgr);
    KERNEL_NS::LibTimer *timer3 = new KERNEL_NS::LibTimer(&timerMgr);

    timer1.SetTimeOutHandler(&TimerOut1);
    timer2.SetTimeOutHandler(&TimerOut2);
    timer3->SetTimeOutHandler(&TimerOut3);

    timer1.Schedule(1000);
    timer2.Schedule(5000);
    timer2.GetParams()[TEST_TIMER_TIMER1_DESC] = &timer1;
    timer2.GetParams()[TEST_TIMER_TIMER3_DESC] = timer3;
    timer3->GetParams()[TEST_TIMER_TIMER2_DESC] = &timer2;

    auto timer4 = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer(&timerMgr);
    timer4->GetMgr()->TakeOverLifeTime(timer4, [](KERNEL_NS::LibTimer *t){
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    });
    timer4->SetTimeOutHandler([](KERNEL_NS::LibTimer *t){
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestTimer, "timer4 timeout"));
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    });
    timer4->Schedule(10);
    timerMgr.TakeOverLifeTime(timer4, [](KERNEL_NS::LibTimer *t){
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestTimer, "timer4 delete"));
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    });
    
    // 扫描精度
    // const auto milliSec = resolutionMicroSec / KERNEL_NS::TimeDefs::MICRO_SECOND_PER_MILLI_SECOND;
    Int32 loop = 100;
    while (--loop >= 0)
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1);
        timerMgr.Drive();

        // if(--loop < 0)
        //     break;
    }

    timerMgr.Close();
    
}
