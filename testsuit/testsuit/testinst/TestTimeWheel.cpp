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
 * Date: 2024-01-02 10:58:00
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestTimeWheel.h>

static void Timout1(KERNEL_NS::TimeWheelTimer *t)
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestTimeWheel, "timer:%s"), t->ToString().c_str());
}

void TestTimeWheel::Run()
{
    {// 1级时间轮
        KERNEL_NS::SmartPtr<KERNEL_NS::TimerWheel, KERNEL_NS::AutoDelMethods::CustomDelete> timeWheel = KERNEL_NS::TimerWheel::NewThreadLocal_TimerWheel();
        timeWheel.SetClosureDelegate([](void *p){
            KERNEL_NS::TimerWheel::DeleteThreadLocal_TimerWheel(KERNEL_NS::KernelCastTo<KERNEL_NS::TimerWheel>(p));
        });

        KERNEL_NS::SmartPtr<KERNEL_NS::TimeWheelTimer, KERNEL_NS::AutoDelMethods::CustomDelete> timer = KERNEL_NS::TimeWheelTimer::NewThreadLocal_TimeWheelTimer(timeWheel.AsSelf());
        timer.SetClosureDelegate([](void *p){
            KERNEL_NS::TimeWheelTimer::DeleteThreadLocal_TimeWheelTimer(KERNEL_NS::KernelCastTo<KERNEL_NS::TimeWheelTimer>(p));
        });

        timeWheel->Init(2, 100);

        timer->SetTimeOutHandler(&Timout1);
        timer->Schedule(3000);

        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestTimeWheel, "Start test wheel"));
        while (true)
        {
            KERNEL_NS::SystemUtil::ThreadSleep(timeWheel->GetTickIntervalMs());
            timeWheel->Tick();
        }
        
    }

    {// 2级时间轮

    }

    {// 3级时间轮

    }

    {// 4级时间抡

    }

    {// 5级时间轮

    }
}