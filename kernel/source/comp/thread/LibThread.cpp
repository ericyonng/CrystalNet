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
 * Date: 2020-12-07 01:49:06
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/thread/LibThread.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Tls/Tls.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/thread/ThreadTool.h>
#include <kernel/comp/thread/LibThreadGlobalId.h>
#include <kernel/comp/Task/DelegateTask.h>
#include <kernel/comp/Task/DelegateWithParamsTask.h>

#include <kernel/comp/Log/log.h>

#include "kernel/comp/Coroutines/CoDelay.h"
#include <kernel/comp/thread/IThreadStartUp.h>

KERNEL_BEGIN

LibThread::LibThread(IThreadStartUp *startUp)
    :_id(LibThreadGlobalId::GenId())
    , _threadId{0}
    , _isStart{false}
    , _isWorking{false}
    , _isDestroy{ false }
    ,_isBusy{false}
    ,_enableAddTask{true}
    ,_poller{NULL}
    ,_threadStartUp{startUp}
{

}

LibThread::~LibThread()
{
    Close();
}

void LibThread::Release()
{
    CRYSTAL_DELETE(this);
}


void LibThread::FinishClose()
{
    // 线程退出
    while (true)
    {
        // 唤醒
        Wakeup();

        // 睡眠等待线程退出
        _quitLck.Lock();
        _quitLck.TimeWait(THREAD_POOL_WAIT_FOR_COMPLETED_TIME);

        if (!_isWorking.load(std::memory_order_acquire))
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

    _poller.store(NULL, std::memory_order_release);

    // 释放
    auto startUp = _threadStartUp.exchange(NULL, std::memory_order_acq_rel);
    if (LIKELY(startUp))
    {
        startUp->Release();
    }
}

bool LibThread::AddTask(IDelegate<void, LibThread *> *callback)
{
    if(!_enableAddTask.load(std::memory_order_acquire))
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

bool LibThread::AddTask2(IDelegate<void, LibThread *, Variant *> *callback, Variant *params)
{
    if(!_enableAddTask.load(std::memory_order_acquire))
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

LibString LibThread::ToString() const
{
    LibString info;
    info.AppendFormat("id = %llu, thread name:%s, threadId = %llu, isStart = %d, isWorking = %d, \n"
                    "isBusy = %d, enableAddTask = %d"
                    , _id, _threadName.c_str(), _threadId.load(std::memory_order_acquire), _isStart.load(std::memory_order_acquire)
                    , _isWorking.load(std::memory_order_acquire)
                    , _isBusy.load(std::memory_order_acquire), _enableAddTask.load(std::memory_order_acquire));

    return info;
}

CoTask<const Poller *> LibThread::GetPoller() const
{
    const Poller *poller = NULL;
    if (_isDestroy.load(std::memory_order_acquire))
        co_return poller;

    poller = _poller.load(std::memory_order_acquire);
    while (poller == NULL)
    {
        co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromMilliSeconds(10));
        poller = _poller.load(std::memory_order_acquire);
    }

    if (_isDestroy.load(std::memory_order_acquire))
        co_return NULL;

    co_return poller;
}

CoTask<Poller *> LibThread::GetPoller()
{
    KERNEL_NS::Poller * poller = NULL;
    if (_isDestroy.load(std::memory_order_acquire))
        co_return poller;

    poller = _poller.load(std::memory_order_acquire);
    while (poller == NULL)
    {
        co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromMilliSeconds(10));
        poller = _poller.load(std::memory_order_acquire);
    }

    if (_isDestroy.load(std::memory_order_acquire))
        co_return NULL;

    co_return poller;
}

void LibThread::LibThreadHandlerLogic(void *param)
{
    LibThread *libThread = static_cast<LibThread *>(param);
    ConditionLocker &condLck = libThread->_condLck;
    ConditionLocker &quitLck = libThread->_quitLck;
    SpinLock &taskLck = libThread->_taskLck;
    std::atomic_bool &isBusy = libThread->_isBusy;
    std::atomic_bool &isWorking = libThread->_isWorking;
    std::atomic_bool &isDestroy = libThread->_isDestroy;
    std::atomic_bool &enableAddTask = libThread->_enableAddTask;
    std::list<ITask *> &taskList = libThread->_tasks;
    libThread->_threadId.store(SystemUtil::GetCurrentThreadId(), std::memory_order_release);

    if(!libThread->GetThreadName().empty())
    {
        LibString err;
        if(!SystemUtil::SetCurrentThreadName(libThread->GetThreadName(), err))
        {
            CRYSTAL_TRACE("set current thread name fail thread id:%llu, name:%s, err:%s"
            , libThread->_threadId.load(std::memory_order_acquire), libThread->GetThreadName().c_str(), err.c_str());
        }
    }

    // 初始化
    ThreadTool::OnInit(libThread, NULL, libThread->_threadId.load(::std::memory_order_acquire), libThread->GetId(), "lib thread local tls memorypool");

    ThreadTool::OnStart();

    libThread->_poller.store(KERNEL_NS::TlsUtil::GetPoller(), std::memory_order_release);

    // 调用startup
    if (auto startUp = libThread->_threadStartUp.load(std::memory_order_acquire))
    {
        startUp->Run();
    } 

    try
    {
        bool isEmpty = false;
        while(!isDestroy.load(std::memory_order_acquire) || !isEmpty)
        {
            taskLck.Lock();
            if(LIKELY(!taskList.empty()))
            {
                isBusy.store(true, std::memory_order_release);
                auto task = taskList.front();
                taskList.pop_front();
                taskLck.Unlock();

                isEmpty = false;
                task->Run();
                if(LIKELY(!task->CanReDo() || !enableAddTask.load(std::memory_order_acquire)))
                    task->Release();
                else
                {
                    taskLck.Lock();
                    taskList.push_back(task);
                    taskLck.Unlock();
                }
                continue;
            }
            else
            {
                taskLck.Unlock();
            }

            isBusy.store(false, std::memory_order_release);
            condLck.Lock();
            condLck.Wait();
            condLck.Unlock();

            taskLck.Lock();
            isEmpty = taskList.empty();
            taskLck.Unlock();
        }

    }
    catch (...)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(LibThread, "thread exception..."));
    }

    libThread->_poller.store(NULL, std::memory_order_release);
    
    // 释放线程局部存储资源
    ThreadTool::OnDestroy();

    try
    {
        quitLck.Lock();

        isWorking.store(false, std::memory_order_release);
        isBusy.store(false, std::memory_order_release);
        enableAddTask.store(false, std::memory_order_release);
        quitLck.Unlock();

        quitLck.Sinal();
    }
    catch(...)
    {
        std::cout << "LibThreadHandlerLogic quitLck crash" << std::endl;
    }
}

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
unsigned __stdcall LibThread::ThreadHandler(void *param)
{
    LibThreadHandlerLogic(param);
//    CRYSTAL_TRACE("LibThread::ThreadHandler _endthreadex end thread id[%llu]", SystemUtil::GetCurrentThreadId());
    _endthreadex(0L);

    return 0L;
}

#else

void *LibThread::ThreadHandler(void *param)
{
    // 线程分离
    pthread_detach(::pthread_self());

    LibThreadHandlerLogic(param);

    // CRYSTAL_TRACE("LibThread::ThreadHandler _endthreadex end thread id[%llu]", SystemUtil::GetCurrentThreadId());
    pthread_exit((void *)0);
}

#endif

void LibThread::_CreateThread(UInt64 unixStackSize)
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

        _isWorking.store(true, std::memory_order_release);
}


KERNEL_END

