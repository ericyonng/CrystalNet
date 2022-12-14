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
 * Date: 2022-10-05 15:36:06
 * Author: Eric Yonng
 * Description: 处理信号,避免
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_SIGNAL_HANDLE_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_SIGNAL_HANDLE_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/Lock/Lock.h>

KERNEL_BEGIN

class KERNEL_EXPORT SignalHandleUtil
{
public:
    static void Lock();
    static void Unlock();

    /* linux的信号处理 */
public:
    // 1.注册信号处理要执行的任务
    static void PushSignalTask(Int32 signalId, IDelegate<void> *task);
    // 3.底层异常处理
    static void PushAllConcernSignalTask(IDelegate<void> *task);

    // 2.初始化信号处理
    static Int32 Init();
    // 3.销毁(恢复信号)
    static void Destroy();

    // 4.获取信号处理任务集合
    static std::vector<IDelegate<void> *> &GetTasks(Int32 signalId);

    // 5.恢复系统处理方式
    static void RecoverDefault(Int32 signalId);

    // 6.重新发起信号
    static void ResendSignal(Int32 signalId);

    // 信号名
    static LibString SignalToString(Int32 signalId);

private:
    static SpinLock _lck;
    static std::unordered_map<Int32, std::vector<IDelegate<void> *>> _signalRefTasks;
    static std::vector<IDelegate<void> *> _allSignalTasksPending;
    static std::unordered_set<Int32> _concernSignals;
};

KERNEL_END

#endif

