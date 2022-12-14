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
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/Utils.h>
#include <kernel/comp/Event/event_inc.h>
#include <kernel/comp/Variant/variant_inc.h>
#include <kernel/comp/App/BaseAppOption.h>
#include <kernel/comp/Service/Service.h>
#include <kernel/comp/Cpu/cpu.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IApplication);

IApplication::IApplication()
    :_processId(0)
    ,_threadId(0)
    ,_runErr{Status::Success}
    ,_cpuCoreAmount(0)
{
    _appStartTime = LibTime::Now();
}

IApplication::~IApplication()
{
    _Clear();
}

void IApplication::WaitFinish(Int32 &err)
{
    // #if _DEBUG
        // getchar();
    // #else
        _lck.Lock();
        _lck.Wait();
        _lck.Unlock();

        err = _runErr;
    // #endif
}

void IApplication::SinalFinish(Int32 err)
{
    _runErr = err;
    _lck.Sinal();
}

// ????????????app????????????????????????????????????
LibString IApplication::ToString() const
{
    LibString info;

    // TODO: ????????????
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

    // windows?????????????????????????????????
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

// ????????????????????????
Int32 IApplication::_OnCompsCreated()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app %s comps created."), _appName.c_str());
    return Status::Success;
}

// ?????????????????????
Int32 IApplication::_OnHostWillStart()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app %s host will start."), _appName.c_str());
    return Status::Success;
}

// ??????????????????
Int32 IApplication::_OnHostStart()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app:%s started."), _appName.c_str());

    return Status::Success;
}

// ?????????willclose??????
void IApplication::_OnHostWillClose()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app %s host will close."), _appName.c_str());
}

// ?????????Close??????
void IApplication::_OnHostClose()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app %s host closed."), _appName.c_str());
}

void IApplication::_OnHostUpdate()
{
    g_Log->Info(LOGFMT_OBJ_TAG("app %s on update."), _appName.c_str());
}


KERNEL_END

