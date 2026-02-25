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
 * 当前协程CoParam需要考虑协程销毁, 协程结束的时候, 如果有父协程则设置父协程的CoParam
*/


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COTASK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COTASK_H__

#pragma once


#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#ifdef CRYSTAL_NET_CPP20
#include <coroutine>
#endif

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

#include "kernel/comp/LibTraceId.h"

#ifdef CRYSTAL_NET_CPP20

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
    ,_params(CoTaskParam::NewThreadLocal_CoTaskParam())
    {
    }

    CoTask(coro_handle h, SmartPtr<CoTaskParam, AutoDelMethods::Release> &param) noexcept
    : _handle(h)
    ,_params(param)
    {
    }

    CoTask(CoTask&& t) noexcept
        : _handle(std::exchange(t._handle, {}))
        ,_params(std::move(t._params))
        {

        }

    CoTask& operator=(CoTask&& other)
    {
        Destroy();

        _handle = std::exchange(other._handle, {});
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
            if (LIKELY(_selfCoro))
            {
                if(_selfCoro.done())
                {
                    // 当前h结束了, 那么接着下一个(其实就是包裹着h的那个协程)
                    if (auto cont = _selfCoro.promise()._parent) 
                    {
                        KERNEL_NS::CoTaskParam::SetCurrentCoParam(cont->GetParam().AsSelf());
                    }
                    else
                    {
                        KERNEL_NS::CoTaskParam::SetCurrentCoParam(NULL);
                    }
                    
                    return true;
                }

                return false;
            }

            return true;
        }

        template<typename Promise>
        void await_suspend(std::coroutine_handle<Promise> caller) const noexcept 
        {
            ASSERT(! _selfCoro.promise()._parent);

            // 协程挂起了
            CoTaskParam::SetCurrentCoParam(NULL);

            auto &selfPromise = _selfCoro.promise();
            auto &callerPromise = caller.promise();
            // resumer为co_await CoTask时候创建的子协程句柄
            callerPromise.SetState(KernelHandle::SUSPEND);
            selfPromise._parent = &caller.promise();
            callerPromise._child = &selfPromise;

            // trace继承
            auto selfParam = selfPromise.GetParam();
            *(selfParam->_trace) = *(callerPromise._params->_trace);

            auto selfCoHandleId = _selfCoro.promise().GetHandleId();

            // 超时唤醒并销毁协程
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
                        CoTaskParam::SetCurrentCoParam(selfHandle->GetParam());

                        auto printStack = selfHandle->GetParam()->_printStackIfTimeout;
                        selfHandle->GetParam()->_errCode = Status::CoTaskTimeout;
                        if(printStack)
                            selfHandle->DumpBacktrace();
                        selfHandle->ForceAwake();
                    }
                });
                auto &&now = KERNEL_NS::LibTime::Now();
                timer->Schedule(now > selfParam->_endTime ? KERNEL_NS::TimeSlice::FromSeconds(0) : (selfParam->_endTime - now));
                selfParam->_timeout = timer;
            }

            // 投递到poller中异步执行
            if(UNLIKELY(_params->_enableSuspend))
            {
                _selfCoro.promise().Schedule();
                return;
            }
            
            // 不挂起唤醒执行
            _selfCoro.promise().SetState(KernelHandle::SCHEDULED);
            _selfCoro.promise().Run(KernelHandle::UNSCHEDULED);

            // 投递到poller中异步执行
            // if(_enableSuspend)
            // {
            //     // 不挂起唤醒执行
            //     _selfCoro.promise().SetState(KernelHandle::SCHEDULED);
            //     _selfCoro.promise().Run(KernelHandle::UNSCHEDULED);
            //     
            //     return;
            // }
            //
            // _selfCoro.promise().Schedule();
        }

        coro_handle _selfCoro {};
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

                // 切换当前协程
                auto &promise = AwaiterBase::_selfCoro.promise();
                return promise.GetResult();
            }
        };

        return Awaiter {_handle, _params};
    }

    auto operator co_await() const && noexcept 
    {
        struct Awaiter: AwaiterBase 
        {
            decltype(auto) await_resume() const 
            {
                if (UNLIKELY(! AwaiterBase::_selfCoro))
                    throw InvalidFuture{};

                // 切换协程
                auto &promise = AwaiterBase::_selfCoro.promise();
                return std::move(promise).GetResult();
            }
        };

        return Awaiter {_handle, _params};
    }

    struct promise_type: CoHandle, CoResult<R> 
    {
        promise_type()
        {
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();
            _params->_handle = this;
            _params->_trace = LibTraceId::NewThreadLocal_LibTraceId();
        }

        template<typename... Args> // from free function
        promise_type(NoWaitAtInitialSuspend, Args&&...): _wait_at_initial_suspend{false} 
        {
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();
            _params->_handle = this;
            _params->_trace = LibTraceId::NewThreadLocal_LibTraceId();
        }

        template<typename Obj, typename... Args> // from member function
        promise_type(Obj&&, NoWaitAtInitialSuspend, Args&&...): _wait_at_initial_suspend{false} 
        {
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();
            _params->_handle = this;
            _params->_trace = LibTraceId::NewThreadLocal_LibTraceId();
        }

        ~promise_type()
        {
            if(_parent)
                _parent->PopChild();
            
            Cancel();

            if(LIKELY(_params))
            {
                if(_params.AsSelf() == CoTaskParam::GetCurrentCoParam())
                {
                    CoTaskParam::SetCurrentCoParam(NULL);
                }

                if (_params->_releaseSource)
                {
                    auto releaseSource = _params->_releaseSource;
                    _params->_releaseSource = NULL;
                    releaseSource->Invoke();
                    releaseSource->Release();
                }

                _params->_handle = NULL;
            }
        }

        auto initial_suspend() noexcept 
        {
            struct InitialSuspendAwaiter 
            {
                constexpr bool await_ready() const noexcept { return !_wait_at_initial_suspend; }
                constexpr void await_suspend(std::coroutine_handle<>) const noexcept
                {
                    CoTaskParam::SetCurrentCoParam(NULL);
                }
                
                constexpr void await_resume() const noexcept
                {
                }

                const bool _wait_at_initial_suspend{true};
                coro_handle _handle;
            };
            return InitialSuspendAwaiter{_wait_at_initial_suspend, coro_handle::from_promise(*this)};
        }

        struct FinalAwaiter 
        {
            constexpr bool await_ready() const noexcept { return false; }
            template<typename Promise>
            constexpr void await_suspend(std::coroutine_handle<Promise> h) const noexcept 
            {
                // 当前协程结束了
                CoTaskParam::SetCurrentCoParam(NULL);

                // 销毁子协程资源如果子协程还没结束
                auto &curProm = h.promise();
                auto child = curProm._child;
                if (child)
                {
                    child->DestroyHandle(Status::CoTaskFinishDestroyChild);
                }
                // auto lastChild = child;
                // while (child)
                // {
                //     child = child->GetChild();
                //     if (child)
                //         lastChild = child;
                // }
                // while (lastChild)
                // {
                //     auto parent = lastChild->GetParent();
                //     lastChild->SetState(KERNEL_NS::KernelHandle::SCHEDULED);
                //
                //     if (parent)
                //         parent->PopChild();
                //
                //     // 销毁协程
                //     lastChild->GetCoHandle().destroy();
                //     lastChild = parent;
                // }
                
                // 当前h结束了, 那么接着下一个(其实就是包裹着h的那个协程)
                if (auto cont = h.promise()._parent) 
                {
                    // 如果有Child则销毁并释放资源
                    
                    cont->PopChild();
                    // TODO:cout资源释放的问题需要考虑
                    cont->SetState(KERNEL_NS::KernelHandle::SCHEDULED);
                    auto handleId = cont->GetHandleId();
                    if(UNLIKELY(cont->GetParam()->_enableSuspend))
                    {
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
                        auto handle = KERNEL_NS::TlsUtil::GetTlsCoDict()->GetHandle(handleId);
                        if(LIKELY(handle))
                            handle->Run(KERNEL_NS::KernelHandle::UNSCHEDULED);
                    }
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

        virtual void DestroyHandle(Int32 errCode) override
        {
            // 先从父级弹出
            if(_parent)
                _parent->PopChild();
            _parent = NULL;
            
            if(_params->_errCode == Status::Success)
                _params->_errCode = errCode;
            
            // 有子协程, 子协程也需要释放
            if(_child)
            {
                _child->DestroyHandle(errCode);
                PopChild();
            }
            
            Cancel();
            auto handle = coro_handle::from_promise(*this);
            if(!handle.done())
                handle.destroy();
        }

        virtual SmartPtr<CoTaskParam, AutoDelMethods::Release> OnGetResult() override
        {
            if(_params->_timeout)
                _params->_timeout->Cancel();

            return _params;
        }
        virtual void PopChild() override
        {
            _child = NULL;
        }

        virtual void ThrowErrorIfExists() override 
        {
            // 有异常重新抛出
            auto &&exception = CoResult<R>::GetException();                                                     

            // 父级
            auto coPre = this->_parent;
            coPre->PopChild();

            // 销毁自己
            DestroyHandle(Status::CoTaskException);
            
            do
            {
                if(!coPre)
                    break;
                
                auto parent = coPre->GetParent();
                coPre->DestroyHandle(Status::CoTaskException);
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
                CoTaskParam::SetCurrentCoParam(NULL);
                return;
            }

            // 唤醒
            SetState(changeState);

            // 说明外部要手动唤醒
            // 设置当前协程
            CoTaskParam::SetCurrentCoParam(GetParam().AsSelf());
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
            {
                CoTaskParam::SetCurrentCoParam(GetParam().AsSelf());
                handle.resume();
            }
            else
            {
                CoTaskParam::SetCurrentCoParam(NULL);
            }
        }

        virtual bool IsDone() const override
        {
            auto handle = coro_handle::from_promise(*const_cast<promise_type *>(this));
            return handle.done();
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
            CoTaskParam::SetCurrentCoParam(NULL);
            auto handleId = resumer.promise().GetHandleId();
            KERNEL_NS::PostAsyncTask([handleId]() 
            {
                auto handle = KERNEL_NS::TlsUtil::GetTlsCoDict()->GetHandle(handleId);
                if(UNLIKELY(!handle))
                    return;

                handle->ForceAwake();
            });
        }

        void await_resume() const
        {
            
        }

        struct AwaiterYieldFinalAwaiter 
        {
            constexpr bool await_ready() const noexcept { return false; }
            template<typename Promise>
            constexpr void await_suspend(std::coroutine_handle<Promise> h) const noexcept 
            {
                CoTaskParam::SetCurrentCoParam(NULL);

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
    CoTask<R> &SetEnableSuspend(bool enableSuspend = true)
    {
        if(UNLIKELY(!_params))
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();

        _params->_enableSuspend = enableSuspend;

        return *this;
    }

    bool IsEnableSuspend() const { return _params ? _params->_enableSuspend : false; }

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

    CoTask<R> &SetRelease(IDelegate<void> *releaseSource)
    {
        if(UNLIKELY(!_params))
        {
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();
        }

        if (UNLIKELY(_params->_releaseSource))
            _params->_releaseSource->Release();
        
        _params->_releaseSource = releaseSource;

        return *this;
    }

    CoTask<R>& SetTimeout(const TimeSlice &slice)
    {
        if(UNLIKELY(!_params))
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();

        _params->_endTime = KERNEL_NS::LibTime::Now() + slice;

        return *this;
    }

    CoTask<R>& SetPrintStackIfTimeout(bool printStack)
    {
        if(UNLIKELY(!_params))
            _params = CoTaskParam::NewThreadLocal_CoTaskParam();

        _params->_printStackIfTimeout = printStack;

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
    mutable SmartPtr<CoTaskParam, AutoDelMethods::Release> _params;
};

static_assert(Promise<CoTask<>::promise_type>);
static_assert(Future<CoTask<>>);

KERNEL_END

#endif

#endif
