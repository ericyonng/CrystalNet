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
 * Date: 2020-12-06 23:33:32
 * Author: Eric Yonng
 * Description: 
 *              1.AddTask
 *              2.Start
 *              3.Close 或者如果有多个线程可以: 执行所有线程的HalfClose,再执行waitDestroy
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_THREAD_LIB_THREAD_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_THREAD_LIB_THREAD_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/comp/Variant/Variant.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/Lock/Impl/ConditionLocker.h>
#include <kernel/comp/thread/ThreadDefs.h>

#include "kernel/comp/Coroutines/CoTask.h"

KERNEL_BEGIN

class ITask;
class Poller;
class IThreadStartUp;



class KERNEL_EXPORT LibThread
{
public:
    LibThread(IThreadStartUp *startUp = NULL);
    virtual ~LibThread();
    virtual void Release();

    // 启动
    void Start();
    // 关闭 = HalfClose + WaitDestroy合并
    void Close();
    // 半关闭 返回值表示可否执行WaitDestroy
    bool HalfClose();
    // 等待退出
    void FinishClose();
    // 唤醒
    void Wakeup();

    // 添加任务 仅仅添加任务，执行请调用唤醒, 失败不释放task或者callback
    bool AddTask(ITask *task);
    bool AddTask(IDelegate<void, LibThread *> *callback);
    template<typename ObjType>
    bool AddTask(ObjType *obj, void (ObjType::*callback)(LibThread *));
    bool AddTask(void (*callback)(LibThread *));

    bool AddTask2(void (*callback)(LibThread *,  Variant *), Variant *params);
    bool AddTask2(IDelegate<void, LibThread *, Variant *> *callback, Variant *params);
    template<typename ObjType>
    bool AddTask2(ObjType *obj, void (ObjType::*callback)(LibThread *, Variant *), Variant *params);
    template<typename LambdaType>
    requires requires(LambdaType &&invoke, LibThread *thread, Variant *var)
    {
        invoke(thread, var);
    }
    bool AddTask2(LambdaType &&lamb, Variant *params);
    template<typename LambdaType>
    requires requires(LambdaType &&invoke, LibThread *thread, Variant *var)
    {
        invoke(thread, var);
    }
    bool AddTask2(LambdaType &&lamb);
    
    // 是否销毁
    bool IsDestroy() const;
    // 是否忙
    bool IsBusy() const;
    // 是否启动
    bool IsStart() const;
    // 全局自增id
    UInt64 GetId() const;
    // 线程id
    UInt64 GetTheadId() const;
    // 设置是否添加任务
    void SetEnableTask(bool enable);

    // 设置线程名
    void SetThreadName(const KERNEL_NS::LibString &name);
    const LibString &GetThreadName() const;

    LibString ToString() const;

    CoTask<const Poller *> GetPoller() const;
    CoTask<Poller *> GetPoller();
    const Poller * GetPollerNoAsync() const;
    Poller * GetPollerNoAsync();

private:
    // 线程处理函数
    static void LibThreadHandlerLogic(void *param);
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    static unsigned __stdcall ThreadHandler(void *param);
#else
    static void *ThreadHandler(void *param);
#endif

    // 创建线程
    void _CreateThread(UInt64 unixStackSize = THREAD_DEF_STACK_SIZE);

private:

    // 全局的线程id 使用globalthreadid生成的
    const UInt64 _id;
    // 原生线程id
    std::atomic<UInt64> _threadId;   

    // 状态
    std::atomic_bool _isStart;          // 调用启动
    std::atomic_bool _isWorking;        // 线程工作标识由线程控制
    std::atomic_bool _isDestroy;         // 线程是否销毁
    std::atomic_bool _isBusy;           // 是否在处理任务
    std::atomic_bool _enableAddTask;    // 是否可以添加任务

    // 切换任务时
    SpinLock _taskLck;                  // 避免线程冲突添加任务
    std::list<ITask *> _tasks;  

    ConditionLocker _condLck;           // 线程控制
    ConditionLocker _quitLck;           // 等待退出线程

    LibString _threadName;

    std::atomic<KERNEL_NS::Poller *> _poller;
    std::atomic<IThreadStartUp *> _threadStartUp;
};

// 启动
ALWAYS_INLINE void LibThread::Start()
{
    if(_isStart.exchange(true, std::memory_order_acq_rel))
        return;

    _CreateThread();
}

// 关闭
ALWAYS_INLINE void LibThread::Close()
{
    if (!HalfClose())
        return;

    FinishClose();
}

ALWAYS_INLINE bool LibThread::HalfClose()
{
    // 已经关闭则不需要重复关闭
    if (!_isStart.exchange(false, std::memory_order_acq_rel))
        return false;

    // 先停止添加任务
    _enableAddTask.store(false, std::memory_order_release);
    // 停止工作
    _isDestroy.store(true, std::memory_order_release);

    return true;
}

// 唤醒
ALWAYS_INLINE void LibThread::Wakeup()
{
    // 唤醒
    _condLck.Sinal();
}

ALWAYS_INLINE bool LibThread::AddTask(ITask *task)
{
    if(!_enableAddTask.load(std::memory_order_acquire))
        return false;

    _taskLck.Lock();
    _tasks.push_back(task);
    _taskLck.Unlock();

    // 唤醒
    _condLck.Sinal();

    return true;
}

template<typename ObjType>
ALWAYS_INLINE bool LibThread::AddTask(ObjType *obj, void (ObjType::*callback)(LibThread *))
{
    IDelegate<void, LibThread *> *deleg = DelegateFactory::Create(obj, callback);
    if(UNLIKELY(!AddTask(deleg)))
    {
        CRYSTAL_RELEASE_SAFE(deleg);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibThread::AddTask(void (*callback)(LibThread *))
{
    IDelegate<void, LibThread *> *deleg = DelegateFactory::Create(callback);
    if(UNLIKELY(!AddTask(deleg)))
    {
        CRYSTAL_RELEASE_SAFE(deleg);
        return false;
    }
    
    return true;
}

ALWAYS_INLINE bool LibThread::AddTask2(void (*callback)(LibThread *,  Variant *), Variant *params)
{
    auto *deleg = DelegateFactory::Create(callback);
    if(!UNLIKELY(AddTask2(deleg, params)))
    {
        CRYSTAL_RELEASE_SAFE(deleg);
        if(params)
           Variant::Delete_Variant(params);
        return false;
    }

    return true;
}

template<typename ObjType>
ALWAYS_INLINE bool LibThread::AddTask2(ObjType *obj, void (ObjType::*callback)(LibThread *, Variant *), Variant *params)
{
    IDelegate<void, LibThread *, Variant *> *deleg = DelegateFactory::Create(obj, callback);
    if(UNLIKELY(!AddTask2(deleg, params)))
    {
        CRYSTAL_RELEASE_SAFE(deleg);
        if(params)
            Variant::Delete_Variant(params);
        return false;
    }

    return true;
}

template<typename LambdaType>
requires requires(LambdaType &&invoke, LibThread *thread, Variant *var)
{
    invoke(thread, var);
}
ALWAYS_INLINE bool LibThread::AddTask2(LambdaType &&lamb, Variant *params)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(lamb, void, LibThread *, Variant *);
    if(UNLIKELY(!AddTask2(delg, params)))
    {
        CRYSTAL_RELEASE_SAFE(delg);
        if(params)
            Variant::Delete_Variant(params);
        return false;
    }

    return true;
}

template<typename LambdaType>
requires requires(LambdaType &&invoke, LibThread *thread, Variant *var)
{
    invoke(thread, var);
}
ALWAYS_INLINE bool LibThread::AddTask2(LambdaType &&lamb)
{
    return AddTask2(std::forward<LambdaType>(lamb), nullptr);
}

ALWAYS_INLINE bool LibThread::IsDestroy() const
{
    return _isDestroy.load(std::memory_order_acquire);
}

ALWAYS_INLINE bool LibThread::IsBusy() const
{
    return _isBusy.load(std::memory_order_acquire);
}

ALWAYS_INLINE bool LibThread::IsStart() const
{
    return _isStart.load(std::memory_order_acquire);
}

ALWAYS_INLINE UInt64 LibThread::GetId() const
{
    return _id;
}

ALWAYS_INLINE UInt64 LibThread::GetTheadId() const
{
    return _threadId.load(std::memory_order_acquire);
}

ALWAYS_INLINE void LibThread::SetEnableTask(bool enable)
{
    _enableAddTask.store(enable, std::memory_order_release);
}

ALWAYS_INLINE void LibThread::SetThreadName(const KERNEL_NS::LibString &name)
{
    _threadName = name;
}

ALWAYS_INLINE const LibString &LibThread::GetThreadName() const
{
    return _threadName;
}

ALWAYS_INLINE const Poller * LibThread::GetPollerNoAsync() const
{
    return _poller.load(std::memory_order_acquire);
}

ALWAYS_INLINE Poller * LibThread::GetPollerNoAsync()
{
    return _poller.load(std::memory_order_acquire);
}

KERNEL_END


#endif
