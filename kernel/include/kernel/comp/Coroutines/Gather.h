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
 * Date: 2024-10-26 20:13:10
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_GATHER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_GATHER_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/NonCopyabale.h>
#include <kernel/comp/Coroutines/VoldValue.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Coroutines/CoHandle.h>
#include <kernel/comp/Coroutines/Concept/Awaitable.h>
#include <kernel/comp/Coroutines/CoTask.h>

#include <stdexcept>
#include <tuple>
#include <variant>
#include <exception>
#include <utility>

KERNEL_BEGIN

template<typename... Rs>
class GatherAwaiter: NonCopyable 
{
    using ResultTypes = std::tuple<GetTypeIfVoidType<Rs>...>;

public:
    constexpr bool await_ready() noexcept { return _IsFinished(); }
    constexpr auto await_resume() const 
    {
        if (auto exception = std::get_if<std::exception_ptr>(&_result)) 
        {
            std::rethrow_exception(*exception);
        }

        if (auto res = std::get_if<ResultTypes>(&_result)) 
            return *res;

        throw std::runtime_error(KERNEL_NS::LibString().AppendFormat("result is unset ResultTypes:%s", typeid(ResultTypes).name()).GetRaw());
    }

    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> continuation) noexcept 
    {
        _continuation = &continuation.promise();

        // set continuation_ to SUSPEND, don't schedule anymore, until it resume continuation_
        _continuation->SetState(KernelHandle::SUSPEND);
    }

    template<Awaitable... Futs>
    explicit GatherAwaiter(Futs&&... futs)
    : GatherAwaiter( std::make_index_sequence<sizeof...(Futs)>{}
                   , std::forward<Futs>(futs)...) {}
private:
    template<Awaitable... Futs, size_t ...Is>
    explicit GatherAwaiter(std::index_sequence<Is...>, Futs&&... futs)
            : _tasks{ std::make_tuple(_CollectResult<Is>(no_wait_at_initial_suspend, std::forward<Futs>(futs))...) }
            { }

    template<size_t Idx, Awaitable Fut>
    CoTask<> _CollectResult(NoWaitAtInitialSuspend, Fut&& fut) 
    {
        try 
        {
            auto& results = std::get<ResultTypes>(_result);
            
            if constexpr (std::is_void_v<AwaitResult<Fut>>) 
            { 
                co_await std::forward<Fut>(fut); 
            }
            else 
            { 
                std::get<Idx>(results) = std::move(co_await std::forward<Fut>(fut)); 
            }

            ++_count;
        } 
        catch(...) 
        {
            _result = std::current_exception();
        }

        if (_IsFinished()) 
        {
            // TODO:调度
            _continuation->SetState(KERNEL_NS::KernelHandle::SCHEDULED);
            auto handleId = _continuation->GetHandleId();
            KERNEL_NS::PostAsyncTask([handleId](){
                auto handle = KERNEL_NS::TlsUtil::GetTlsCoDict()->GetHandle(handleId);
                if(UNLIKELY(!handle))
                    return;
                
                handle->Run(KERNEL_NS::KernelHandle::UNSCHEDULED);
            });
        }
    }
private:
    bool _IsFinished() 
    {
        return (_count == sizeof...(Rs)
                || std::get_if<std::exception_ptr>(&_result) != nullptr);
    }

private:
    std::variant<ResultTypes, std::exception_ptr> _result;
    std::tuple<CoTask<std::void_t<Rs>>...> _tasks;
    CoHandle* _continuation{};
    Int64 _count{0};
};

// 扣接构造函数, 利用移动构造自动推导出模板参数Futs
template<Awaitable... Futs> // C++17 deduction guide
GatherAwaiter(Futs&&...) -> GatherAwaiter<AwaitResult<Futs>...>;

template<Awaitable... Futs>
struct GatherAwaiterRepositry 
{
    explicit GatherAwaiterRepositry(Futs&&... futs)
    : _futs(std::forward<Futs>(futs)...) { }

    auto operator co_await() && 
    {
        // 将tuple的_futs展开给lambda等可调用对象
        return std::apply([]<Awaitable... F>(F&&... f) 
        {
            // 利用扣接移动构造, 来自动推导处模板类GatherAwaiter的模板参数类型
            return GatherAwaiter { std::forward<F>(f)... };
        }, std::move(_futs));
    }

private:
    // futs_ to lift Future's lifetime
    // 1. if Future is rvalue(Fut&&), then move it to tuple(Fut)
    // 2. if Future is xvalue(Fut&&), then move it to tuple(Fut)
    // 3. if Future is lvalue(Fut&), then store as lvalue-ref(Fut&)
    std::tuple<Futs...> _futs;
};

// 扣接构造函数, 利用移动构造自动推导出模板参数Futs
template<Awaitable... Futs> // need deduction guide to deduce future type
GatherAwaiterRepositry(Futs&&...) -> GatherAwaiterRepositry<Futs...>;

template<Awaitable... Futs>
auto Gather(NoWaitAtInitialSuspend, Futs&&... futs) // need NoWaitAtInitialSuspend to lift futures lifetime early
-> CoTask<std::tuple<GetTypeIfVoidType<AwaitResult<Futs>>...>> 
{ // lift awaitable type(GatherAwaiterRepositry) to coroutine
    // 利用移动构造推导模板参数类型
    co_return co_await GatherAwaiterRepositry { std::forward<Futs>(futs)... };
}

template<Awaitable... Futs>
// nodiscard表示不可忽略返回值, 编译期会提醒
[[nodiscard("discard gather doesn't make sense")]]
auto Gather(Futs&&... futs) 
{
    return Gather(no_wait_at_initial_suspend, std::forward<Futs>(futs)...);
}

KERNEL_END

#endif
