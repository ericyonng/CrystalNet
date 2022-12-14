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
 * Date: 2021-09-11 17:51:27
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/KernelUtil.h>
#include <kernel/comp/thread/thread.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/SockErrorMsgUtil.h>
#include <kernel/comp/Cpu/cpu.h>
#include <kernel/comp/Endian/LibEndian.h>
#include <kernel/comp/Utils/DirectoryUtil.h>
#include <kernel/comp/Utils/BackTraceUtil.h>
#include <kernel/comp/Utils/SocketUtil.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/EpollTcpPoller.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/IocpTcpPoller.h>
#include <kernel/comp/Utils/SignalHandleUtil.h>

KERNEL_NS::LibCpuInfo *g_cpu = NULL;
// KERNEL_NS::CpuFeature *g_cpuFeature = NULL;
KERNEL_NS::MemoryMonitor *g_MemoryMonitor = NULL;
KERNEL_NS::LibString g_LogIniName;
KERNEL_NS::LibString g_LogIniRootPath;

static std::vector<KERNEL_NS::IDelegate<void> *> s_signalCloseHandler;

KERNEL_BEGIN

Int32 KernelUtil::Init(ILogFactory *logFactory, const Byte8 *logIniName, const Byte8 *iniPath, const LibString &logContent, const LibString &consoleContent)
{
    // ????????????
    KERNEL_NS::LibString rootDir = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    // CRYSTAL_TRACE("kernel current root dir:%s", rootDir.c_str());

    // ??????????????????
    SystemUtil::ChgWorkDir(rootDir);
    
    // ????????????,???????????????????????????????????? TODO:????????????????????????????????????,????????????????????????
    // auto stdiolog = rootDir + "stdio.log";
    // KERNEL_NS::SystemUtil::TurnDaemon(stdiolog, rootDir);

    // ?????????????????????
    LibEndian::Init();

    // ?????????????????????
    if(!LibEndian::GetLocalMachineEndianType() != LibEndianType::LittleEndian)
    {
        CRYSTAL_TRACE("system endian type:%s not little endian system, kernel only support little endian!", LibEndianType::ToString(LibEndian::GetLocalMachineEndianType()));
        return Status::SystemUtil_NotLittleEndian;
    }

    // ini ????????????
    LibString iniRoot;
    if(!iniPath)
    {
        iniRoot = rootDir + ROOT_DIR_INI_SUB_DIR;
    }
    else
    {
        iniRoot = iniPath;
    }

    // ????????????
    if(logContent.empty())
        KERNEL_NS::DirectoryUtil::CreateDir(iniRoot);

    // ?????????tls
    if(KERNEL_NS::TlsUtil::GetUtileTlsHandle() == INVALID_TLS_HANDLE)
    {
        CRYSTAL_TRACE("GetUtileTlsHandle fail.");
        return false;
    }

    // ???????????????
    KERNEL_NS::TimeUtil::SetTimeZone();
    // ???????????????????????????
    KERNEL_NS::VariantRtti::InitRttiTypeNames();
    // ?????????id
    KERNEL_NS::LibThreadGlobalId::GenId();
    // ??????????????? ????????????????????????
    g_MemoryPool = KERNEL_NS::MemoryPool::GetDefaultInstance();
    auto err = g_MemoryPool->Init();
    if (err != Status::Success)
    {
        CRYSTAL_TRACE("memory pool init fail err = [%d]", err);
        return false;
    }

    // ??????????????????
    const auto mainThreadId = SystemUtil::GetCurrentThreadId();
    ThreadTool::OnInit(NULL, NULL, mainThreadId, "main thread tls memory pool");

    // cpu??????
    g_cpu = KERNEL_NS::LibCpuInfo::GetInstance();
    if(!g_cpu->Initialize())
    {
        CRYSTAL_TRACE("cpu init fail.");
        return false;
    }

    // cpu??????
//     g_cpuFeature = KERNEL_NS::CpuFeature::GetInstance();
//     g_cpuFeature->Init();

    // cpu frequancy
    LibCpuFrequency::InitFrequancy();

    // sock error init
    SockErrorMsgUtil::Init();

    // ????????????????????????
    g_Log = logFactory->Create();
    if(!g_Log->Init(logIniName, iniRoot.c_str(), logContent, consoleContent))
    {
        CRYSTAL_TRACE("fail init log log file name[%s], ini root dir[%s]", logIniName, iniRoot.c_str());
        return false;
    }

    g_LogIniName = logIniName;
    g_LogIniRootPath = iniRoot;

    // ??????
    KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void>> destroyDelg = KERNEL_NS::DelegateFactory::Create(&KernelUtil::Destroy);
    err = KERNEL_NS::BackTraceUtil::InitCrashHandleParams(g_Log, destroyDelg.AsSelf());
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "InitCrashHandleParams fail err=[%d]."), err);
        return err;
    }
    destroyDelg.pop();

    // ????????????
    g_MemoryMonitor = KERNEL_NS::MemoryMonitor::GetInstance();
    err = g_MemoryMonitor->Init(60*1000);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "g_MemoryMonitor Init fail err=[%d]."), err);
        return err;
    }

    // ?????????????????????
    err = SocketUtil::InitSocketEnv();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "socket env Init fail err=[%d]."), err);
        return err;
    }

    // ?????????????????????
    #if CRYSTAL_TARGET_PLATFORM_LINUX
     EpollTcpPoller::InitStatic();
    #endif
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
     IocpTcpPoller::InitStatic();
    #endif

    // ??????????????????
    err = SignalHandleUtil::Init();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "signal handle util fail err:%d"), err);
        return err;
    }

    // ??????????????????
// #if CRYSTAL_TARGET_PLATFORM_LINUX
    auto signalCloseHandler = DelegateFactory::Create(&KernelUtil::_OnSinalOccur);
    SignalHandleUtil::PushAllConcernSignalTask(signalCloseHandler);
// #endif

    g_Log->Sys(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "kernel inited root path:%s."), rootDir.c_str());

    return Status::Success;
}

void KernelUtil::Start()
{
    // ????????????
    g_Log->Start();
    // ????????????????????????
    KERNEL_NS::GarbageThread::GetInstence()->Start();
    // ??????????????????
    g_MemoryMonitor->Start();

    g_Log->Sys(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "kernel started."));
}

void KernelUtil::Destroy()
{
    if(LIKELY(g_Log))
    {
        g_Log->Sys(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "will destroy kernel log addr:[%p], log IsStart[%d], memory pool addr:[%p], Statistics addr:[%p]...")
        , g_Log, g_Log->IsStart(), g_MemoryPool,  g_MemoryMonitor->GetStatistics());
    }

    // CRYSTAL_TRACE("will destroy kernel log addr:[%p], log IsStart[%d], memory pool addr:[%p], Statistics addr:[%p]..."
    // , g_Log, g_Log && g_Log->IsStart(), g_MemoryPool,  g_MemoryPool ? g_MemoryMonitor->GetStatistics() : NULL);

    SocketUtil::ClearSocketEnv();

    // if(LIKELY(g_Log))
    //     g_Log->Sys(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "comp will destroy."));

    if (g_MemoryMonitor)
        g_MemoryMonitor->Close();
    g_MemoryMonitor = NULL;

    // ????????????
    if(LIKELY(g_Log))
    {
        g_Log->Close();
        CRYSTAL_DELETE_SAFE(g_Log);
    }
    g_Log = NULL;
    
    // gc??????
    KERNEL_NS::GarbageThread::GetInstence()->Close();

    // ????????????
    ThreadTool::OnDestroy();
    // ??????????????????????????????????????????????????????
    if(g_MemoryPool)
        g_MemoryPool->Destroy();
    g_MemoryPool = NULL;
    // tls??????
    KERNEL_NS::TlsUtil::DestroyUtilTlsHandle();

    // CRYSTAL_TRACE("kernel destroy finish.");
}

void KernelUtil::OnSignalClose()
{
    KernelUtil::Destroy();
}

void KernelUtil::OnAbnormalClose()
{
    if(LIKELY(g_Log))
    {
        g_Log->Sys(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "will close kernel log abnormal addr:[%p], log IsStart[%d], memory pool addr:[%p], Statistics addr:[%p]...")
        , g_Log, g_Log->IsStart(), g_MemoryPool,  g_MemoryMonitor->GetStatistics());
    }

    CRYSTAL_TRACE("will abnormal close kernel log addr:[%p], log IsStart[%d], memory pool addr:[%p], Statistics addr:[%p]..."
    , g_Log, g_Log && g_Log->IsStart(), g_MemoryPool,  g_MemoryPool ? g_MemoryMonitor->GetStatistics() : NULL);

    if (g_MemoryMonitor)
        g_MemoryMonitor->Close();

    // ????????????
    if(LIKELY(g_Log))
    {
        g_Log->ForceLogToDiskAll();
    }
    
    CRYSTAL_TRACE("kernel abnormal close finish.");
}

void KernelUtil::InstallSignalCloseHandler(IDelegate<void> *task)
{
    s_signalCloseHandler.push_back(task);
}

void KernelUtil::_OnSinalOccur()
{
    for (auto &delg : s_signalCloseHandler)
        delg->Invoke();
    ContainerUtil::DelContainer2(s_signalCloseHandler);
}

KERNEL_END