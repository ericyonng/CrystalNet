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

KERNEL_END

