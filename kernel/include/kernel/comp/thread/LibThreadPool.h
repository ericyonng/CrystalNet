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
 * Date: 2021-02-08 10:50:40
 * Description: 
 *              1.Init->Start
 *              2.AddTask
 *              3.HalfClose
 *              4.FinishClose
 *              或
 *              1.Init->AddTask->Start
 *              2.HalfClose
 *              3.FinishClose
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_THREAD_LIB_THREAD_POOL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_THREAD_LIB_THREAD_POOL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Task/Task.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/thread/ThreadDefs.h>

KERNEL_BEGIN

class KERNEL_EXPORT LibThreadPool
{
public:
    explicit LibThreadPool();
    virtual ~LibThreadPool();
    virtual void Release();

    // // 对外接口线程安全
public:
    // 初始化
    /*
    * 初始化
    * @param(minNum):最小线程数
    * @param(maxNum):最大线程数
    * @param(unixStackSize):linux下设置的线程栈大小, 0表示不设置,THREAD_DEF_STACK_SIZE默认值给10MB
    */
    void Init(Int32 minNum, Int32 maxNum, UInt64 unixStackSize = THREAD_DEF_STACK_SIZE);
    // 启动线程池
    bool Start(bool forceNewThread = false, Int32 numOfThreadToCreateIfNeed = 1);
    // = HalfClose + FinishClose
    void Close();
    // 半关闭 返回值用于判断是否可执行FinishClose
    bool HalfClose();
    void FinishClose();
    // 设置是否添加任务
    void SetEnableTask(bool enable);
    // 线程池工作状态
    // 只可往上加线程上限,不可下调
    void AddMaxThreadNum(UInt32 addNum);
    // 给当前线程池新增工作线程 上限是max thread num
    bool AddThreads(Int32 addNum);
    LibString ToString();
    bool IsDestroy() const;

    // 设置线程池名
    void SetPoolName(const LibString &name);
    const LibString &GetPoolName() const;

    /*
    * 添加任务
    * @param(task):任务 无需关系释放问题,但失败不会释放task或者callback
    * @param(forceNewThread):强制创建线程执行
    * @param(numOfThreadToCreateIfNeed):一次创建多少个线程
    */
    bool AddTask(ITask *task, bool forceNewThread = false, Int32 numOfThreadToCreateIfNeed = 1);
    bool AddTask(IDelegate<void, LibThreadPool *> *callback, bool forceNewThread = false, Int32 numOfThreadToCreateIfNeed = 1);
    template<typename ObjType>
    bool AddTask(ObjType *obj, void (ObjType::*callback)(LibThreadPool *), bool forceNewThread = false, Int32 numOfThreadToCreateIfNeed = 1);
    bool AddTask(void (*callback)(LibThreadPool *), bool forceNewThread = false, Int32 numOfThreadToCreateIfNeed = 1);
    
    bool AddTask2(void (*callback)(LibThreadPool *,  Variant *), Variant *params, bool forceNewThread = false, Int32 numOfThreadToCreateIfNeed = 1);
    bool AddTask2(IDelegate<void, LibThreadPool *, Variant *> *callback, Variant *params, bool forceNewThread = false, Int32 numOfThreadToCreateIfNeed = 1);
    template<typename ObjType>
    bool AddTask2(ObjType *obj, void (ObjType::*callback)(LibThreadPool *, Variant *), Variant *params, bool forceNewThread = false, Int32 numOfThreadToCreateIfNeed = 1);
    
    // // 线程执行函数
protected:
    static void _LibThreadHandlerLogic(void *param);

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    // 线程任务执行
    static unsigned __stdcall ThreadHandler(void *param);
#else
    static void *ThreadHandler(void *param);//线程处理函数
#endif
    

private:
    // 创建线程 return 若一个线程都没有创建出来则返回false
    bool _CreateThread(Int32 numToCreate, UInt64 unixStackSize = THREAD_DEF_STACK_SIZE);

    // // 内部数据
private:
    std:: atomic<Int32> _minNum;       // 最小线程数
    std:: atomic<Int32> _maxNum;       // 最大线程数
    std:: atomic<Int32> _curTotalNum;  // 当前线程数
    std:: atomic<Int32> _waitNum;      // 正在挂起的线程数
    UInt64 _unixStackSize;             // 只有linux下可设置线程栈大小
    std:: atomic_bool _isInit;         // 是否初始化过
    std:: atomic_bool _isWorking;      // 是否正在工作
    std:: atomic_bool _isDestroy;      // 是否正在销毁
    std:: atomic_bool _isEnableTask;   // 是否可以添加任务

    SpinLock _lck;                     // 线程安全锁
    std:: list<ITask *> _tasks;        // 任务队列

    ConditionLocker _wakeupAndWait;    // 用于闲时挂起线程,与唤醒线程
    ConditionLocker _quitLck;          // 等待退出线程
    
    LibString _poolName;                // 池名
};

ALWAYS_INLINE void LibThreadPool::Close()
{
    if(!HalfClose())
        return;

    FinishClose();
}

ALWAYS_INLINE bool LibThreadPool::HalfClose()
{
    // 已经关闭
    if(!_isWorking.exchange(false))
        return false;

    _isDestroy.store(true);
    _isEnableTask.store(false);

    CRYSTAL_TRACE("LibThreadPool HalfClose");
    return true;
}

ALWAYS_INLINE void LibThreadPool::FinishClose()
{
    // 线程数为0为止
    while ( _curTotalNum.load() > 0)
    {
        // 唤醒
        _wakeupAndWait.Sinal();

        // 等待
        _quitLck.Lock();
        _quitLck.TimeWait(THREAD_POOL_WAIT_FOR_COMPLETED_TIME);
        _quitLck.Unlock();
    }

    CRYSTAL_TRACE("LibThreadPool FinishClose");
}

ALWAYS_INLINE void LibThreadPool::SetEnableTask(bool enable)
{
    _isEnableTask.exchange(enable);
}

ALWAYS_INLINE void LibThreadPool::AddMaxThreadNum(UInt32 addNum)
{
    _maxNum += addNum;
}


ALWAYS_INLINE bool LibThreadPool::IsDestroy() const
{
    return _isDestroy.load();
}

ALWAYS_INLINE void LibThreadPool::SetPoolName(const LibString &name)
{
    _poolName = "POOL_" + name;
}

ALWAYS_INLINE const LibString &LibThreadPool::GetPoolName() const
{
    return _poolName;
}

ALWAYS_INLINE bool LibThreadPool::AddTask(IDelegate<void, LibThreadPool *> *callback, bool forceNewThread, Int32 numOfThreadToCreateIfNeed)
{
    DelegateTask<LibThreadPool> *newTask = DelegateTask<LibThreadPool>::New_DelegateTask(this, callback);
    if(UNLIKELY(!AddTask(newTask, forceNewThread, numOfThreadToCreateIfNeed)))
    {
        newTask->PopCallback();
        newTask->Release();
        return false;
    }

    return true;
}

template<typename ObjType>
ALWAYS_INLINE bool LibThreadPool::AddTask(ObjType *obj, void (ObjType::*callback)(LibThreadPool *), bool forceNewThread, Int32 numOfThreadToCreateIfNeed)
{
    auto deleg = DelegateFactory::Create(obj, callback);
    if(UNLIKELY(!AddTask(deleg, forceNewThread, numOfThreadToCreateIfNeed)))
    {
        CRYSTAL_RELEASE_SAFE(deleg);
        return false;
    }

    return true;
}

ALWAYS_INLINE bool LibThreadPool::AddTask(void (*callback)(LibThreadPool *), bool forceNewThread, Int32 numOfThreadToCreateIfNeed)
{
    auto deleg = DelegateFactory::Create(callback);
    if(UNLIKELY(!AddTask(deleg, forceNewThread, numOfThreadToCreateIfNeed)))
    {
        CRYSTAL_RELEASE_SAFE(deleg);
        return false;
    }

    return true;   
}

ALWAYS_INLINE bool LibThreadPool::AddTask2(void (*callback)(LibThreadPool *,  Variant *), Variant *params, bool forceNewThread, Int32 numOfThreadToCreateIfNeed)
{
    auto *deleg = DelegateFactory::Create(callback);
    if(!UNLIKELY(AddTask2(deleg, params, forceNewThread, numOfThreadToCreateIfNeed)))
    {
        CRYSTAL_RELEASE_SAFE(deleg);
        if(params)
           Variant::Delete_Variant(params);
        return false;
    }

    return true;
}

template<typename ObjType>
ALWAYS_INLINE bool LibThreadPool::AddTask2(ObjType *obj, void (ObjType::*callback)(LibThreadPool *, Variant *), Variant *params, bool forceNewThread, Int32 numOfThreadToCreateIfNeed)
{
    IDelegate<void, LibThreadPool *, Variant *> *deleg = DelegateFactory::Create(obj, callback);
    if(UNLIKELY(!AddTask2(deleg, params, forceNewThread, numOfThreadToCreateIfNeed)))
    {
        CRYSTAL_RELEASE_SAFE(deleg);
        if(params)
            Variant::Delete_Variant(params);
        return false;
    }

    return true;
}

KERNEL_END

#endif
