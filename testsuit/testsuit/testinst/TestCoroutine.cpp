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
    co_return KERNEL_NS::LibString().AppendFormat("hello world");

    // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "GetContent start 1"));
    // co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(10)).SetDisableSuspend();
    // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "GetContent start 2"));
    // co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(10));
    // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "GetContent start 3"));
    //
    // throw std::runtime_error("GetContent error");
    // co_return KERNEL_NS::LibString().AppendFormat("hello world");
}

KERNEL_NS::CoTask<> test_hello_world2() 
{
    auto content = co_await GetContent();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "GetContent: %s"), content.c_str());
}

KERNEL_NS::CoTask<KERNEL_NS::LibString> TestCursionGetContent3() 
{
    // co_await KERNEL_NS::Waiting().SetDisableSuspend(true);
    auto content = co_await GetContent();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "TestCursionGetContent3: %s"), content.c_str());

    co_return content;
}

KERNEL_NS::CoTask<KERNEL_NS::LibString> TestCursionGetContent2() 
{
    KERNEL_NS::SmartPtr<KERNEL_NS::CoTaskParam, KERNEL_NS::AutoDelMethods::Release> param;
    auto content = co_await TestCursionGetContent3().GetParam(param);

    if(param->_errCode != Status::Success)
    {
        if(param->_handle)
        {
            param->_handle->DestroyHandle(param->_errCode);
        }
    }
    
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "TestCursionGetContent2: %s"), content.c_str());

    co_return content;
}

KERNEL_NS::CoTask<KERNEL_NS::LibString> TestCursionGetContent() 
{
    auto currentCoParam = KERNEL_NS::CoTaskParam::GetCurrentCoParam();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "currentCoParam:%p"), currentCoParam);
    
    KERNEL_NS::SmartPtr<KERNEL_NS::CoTaskParam, KERNEL_NS::AutoDelMethods::Release> param;
    auto content = co_await TestCursionGetContent2().SetTimeout(KERNEL_NS::TimeSlice::FromSeconds(3600)).GetParam(param);

    currentCoParam = KERNEL_NS::CoTaskParam::GetCurrentCoParam();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "currentCoParam:%p"), currentCoParam);
    
    if(param->_errCode == Status::Success)
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "TestCursionGetContent: %s"), content.c_str());
        co_return content;
    }


    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "TestCursionGetContent fail"));

    co_return "";
}

KERNEL_NS::CoTask<> TestCursionTask()
{
    auto currentCoParam = KERNEL_NS::CoTaskParam::GetCurrentCoParam();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "currentCoParam:%p"), currentCoParam);
    co_await TestCursionGetContent();
    currentCoParam = KERNEL_NS::CoTaskParam::GetCurrentCoParam();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "currentCoParam:%p"), currentCoParam);
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

    KERNEL_NS::RunRightNow([]()->KERNEL_NS::CoTask<>
    {
        co_await TestCursionTask();

        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "run right now..."));
    });
    
    // 调用 hello_world 的时候, 会返回一个协程, 并抛给调度器去继续执行
    // KERNEL_NS::PostCaller([]() -> KERNEL_NS::CoTask<> 
    // {
    //     auto currentCoParam = KERNEL_NS::CoTaskParam::GetCurrentCoParam();                                                                                                                                                                                     
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "currentCoParam:%p"), currentCoParam);
    //
    //     co_await TestCursionTask().SetDisableSuspend(true);
    //     currentCoParam = KERNEL_NS::CoTaskParam::GetCurrentCoParam();
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "currentCoParam:%p"), currentCoParam);
    //     co_await KERNEL_NS::Waiting(KERNEL_NS::TimeSlice::FromSeconds(10));
    //     currentCoParam = KERNEL_NS::CoTaskParam::GetCurrentCoParam();
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "currentCoParam:%p"), currentCoParam);
    //
    //     // co_await test_hello_world2().SetDisableSuspend(true);
    //
    //     // KERNEL_NS::SmartPtr<KERNEL_NS::TaskParamRefWrapper, KERNEL_NS::AutoDelMethods::Release> params = KERNEL_NS::TaskParamRefWrapper::NewThreadLocal_TaskParamRefWrapper();
    //     // co_await KERNEL_NS::Waiting().SetDisableSuspend(true).SetTimeout(KERNEL_NS::TimeSlice::FromSeconds(10)).GetParam(params);
    //     // g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestCoroutine, "co time out errCode:%d"), params->_params->_errCode);
    // });

    poller->SafeEventLoop();

    poller->OnLoopEnd();
}
