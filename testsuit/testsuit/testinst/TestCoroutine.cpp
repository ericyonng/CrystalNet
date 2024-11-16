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
 * Date: 2024-02-28 16:45:21
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include "TestCoroutine.h"
#include <unordered_set>
#include <iostream>
// #include <format>
#include <functional>

#pragma region co

#include <coroutine>

// 协程打印堆栈
#include <source_location>

#include <exception>
#include <variant>
#include <optional>

#include "Comps/Test/Impl/TestMgr.h"
#include "service/common/macro.h"

#pragma endregion

KERNEL_NS::CoTask<std::string> hello() {
    co_return "hello";
}

KERNEL_NS::CoTask<std::string> world() {
    co_return "world";
}

KERNEL_NS::CoTask<KERNEL_NS::LibString> hello_world() {
    co_return KERNEL_NS::LibString().AppendFormat("%s %s", (co_await hello()).c_str(), (co_await world()).c_str());
}

KERNEL_NS::CoTask<KERNEL_NS::LibString> test_hello_world() 
{
    co_return KERNEL_NS::LibString().AppendFormat("hello world");
}

KERNEL_NS::CoTask<KERNEL_NS::LibString> GetContent() 
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "GetContent start 1"));
    co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(10)).SetDisableSuspend();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "GetContent start 2"));
    co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(10));
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "GetContent start 3"));

    throw std::runtime_error("GetContent error");
    co_return KERNEL_NS::LibString().AppendFormat("hello world");
}

KERNEL_NS::CoTask<> test_hello_world2() 
{
    auto content = co_await GetContent().SetDisableSuspend(true);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "GetContent: %s"), content.c_str());
}

void TestCoroutine::Run()
{
    auto poller = KERNEL_NS::TlsUtil::GetPoller();

    poller->PrepareLoop();

    // auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    // timer->SetTimeOutHandler([](KERNEL_NS::LibTimer *t){
    //     testContent();
    // });
    // timer->Schedule(KERNEL_NS::TimeSlice::FromSeconds(1));

    // 调用 hello_world 的时候, 会返回一个协程, 并抛给调度器去继续执行
    KERNEL_NS::PostCaller([]() -> KERNEL_NS::CoTask<> 
    {
        // co_await test_hello_world2().SetDisableSuspend(true);

        KERNEL_NS::SmartPtr<KERNEL_NS::TaskParamRefWrapper, KERNEL_NS::AutoDelMethods::Release> params = KERNEL_NS::TaskParamRefWrapper::NewThreadLocal_TaskParamRefWrapper();
        co_await KERNEL_NS::Waiting().SetDisableSuspend(true).SetTimeout(KERNEL_NS::TimeSlice::FromSeconds(10)).GetParam(params);
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestCoroutine, "co time out errCode:%d"), params->_params->_errCode);
    });

    poller->SafeEventLoop();

    poller->OnLoopEnd();
}
