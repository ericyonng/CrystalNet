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
 * Date: 2021-02-17 15:59:06
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Log/LibLog.h>
#include <kernel/source/comp/Log/SpecifyLog.h>
#include <kernel/source/comp/Log/LogIniCfgMgr.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Utils/DirectoryUtil.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/source/comp/Log/LogConfigTemplate.h>
#include <kernel/comp/Variant/variant_inc.h>
#include <kernel/comp/thread/thread.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Utils/BackTraceUtil.h>
#include <kernel/comp/Utils/SignalHandleUtil.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Timer/Timer.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/Lock/Impl/ConditionLocker.h>
#include <kernel/comp/Poller/Poller.h>

#include "kernel/comp/Coroutines/Runner.h"
#include "kernel/comp/Cpu/LibCpuInfo.h"
#include "kernel/comp/FileMonitor/YamlMemory.h"
#include "kernel/comp/Lock/Impl/CoLocker.h"
#include "kernel/comp/thread/LibEventLoopThreadPool.h"
#include "kernel/comp/Utils/KernelUtil.h"
#include <kernel/comp/ObjLife.h>


KERNEL_BEGIN
    LibLog::LibLog()
    :_isInit{false}
    ,_isStart{false}
    ,_isFinish{false}
    ,_fileMonitor(FileMonitor<LogCfg, YamlDeserializer>::New_FileMonitor())
    ,_curCacheBytes{0}
,_workingNum{0}
    ,_isForceLogDiskAll{false}
{
    
}

LibLog::~LibLog()
{
    Close();
}

bool LibLog::Init(const Byte8 *logConfigFile, const Byte8 *logCfgDir, YamlMemory *yamlMemory)
{
    if (_isInit.exchange(true, std::memory_order_acq_rel))
    {
        CRYSTAL_TRACE("already init log %s", logConfigFile);
        return true;
    }

    // 获取目录
    LibString iniDir;
    const LibString progDir = SystemUtil::GetCurProgRootPath();

    if(!logCfgDir)
    {
        iniDir = progDir + ROOT_DIR_INI_SUB_DIR;
    }
    else
    {
        iniDir = logCfgDir;
    }

    // 路径校验
    LibString cfgFile = iniDir;
    cfgFile += logConfigFile;

    // 不使用内存配置
    if(yamlMemory == NULL)
    {
        CRYSTAL_TRACE("log init log config file:%s", cfgFile.c_str());

        if(UNLIKELY(!FileUtil::IsFileExist(cfgFile.c_str())))
        {
            CRYSTAL_TRACE("cfg file:%s not foud, will auto create log file.", cfgFile.c_str());

            // 创建目录
            if(!DirectoryUtil::CreateDir(iniDir))
            {
                CRYSTAL_TRACE("create ini dir fail :%s", iniDir.c_str());
                return false;
            }

            auto fp = FileUtil::OpenFile(cfgFile.c_str(), true);
            if(!fp)
            {
                CRYSTAL_TRACE("create ini file fail :%s", cfgFile.c_str())
                return false;
            }
            
            const auto &content = LogConfigTemplate::GetLogConfigYamlContent();
            FileUtil::WriteFile(*fp, content.data(), content.size());
            FileUtil::FlushFile(*fp);
            FileUtil::CloseFile(*fp);
        }
    }
    
    // 初始化配置
    if (!_fileMonitor->Init(cfgFile, yamlMemory))
    {
        CRYSTAL_TRACE("log config init fail %s", cfgFile.c_str());
        return false;
    }
    
    // _logConfigMgr = new LogIniCfgMgr();
    // if(!_logConfigMgr->Init(cfgFile.c_str(), logContent, consoleConntent))
    // {
    //     CRYSTAL_TRACE("log config init fail %s", cfgFile.c_str());
    //     return false;
    // }

    // 是否需要开启日志
    auto currentConfig = _fileMonitor->Current();
    if(!currentConfig->LogCommon.IsEnableLog)
    {
        CRYSTAL_TRACE("LOG IS CLOSE!!!");
        return true;
    }

    // 初始化日志对象
    const Int32 maxFileId = currentConfig->GetMaxLogFileId();
    const Int32 logCount = static_cast<Int32>(currentConfig->LogLevelList.size());
    _fileIdIdxRefLog.resize(maxFileId + 1);
    const auto logPath = currentConfig->GetLogPath();
    for(Int32 i = 0; i < logCount; ++i)
    {
        auto &cfg = currentConfig->LogLevelList[i];
        if(cfg.Enable)
        {
            auto newSpecifyLog = SpecifyLog::New_SpecifyLog();
            auto logFileCfg = currentConfig->GetLogFile(cfg.LevelId);

            const Int32 status = newSpecifyLog->Init(logPath, _fileMonitor, cfg.FileId, logFileCfg->ThreadRelationshipId);
            if(status != Status::Success)
            {
                CRYSTAL_TRACE("newSpecifyLog INIT fail log file name=%s, file id:%d logPath=%s, status=%d"
                , logFileCfg ? logFileCfg->FileName.c_str() : "", cfg.FileId, logPath.c_str(), status);

                CRYSTAL_DELETE_SAFE(newSpecifyLog);
                return false;
            }

            // 日志着盘线程绑定关系
            const auto workingNum = g_LibEventLoopThreadPool->GetWorkerNum();
            auto &threads= g_LibEventLoopThreadPool->GetThreads();
            if(workingNum <= newSpecifyLog->GetThreadRelationId())
            {
                g_LibEventLoopThreadPool->MakeThreadEnough(newSpecifyLog->GetThreadRelationId() + 1);
            }
            auto realRelationId = newSpecifyLog->GetThreadRelationId() % threads.size();
            if(_threadRelationLogs.size() <= realRelationId)
                _threadRelationLogs.resize(realRelationId + 1);
            if(_flushLocks.size() <= realRelationId)
                _flushLocks.resize(realRelationId + 1);
            auto flushThread = threads[realRelationId];
            if (UNLIKELY(_flushLocks[realRelationId] == NULL))
            {
                auto coreNum = g_cpu->GetCpuCoreCnt();
                _flushLocks[realRelationId] = new CoLocker(coreNum > 16 ? coreNum : 16);
            }
            auto threadBindLogs = _threadRelationLogs[realRelationId];
            if(UNLIKELY(!threadBindLogs))
            {
                threadBindLogs = new std::vector<SpecifyLog *>;
                _threadRelationLogs[realRelationId] = threadBindLogs;
            }
            threadBindLogs->push_back(newSpecifyLog);

            newSpecifyLog->BindWakeupFlush(_flushLocks[realRelationId]);
            _fileIdIdxRefLog[cfg.FileId] = newSpecifyLog;
        }
    }

    _rootDirName = logPath;

    _isForceDiskFlag.resize(_flushLocks.size());
    for(Int32 idx = 0; idx < static_cast<Int32>(_isForceDiskFlag.size()); ++idx)
    {
        _isForceDiskFlag[idx] = new std::atomic_bool;
        _isForceDiskFlag[idx]->store(false, std::memory_order_release);
    }

    // // 异常信号处理任务 统一走kernel destroy
    // auto abnormalHandler = DelegateFactory::Create(this, &LibLog::_OnAbnormalSignalHandler);
    // SignalHandleUtil::PushAllConcernSignalTask(abnormalHandler);

    // CRYSTAL_TRACE("log config init suc log cfg file[%s], root dir name[%s]", cfgFile.c_str(), _rootDirName.c_str());

    return true;
}

void LibLog::Start()
{
    if(UNLIKELY(!_isInit.load(std::memory_order_acquire) || _isFinish.load(std::memory_order_acquire)))
    {
        CRYSTAL_TRACE("have no init cant start");
        return;
    }

    if(_isStart.exchange(true, std::memory_order_acq_rel))
    {
        CRYSTAL_TRACE("LIB LOG has already start.");
        return;
    }

    for(auto &log:_fileIdIdxRefLog)
    {
        if(log)
        {
            auto status = log->Start();
            if(status != Status::Success)
            {
                CRYSTAL_TRACE("log start fail log nam=%s status=%d"
                , log->GetLogName().c_str(), status);
            }
        }
    }

    auto &threads = g_LibEventLoopThreadPool->GetThreads();
    const Int32 threadCount = static_cast<Int32>(_flushLocks.size());
    for(Int32 idx = 0; idx < threadCount; ++idx)
    {
        auto lck = _flushLocks[idx];
        auto t = threads[idx];

        // 按lck的为准, 不一定所有线程池的线程都是日志线程
        if (lck)
        {
            t->Send([idx, this]() mutable
            {
                KERNEL_NS::PostCaller([idx, this]() mutable ->CoTask<>
                {
                    ObjLife<std::atomic<Int32>> workingLife(_workingNum);
                    
                    CRYSTAL_TRACE("_OnLogThreadFlush LogThread in ThreadPool %llu", KERNEL_NS::SystemUtil::GetCurrentThreadId());

                    KERNEL_NS::SmartPtr<Variant, KERNEL_NS::AutoDelMethods::CustomDelete> params = Variant::NewThreadLocal_Variant();
                    params.SetClosureDelegate([](void *p)
                    {
                        Variant::DeleteThreadLocal_Variant(KERNEL_NS::KernelCastTo<Variant>(p));
                    });
                    
                    params->BecomeDict();
                    (*params)[1] = idx;

                    // 调用Log着盘
                    co_await _OnLogThreadFlush(params.AsSelf());

                    CRYSTAL_TRACE("_OnLogThreadFlush log coroutine quit idx:%d", idx);
                });
            });
        }
    }

    // CRYSTAL_TRACE("LIB LOG START FINISH.");
}

void LibLog::Close()
{
    if(_isFinish.exchange(true, std::memory_order_acq_rel))
        return;

    _isInit.store(false, std::memory_order_release);
    _isStart.store(false, std::memory_order_release);

    // CRYSTAL_TRACE("Close log start");

    // 退出协程
    const Int32 threadCount = static_cast<Int32>(_flushLocks.size());
    for(Int32 idx = 0; idx < threadCount; ++idx)
    {
        auto lck = _flushLocks[idx];
        if(LIKELY(lck))
        {
            lck->Quit();
        }
    }

    CRYSTAL_TRACE("wait all log coroutine quit...");

    for(Int32 idx = 0; idx < threadCount; ++idx)
    {
        auto lck = _flushLocks[idx];
        if(LIKELY(lck))
        {
            while (lck->HasWaiter())
            {
                lck->Broadcast();
                CRYSTAL_TRACE("wait log coroutine quit idx:%d ...", idx);
            }
        }
    }

    // 等所有线程退出
    while (_workingNum.load(std::memory_order_acquire) != 0)
    {
        CRYSTAL_TRACE("wait all log coroutine quit working num:%d", _workingNum.load(std::memory_order_acquire));
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
    }

    CRYSTAL_TRACE("all log coroutine quit success...");

    // CRYSTAL_TRACE("Close log thread finish close");

    const Int32 flagCount = static_cast<Int32>(_isForceDiskFlag.size());
    for (Int32 idx = 0; idx < flagCount; ++idx)
    {
        auto flag = _isForceDiskFlag[idx];
        if (flag)
        {
            delete flag;
            _isForceDiskFlag[idx] = NULL;
        }
    }

    for(auto &log:_fileIdIdxRefLog)
    {
        if(log)
        {
            log->Close();
            SpecifyLog::Delete_SpecifyLog(log);
        }
    }

    // CRYSTAL_TRACE("Close log close log files");

    ContainerUtil::DelContainer(_flushLocks);
    ContainerUtil::DelContainer(_threadRelationLogs);

    _fileIdIdxRefLog.clear();

    if (_fileMonitor)
        FileMonitor<LogCfg, YamlDeserializer>::Delete_FileMonitor(_fileMonitor);

    _fileMonitor = NULL;
}

void LibLog::FlushAll()
{
    for(auto &lck:_flushLocks)
    {
        if(LIKELY(lck))
            lck->Sinal();
    }
}

void LibLog::ForceLogToDiskAll()
{
    _isForceLogDiskAll.store(false, std::memory_order_release);

    for(Int32 idx = 0; idx <  static_cast<Int32>(_isForceDiskFlag.size()); ++idx)
    {
        if(_isForceDiskFlag[idx])
            _isForceDiskFlag[idx]->store(false, std::memory_order_release);
    }

    CRYSTAL_TRACE("will force log to disk all..");
    _isForceLogDiskAll.store(true, std::memory_order_release);

    FlushAll();

    const UInt64 currentThreadId = SystemUtil::GetCurrentThreadId();
    auto &threads = g_LibEventLoopThreadPool->GetThreads();
    while(true)
    {
        SystemUtil::ThreadSleep(0);
        const Int32 threadCount = static_cast<Int32>(_flushLocks.size());
        Int32 allToDiskThreadCount = 0;
        Int32 totalThreadCount = 0;
        for(Int32 idx = 0; idx < threadCount; ++idx)
        {
            if(_flushLocks[idx])
            {
                ++totalThreadCount;
                if(currentThreadId == threads[idx]->GetPollerNoAsync()->GetWorkerThreadId())
                {// 本线程就是日志线程直接着盘
                    auto logs = _threadRelationLogs[idx];
                    const Int32 logCount = static_cast<Int32>(logs->size());
                    _OnLogFlush(*logs, idx, logCount);
                }
                else
                {
                    if(!_isForceDiskFlag[idx]->load(std::memory_order_acquire))
                        break;
                }

                ++allToDiskThreadCount;
            }
        }

        // 所有日志未着盘
        if(allToDiskThreadCount < totalThreadCount)
        {
            // CRYSTAL_TRACE("allToDiskThreadCount :%d, threadCount:%d", allToDiskThreadCount, totalThreadCount);
            continue;
        }

        CRYSTAL_TRACE("all log to disk finish allToDiskThreadCount:%d, totalThreadCount:%d, currentThreadId:%llu"
                    , allToDiskThreadCount, totalThreadCount, currentThreadId);
        break;
    }

    CRYSTAL_TRACE("all log to disk finish currentThreadId:%llu", currentThreadId);

    _isForceLogDiskAll.store(false, std::memory_order_release);
}

const LibString &LibLog::GetLogRootPath() const
{
    return _rootDirName;
}

bool LibLog::IsStart() const
{
    return _isStart.load(std::memory_order_acquire);
}

bool LibLog::IsLogOpen() const
{
    return (!_isFinish.load(std::memory_order_acquire)) && _fileMonitor && _fileMonitor->Current()->IsEnableLog();
}

void LibLog::UnInstallAfterLogHookFunc(Int32 level, const IDelegate<void> *delegate)
{
    if(UNLIKELY(!IsLogOpen()))
        return;

    if(UNLIKELY(_isStart.load(std::memory_order_acquire)))
    {
        CRYSTAL_TRACE("log has already start UnInstallAfterLogHookFunc");
        return;
    }

    // 能够调用uninstall 说明已经install过了
    auto logCfg = _fileMonitor->Current()->GetLogFile(level);
    auto specifyLog = _fileIdIdxRefLog[logCfg->FileId];
    specifyLog->UnInstallAfterLogHookFunc(level, delegate);
}

void LibLog::UnInstallBeforeLogHookFunc(Int32 level, const IDelegate<void, LogData *> *delegate)
{
    if(UNLIKELY(!IsLogOpen()))
        return;

    if(UNLIKELY(_isStart.load(std::memory_order_acquire)))
    {
        CRYSTAL_TRACE("log has already start UnInstallBeforeLogHookFunc");
        return;
    }

    // 能够调用uninstall 说明已经install过了
    auto logCfg = _fileMonitor->Current()->GetLogFile(level);
    auto specifyLog = _fileIdIdxRefLog[logCfg->FileId];
    specifyLog->UnInstallBeforeLogHookFunc(level, delegate);
}

const void *LibLog::GetFileMonitor() const
{
    return _fileMonitor;
}

    
void LibLog::_InstallAfterLogHookFunc(const LogLevelInfoCfg *levelCfg, IDelegate<void> *delegate)
{
    // 外部已排除是否开启日志
    auto specifyLog = _fileIdIdxRefLog[levelCfg->FileId];
    specifyLog->InstallAfterHook(levelCfg->LevelId, delegate);
}

void LibLog::_InstallBeforeLogHookFunc(const LogLevelInfoCfg *levelCfg, IDelegate<void, LogData *> *delegate)
{
    // 外部已排除是否开启日志
    auto specifyLog = _fileIdIdxRefLog[levelCfg->FileId];
    specifyLog->InstallBeforeHook(levelCfg->LevelId, delegate);
}

SmartPtr<LogCfg, AutoDelMethods::Release> LibLog::GetLogCfg() const
{
    return _fileMonitor->Current();
}

void LibLog::_WriteLog(const LogLevelInfoCfg *levelCfg, LogData *logData)
{
    auto specifyLog = _fileIdIdxRefLog[levelCfg->FileId];

    // 开启打印堆栈信息
    if(levelCfg->PRINT_STACK_TRACE_BACK)
        logData->_logInfo << "\nStack Traceback:\n" << BackTraceUtil::CrystalCaptureStackBackTrace() << "\n";

    auto calcBytes = static_cast<Int64>(logData->CalcBytes());
    Int64 curAllBytes = _curCacheBytes.fetch_add(calcBytes, std::memory_order_acq_rel) + calcBytes;
    specifyLog->WriteLog(*levelCfg, logData);

    // 缓存超过阈值强制着盘
    auto currentCfgs = _fileMonitor->Current();
    if(UNLIKELY(curAllBytes >= currentCfgs->GetMaxLogCacheBytes()))
    {
        _curCacheBytes.store(0, std::memory_order_release);
        FlushAll();
    }
}

CoTask<> LibLog::_OnLogThreadFlush(Variant *params)
{
    const Int32 idx = (*params)[1].AsInt32();
    auto logs = _threadRelationLogs[idx];
    auto lck = _flushLocks[idx];
    const Int32 logCount = static_cast<Int32>(logs->size());
    LibString logNameList;
    for(auto &log:*logs)
    {
        if(!log->GetLogName().empty())
            logNameList.AppendFormat("%s|", log->GetLogName().c_str());
    }

    auto currentConfgs = _fileMonitor->Current();
    UInt64 intervalMs = static_cast<UInt64>(currentConfgs->GetIntervalMs().GetTotalMilliSeconds());
    // CRYSTAL_TRACE("log file name thread start id[%llu] relation id = [%d], bind log file name=[%s] intervalMs[%llu]"
    // , SystemUtil::GetCurrentThreadId(), idx, logNameList.c_str(), intervalMs);

    const Int32 holdCount = 10;
    Int32 dynamicHoldCount = 0;

    // 定时管理
    auto poller = KERNEL_NS::TlsUtil::GetPoller();

    while (true)
    {
        if(UNLIKELY(poller->IsQuit() || lck->IsQuit()))
        {
            _OnLogFlush(*logs, idx, logCount);
            break;
        }

        // 写日志
        _OnLogFlush(*logs, idx, logCount);

        // 
        if (intervalMs != 0)
            co_await lck->TimeWait(intervalMs);

        // 每隔10次重新算一次时间间隔
        if(dynamicHoldCount <= 0)
        {// 重新计算刷新间隔
            UInt64 logDataCount = 0;
            for(auto &log : * logs)
                logDataCount += log->GetLogCount();

            intervalMs = _GetDynamicInterval(logDataCount);
            dynamicHoldCount = holdCount;
        }
        else
        {
            --dynamicHoldCount;
        }

        // CRYSTAL_TRACE("log file name thread wake up file thread relation id:[%d]", idx);
    }

    // CRYSTAL_TRACE("log thread close relation id = [%d], thread id=[%llu] file list name:%s", idx, SystemUtil::GetCurrentThreadId(), logNameList.c_str());
}

void LibLog::_OnLogFlush(std::vector<SpecifyLog *> &logs, Int32 threadRelationIdx, Int32 logCount)
{
    for(Int32 i = 0; i < logCount; ++i)
    {
        auto log = logs[i];
        log->_OnThreadWriteLog();
    }

    // 强制落盘
    if(_isForceLogDiskAll.load(std::memory_order_acquire))
    {
        for(Int32 i = 0; i < logCount; ++i)
        {
            auto log = logs[i];
            log->ForceToDisk();
        }

        _isForceDiskFlag[threadRelationIdx]->store(true, std::memory_order_release);
    }

    // CRYSTAL_TRACE("_OnLogFlush logCount:[%d]", logCount);
}

void LibLog::StopAllLogThreadAndFallingDisk()
{
    CRYSTAL_TRACE("StopAllLogThreadAndFallingDisk start");
    // 关闭线程
    const Int32 threadCount = static_cast<Int32>(_flushLocks.size());
    for(Int32 idx = 0; idx < threadCount; ++idx)
    {
        auto lck = _flushLocks[idx];
        if(LIKELY(lck))
        {
            lck->Quit();
        }
    }

    CRYSTAL_TRACE("StopAllLogThreadAndFallingDisk quit coroutine...");

    for(Int32 idx = 0; idx < threadCount; ++idx)
    {
        auto lck = _flushLocks[idx];
        if(LIKELY(lck))
        {
            while (lck->HasWaiter())
            {
                lck->Broadcast();
                CRYSTAL_TRACE("StopAllLogThreadAndFallingDisk wait log coroutine quit idx:%d...", idx);
            }
        }
    }

    while (_workingNum.load(std::memory_order_acquire) != 0)
    {
        CRYSTAL_TRACE("StopAllLogThreadAndFallingDisk wait all log coroutine quit working num:%d", _workingNum.load(std::memory_order_acquire));
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
    }
    
    CRYSTAL_TRACE("StopAllLogThreadAndFallingDisk finish close log coroutine");


    // 关闭文件并重新开启
    for(auto &log:_fileIdIdxRefLog)
    {
        if(log)
            log->CloseAndReopen();
    }

    CRYSTAL_TRACE("StopAllLogThreadAndFallingDisk close and reopen log");
}

void LibLog::_OnAbnormalSignalHandler()
{
    g_Log->Warn(LOGFMT_OBJ_TAG("abnormal signal occur and will falling all logs to disk."));

    FlushAll();
    StopAllLogThreadAndFallingDisk();
}


KERNEL_END
