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
 * Date: 2021-02-08 10:53:15
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/thread/LibThreadPool.h>
#include <kernel/comp/Tls/Tls.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/thread/ThreadTool.h>
#include <kernel/comp/thread/LibThreadGlobalId.h>
#include <kernel/comp/Task/Task.h>

#include "kernel/comp/Log/log.h"

KERNEL_BEGIN
    LibThreadPool::LibThreadPool()
:_minNum{0}
,_maxNum{0}
,_curTotalNum{0}
,_waitNum{0}
,_unixStackSize(0)
,_isInit{false}
,_isWorking{false}
,_isDestroy{false}
,_isEnableTask(true)
{

}

LibThreadPool::~LibThreadPool()
{
    Close();
}

void LibThreadPool::Release()
{
    CRYSTAL_DELETE(this);
}

// 初始化
void LibThreadPool::Init(Int32 minNum, Int32 maxNum, UInt64 unixStackSize)
{
    // 初始化过不可再修改 初始化前不可启动线程池
    if(_isInit.exchange(true, std::memory_order_acq_rel))
        return;

    _isDestroy.store(false, std::memory_order_release);
    _minNum.store(minNum, std::memory_order_release);
    _maxNum.store(maxNum, std::memory_order_release);
    _curTotalNum.store(0, std::memory_order_release);
    _waitNum.store(0, std::memory_order_release);
    _isEnableTask.store(true, std::memory_order_release);
    _unixStackSize = unixStackSize;
}

bool LibThreadPool::Start(bool forceNewThread, Int32 numOfThreadToCreateIfNeed)
{
    if(!_isInit.load(std::memory_order_acquire))
    {
        CRYSTAL_TRACE("have no init before");
        return false;
    }

    if(_isWorking.exchange(true, std::memory_order_acq_rel))
        return true;

    const auto minNum = _minNum.load(std::memory_order_acquire);
    if(minNum)
      _CreateThread(minNum, _unixStackSize);

    // 唤醒
    if(_waitNum.load(std::memory_order_acquire) > 0 && !forceNewThread)
    {
        _wakeupAndWait.Sinal();
        return true;
    }

    // 是否需要创建线程来执行任务
    if(UNLIKELY(numOfThreadToCreateIfNeed <= 0))
        return true;

    _wakeupAndWait.Lock();
    const Int32 curTotalNum = _curTotalNum.load(std::memory_order_acquire);
    const Int32 maxNum = _maxNum.load(std::memory_order_acquire);
    const Int32 diffNum = maxNum - curTotalNum;
    numOfThreadToCreateIfNeed = diffNum > numOfThreadToCreateIfNeed ? numOfThreadToCreateIfNeed : diffNum;

    // 超过最大线程数
    if(UNLIKELY(curTotalNum + numOfThreadToCreateIfNeed > maxNum))
    {
        _wakeupAndWait.Unlock();
        return false;
    }

    _CreateThread(numOfThreadToCreateIfNeed, _unixStackSize);
    _wakeupAndWait.Unlock();

    return true;
}

bool LibThreadPool::AddThreads(Int32 addNum)
{
    if(!_isInit.load(std::memory_order_acquire) || _isDestroy.load(std::memory_order_acquire))
    {
        CRYSTAL_TRACE("POOL NOT INIT OR IS DESTROY WHEN AddThreads");
        return false;
    }

    _wakeupAndWait.Lock();
    const Int32 curTotalNum = _curTotalNum.load(std::memory_order_acquire);
    const Int32 maxNum = _maxNum.load(std::memory_order_acquire);
    const Int32 diffNum = maxNum - curTotalNum;
    addNum = diffNum > addNum ? addNum : diffNum;

    if(UNLIKELY(curTotalNum + addNum > maxNum))
    {
        _wakeupAndWait.Unlock();
        return false;
    }

    if(LIKELY(addNum))
        _CreateThread(addNum, _unixStackSize);

    _wakeupAndWait.Unlock();

    return true;
}

LibString LibThreadPool::ToString() const
{
    LibString info;
    const Int32 minNum = _minNum.load(std::memory_order_acquire);
    const Int32 maxNum = _maxNum.load(std::memory_order_acquire);
    const Int32 curTotalNum = _curTotalNum.load(std::memory_order_acquire);
    const Int32 waitNum = _waitNum.load(std::memory_order_acquire);
    const UInt64 unixStackSize = _unixStackSize;
    const bool isInit = _isInit.load(std::memory_order_acquire);
    const bool isWorking = _isWorking.load(std::memory_order_acquire);
    const bool isEnableTask = _isEnableTask.load(std::memory_order_acquire);

    info.AppendFormat("thread pool:%s status:minNum[%d], maxNum[%d], curTotalNum[%d], waitNum[%d]"
    ", unixStackSize[%llu], isInit[%d], isWorking[%d], isEnableTask[%d]"
    , _poolName.c_str(), minNum, maxNum, curTotalNum, waitNum, unixStackSize, isInit, isWorking, isEnableTask);

    return info;
}

bool LibThreadPool::AddTask(IDelegate<void, LibThreadPool *> *callback, bool forceNewThread, Int32 numOfThreadToCreateIfNeed)
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


bool LibThreadPool::AddTask(ITask *task, bool forceNewThread, Int32 numOfThreadToCreateIfNeed)
{
    if(UNLIKELY(!_isEnableTask.load(std::memory_order_acquire) || _isDestroy.load(std::memory_order_acquire)))
        return false;

    _lck.Lock();
    _tasks.push_back(task);
    _lck.Unlock();

    // 唤醒
    if(_waitNum.load(std::memory_order_acquire) > 0 && !forceNewThread)    // 
    {
        _wakeupAndWait.Sinal();
        return true;
    }

    // 是否需要创建线程来执行任务
    if(UNLIKELY(numOfThreadToCreateIfNeed <= 0))
        return true;

    _wakeupAndWait.Lock();
    const Int32 curTotalNum = _curTotalNum.load(std::memory_order_acquire);
    const Int32 maxNum = _maxNum.load(std::memory_order_acquire);
    const Int32 diffNum = maxNum - curTotalNum;
    numOfThreadToCreateIfNeed = diffNum > numOfThreadToCreateIfNeed ? numOfThreadToCreateIfNeed : diffNum;

    // 超过最大线程数
    if(UNLIKELY(curTotalNum + numOfThreadToCreateIfNeed > maxNum))
    {
        _wakeupAndWait.Unlock();
        return false;
    }

    _CreateThread(numOfThreadToCreateIfNeed, _unixStackSize);
    _wakeupAndWait.Unlock();

    return true; 
}

bool LibThreadPool::AddTask2(IDelegate<void, LibThreadPool *, Variant *> *callback, Variant *params, bool forceNewThread, Int32 numOfThreadToCreateIfNeed)
{
    if(UNLIKELY(!_isEnableTask.load(std::memory_order_acquire) || _isDestroy.load(std::memory_order_acquire)))
        return false;

    DelegateWithParamsTask<LibThreadPool> *newTask = DelegateWithParamsTask<LibThreadPool>::New_DelegateWithParamsTask(this, callback, params);
    _lck.Lock();
    _tasks.push_back(newTask);
    _lck.Unlock();

    // 唤醒
    if(_waitNum.load(std::memory_order_acquire) > 0 && !forceNewThread)    // 
    {
        _wakeupAndWait.Sinal();
        return true;
    }

    // 是否需要创建线程来执行任务
    if(UNLIKELY(numOfThreadToCreateIfNeed <= 0))
        return true;

    _wakeupAndWait.Lock();
    const Int32 curTotalNum = _curTotalNum.load(std::memory_order_acquire);
    const Int32 maxNum = _maxNum.load(std::memory_order_acquire);
    const Int32 diffNum = maxNum - curTotalNum;
    numOfThreadToCreateIfNeed = diffNum > numOfThreadToCreateIfNeed ? numOfThreadToCreateIfNeed : diffNum;

    // 超过最大线程数
    if(UNLIKELY(curTotalNum + numOfThreadToCreateIfNeed > maxNum))
    {
        _wakeupAndWait.Unlock();
        return false;
    }

    _CreateThread(numOfThreadToCreateIfNeed, _unixStackSize);
    _wakeupAndWait.Unlock();

    return true;
}

void LibThreadPool::_LibThreadHandlerLogic(void *param)
{
    LibThreadPool *pool = static_cast<LibThreadPool *>(param);
    ConditionLocker &wakeupAndWaitLck = pool->_wakeupAndWait;
    ConditionLocker &quitLck = pool->_quitLck;
    SpinLock &taskLck = pool->_lck;
    std::atomic_bool &isWorking = pool->_isWorking;
    std::atomic<Int32> &waitNum = pool->_waitNum;
    std::atomic<Int32> &curTotalNum = pool->_curTotalNum;
    std::list<ITask *> &taskList = pool->_tasks;
    std:: atomic_bool &isEnableTask = pool->_isEnableTask;

    // 当前线程id
    const auto threadId = SystemUtil::GetCurrentThreadId();

    if(!pool->GetPoolName().empty())
    {
        LibString err;
        if(!SystemUtil::SetCurrentThreadName(pool->GetPoolName(), err))
        {
            CRYSTAL_TRACE("thread pool set current thread name fail thread id:%llu, name:%s, err:%s"
            , threadId, pool->GetPoolName().c_str(), err.c_str());
        }
    }

    const auto threadGlobalId = LibThreadGlobalId::GenId();
    ThreadTool::OnInit(NULL, pool, threadId, threadGlobalId,  "thread pool tls memory pool");

    ThreadTool::OnStart();

    try
    {
        bool isEmpty = false;
        while(isWorking.load(std::memory_order_acquire) || !isEmpty)
        {
            taskLck.Lock();
            if(LIKELY(!taskList.empty()))
            {
                auto task = taskList.front();
                taskList.pop_front();
                taskLck.Unlock();

                isEmpty = false;
                task->Run();
                if(LIKELY(!task->CanReDo() || !isEnableTask.load(std::memory_order_acquire)))
                    task->Release();
                else
                {
                    taskLck.Lock();
                    taskList.push_back(task);
                    taskLck.Unlock();
                }
                continue;
            }
            taskLck.Unlock();

            wakeupAndWaitLck.Lock();
            waitNum.fetch_add(1, std::memory_order_release);
            wakeupAndWaitLck.Wait();
            waitNum.fetch_sub(1, std::memory_order_release);
            wakeupAndWaitLck.Unlock();

            taskLck.Lock();
            isEmpty = taskList.empty();
            taskLck.Unlock();
        }
    }
    catch (...)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(LibThreadPool, "thread pool exception..."));
    }
   
    // 释放线程局部存储资源
    ThreadTool::OnDestroy();

    try
    {
        const Int32 curNum = curTotalNum.fetch_sub(1, std::memory_order_release) - 1;
        if(!curNum)
        {
            quitLck.Sinal();
        }

        CRYSTAL_TRACE("thread id[%llu] exit.", threadId);
    }
    catch(...)
    {
        std::cout << "LibThreadHandlerLogic quitLck crash" << std::endl;
    }
}

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

unsigned __stdcall LibThreadPool::ThreadHandler(void *param)
{
    _LibThreadHandlerLogic(param);

    CRYSTAL_TRACE("LibThreadPool::ThreadHandler _endthreadex end thread id[%llu]", SystemUtil::GetCurrentThreadId());
    _endthreadex(0L);

    return 0L;
}

#else

void *LibThreadPool::ThreadHandler(void *param)
{
    // 线程分离
    pthread_detach(::pthread_self());
    _LibThreadHandlerLogic(param);

    CRYSTAL_TRACE("LibThreadPool::ThreadHandler _endthreadex end thread id[%llu]", SystemUtil::GetCurrentThreadId());
    pthread_exit((void *)0);
}


#endif

bool LibThreadPool::_CreateThread(Int32 numToCreate, UInt64 unixStackSize)
{
    Int32 ret = 0;
    for(Int32 i = 0; i < numToCreate; ++i)
    {
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
        UInt32 threadId = 0;
        auto threadHandle = reinterpret_cast<HANDLE>(::_beginthreadex(NULL, static_cast<UInt32>(unixStackSize)
        , &LibThreadPool::ThreadHandler, static_cast<void *>(this), 0, &threadId));
        if(LIKELY(threadHandle != INVALID_HANDLE_VALUE))
            ::CloseHandle(threadHandle);    // 移除避免浪费内核资源
        else
        {
            if(i == 0)
                return false;

            break;
        }
#else
        pthread_t theadkey;
        pthread_attr_t threadAttr;
        pthread_attr_init(&threadAttr);
        if(unixStackSize)
            pthread_attr_setstacksize(&threadAttr, unixStackSize);
        ret = pthread_create(&theadkey, &threadAttr, &LibThreadPool::ThreadHandler, (void *)this);
        pthread_attr_destroy(&threadAttr);
        if(ret != 0)
        {
            printf("\nret=%d\n", ret);
            perror("pthread_create error!");

            if(i == 0)
                return false;

            break;
        }
#endif
        _curTotalNum.fetch_add(1, std::memory_order_release);
    }

    return true;
}

KERNEL_END
