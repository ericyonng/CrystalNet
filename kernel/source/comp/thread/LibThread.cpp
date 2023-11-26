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

KERNEL_BEGIN

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

LibString LibThread::ToString()
{
    LibString info;
    info.AppendFormat("id = %llu, thread name:%s, threadId = %llu, isStart = %d, isWorking = %d, \n"
                    "isBusy = %d, enableAddTask = %d"
                    , _id, _threadName.c_str(), _threadId.load(), _isStart.load(), _isWorking.load()
                    , _isBusy.load(), _enableAddTask.load());

    return info;
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
    libThread->_threadId = SystemUtil::GetCurrentThreadId();

    if(!libThread->GetThreadName().empty())
    {
        LibString err;
        if(!SystemUtil::SetCurrentThreadName(libThread->GetThreadName(), err))
        {
            CRYSTAL_TRACE("set current thread name fail thread id:%llu, name:%s, err:%s"
            , libThread->_threadId.load(), libThread->GetThreadName().c_str(), err.c_str());
        }
    }

    // 初始化
    ThreadTool::OnInit(libThread, NULL, libThread->_threadId, libThread->GetId(), "lib thread local tls memorypool");

    bool isEmpty = false;
    while(!isDestroy.load() || !isEmpty)
    {
        taskLck.Lock();
        if(LIKELY(!taskList.empty()))
        {
            isBusy = true;
            auto task = taskList.front();
            taskList.pop_front();
            taskLck.Unlock();

            isEmpty = false;
            task->Run();
            if(LIKELY(!task->CanReDo() || !enableAddTask))
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

        isBusy = false;
        condLck.Lock();
        condLck.Wait();
        condLck.Unlock();

        taskLck.Lock();
        isEmpty = taskList.empty();
        taskLck.Unlock();
    }

    // 释放线程局部存储资源
    ThreadTool::OnDestroy();

    try
    {

        quitLck.Lock();

        isWorking = false;
        isBusy = false;
        enableAddTask = false;
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

        _isWorking = true;
}


KERNEL_END

