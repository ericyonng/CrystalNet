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
 * Date: 2023-03-24 10:00:00
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <testsuit/testinst/TestSimpleApi.h>

static const Int32 TestCount = 1000000;
void TestSimpleApi::Run()
{
    // auto err = SimpleApiInit(false);
    // if(err != Status::Success)
    // {
    //     g_Log->Error(LOGFMT_NON_OBJ_TAG(TestSimpleApi, "init simple api fail err:%d"), err);
    //     return;
    // }

    // {
    //     auto cpuCountBegin = KERNEL_NS::LibCpuCounter::Current();
    //     for(Int32 idx = 0; idx < TestCount; ++idx)
    //     {
    //         const auto &nowTime = KERNEL_NS::LibTime::Now();
    //         SimpleApiPushProfile(0, nowTime.GetMilliTimestamp(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    //     }
    //     auto cpuCountEnd = KERNEL_NS::LibCpuCounter::Current();
    //     auto slice = (cpuCountEnd - cpuCountBegin);
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSimpleApi, "SimpleApiPushProfile cost total time:%llu(microseconds), cost time:%llu(microseconds) per invoke")
    //                 , slice.GetTotalNanoseconds(), (slice.GetTotalNanoseconds() / TestCount));
    // }

    // {
    //     auto cpuCountBegin = KERNEL_NS::LibCpuCounter::Current();
    //     for(Int32 idx = 0; idx < TestCount; ++idx)
    //     {
    //         const auto &nowTime = KERNEL_NS::LibTime::Now();
    //         SimpleApiPushProfile2(0, nowTime.GetMilliTimestamp(), 0, 0, 0, 0, 0, 0, 0, 0);
    //     }
    //     auto cpuCountEnd = KERNEL_NS::LibCpuCounter::Current();
    //     auto slice = (cpuCountEnd - cpuCountBegin);
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSimpleApi, "SimpleApiPushProfile2 cost total time:%llu(microseconds), cost time:%llu(microseconds) per invoke")
    //                 , slice.GetTotalNanoseconds(), (slice.GetTotalNanoseconds() / TestCount));
    // }

    // {
    //     auto cpuCountBegin = KERNEL_NS::LibCpuCounter::Current();
    //     for(Int32 idx = 0; idx < TestCount; ++idx)
    //     {
    //         const auto &nowTime = KERNEL_NS::LibTime::Now();
    //         SimpleApiPushProfile3(0, nowTime.GetMilliTimestamp(), 0, 0, 0, 0, 0, 0, 0);
    //     }
    //     auto cpuCountEnd = KERNEL_NS::LibCpuCounter::Current();
    //     auto slice = (cpuCountEnd - cpuCountBegin);
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSimpleApi, "SimpleApiPushProfile3 cost total time:%llu(microseconds), cost time:%llu(microseconds) per invoke")
    //                 , slice.GetTotalNanoseconds(), (slice.GetTotalNanoseconds() / TestCount));
    // }

    // KERNEL_NS::SystemUtil::ThreadSleep(2000);

    // SimpleApiDestroy();
}