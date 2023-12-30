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
 * 可恢复栈帧的信号将直接恢复到栈帧
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_SIGNAL_HANDLE_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_SIGNAL_HANDLE_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/LibString.h>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <setjmp.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <atomic>

KERNEL_BEGIN

template <typename Rtn, typename... Args>
class IDelegate;

class KERNEL_EXPORT SignoList
{
public:
    enum ENUMS
    {
        // 内存日志的信号
        MEMORY_LOG_SIGNO = 63,
    };
};

class KERNEL_EXPORT SignalHandleUtil
{
public:
    static void Lock();
    static void Unlock();

    /* linux的信号处理 */
public:
    // 1.注册信号处理要执行的任务
    static void PushSignalTask(Int32 signalId, IDelegate<void> *task);
    template<typename LambdaCb>
    static void PushSignalTask(Int32 signalId, LambdaCb &&);
    // 3.底层异常处理
    static void PushAllConcernSignalTask(IDelegate<void> *task);

    // 是否信号触发过 仅支持 1 - 63信号
    static bool IsSignoTrigger(Int32 signo);
    static void ClearSignoTrigger(Int32 signo);
    static void SetSignoTrigger(Int32 signo);
    static bool ExchangeSignoTriggerFlag(Int32 signo, bool isTrigger);

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

    // 设置恢复点
    static Int32 PushRecoverPoint(jmp_buf *stackFramePoint);
    static void PopRecoverPoint();
    static void RecoverToLastPoint(bool skipLock = false);

    // 设置某些信号栈恢复 recoverable
    static void SetSignalRecoverable(Int32 signalId);

    // 是否是可恢复的信号
    static bool IsSignalRecoverable(Int32 signalId, bool skipLock = false);

    // 发信号
    static void KillSelf(Int32 sig);

private:
    static SpinLock _lck;
    static std::unordered_map<Int32, std::vector<IDelegate<void> *>> _signalRefTasks;
    static std::vector<IDelegate<void> *> _allSignalTasksPending;
    static std::unordered_set<Int32> _concernSignals;
    static std::unordered_set<Int32> _recoverableSignals;
    static std::atomic<UInt64> _signoTriggerFlags;
};

template<typename LambdaCb>
ALWAYS_INLINE void SignalHandleUtil::PushSignalTask(Int32 signalId, LambdaCb &&cb)
{
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void);
    PushSignalTask(signalId, deleg);
}

KERNEL_END

#endif

