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
 * Date: 2020-12-07 02:51:25
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_GARBAGE_THREAD_TASK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_GARBAGE_THREAD_TASK_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Task/ITask.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/memory/ObjPoolMacro.h>

KERNEL_BEGIN

class LibThread;
class ConditionLocker;

class KERNEL_EXPORT GarbageThreadTask : public ITask
{
    POOL_CREATE_OBJ_DEFAULT_P1(ITask, GarbageThreadTask);
    
public:
    GarbageThreadTask(LibThread *libThread, IDelegate<void> *callback, UInt64 &workIntervalMsTime, ConditionLocker &lck);
    ~GarbageThreadTask(){ CRYSTAL_DELETE_SAFE(_callback); }

public:
    void Run();
    virtual void Release();

private:
    LibThread *_thead;
    IDelegate<void> *_callback;
    UInt64 &_workIntervalMsTime;    // 清理内存时间间隔
    ConditionLocker &_lck;
};

KERNEL_END

#endif
