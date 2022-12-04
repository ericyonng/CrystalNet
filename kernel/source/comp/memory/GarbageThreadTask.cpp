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
 * Date: 2020-12-07 02:54:44
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/memory/GarbageThreadTask.h>
#include <kernel/comp/thread/thread.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(GarbageThreadTask);

GarbageThreadTask::GarbageThreadTask(LibThread *libThread, IDelegate<void> *callback, UInt64 workIntervalMsTime, ConditionLocker &lck)
    :_thead(libThread)
    ,_callback(callback)
    ,_workIntervalMsTime(workIntervalMsTime)
    ,_lck(lck)
{

}

void GarbageThreadTask::Run()
{
    g_Log->Sys(LOGFMT_OBJ_TAG("start garbage thread task thread id[%llu]"), SystemUtil::GetCurrentThreadId());

    for(;;)
    {
        if ( UNLIKELY(_thead->IsDestroy()) )
        {
            _callback->Invoke();
            break;
        }

        _callback->Invoke();

        // 休息一会儿
        _lck.Lock();
        _lck.TimeWait(_workIntervalMsTime);
        _lck.Unlock();        
    }   
}

void GarbageThreadTask::Release()
{
    GarbageThreadTask::Delete_GarbageThreadTask(this);
}

KERNEL_END
