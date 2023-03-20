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

#include <kernel/kernel_inc.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/Task/ITask.h>
#include <kernel/comp/thread/LibThreadGlobalId.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/thread/ThreadDefs.h>
#include <kernel/comp/Task/DelegateTask.h>
#include <kernel/comp/Task/DelegateWithParamsTask.h>

KERNEL_BEGIN

class KERNEL_EXPORT LibThread
{
public:
    LibThread();
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
    
    // 是否销毁
    bool IsDestroy();
    // 是否忙
    bool IsBusy();
    // 是否启动
    bool IsStart();
    // 全局自增id
    UInt64 GetId();
    // 线程id
    UInt64 GetTheadId();
    // 设置是否添加任务
    void SetEnableTask(bool enable);

    LibString ToString();

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
};


inline LibThread::LibThread()
    :_id(LibThreadGlobalId::GenId())
    , _threadId{0}
    , _isStart{false}
    , _isWorking{false}
    , _isDestroy{ false }
    ,_isBusy{false}
    ,_enableAddTask{true}
{

}

inline LibThread::~LibThread()
{
    Close();
}

// 启动
inline void LibThread::Start()
{
    if(_isStart.exchange(true))
        return;

    _CreateThread();
}

// 关闭
inline void LibThread::Close()
{
    if (!HalfClose())
        return;

    FinishClose();
}

inline bool LibThread::HalfClose()
{
    // 已经关闭则不需要重复关闭
    if (!_isStart.exchange(false))
        return false;

    // 先停止添加任务
    _enableAddTask.store(false);
    // 停止工作
    _isDestroy.store(true);

    return true;
}

inline void LibThread::FinishClose()
{
    // 线程退出
    while (true)
    {
        // 唤醒
        Wakeup();

        // 睡眠等待线程退出
        _quitLck.Lock();
        _quitLck.TimeWait(THREAD_POOL_WAIT_FOR_COMPLETED_TIME);

        if (!_isWorking.load())
        {
            _quitLck.Unlock();
            break;
        }
        _quitLck.Unlock();
    }

    // CRYSTAL_TRACE("FinishClose thread end threadId[%llu], uid[%llu] left task[%llu]"
    // , _threadId.load(), _id, static_cast<UInt64>(_tasks.size()));

    // 移除数据
    for(auto iter = _tasks.begin(); iter != _tasks.end();)
    {
        (*iter)->Release();
        iter = _tasks.erase(iter);
    }
}

// 唤醒
inline void LibThread::Wakeup()
{
    // 唤醒
    _condLck.Sinal();
}

inline bool LibThread::AddTask(ITask *task)
{
    if(!_enableAddTask.load())
        return false;

    _taskLck.Lock();
    _tasks.push_back(task);
    _taskLck.Unlock();

    // 唤醒
    _condLck.Sinal();

    return true;
}

inline bool LibThread::AddTask(IDelegate<void, LibThread *> *callback)
{
    if(!_enableAddTask.load())
    {
        //CRYSTAL_RELEASE(callback);
        return false;
    }

    auto newTask = DelegateTask<LibThread>::New_DelegateTask(this, callback);
    _taskLck.Lock();
    _tasks.push_back(newTask);
    _taskLck.Unlock();

    // 唤醒
    _condLck.Sinal();

    return true;
}

template<typename ObjType>
inline bool LibThread::AddTask(ObjType *obj, void (ObjType::*callback)(LibThread *))
{
    IDelegate<void, LibThread *> *deleg = DelegateFactory::Create(obj, callback);
    if(UNLIKELY(!AddTask(deleg)))
    {
        CRYSTAL_RELEASE_SAFE(deleg);
        return false;
    }

    return true;
}

inline bool LibThread::AddTask(void (*callback)(LibThread *))
{
    IDelegate<void, LibThread *> *deleg = DelegateFactory::Create(callback);
    if(UNLIKELY(!AddTask(deleg)))
    {
        CRYSTAL_RELEASE_SAFE(deleg);
        return false;
    }
    
    return true;
}

inline bool LibThread::AddTask2(void (*callback)(LibThread *,  Variant *), Variant *params)
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

inline bool LibThread::AddTask2(IDelegate<void, LibThread *, Variant *> *callback, Variant *params)
{
    if(!_enableAddTask.load())
    {
        //CRYSTAL_RELEASE(callback);
        return false;
    }

    DelegateWithParamsTask<LibThread> *newTask = DelegateWithParamsTask<LibThread>::New_DelegateWithParamsTask(this, callback, params);
    _taskLck.Lock();
    _tasks.push_back(newTask);
    _taskLck.Unlock();

    // 唤醒
    _condLck.Sinal();

    return true;
}

template<typename ObjType>
inline bool LibThread::AddTask2(ObjType *obj, void (ObjType::*callback)(LibThread *, Variant *), Variant *params)
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

inline bool LibThread::IsDestroy()
{
    return _isDestroy.load();
}

inline bool LibThread::IsBusy()
{
    return _isBusy.load();
}

inline bool LibThread::IsStart()
{
    return _isStart.load();
}

inline UInt64 LibThread::GetId()
{
    return _id;
}

inline UInt64 LibThread::GetTheadId()
{
    return _threadId.load();
}

inline void LibThread::SetEnableTask(bool enable)
{
    _enableAddTask = enable;
}

inline LibString LibThread::ToString()
{
    LibString info;
    info.AppendFormat("id = %llu, threadId = %llu, isStart = %d, isWorking = %d, \n"
                    "isBusy = %d, enableAddTask = %d"
                    , _id, _threadId.load(), _isStart.load(), _isWorking.load()
                    , _isBusy.load(), _enableAddTask.load());

    return info;
}

inline void LibThread::_CreateThread(UInt64 unixStackSize)
{
    Int32 ret = 0;
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
        UInt32 threadId = 0;
        auto threadHandle = reinterpret_cast<HANDLE>(::_beginthreadex(NULL, static_cast<UInt32>(unixStackSize)
        , LibThread::ThreadHandler, static_cast<void *>(this), 0, &threadId));
        
        // 释放资源
        if(LIKELY(threadHandle != INVALID_HANDLE_VALUE))
            ::CloseHandle(threadHandle);
#else
        pthread_t theadkey;
        pthread_attr_t threadAttr;
        pthread_attr_init(&threadAttr);
        if(unixStackSize)
           pthread_attr_setstacksize(&threadAttr, unixStackSize);
        ret = pthread_create(&theadkey, &threadAttr, &LibThread::ThreadHandler, (void *)this);
        pthread_attr_destroy(&threadAttr);
        if(ret != 0)
        {
            printf("\nret=%d\n", ret);
            perror("pthread_create error!");
        }
#endif

        _isWorking = true;
}

KERNEL_END


#endif
