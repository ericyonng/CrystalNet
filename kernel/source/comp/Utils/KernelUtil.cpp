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
#include <kernel/comp/memory/CenterMemoryCollector.h>

KERNEL_NS::LibCpuInfo *g_cpu = NULL;
// KERNEL_NS::CpuFeature *g_cpuFeature = NULL;
KERNEL_NS::MemoryMonitor *g_MemoryMonitor = NULL;
KERNEL_NS::LibString g_LogIniName;
KERNEL_NS::LibString g_LogIniRootPath;

static std::vector<KERNEL_NS::IDelegate<void> *> s_signalCloseHandler;


KERNEL_BEGIN

std::atomic_bool g_KernelInit{false};
std::atomic_bool s_KernelStart{false};
std::atomic_bool s_KernelDestroy{false};

Int32 KernelUtil::Init(ILogFactory *logFactory, const Byte8 *logIniName, const Byte8 *iniPath, const LibString &logContent, const LibString &consoleContent, bool needSignalHandle, Int64 fileSoftLimit, Int64 fileHardLimit)
{
    if(g_KernelInit.exchange(true))
    {
        CRYSTAL_TRACE("kernel is init before logIniName:%s iniPath:%s", logIniName ? logIniName:"NONE", iniPath ? iniPath:"NONE");
        return Status::Success;
    }

    s_KernelStart = false;
    s_KernelDestroy = false;

    // 转入后台
    KERNEL_NS::LibString rootDir = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    // CRYSTAL_TRACE("kernel current root dir:%s", rootDir.c_str());

    // 设置工作目录
    SystemUtil::ChgWorkDir(rootDir);
    
    // 标准输出,标准错误输出重定向到文件 TODO:版本发布情况下在外部开启,内核初始化不需要
    // auto stdiolog = rootDir + "stdio.log";
    // KERNEL_NS::SystemUtil::TurnDaemon(stdiolog, rootDir);

    // 系统大小端识别
    LibEndian::Init();

    // 只支持小端系统
    if(!LibEndian::GetLocalMachineEndianType() != LibEndianType::LittleEndian)
    {
        CRYSTAL_TRACE("system endian type:%s not little endian system, kernel only support little endian!", LibEndianType::ToString(LibEndian::GetLocalMachineEndianType()));
        return Status::SystemUtil_NotLittleEndian;
    }

    // 设置最大文件描述符数量
    Int64 oldSoftLimit = 0;
    Int64 oldHardLimit = 0;

    Int32 err = Status::Success;
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    KERNEL_NS::LibString limitErr;
    err = KERNEL_NS::SystemUtil::GetProcessFileDescriptLimit(KERNEL_NS::LinuxRlimitId::E_RLIMIT_NOFILE, oldSoftLimit, oldHardLimit, limitErr);
    if(err != Status::Success)
    {
        CRYSTAL_TRACE("GetProcessFileDescriptLimit fail %d, %s", err, limitErr.c_str());
        return Status::Failed;
    }

    err = KERNEL_NS::SystemUtil::SetProcessFileDescriptLimit(KERNEL_NS::LinuxRlimitId::E_RLIMIT_NOFILE, fileSoftLimit, fileHardLimit, limitErr);
    if(err != Status::Success)
    {
        CRYSTAL_TRACE("SetProcessFileDescriptLimit fail %d, %s, oldSoftLimit:%lld, oldHardLimit:%lld, will set soft limit:%lld, will set hard limit:%lld"
                , err, limitErr.c_str(), oldSoftLimit, oldHardLimit, fileSoftLimit, fileHardLimit);
        return Status::Failed;
    }

    // core dump 输出到当前程序下
    #endif

    // ini 文件路径
    LibString iniRoot;
    if(!iniPath)
    {
        iniRoot = rootDir + ROOT_DIR_INI_SUB_DIR;
    }
    else
    {
        iniRoot = iniPath;
    }

    // 创建路径
    if(logContent.empty())
        KERNEL_NS::DirectoryUtil::CreateDir(iniRoot);

    // 初始化tls
    if(KERNEL_NS::TlsUtil::GetUtileTlsHandle() == INVALID_TLS_HANDLE)
    {
        CRYSTAL_TRACE("GetUtileTlsHandle fail.");
        return false;
    }

    // 初始化时区
    KERNEL_NS::TimeUtil::SetTimeZone();
    // 变体类型识别初始化
    KERNEL_NS::VariantRtti::InitRttiTypeNames();
    // 初始化id
    KERNEL_NS::LibThreadGlobalId::GenId();
    // 全局内存池 先于所有对象创建
    g_MemoryPool = KERNEL_NS::MemoryPool::GetDefaultInstance();
    err = g_MemoryPool->Init();
    if (err != Status::Success)
    {
        CRYSTAL_TRACE("memory pool init fail err = [%d]", err);
        return false;
    }

    // 主线程初始化
    const auto mainThreadId = SystemUtil::GetCurrentThreadId();
    ThreadTool::OnInit(NULL, NULL, mainThreadId, "main thread tls memory pool");

    // cpu信息
    g_cpu = KERNEL_NS::LibCpuInfo::GetInstance();
    if(!g_cpu->Initialize())
    {
        CRYSTAL_TRACE("cpu init fail.");
        return false;
    }

    // cpu特性
//     g_cpuFeature = KERNEL_NS::CpuFeature::GetInstance();
//     g_cpuFeature->Init();

    // cpu frequancy
    LibCpuFrequency::InitFrequancy();

    // 初始化系统高性能时间
    TimeUtil::InitFastTime();

    // sock error init
    SockErrorMsgUtil::Init();

    // 日志初始化与启动
    g_Log = logFactory->Create();
    if(!g_Log->Init(logIniName, iniRoot.c_str(), logContent, consoleContent))
    {
        CRYSTAL_TRACE("fail init log log file name[%s], ini root dir[%s]", logIniName, iniRoot.c_str());
        return false;
    }

    g_LogIniName = logIniName;
    g_LogIniRootPath = iniRoot;

    // 堆栈
    KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void>> destroyDelg = KERNEL_NS::DelegateFactory::Create(&KernelUtil::Destroy);
    err = KERNEL_NS::BackTraceUtil::InitCrashHandleParams(g_Log, destroyDelg.AsSelf());
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "InitCrashHandleParams fail err=[%d]."), err);
        return err;
    }
    destroyDelg.pop();

    // 内存监控
    g_MemoryMonitor = KERNEL_NS::MemoryMonitor::GetInstance();
    // err = g_MemoryMonitor->Init(60*1000);
    err = g_MemoryMonitor->Init(10*1000);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "g_MemoryMonitor Init fail err=[%d]."), err);
        return err;
    }

    // 初始化网络环境
    err = SocketUtil::InitSocketEnv();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "socket env Init fail err=[%d]."), err);
        return err;
    }

    // 静态成员初始化
    #if CRYSTAL_TARGET_PLATFORM_LINUX
     EpollTcpPoller::InitStatic();
    #endif
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
     IocpTcpPoller::InitStatic();
    #endif

    // 异常信号处理
    if(needSignalHandle)
    {
        err = SignalHandleUtil::Init();
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "signal handle util fail err:%d"), err);
            return err;
        }
    }

    // 信号处理任务
// #if CRYSTAL_TARGET_PLATFORM_LINUX
    if(needSignalHandle)
    {
        auto signalCloseHandler = DelegateFactory::Create(&KernelUtil::_OnSinalOccur);
        SignalHandleUtil::PushAllConcernSignalTask(signalCloseHandler);
    }
// #endif

    g_Log->Sys(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "kernel inited root path:%s, old file soft limit:%lld, old file hard limit:%lld, new file soft limit:%lld, new file hard limit:%lld.")
                , rootDir.c_str(), oldSoftLimit, oldHardLimit, fileSoftLimit, fileHardLimit);

    return Status::Success;
}

void KernelUtil::Start()
{
    if(s_KernelStart.exchange(true))
    {
        CRYSTAL_TRACE("kernel start before.");
        return;
    }

    // 日志启动
    g_Log->Start();
    // 启动垃圾回收线程
    KERNEL_NS::GarbageThread::GetInstence()->Start();
    // 启动内存监控
    g_MemoryMonitor->Start();
    // 启动中央内存收集器
    CenterMemoryCollector::GetInstance()->Start();

    g_Log->Sys(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "kernel started."));

    s_KernelDestroy = false;
}

void KernelUtil::Destroy()
{
    if(s_KernelDestroy.exchange(true))
    {
        CRYSTAL_TRACE("kernel destroy before.");
        return;
    }
 
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

    // 先关闭
    KERNEL_NS::CenterMemoryCollector::GetInstance()->WillClose();

    if (g_MemoryMonitor)
        g_MemoryMonitor->Close();
    g_MemoryMonitor = NULL;

    // 日志关闭
    if(LIKELY(g_Log))
    {
        g_Log->Close();
        // CRYSTAL_DELETE_SAFE(g_Log);
    }
    // g_Log = NULL;
    
    // gc停止
    KERNEL_NS::GarbageThread::GetInstence()->Close();

    // 销毁资源
    ThreadTool::OnDestroy();
    // 关闭内存池全局内存池晚于所有对象销毁
    if(g_MemoryPool)
        g_MemoryPool->Destroy();
    g_MemoryPool = NULL;
    // tls销毁
    KERNEL_NS::TlsUtil::DestroyUtilTlsHandle();

    g_KernelInit = false;
    s_KernelStart = false;

    KERNEL_NS::CenterMemoryCollector::GetInstance()->Close();

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

    // 先关闭
    KERNEL_NS::CenterMemoryCollector::GetInstance()->WillClose();

    if (g_MemoryMonitor)
        g_MemoryMonitor->Close();

    // 日志关闭
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