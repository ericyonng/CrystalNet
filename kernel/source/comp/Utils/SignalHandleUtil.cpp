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
 * Date: 2022-10-05 15:41:08
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/SignalHandleUtil.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Utils/BackTraceUtil.h>
#include <kernel/comp/Log/log.h>

extern "C"
{
    // 1.段错误捕获
    static void CatchSigHandler(Int32 signalNo)
    {
        // 1.打印堆栈
        if(LIKELY(g_Log))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SignalHandleUtil, "catch signal:%d, %s, thread id:%llu stack backtrace:%s")
                , signalNo, KERNEL_NS::SignalHandleUtil::SignalToString(signalNo).c_str(), KERNEL_NS::SystemUtil::GetCurrentThreadId(), KERNEL_NS::BackTraceUtil::CrystalCaptureStackBackTrace().c_str());
        }

        // 可恢复栈帧的信号
        if(KERNEL_NS::SignalHandleUtil::IsSignalRecoverable(signalNo, true))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SignalHandleUtil, "will recover last point"));
            KERNEL_NS::SignalHandleUtil::RecoverToLastPoint(true);
        }
        else
        {
            // 3.底层信号处理集合
            {
                auto &tasks = KERNEL_NS::SignalHandleUtil::GetTasks(signalNo);
                for (auto delg : tasks)
                    delg->Invoke();
            }

            // 4.恢复系统默认处理方式
            KERNEL_NS::SignalHandleUtil::RecoverDefault(signalNo);

            // 5.重新发射信号 执行默认处理
            KERNEL_NS::SignalHandleUtil::ResendSignal(signalNo);
        }
    }
}

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

// 拦截关闭窗口消息
LRESULT CALLBACK WindowProc(
    __in HWND hWindow,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        CatchSigHandler(SIGTERM);
        break;
    default:
        return DefWindowProc(hWindow, uMsg, wParam, lParam);
    }

    return 0;
}

#endif

KERNEL_BEGIN

SpinLock SignalHandleUtil::_lck;
std::unordered_map<Int32, std::vector<IDelegate<void> *>> SignalHandleUtil::_signalRefTasks;
std::vector<IDelegate<void> *> SignalHandleUtil::_allSignalTasksPending;
std::unordered_set<Int32> SignalHandleUtil::_concernSignals;
std::unordered_set<Int32> SignalHandleUtil::_recoverableSignals;
std::vector<jmp_buf *> s_stackFrames;

void SignalHandleUtil::Lock()
{
    _lck.Lock();
}

void SignalHandleUtil::Unlock()
{
    _lck.Unlock();
}

void SignalHandleUtil::PushSignalTask(Int32 signalId, IDelegate<void> *task)
{
    auto iter = _signalRefTasks.find(signalId);
    if(iter == _signalRefTasks.end())
        iter = _signalRefTasks.insert(std::make_pair(signalId, std::vector<IDelegate<void> *>())).first;

    iter->second.push_back(task);
}

void SignalHandleUtil::PushAllConcernSignalTask(IDelegate<void> *task)
{
    if(_concernSignals.empty())
    {
        _lck.Lock();
        _allSignalTasksPending.push_back(task);
        _lck.Unlock();

        return;
    }
    else
    {
        for(auto signalId : _concernSignals)
            PushSignalTask(signalId, task);
    }
}

Int32 SignalHandleUtil::Init()
{
    // #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS

    // 1.用户退出linux登录时,进程收到sighup信号终止进程
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGHUP, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGHUP).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGHUP);
    #endif
    
    // // ctrl + c 终止进程
    if(::signal(SIGINT, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGINT).c_str());
        return Status::Error;
    }

    _concernSignals.insert(SIGINT);

    // // SIGQUIT 终止进程并产生core
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGQUIT, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGQUIT).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGQUIT);
    #endif

    // SIGILL 非法指令 不可恢复至默认
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGILL, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGILL).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGILL);
    #endif

    // SIGABRT abort函数
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGABRT, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGABRT).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGABRT);
    #endif

    // // SIGBUS 非法地址,包括内存地址对齐出错等
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGBUS, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGBUS).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGBUS);
    #endif

    // // SIGFPE 在发生致命算术运算错误时发出 TODO:windows下不需要捕获, core出来
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGFPE, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGFPE).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGFPE);
    #endif

    // SIGKILL 不可被处理阻塞，与忽略
    // ::signal(SIGKILL, CatchSigHandler);

    // SIGUSR1 留给用户使用
    // ::signal(SIGUSR1, CatchSigHandler);

    // SIGSEGV 试图访问未分配给自己的内存，或试图往没有写权限的内存地址写数据 TODO:windows下不需要捕获
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGSEGV, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGSEGV).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGSEGV);
    #endif
    
    // SIGPIPE 管道破裂需要忽略
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGPIPE).c_str());
        return Status::Error;
    }
    #endif

    // // SIGTERM 程序结束信号，通常kill 默认发送这个信号如果该信号处理不了才会去执行sigkill信号
    if(::signal(SIGTERM, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGTERM).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGTERM);

    // // SIGTSTP 暂停进程 CTRL + Z
    // if(::signal(SIGTSTP, CatchSigHandler) == SIG_ERR)
    // {
    //     CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGTSTP).c_str());
    //     return Status::Error;
    // }
    // _concernSignals.insert(SIGTSTP);
    
    // // SIGTTIN 后台作业要从用户中断读数据时,暂停进程
    // if(::signal(SIGTTIN, CatchSigHandler) == SIG_ERR)
    // {
    //     CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGTTIN).c_str());
    //     return Status::Error;
    // }
    // _concernSignals.insert(SIGTTIN);

    // // SIGTTOU 类似于SIGTTIN, 但在写终端(或修改终端模式)时收到.
    // if(::signal(SIGTTOU, CatchSigHandler) == SIG_ERR)
    // {
    //     CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGTTOU).c_str());
    //     return Status::Error;
    // }
    // _concernSignals.insert(SIGTTOU);

    // SIGXFSZ 当进程企图扩大文件以至于超过文件大小资源限制 core
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGXFSZ, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGXFSZ).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGXFSZ);
    #endif

    // SIGXCPU 超过CPU时间资源限制. 这个限制可以由getrlimit/setrlimit来读取/ 改变 
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGXCPU, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGXCPU).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGXCPU);
    #endif

    // // SIGALRM 时钟定时信号, 计算的是实际的时间或时钟时间. alarm函数使用该 信号. 
    // if(::signal(SIGALRM, CatchSigHandler) == SIG_ERR)
    // {
    //     CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGALRM).c_str());
    //     return Status::Error;
    // }
    // _concernSignals.insert(SIGALRM);

    // // SIGPROF 类似于SIGALRM/SIGVTALRM, 但包括该进程用的CPU时间以及系统调用的时间. 
    // if(::signal(SIGPROF, CatchSigHandler) == SIG_ERR)
    // {
    //     CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGPROF).c_str());
    //     return Status::Error;
    // }
    // _concernSignals.insert(SIGPROF);

    // SIGSYS 非法的系统调用。
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGSYS, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGSYS).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGSYS);
    #endif

    // SIGPWR 关机
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGPWR, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGPWR).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGPWR);
    #endif

    // SIGSTKFLT 栈溢出
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if(::signal(SIGSTKFLT, CatchSigHandler) == SIG_ERR)
    {
        CRYSTAL_TRACE("signal set handler error, signal:%s", SignalToString(SIGSTKFLT).c_str());
        return Status::Error;
    }
    _concernSignals.insert(SIGSTKFLT);
    #endif

    for(auto pending : _allSignalTasksPending)
    {
        for(auto signalId : _concernSignals)
            PushSignalTask(signalId, pending);
    }
    _allSignalTasksPending.clear();

    // #endif

    // 需要恢复栈帧的信号 windows下有问题就不需要恢复
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        _recoverableSignals.insert(SIGILL);
        _recoverableSignals.insert(SIGABRT);
        _recoverableSignals.insert(SIGBUS);
        _recoverableSignals.insert(SIGFPE);
        _recoverableSignals.insert(SIGSEGV);
        _recoverableSignals.insert(SIGSYS);

        // TODO:测试恢复栈帧
        _recoverableSignals.insert(SIGINT);
    #endif

    return Status::Success;
}

void SignalHandleUtil::Destroy()
{
    // 此时不做自定义的信号处理
    _lck.Lock();
    ContainerUtil::DelContainer(_signalRefTasks, [](std::vector<IDelegate<void> *> &vec){
        ContainerUtil::DelContainer2(vec);
    });

    // ContainerUtil::DelContainer(s_stackFrames, [](jmp_buf *p){
    //     KernelFreeMemory<_Build::MT>(p);
    // });

    _lck.Unlock();
}

std::vector<IDelegate<void> *> &SignalHandleUtil::GetTasks(Int32 signalId)
{
    static std::vector<IDelegate<void> *> s_empty;
    auto iter = _signalRefTasks.find(signalId);
    return iter == _signalRefTasks.end() ? s_empty : iter->second;
}

void SignalHandleUtil::RecoverDefault(Int32 signalId)
{
    // #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
     ::signal(signalId, SIG_DFL);
    // #endif
}

void SignalHandleUtil::ResendSignal(Int32 signalId)
{
    // #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
     ::raise(signalId);
    // #endif
}

LibString SignalHandleUtil::SignalToString(Int32 signalId)
{
    LibString info;
    info.AppendFormat("signal id:%d, ", signalId);

    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    switch (signalId)
    {
    case SIGHUP: return info.AppendFormat("SIGHUP");
    case SIGINT: return info.AppendFormat("SIGINT");
    case SIGQUIT: return info.AppendFormat("SIGQUIT");
    case SIGILL: return info.AppendFormat("SIGILL");
    // case SIGTRAP: return info.AppendFormat("SIGTRAP");
    case SIGABRT: return info.AppendFormat("SIGABRT");
    // case SIGIOT: return info.AppendFormat("SIGIOT");
    case SIGBUS: return info.AppendFormat("SIGBUS");
    case SIGFPE: return info.AppendFormat("SIGFPE");
    case SIGKILL: return info.AppendFormat("SIGKILL");
    case SIGUSR1: return info.AppendFormat("SIGUSR1");
    case SIGSEGV: return info.AppendFormat("SIGSEGV");
    case SIGUSR2: return info.AppendFormat("SIGUSR2");
    case SIGPIPE: return info.AppendFormat("SIGPIPE");
    case SIGALRM: return info.AppendFormat("SIGALRM");
    case SIGTERM: return info.AppendFormat("SIGTERM");
    case SIGSTKFLT: return info.AppendFormat("SIGSTKFLT");
    case SIGCHLD: return info.AppendFormat("SIGCHLD");
    case SIGCONT: return info.AppendFormat("SIGCONT");
    case SIGSTOP: return info.AppendFormat("SIGSTOP");
    case SIGTSTP: return info.AppendFormat("SIGTSTP");
    case SIGTTIN: return info.AppendFormat("SIGTTIN");
    case SIGTTOU: return info.AppendFormat("SIGTTOU");
    case SIGURG: return info.AppendFormat("SIGURG");
    case SIGXCPU: return info.AppendFormat("SIGXCPU");
    case SIGXFSZ: return info.AppendFormat("SIGXFSZ");
    case SIGVTALRM: return info.AppendFormat("SIGVTALRM");
    case SIGPROF: return info.AppendFormat("SIGPROF");
    case SIGWINCH: return info.AppendFormat("SIGWINCH");
    case SIGIO: return info.AppendFormat("SIGIO");
    case SIGPWR: return info.AppendFormat("SIGPWR");
    case SIGSYS: return info.AppendFormat("SIGSYS");
    default:
        break;
    }
    #endif

    return info.AppendFormat("unknown signal");
} 

Int32 SignalHandleUtil::PushRecoverPoint(jmp_buf *stackFramePoint)
{
    Int32 ret = 0;
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        ret = setjmp(*stackFramePoint);
    #else
        ret = sigsetjmp(*stackFramePoint, 1);
    #endif

    _lck.Lock();
    s_stackFrames.push_back(stackFramePoint);
    _lck.Unlock();

    return ret;
}

void SignalHandleUtil::PopRecoverPoint()
{
    _lck.Lock();
    s_stackFrames.pop_back();
    _lck.Unlock();
}

void SignalHandleUtil::RecoverToLastPoint(bool skipLock)
{
    if(!skipLock)
        _lck.Lock();

    auto p = s_stackFrames.back();
    s_stackFrames.pop_back();
    
    if(!skipLock)
        _lck.Unlock();

    Int32 ret = 0;
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        longjmp(*p, 1);
    #else
        siglongjmp(*p, 1);
    #endif
}

void SignalHandleUtil::SetSignalRecoverable(Int32 signalId)
{
    _lck.Lock();
    _recoverableSignals.insert(signalId);
    _lck.Unlock();
}

bool SignalHandleUtil::IsSignalRecoverable(Int32 signalId, bool skipLock)
{
    if(!skipLock)
        _lck.Lock();

    auto ret = _recoverableSignals.find(signalId) != _recoverableSignals.end();
    if(!skipLock)
        _lck.Unlock();

    return ret;
}

KERNEL_END
