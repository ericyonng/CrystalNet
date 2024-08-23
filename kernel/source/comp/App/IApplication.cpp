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
 * Date: 2021-04-18 22:02:31
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/App/IApplication.h>
#include <kernel/comp/Utils/KernelUtil.h>
#include <kernel/comp/Tls/Tls.h>

#include <kernel/common/timedefs.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/LibTime.h>

#include <kernel/comp/Log/log.h>

#include <kernel/comp/Poller/PollerInc.h>
#include <kernel/comp/Cpu/LibCpuInfo.h>
#include <kernel/comp/Utils/SignalHandleUtil.h>

KERNEL_NS::IApplication *g_Application = NULL;

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IApplication);

IApplication::IApplication(UInt64 objTypeId)
    :CompHostObject(objTypeId)
    ,_processId(0)
    ,_threadId(0)
    ,_runErr{Status::Success}
    ,_cpuCoreAmount(0)
    ,_poller(NULL)
    ,_maxPieceTimeInMicroseconds(TimeDefs::MICRO_SECOND_PER_SECOND) // 默认1秒扫描间隔
    ,_maxPriorityLevel(0)
    ,_maxSleepMilliseconds(TimeDefs::MILLI_SECOND_PER_SECOND)   // 默认1秒
    ,_memoryLogSigno(KERNEL_NS::SignoList::MEMORY_LOG_SIGNO)
{
    if(g_Application == NULL)
        g_Application = this;

    _appStartTime = LibTime::Now();
}

IApplication::~IApplication()
{
    _Clear();

    if(g_Application)
        g_Application = NULL;
}

void IApplication::WaitFinish(Int32 &err)
{
    // #if _DEBUG
        // getchar();
    // #else
    if(!_poller->PrepareLoop())
    {
        err = _runErr;
        g_Log->Error(LOGFMT_OBJ_TAG("application %s prepare loop fail run err:%d.")
                    , _appName.c_str(), _runErr.load());
        return;
    }

    _poller->EventLoop();
    _poller->OnLoopEnd();
    // _lck.Lock();
    // _lck.Wait();
    // _lck.Unlock();

    err = _runErr;

    // #endif
}

// 需要打印app内部组件资源列表以及状态
LibString IApplication::ToString() const
{
    LibString info;

    // TODO: 系统信息
    info
        .AppendFormat("app name:%s, ", _appName.c_str())
        ;

    return info;
}

LibString IApplication::IntroduceStr() const
{
    LibString info;

    info
        .AppendFormat("app:%s \n", _appName.c_str())
        .AppendFormat("path:%s \n", _path.c_str())
        .AppendFormat("processId:%d\n", _processId)
        .AppendFormat("start time:%lld, %s\n", _appStartTime.GetSecTimestamp(), _appStartTime.ToString().c_str())
        .AppendFormat("cpu vendor:%s, ", _cpuVendor.c_str())
        .AppendFormat("brand:%s, ", _cpuBrand.c_str())
        .AppendFormat("core amount:%d, \n", _cpuCoreAmount)
        ;

    return info;
}

void IApplication::Clear()
{
    _Clear();
    CompHostObject::Clear();
}

void IApplication::_Clear()
{
    // TODO:
    _poller = NULL;
}

void IApplication::OnRegisterComps()
{
}

Int32 IApplication::_OnHostInit()
{
    _threadId = SystemUtil::GetCurrentThreadId();
    _appName = SystemUtil::GetCurProgramNameWithoutExt();
    Int32 errCode = SystemUtil::GetProgramPath(true, _path);
    if(errCode != Status::Success)
        g_Log->Warn(LOGFMT_OBJ_TAG("get program path fail errCode:%d"), errCode);

    _processId = SystemUtil::GetCurProcessId();
    _cpuCoreAmount = g_cpu->GetCpuCoreCnt();
//     _cpuVendor = g_cpuFeature->GetVendor();
//     _cpuBrand = g_cpuFeature->GetBrand();

    // windows下禁用最小化和关闭按钮
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        HWND hwnd = GetConsoleWindow();
        HMENU hmenu = GetSystemMenu(hwnd, false);
        RemoveMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style &= ~(WS_MINIMIZEBOX);
        SetWindowLong(hwnd, GWL_STYLE, style);
        SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        ShowWindow(hwnd, SW_SHOWNORMAL);
        DestroyMenu(hmenu);
        ReleaseDC(hwnd, NULL);
    #endif

    g_Log->Info(LOGFMT_OBJ_TAG("app %s init suc."), _appName.c_str());

    return Status::Success;
}

Int32 IApplication::_OnPriorityLevelCompsCreated()
{
    // 设置poller参数
    _poller = KERNEL_NS::TlsUtil::GetDefTls()->_tlsComps->GetPoller();

    // poller 设置
    KERNEL_NS::TimeSlice span(0, 0, _maxPieceTimeInMicroseconds);
    _poller->SetMaxPriorityLevel(_maxPriorityLevel);
    _poller->SetMaxPieceTime(span);
    _poller->SetMaxSleepMilliseconds(_maxSleepMilliseconds);
    // _poller->SetPepareEventWorkerHandler(this, &IApplication::_OnPollerPrepare);
    // _poller->SetEventWorkerCloseHandler(this, &IApplication::_OnPollerWillDestroy);

    return Status::Success;
}

// 所有组件创建完成
Int32 IApplication::_OnCompsCreated()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app %s comps created."), _appName.c_str());
    return Status::Success;
}

// 在组件启动之前
Int32 IApplication::_OnHostWillStart()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app %s host will start."), _appName.c_str());
    return Status::Success;
}

// 组件启动之后
Int32 IApplication::_OnHostStart()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app:%s started."), _appName.c_str());

    return Status::Success;
}

// 在组件willclose之后
void IApplication::_OnHostWillClose()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app %s host will close."), _appName.c_str());
}

// 在组件Close之后
void IApplication::_OnHostClose()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app %s host closed."), _appName.c_str());
}

void IApplication::_OnHostUpdate()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app %s on update."), _appName.c_str());
}

KERNEL_END

