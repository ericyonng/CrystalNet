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

KERNEL_BEGIN

LibLog::LibLog()
    :_isInit{false}
    ,_isStart{false}
    ,_isFinish{false}
    ,_logConfigMgr(NULL)
    ,_curCacheBytes{0}
    ,_logTimerIntervalMs(0)
{
    
}

LibLog::~LibLog()
{
    Close();
}

bool LibLog::Init(const Byte8 *logConfigFile, const Byte8 *logCfgDir)
{
    if (_isInit.exchange(true))
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

    // 配置文件不存在则自动创建
    if(UNLIKELY(!FileUtil::IsFileExist(cfgFile.c_str())))
    {
        // 创建目录
        if(!DirectoryUtil::CreateDir(iniDir))
        {
            CRYSTAL_TRACE("create ini dir fail :%s", iniDir.c_str());
            return false;
        }

        auto fp = FileUtil::OpenFile(cfgFile.c_str(), true);
        if(!fp)
        {
            CRYSTAL_TRACE("create ini file fail :%s", cfgFile.c_str());
            return false;
        }
        
        const auto &content = LogConfigTemplate::GetLogConfigIniContent();
        FileUtil::WriteFile(*fp, content.data(), content.size());
        FileUtil::FlushFile(*fp);
        FileUtil::CloseFile(*fp);
    }

    // 初始化配置
    _logConfigMgr = new LogIniCfgMgr();
    if(!_logConfigMgr->Init(cfgFile.c_str()))
    {
        CRYSTAL_TRACE("log config init fail %s", cfgFile.c_str());
        return false;
    }

    // 是否需要开启日志
    if(!_logConfigMgr->IsEnableLog())
    {
        CRYSTAL_TRACE("LOG IS CLOSE!!!");
        return true;
    }

    _logTimerIntervalMs = _logConfigMgr->GetIntervalMs();
    LibString specifyLogFileDir = _logConfigMgr->GetLogPath();

    // 初始化日志对象
    auto &logCfgs = _logConfigMgr->GetLogFileCfgs();
    const Int32 logCount = static_cast<Int32>(logCfgs.size());
    _fileIdIdxRefLog.resize(_logConfigMgr->GetMaxLogFileId() + 1);
    for(Int32 i = 0; i < logCount; ++i)
    {
        auto cfg = logCfgs[i];
        if(cfg && cfg->_isEnable)
        {
            auto newSpecifyLog = SpecifyLog::New_SpecifyLog();
            const Int32 status = newSpecifyLog->Init(specifyLogFileDir, cfg);
            if(status != Status::Success)
            {
                CRYSTAL_TRACE("newSpecifyLog INIT fail log file name=%s, specifyLogFileDir=%s, status=%d"
                , cfg->_logFileName.c_str(), specifyLogFileDir.c_str(), status);

                CRYSTAL_DELETE_SAFE(newSpecifyLog);
                continue;
            }

            // 日志着盘线程绑定关系
            if(static_cast<Int32>(_flushThreads.size()) <= newSpecifyLog->GetThreadRelationId())
                _flushThreads.resize(newSpecifyLog->GetThreadRelationId() + 1);
            if(static_cast<Int32>(_threadRelationLogs.size()) <= newSpecifyLog->GetThreadRelationId())
                _threadRelationLogs.resize(newSpecifyLog->GetThreadRelationId() + 1);
            if(static_cast<Int32>(_flushLocks.size()) <= newSpecifyLog->GetThreadRelationId())
                _flushLocks.resize(newSpecifyLog->GetThreadRelationId() + 1);
            auto flushThread = _flushThreads[newSpecifyLog->GetThreadRelationId()];
            if(UNLIKELY(!flushThread))
            {
                flushThread = CRYSTAL_NEW(LibThread);
                _flushThreads[newSpecifyLog->GetThreadRelationId()] = flushThread;
                _flushLocks[newSpecifyLog->GetThreadRelationId()] = new ConditionLocker;
            }
            auto threadBindLogs = _threadRelationLogs[newSpecifyLog->GetThreadRelationId()];
            if(UNLIKELY(!threadBindLogs))
            {
                threadBindLogs = new std::vector<SpecifyLog *>;
                _threadRelationLogs[newSpecifyLog->GetThreadRelationId()] = threadBindLogs;
            }
            threadBindLogs->push_back(newSpecifyLog);

            newSpecifyLog->BindWakeupFlush(_flushLocks[newSpecifyLog->GetThreadRelationId()]);
            _fileIdIdxRefLog[cfg->_logFileId] = newSpecifyLog;
        }
    }

    _rootDirName = specifyLogFileDir;

    CRYSTAL_TRACE("log config init suc log cfg file[%s], root dir name[%s]", cfgFile.c_str(), _rootDirName.c_str());

    return true;
}

void LibLog::Start()
{
    if(UNLIKELY(!_isInit.load() || _isFinish.load()))
    {
        CRYSTAL_TRACE("have no init cant start");
        return;
    }

    if(_isStart.exchange(true))
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

    const Int32 threadCount = static_cast<Int32>(_flushThreads.size());
    for(Int32 idx = 0; idx < threadCount; ++idx)
    {
        auto t = _flushThreads[idx];
        if(t)
        {
            auto params = Variant::New_Variant();
            params->BecomeDict();
            (*params)[1] = idx;
            t->AddTask2(this, &LibLog::_OnLogThreadFlush, params);
            t->Start();
        }
    }

    CRYSTAL_TRACE("LIB LOG START FINISH.");
}

void LibLog::Close()
{
    if(_isFinish.exchange(true))
        return;

    _isInit = false;
    _isStart = false;

    // 关闭线程
    const Int32 threadCount = static_cast<Int32>(_flushThreads.size());
    for(Int32 idx = 0; idx < threadCount; ++idx)
    {
        auto t = _flushThreads[idx];
        if(LIKELY(t))
        {
            t->HalfClose();
            _flushLocks[idx]->Sinal();
        }
    }
    for(Int32 idx = 0; idx < threadCount; ++idx)
    {
        auto t = _flushThreads[idx];
        if(LIKELY(t))
            t->FinishClose();
    }
    
    for(auto &log:_fileIdIdxRefLog)
    {
        if(log)
        {
            log->Close();
            SpecifyLog::Delete_SpecifyLog(log);
        }
    }

    ContainerUtil::DelContainer(_flushThreads);
    ContainerUtil::DelContainer(_flushLocks);
    ContainerUtil::DelContainer(_threadRelationLogs);

    _fileIdIdxRefLog.clear();

    _logConfigMgr->Close();
    CRYSTAL_DELETE_SAFE(_logConfigMgr);
}

void LibLog::FlushAll()
{
    for(auto &lck:_flushLocks)
    {
        if(LIKELY(lck))
            lck->Sinal();
    }
}


const LibString &LibLog::GetLogRootPath() const
{
    return _rootDirName;
}

bool LibLog::IsStart() const
{
    return _isStart;
}

bool LibLog::IsLogOpen() const
{
    return _logConfigMgr->IsEnableLog();
}

void LibLog::UnInstallAfterLogHookFunc(Int32 level, const IDelegate<void> *delegate)
{
    if(UNLIKELY(!IsLogOpen()))
        return;

    if(UNLIKELY(_isStart.load()))
    {
        CRYSTAL_TRACE("log has already start UnInstallAfterLogHookFunc");
        return;
    }

    // 能够调用uninstall 说明已经install过了
    auto logCfg = _logConfigMgr->GetLogCfg(level);
    auto specifyLog = _fileIdIdxRefLog[logCfg->_logFileId];
    specifyLog->UnInstallAfterLogHookFunc(level, delegate);
}

void LibLog::UnInstallBeforeLogHookFunc(Int32 level, const IDelegate<void, LogData *> *delegate)
{
    if(UNLIKELY(!IsLogOpen()))
        return;

    if(UNLIKELY(_isStart.load()))
    {
        CRYSTAL_TRACE("log has already start UnInstallBeforeLogHookFunc");
        return;
    }

    // 能够调用uninstall 说明已经install过了
    auto logCfg = _logConfigMgr->GetLogCfg(level);
    auto specifyLog = _fileIdIdxRefLog[logCfg->_logFileId];
    specifyLog->UnInstallBeforeLogHookFunc(level, delegate);
}
    
void LibLog::_InstallAfterLogHookFunc(const LogLevelCfg *levelCfg, IDelegate<void> *delegate)
{
    // 外部已排除是否开启日志
    auto specifyLog = _fileIdIdxRefLog[levelCfg->_fileId];
    specifyLog->InstallAfterHook(levelCfg->_level, delegate);
}

void LibLog::_InstallBeforeLogHookFunc(const LogLevelCfg *levelCfg, IDelegate<void, LogData *> *delegate)
{
    // 外部已排除是否开启日志
    auto specifyLog = _fileIdIdxRefLog[levelCfg->_fileId];
    specifyLog->InstallBeforeHook(levelCfg->_level, delegate);
}

const LogLevelCfg *LibLog::_GetLevelCfg(Int32 level) const
{
    return _logConfigMgr->GetLevelCfg(level);
}

void LibLog::_WriteLog(const LogLevelCfg *levelCfg, LogData *logData)
{
    auto specifyLog = _fileIdIdxRefLog[levelCfg->_fileId];

    // 开启打印堆栈信息
    if(levelCfg->_printStackTraceBack)
        logData->_logInfo << "\nStack Traceback:\n" << BackTraceUtil::CrystalCaptureStackBackTrace();

    Int64 curAllBytes = _curCacheBytes += logData->CalcBytes();
    specifyLog->WriteLog(*levelCfg, logData);

    // 缓存超过阈值强制着盘
    if(UNLIKELY(curAllBytes >= _logConfigMgr->GetMaxLogCacheBytes()))
    {
        _curCacheBytes = 0;
        FlushAll();
    }
}

void LibLog::_OnLogThreadFlush(LibThread *t, Variant *params)
{
    const Int32 idx = (*params)[1].AsInt32();
    auto logs = _threadRelationLogs[idx];
    auto lck = _flushLocks[idx];
    const Int32 logCount = static_cast<Int32>(logs->size());
    LibString logNameList;
    for(auto &log:*logs)
    {
        logNameList.AppendFormat("%s|", log->GetLogName().c_str());
    }

    const UInt64 intervalMs = static_cast<UInt64>(_logTimerIntervalMs);
    CRYSTAL_TRACE("log file name thread start id[%llu] relation id = [%d], bind log file name=[%s] intervalMs[%llu]"
    , SystemUtil::GetCurrentThreadId(), idx, logNameList.c_str(), intervalMs);

    while (true)
    {
        if(UNLIKELY(t->IsDestroy()))
        {
            _OnLogFlush(*logs, logCount);
            break;
        }

        // 写日志
        _OnLogFlush(*logs, logCount);

        lck->Lock();
        lck->TimeWait(intervalMs);
        lck->Unlock();
        // CRYSTAL_TRACE("log file name thread wake up[%s]", _config->_logFileName.c_str());
    }

    CRYSTAL_TRACE("log thread close relation id = [%d], thread id=[%llu] file list name:%s", idx, SystemUtil::GetCurrentThreadId(), logNameList.c_str());
}

void LibLog::_OnLogFlush(std::vector<SpecifyLog *> &logs, Int32 logCount)
{
    for(Int32 i = 0; i < logCount; ++i)
    {
        auto log = logs[i];
        log->_OnThreadWriteLog();
    }
}

KERNEL_END
