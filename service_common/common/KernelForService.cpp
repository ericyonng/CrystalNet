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
 * Date: 2026-09-09 12:36:36
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service_common/common/KernelForService.h>
#include <kernel/comp/Utils/KernelUtil.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Utils/BitUtil.h>

#include <kernel/comp/Delegate/IDelegate.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Utils/SockErrorMsgUtil.h>
#include <kernel/comp/Cpu/cpu.h>
#include <kernel/comp/Utils/BackTraceUtil.h>
#include <kernel/comp/Utils/SocketUtil.h>
#include <kernel/comp/Utils/SignalHandleUtil.h>
#include <curl/curl.h>
#include <kernel/common/rdtsc.h>

#include "kernel/comp/Utils/BitUtil.h"

namespace
{
    class LogFactory : public KERNEL_NS::ILogFactory
    {
    public:
        virtual KERNEL_NS::ILog *Create()
        {
            return new KERNEL_NS::LibLog();
        }
    };
}

SERVICE_COMMON_BEGIN

std::atomic<UInt64> s_KernelFlags {0};

Int32 KernelForService::Init(int argc, char const *argv[], KERNEL_NS::YamlMemory *yamlMemory, const char *yamlPartPath, const char *logFilaName, UInt64 flags, bool needSignalHandle)
{
    // 1. 初始化
    KERNEL_NS::ParamsInfo params;
    KERNEL_NS::LibString errParamsInfo;
    KERNEL_NS::LibString sucParamsInfo;
    Int32 paramNum = 0;
    if(argv)
        paramNum = KERNEL_NS::ParamsHandler::GetParams(argc, argv, params, sucParamsInfo, errParamsInfo);

    LogFactory logFactory;
    KERNEL_NS::LibString programPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    KERNEL_NS::LibString logPath;
    logPath = programPath + yamlPartPath;
    KERNEL_NS::SystemUtil::GetProgramPath(true, programPath);
    
    // 转入后台
    KERNEL_NS::LibString rootDir = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    // CRYSTAL_TRACE("kernel current root dir:%s", rootDir.c_str());

    // 设置工作目录
    if (KERNEL_NS::BitUtil::IsSet(flags, KernelFlags::CHANGE_WORK_DIR))
        KERNEL_NS::SystemUtil::ChgWorkDir(rootDir);

    // 设置最大文件描述符数量
    Int64 oldSoftLimit = 0;
    Int64 oldHardLimit = 0;

    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (KERNEL_NS::BitUtil::IsSet(flags, KernelFlags::MODIFY_FILE_DESC_LIMIT))
    {
        KERNEL_NS::LibString limitErr;
        auto err = KERNEL_NS::SystemUtil::GetProcessFileDescriptLimit(KERNEL_NS::LinuxRlimitId::E_RLIMIT_NOFILE, oldSoftLimit, oldHardLimit, limitErr);
        if(err != Status::Success)
        {
            CRYSTAL_TRACE("GetProcessFileDescriptLimit fail %d, %s", err, limitErr.c_str());
            return Status::Failed;
        }

        if(params._fileSoftLimit < 0)
            params._fileSoftLimit = 1024000;
        if(params._fileHardLimit < 0)
            params._fileHardLimit = 1024000;

        err = KERNEL_NS::SystemUtil::SetProcessFileDescriptLimit(KERNEL_NS::LinuxRlimitId::E_RLIMIT_NOFILE, params._fileSoftLimit, params._fileHardLimit, limitErr);
        if(err != Status::Success)
        {
            CRYSTAL_TRACE("SetProcessFileDescriptLimit fail %d, %s, oldSoftLimit:%lld, oldHardLimit:%lld, will set soft limit:%lld, will set hard limit:%lld"
                    , err, limitErr.c_str(), oldSoftLimit, oldHardLimit, params._fileSoftLimit, params._fileHardLimit);
            return Status::Failed;
        }

        // core dump 输出到当前程序下
    }
    #endif

    // 初始化时区
    KERNEL_NS::TimeUtil::SetTimeZone(KERNEL_NS::BitUtil::IsSet(flags, KernelFlags::MODIFY_SYSTEM_TIME_ZONE));

    // 异常信号处理
    if (KERNEL_NS::BitUtil::IsSet(flags, KernelFlags::CATCH_ABNORMAL_SIGNAL))
    {
        if(needSignalHandle)
        {
            auto err = KERNEL_NS::SignalHandleUtil::Init();
            if(err != Status::Success)
            {
                CRYSTAL_TRACE("signal handle util fail err:%d"), err);
                return err;
            }
        }
    }

    // 信号处理任务
    // #if CRYSTAL_TARGET_PLATFORM_LINUX
    if (KERNEL_NS::BitUtil::IsSet(flags, KernelFlags::SET_SIGNAL_PROCESSOR))
    {
        if(needSignalHandle)
        {
            auto signalCloseHandler = KERNEL_NS::DelegateFactory::Create(&KERNEL_NS::KernelUtil::_OnSinalOccur);
            KERNEL_NS::SignalHandleUtil::PushAllConcernSignalTask(signalCloseHandler);
        }
    }
    
    Int32 err = KERNEL_NS::KernelUtil::Init(&logFactory, logFilaName, logPath.c_str(), yamlMemory);
    if(err != Status::Success)
    {
        CRYSTAL_TRACE("kernel init fail err:%d", err);
        return -1;
    }

    // 堆栈
    if (KERNEL_NS::BitUtil::IsSet(flags, KernelFlags::MODIFY_CRASH_HOOK))
    {
        KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void>> destroyDelg = KERNEL_NS::DelegateFactory::Create(&KernelForService::Destroy);
        err = KERNEL_NS::BackTraceUtil::InitCrashHandleParams(g_Log, destroyDelg.AsSelf());
        if(err != Status::Success)
        {
            CLOG_ERROR_GLOBAL(KernelForService, "InitCrashHandleParams fail err=[%d].", err);
            return err;
        }
        destroyDelg.pop();
    }

        
    // 初始化网络环境
    if (KERNEL_NS::BitUtil::IsSet(flags, KernelFlags::INIT_SOCKET_ENV))
    {
        err = KERNEL_NS::SocketUtil::InitSocketEnv();
        if(err != Status::Success)
        {
            CLOG_ERROR_GLOBAL(KernelForService, "socket env Init fail err=[%d].", err);
            return err;
        }
    }

    if (KERNEL_NS::BitUtil::IsSet(flags, KernelFlags::INIT_CURL))
    {
        // 初始化curl全局
        auto curlCode = ::curl_global_init(CURL_GLOBAL_DEFAULT);
        if(curlCode != CURLE_OK)
        {
            CLOG_ERROR_GLOBAL(KernelForService, "curl init fail(%d):%s", (Int32)curlCode, curl_easy_strerror(curlCode));
            return Status::Failed;
        }
    }


    s_KernelFlags.store(flags, std::memory_order_release);

    CLOG_INFO_GLOBAL(KernelForService, "Kernel init paramNum:%d.\nsucParamsInfo:\n%s, errParamsInfo:\n%s flags:%llx.", paramNum, sucParamsInfo.c_str(), errParamsInfo.c_str(), flags);

    return Status::Success;
}

void KernelForService::Start()
{
    KERNEL_NS::KernelUtil::Start();
}

void KernelForService::Destroy()
{
    auto flags = s_KernelFlags.load(std::memory_order_acquire);

    if (KERNEL_NS::BitUtil::IsSet(flags, KernelFlags::INIT_SOCKET_ENV))
        KERNEL_NS::SocketUtil::ClearSocketEnv();

    // 清理curl资源
    if (KERNEL_NS::BitUtil::IsSet(flags, KernelFlags::INIT_CURL))
        curl_global_cleanup();

    KERNEL_NS::KernelUtil::Destroy();
}

void KernelForService::DefaultOnSignalClose()
{
    KernelForService::Destroy();
}


SERVICE_COMMON_END