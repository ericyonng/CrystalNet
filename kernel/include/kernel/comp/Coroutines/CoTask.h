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
 * 注意parent如果提前唤醒, 一定要注意child协程的释放
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
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Coroutines/CoTaskParam.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/Timer/LibTimer.h>
#include <kernel/comp/LibTime.h>

KERNEL_BEGIN

struct KERNEL_EXPORT NoWaitAtInitialSuspend {};
constexpr NoWaitAtInitialSuspend no_wait_at_initial_suspend;

template<typename R = void>
struct CoTask : NonCopyable
{
public:
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    explicit CoTask(coro_handle h) noexcept
    : _handle(h)
    ,_disableSuspend(false) 
    ,_params(CoTaskParam::NewThreadLocal_CoTaskParam())
    {
    }

    CoTask(coro_handle h, SmartPtr<CoTaskParam, AutoDelMethods::Release> &param) noexcept
    : _handle(h)
    ,_disableSuspend(false) 
    ,_params(param)
    {
    }

    CoTask(CoTask&& t) noexcept
        : _handle(std::exchange(t._handle, {}))
        ,_disableSuspend(std::exchange(t._disableSuspend, false))
        ,_params(std::move(t._params))
        {

        }

    CoTask& operator=(CoTask&& other)
    {
        Destroy();

        _handle = std::exchange(other._handle, {});
        _disableSuspend = std::exchange(other._disableSuspend, false);
        _params = std::move(other._params);
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
        void await_suspend(std::coroutine_handle<Promise> caller) const noexcept 
        {
            ASSERT(! _selfCoro.promise()._parent);

            // resumer为co_await CoTask时候创建的子协程句柄
            caller.promise().SetState(KernelHandle::SUSPEND);
            _selfCoro.promise()._parent = &caller.promise();
            caller.promise()._child = &_selfCoro.promise();

            auto selfCoHandleId = _selfCoro.promise().GetHandleId();

            // 超时唤醒并销毁协程
            auto selfParam = _selfCoro.promise().GetParam();
            if(selfParam->_endTime)
            {
                auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
                timer->SetTimeOutHandler([selfCoHandleId](KERNEL_NS::LibTimer *t) mutable 
                {
                    t->Cancel();

                    // 给子协程设置错误码
                    auto selfHandle = reinterpret_cast<CoHandle *>(KERNEL_NS::TlsUtil::GetTlsCoDict()->GetHandle(selfCoHandleId));
                    if(UNLIKELY(!selfHandle))
                        return;
                    
                    auto child = selfHandle->GetChild();
                    if(child && child->GetErrCode() == Status::Success)
                    {
                        child->SetErrCode(Status::CoTaskTimeout);
                    }

                    // 超时还没完成, 强制唤醒
                    if(!selfHandle->IsDone())
                    {
                        selfHandle->GetParam()->_errCode = Status::CoTaskTimeout;
                        selfHandle->DumpBacktrace();
                        selfHandle->ForceAwake();
                    }
                });
                auto &&now = KERNEL_NS::LibTime::Now();
                timer->Schedule(now > selfParam->_endTime ? KERNEL_NS::TimeSlice::FromSeconds(0) : (selfParam->_endTime - now));
                selfParam->_timeout = timer;
            }
            
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
        SmartPtr<CoTaskParam, AutoDelMethods::Release> _params;
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

        return Awaiter {_handle, _disableSuspend, _params};
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

        return Awaiter {_handle, _disableSuspend, _params};
    }

    struct promise_type: CoHandle, CoResult<R> 
    {
        promise_type()
        {
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();
            _params->_handle = this;
        }

        template<typename... Args> // from free function
        promise_type(NoWaitAtInitialSuspend, Args&&...): _wait_at_initial_suspend{false} 
        {
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();
            _params->_handle = this;
        }

        template<typename Obj, typename... Args> // from member function
        promise_type(Obj&&, NoWaitAtInitialSuspend, Args&&...): _wait_at_initial_suspend{false} 
        {
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();
            _params->_handle = this;
        }

        ~promise_type()
        {
            Cancel();

            if(LIKELY(_params))
                _params->_handle = NULL;
        }

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
                // 当前h结束了, 那么接着下一个(其实就是包裹着h的那个协程)
                if (auto cont = h.promise()._parent) 
                {
                    cont->PopChild();
                    // TODO:cout资源释放的问题需要考虑
                    cont->SetState(KERNEL_NS::KernelHandle::SCHEDULED);
                    auto handleId = cont->GetHandleId();
                    KERNEL_NS::PostAsyncTask([handleId]()mutable 
                    {
                        auto handle = KERNEL_NS::TlsUtil::GetTlsCoDict()->GetHandle(handleId);
                        if(UNLIKELY(!handle))
                            return;
                        
                        handle->Run(KERNEL_NS::KernelHandle::UNSCHEDULED);
                    });
                }
                else
                {
                    // co task已经销毁了, 但是协程句柄没销毁,异步销毁
                    if(h.done() && h.promise().CanSelfDestroy())
                    {
                        KERNEL_NS::PostAsyncTask([h]()
                        {
                            if(h.done())
                                h.destroy();
                        });
                    }
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
            if(UNLIKELY(!_params))
            {
                _params = KERNEL_NS::CoTaskParam::NewThreadLocal_CoTaskParam();
            }

            return CoTask{coro_handle::from_promise(*this), _params};
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
            _params->_errCode = Status::CoTaskException;
            
            CoResult<R>::unhandled_exception();
            
            // 打印堆栈出来
            DumpBacktrace();

            if(UNLIKELY(CoResult<R>::HasError()))
                ThrowErrorIfExists();
        }

        virtual CoHandle *GetParent() override { return _parent; }
        virtual CoHandle *GetChild() override { return _child; }

        virtual void DestroyHandle() override
        {
            // 有子协程, 子协程也需要释放
            if(_child)
            {
                _child->DestroyHandle();
                PopChild();
            }
            
            Cancel();
            coro_handle::from_promise(*this).destroy();
        }

        virtual void OnGetResult() override
        {
            if(_params->_timeout)
                _params->_timeout->Cancel();
        }
        virtual void PopChild() override
        {
            _child = NULL;
        }

        virtual void ThrowErrorIfExists() override 
        {
            // 有异常重新抛出
            auto &&exception = CoResult<R>::GetException();

            // 先销毁协程
            auto coPre = this->_parent;
            coro_handle::from_promise(*this).destroy();
            do
            {
                if(!coPre)
                    break;
                
                auto parent = coPre->GetParent();
                coPre->DestroyHandle();
                coPre = parent;
                /* code */
            } while (coPre);

            // 再抛异常
            std::rethrow_exception(exception);
        }

        virtual std::coroutine_handle<> GetCoHandle() final
        {
            return coro_handle::from_promise(*this);
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

            // 说明外部要手动唤醒
            coro_handle::from_promise(*this).resume();
        }
        
        virtual void ForceAwake() final
        {
            // 被cancel了, 不恢复协程
            if(_state != KERNEL_NS::KernelHandle::UNSCHEDULED)
                SetState(KERNEL_NS::KernelHandle::UNSCHEDULED);

            // 唤醒
            auto handle = coro_handle::from_promise(*this);
            if(!handle.done())
                handle.resume();
        }

        virtual bool IsDone() const override
        {
            auto handle = coro_handle::from_promise(*const_cast<promise_type *>(this));
            return handle.done();
        }

        virtual void ForceDestroyCo() final
        {
            // 与parent断开
            if(_parent)
                _parent->PopChild();
            _parent = NULL;
            
            // 有子协程, 子协程也要释放
            if(_child)
            {
                _child->ForceDestroyCo();
                PopChild();
            }
            
            auto handle = coro_handle::from_promise(*this);
            if(!handle.done())
                handle.destroy();
        }

        const std::source_location& _GetFrameInfo() const final { return _frameInfo; }

        void GetBacktrace(KERNEL_NS::LibString &content, Int32 depth = 0) const override final 
        {
            CoHandle::GetBacktrace(content, depth);

            if (_parent) 
                _parent->GetBacktrace(content, depth + 1);
        }
        
        void DumpBacktrace(Int32 depth = 0) const override final 
        {
            KERNEL_NS::LibString content;
            if(_params)
                content.AppendFormat("CoTask ErrCode:%d\n", _params->_errCode);
            
            CoHandle::DumpBacktrace(depth, content);

            if (_parent) 
                _parent->DumpBacktrace(depth + 1, content);
            else
                DumpBacktraceFinish(content);
        }

        void DumpBacktrace(Int32 depth, KERNEL_NS::LibString &content) const override final 
        {
            CoHandle::DumpBacktrace(depth, content);

            if (_parent) 
                _parent->DumpBacktrace(depth + 1, content);
            else
                DumpBacktraceFinish(content);
        }

        SmartPtr<CoTaskParam, AutoDelMethods::Release> &GetParam() override
        {
            return _params;
        }

        const SmartPtr<CoTaskParam, AutoDelMethods::Release> &GetParam() const override
        {
            return _params;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

        const bool _wait_at_initial_suspend {true};
        // 上一级的协程
        CoHandle* _parent {};
        // 子级协程
        CoHandle *_child {};
        std::source_location _frameInfo{};
        SmartPtr<CoTaskParam, AutoDelMethods::Release> _params;
    };

    bool Valid() const { return _handle != NULL; }
    bool Done() const { return _handle.done(); }

    struct AwaiterYield 
    {
        constexpr bool await_ready() { return false;}

        template<typename Promise>
        void await_suspend(std::coroutine_handle<Promise> resumer) const noexcept 
        {
            auto handleId = resumer.promise().GetHandleId();
            KERNEL_NS::PostAsyncTask([handleId]() 
            {
                auto handle = KERNEL_NS::TlsUtil::GetTlsCoDict()->GetHandle(handleId);
                if(UNLIKELY(!handle))
                    return;

                handle->ForceAwake();
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

    // 当前的CoTask不挂起直接进入协程体执行
    CoTask<R> &SetDisableSuspend(bool disableSuspend = true)
    {
        _disableSuspend = disableSuspend;

        return *this;
    }

    bool IsDisableSuspend() const { return _disableSuspend; }

    // 外部获取协程的一些状态
    CoTask<R> &GetParam(SmartPtr<CoTaskParam, AutoDelMethods::Release> &param)
    {
        if(UNLIKELY(!_params))
        {
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();
        }

        param = _params;

        return *this;
    }

    CoTask<R> &GetParam(KERNEL_NS::SmartPtr<KERNEL_NS::TaskParamRefWrapper, KERNEL_NS::AutoDelMethods::Release> &paramRef)
    {
        if(UNLIKELY(!_params))
        {
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();
        }

        paramRef->_params = _params;

        return *this;
    }

    CoTask<R>& SetTimeout(const TimeSlice &slice)
    {
        if(UNLIKELY(!_params))
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();

        _params->_endTime = KERNEL_NS::LibTime::Now() + slice;

        return *this;
    }

private:
    void Destroy() 
    {
        // 协程必须真正结束才能销毁
        if (auto handle = std::exchange(_handle, {}))
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
    mutable SmartPtr<CoTaskParam, AutoDelMethods::Release> _params;
};

static_assert(Promise<CoTask<>::promise_type>);
static_assert(Future<CoTask<>>);

KERNEL_END

#endif