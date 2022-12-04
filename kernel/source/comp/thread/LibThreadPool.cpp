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

    // 当前线程id
    const auto threadId = SystemUtil::GetCurrentThreadId();

    ThreadTool::OnInit(NULL, pool, threadId, "thread pool tls memory pool");

    bool isEmpty = false;
    while(isWorking.load() || !isEmpty)
    {
        taskLck.Lock();
        if(LIKELY(!taskList.empty()))
        {
            auto task = taskList.front();
            taskList.pop_front();
            taskLck.Unlock();

            isEmpty = false;
            task->Run();
            task->Release();
            continue;
        }
        taskLck.Unlock();

        wakeupAndWaitLck.Lock();
        ++waitNum;
        wakeupAndWaitLck.Wait();
        --waitNum;
        wakeupAndWaitLck.Unlock();

        taskLck.Lock();
        isEmpty = taskList.empty();
        taskLck.Unlock();
    }

    // 释放线程局部存储资源
    ThreadTool::OnDestroy();

    try
    {
        const Int32 curNum = --curTotalNum;
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

KERNEL_END
