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
 * Date: 2024-10-21 01:31:10
 * Author: Eric Yonng
 * Description: 
*/


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COTASK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COTASK_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <coroutine>
#include <functional>
#include <exception>
#include <utility>

#include <kernel/common/NonCopyabale.h>
#include <kernel/comp/Coroutines/Exceptions.h>
#include <kernel/comp/Coroutines/CoHandle.h>
#include <kernel/comp/Coroutines/CoResult.h>
#include <kernel/comp/Coroutines/Concept/Future.h>
#include <kernel/comp/Coroutines/Concept/Promise.h>
#include <kernel/comp/Delegate/IDelegate.h>
#include <kernel/comp/Coroutines/CoTools.h>

KERNEL_BEGIN

struct KERNEL_EXPORT NoWaitAtInitialSuspend {};
constexpr NoWaitAtInitialSuspend no_wait_at_initial_suspend;

template<typename R = void>
struct CoTask : NonCopyable
{
public:
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    explicit CoTask(coro_handle h) noexcept: _handle(h),_disableSuspend(false) {}

    CoTask(CoTask&& t) noexcept
        : _handle(std::exchange(t._handle, {}))
        ,_disableSuspend(std::exchange(t._disableSuspend, false))
        {

        }

    CoTask& operator=(CoTask&& other)
    {
        Destroy();
        _handle = std::exchange(other._handle, {});
        _disableSuspend = std::exchange(other._disableSuspend, false);
        return *this;
    }


    ~CoTask() { Destroy(); }

    decltype(auto) GetResult() & 
    {
        return _handle.promise().GetResult();
    }

    decltype(auto) GetResult() && 
    {
        return std::move(_handle.promise()).GetResult();
    }

    struct AwaiterBase 
    {
        constexpr bool await_ready() 
        {
            if ( LIKELY(_selfCoro))
                return _selfCoro.done();

            return true;
        }

        template<typename Promise>
        void await_suspend(std::coroutine_handle<Promise> resumer) const noexcept 
        {
            ASSERT(! _selfCoro.promise()._continuation);

            // resumer为co_await CoTask时候创建的子协程句柄
            resumer.promise().SetState(KernelHandle::SUSPEND);
            _selfCoro.promise()._continuation = &resumer.promise();

            if(_disableSuspend)
            {
                _selfCoro.promise().SetState(KernelHandle::SCHEDULED);
                _selfCoro.promise().Run(KernelHandle::UNSCHEDULED);
                return;
            }

            _selfCoro.promise().Schedule();
        }

        coro_handle _selfCoro {};
        bool _disableSuspend = false;
    };
    
    auto operator co_await() const & noexcept 
    {
        struct Awaiter: AwaiterBase 
        {
            decltype(auto) await_resume() const 
            {
                if (UNLIKELY(!AwaiterBase::_selfCoro)) 
                    throw InvalidFuture{};

                return AwaiterBase::_selfCoro.promise().GetResult();
            }
        };

        return Awaiter {_handle, _disableSuspend};
    }

    auto operator co_await() const && noexcept 
    {
        struct Awaiter: AwaiterBase 
        {
            decltype(auto) await_resume() const 
            {
                if (UNLIKELY(! AwaiterBase::_selfCoro))
                    throw InvalidFuture{};

                return std::move(AwaiterBase::_selfCoro.promise()).GetResult();
            }
        };

        return Awaiter {_handle, _disableSuspend};
    }

    struct promise_type: CoHandle, CoResult<R> 
    {
        promise_type() = default;

        template<typename... Args> // from free function
        promise_type(NoWaitAtInitialSuspend, Args&&...): _wait_at_initial_suspend{false} { }

        template<typename Obj, typename... Args> // from member function
        promise_type(Obj&&, NoWaitAtInitialSuspend, Args&&...): _wait_at_initial_suspend{false} { }

        auto initial_suspend() noexcept 
        {
            struct InitialSuspendAwaiter 
            {
                constexpr bool await_ready() const noexcept { return !_wait_at_initial_suspend; }
                constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}
                constexpr void await_resume() const noexcept {}

                const bool _wait_at_initial_suspend{true};
            };
            return InitialSuspendAwaiter{_wait_at_initial_suspend};
        }

        struct FinalAwaiter 
        {
            constexpr bool await_ready() const noexcept { return false; }
            template<typename Promise>
            constexpr void await_suspend(std::coroutine_handle<Promise> h) const noexcept 
            {
                if (auto cont = h.promise()._continuation) 
                {
                    // TODO:cout资源释放的问题需要考虑
                    cont->SetState(KERNEL_NS::KernelHandle::SCHEDULED);
                    KERNEL_NS::PostAsyncTask([cont]()
                    {
                        cont->Run(KERNEL_NS::KernelHandle::UNSCHEDULED);
                    });
                }
            }
            constexpr void await_resume() const noexcept {}
        };
        auto final_suspend() noexcept 
        {
            return FinalAwaiter {};
        }

        CoTask get_return_object() noexcept 
        {
            // 扣接构造函数, 省去了传递模板参数, 编译期推导
            return CoTask{coro_handle::from_promise(*this)};
        }

        template<Awaitable A>
        decltype(auto) await_transform(A&& awaiter, // for save source_location info
                                       std::source_location loc = std::source_location::current()) 
        {
            _frameInfo = loc;
            return std::forward<A>(awaiter);
        }

        virtual void unhandled_exception() override
        { 
            CoResult<R>::unhandled_exception();

            // 打印堆栈出来
            DumpBacktrace();

            if(UNLIKELY(CoResult<R>::HasError()))
                ThrowErrorIfExists();
        }

        virtual CoHandle *GetContinuation() override { return _continuation; }

        virtual void DestroyHandle() override
        {
            Cancel();
            coro_handle::from_promise(*this).destroy();
        }

        virtual void ThrowErrorIfExists() override 
        {
            // 有异常重新抛出
            auto &&exception = CoResult<R>::GetException();

            // 先销毁协程
            auto coPre = this->_continuation;
            coro_handle::from_promise(*this).destroy();
            do
            {
                if(!coPre)
                    break;
                
                auto nextCo = coPre->GetContinuation();
                coPre->DestroyHandle();
                coPre = nextCo;
                /* code */
            } while (coPre);

            // 再抛异常
            std::rethrow_exception(exception);
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void Run(KERNEL_NS::KernelHandle::State changeState) final 
        {
            // 被cancel了, 不恢复协程
            if(_state == KERNEL_NS::KernelHandle::UNSCHEDULED)
            {
                SetState(changeState);
                return;
            }

            // 唤醒
            SetState(changeState);
            coro_handle::from_promise(*this).resume();
        }

        const std::source_location& _GetFrameInfo() const override final { return _frameInfo; }

        void DumpBacktrace(Int32 depth = 0, KERNEL_NS::LibString &&content = "") const override final 
        {
            CoHandle::DumpBacktrace(depth, content);

            if (_continuation) 
                _continuation->DumpBacktrace(depth + 1, content);
            else
                DumpBacktraceFinish(content);
        }

        void DumpBacktrace(Int32 depth, KERNEL_NS::LibString &content) const override final 
        {
            CoHandle::DumpBacktrace(depth, content);

            if (_continuation) 
                _continuation->DumpBacktrace(depth + 1, content);
            else
                DumpBacktraceFinish(content);
        }
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

        const bool _wait_at_initial_suspend {true};
        CoHandle* _continuation {};
        std::source_location _frameInfo{};
    };

    bool Valid() const { return _handle != NULL; }
    bool Done() const { return _handle.done(); }

    struct AwaiterYield 
    {
        constexpr bool await_ready() { return false;}

        template<typename Promise>
        void await_suspend(std::coroutine_handle<Promise> resumer) const noexcept 
        {
            KERNEL_NS::PostAsyncTask([res = std::move(resumer)]() 
            {
                res.resume();
            });
        }

        void await_resume() const {}

        struct AwaiterYieldFinalAwaiter 
        {
            constexpr bool await_ready() const noexcept { return false; }
            template<typename Promise>
            constexpr void await_suspend(std::coroutine_handle<Promise> h) const noexcept 
            {   
                // 协程结束了
                if(h.done() && h.promise().CanSelfDestroy())
                {
                    h.destroy();
                }
            }
            constexpr void await_resume() const noexcept {}
        };
        auto final_suspend() noexcept 
        {
            return AwaiterYieldFinalAwaiter {};
        }
    };

    // 切出当前调度
    static AwaiterYield CoYield() noexcept
    {
        return AwaiterYield{};
    }

    // ALWAYS_INLINE coro_handle &GetHandle()
    // {
    //     return _handle;
    // }

    // 为了能在PostRun 中使用，破例用mutable
    ALWAYS_INLINE coro_handle &GetHandle() const
    {
        return _handle;
    }

    CoTask &SetDisableSuspend(bool disableSuspend = true) const
    {
        _disableSuspend = disableSuspend;

        return *const_cast<CoTask *>(this);
    }

    bool IsDisableSuspend() const { return _disableSuspend; }

private:
    void Destroy() 
    {
        // 协程必须真正结束才能销毁
        if (auto handle = std::exchange(_handle, NULL))
        {
            auto &promise = handle.promise();
            promise.SetSelfDestory(true);
            if(handle.done())
            {
                promise.Cancel();
                handle.destroy();
            }
        }
    }
private:
    // 为了能在PostRun 中使用，破例用mutable
    mutable coro_handle _handle;
    mutable bool _disableSuspend = false;
};

static_assert(Promise<CoTask<>::promise_type>);
static_assert(Future<CoTask<>>);

KERNEL_END

#endif