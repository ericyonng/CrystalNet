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
 * Date: 2021-02-17 16:16:34
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/common/status.h>
#include <kernel/source/comp/Log/SpecifyLog.h>
#include <kernel/comp/File/File.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/thread/thread.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Log/LogDefs.h>
#include <kernel/comp/Lock/Impl/ConditionLocker.h>
#include <kernel/comp/Log/LogConfig.h>
#include <kernel/comp/Log/LogLevel.h>
#include <kernel/comp/Log/LogData.h>
#include <kernel/comp/Delegate/IDelegate.h>

KERNEL_BEGIN


SpecifyLog::SpecifyLog()
{

}

SpecifyLog::~SpecifyLog()
{
    Close();
}

Int32 SpecifyLog::Init(const LibString &rootDirName, const LogConfig *cfg)
{
    if(_isInit.exchange(true, std::memory_order_acq_rel))
    {
        CRYSTAL_TRACE("already init log file name[%s]", cfg->_logFileName.c_str());
        return Status::Success;
    }

    if(UNLIKELY(_logFile))
    {
        CRYSTAL_TRACE("LOG %s has already init.", _logFile->GetFileName().c_str());
        return Status::Repeat;
    }

    _config = cfg;

    // hook
    const auto refLevelCount = _config->_levelIdxRefCfg.size();
    _beforeHook.resize(refLevelCount);
    _afterHook.resize(refLevelCount);

    // 1.文件名 = 路径 + 文件名 + pidStr
    const auto pidStr = StringUtil::Num2Str(SystemUtil::GetCurProcessId());
    LibString logName = rootDirName + _config->_logFileName + "_" + pidStr + _config->_extName;

    // 2.创建日志文件
    if(cfg->_needWriteFile)
    {
        _logFile = LibLogFile::New_LibLogFile();
        // CRYSTAL_TRACE("will create log file:%s", logName.c_str());
        bool isFileExist = true;
        if(!_logFile->Open(logName.c_str(), &isFileExist, true, "ab+", true))
        {
            ASSERT(!"LOG FILE OPEN FAIL");
            return Status::Log_CreateLogFileFail;
        }

        // 3.备份旧日志
        // _logFile->PartitionFile(!isFileExist);
        _logFile->UpdateLastPassDayTime();
    }

    // CRYSTAL_TRACE("init log file suc [%s]", cfg->_logFileName.c_str());

    return Status::Success;
}

Int32 SpecifyLog::Start()
{
    if(_isStart.exchange(true, std::memory_order_acq_rel))
    {
        CRYSTAL_TRACE("already start SpecifyLog log file name[%s]", _config->_logFileName.c_str());
        return Status::Success;
    }

    if(!_isInit.load(std::memory_order_acquire))
    {
        CRYSTAL_TRACE("log not init");
        return Status::NotInit;
    }

    if (_logFile && !_logFile->IsOpen())
    {
        printf("%s file not open .........\n", _logFile->GetFileName().c_str());
        return Status::Failed;
    }
    
    // CRYSTAL_TRACE("start specify log suc[%s]", _config->_logFileName.c_str());
    return Status::Success;
}

void SpecifyLog::Close()
{
    if(_isClose.exchange(true, std::memory_order_acq_rel))
        return;

    _isInit.store(false, std::memory_order_release);
    _isStart.store(false, std::memory_order_release);

    // 着盘
    Flush();

    // 关闭文件
    LibString logName;
    if (LIKELY(_logFile))
    {
        logName = _logFile->GetFileName();
        _logFile->Close();
    }

    // 清理资源
    for(auto iter = _beforeHook.begin(); iter != _beforeHook.end(); ++iter)
    {
        auto hookList = *iter;
        if(!hookList)
            continue;

        ContainerUtil::DelContainer<IDelegate<void, LogData *> *
        , AutoDelMethods::Release>(*hookList);
        CRYSTAL_DELETE_SAFE(hookList);
    }
    for(auto iter = _afterHook.begin(); iter != _afterHook.end(); ++iter)
    {
        auto hookList = *iter;
        if (!hookList)
            continue;

        ContainerUtil::DelContainer<IDelegate<void> *
        , AutoDelMethods::Release>(*hookList);
        CRYSTAL_DELETE_SAFE(hookList);
    }

    // 日志文件
    if(LIKELY(_logFile))
        LibLogFile::Delete_LibLogFile(_logFile);

    _logFile = NULL;

    // 清理数据
    for(auto iter = _logData->begin(); iter != _logData->end();)
    {
        CRYSTAL_TRACE("log data has left data [%s]!!!", (*iter)->_logInfo.c_str());
        LogData::Delete_LogData(*iter);
        iter = _logData->erase(iter);
    }
    for(auto iter = _swapData->begin(); iter != _swapData->end();)
    {
        CRYSTAL_TRACE("swap data has left data [%s]!!!", (*iter)->_logInfo.c_str());
        LogData::Delete_LogData(*iter);
        iter = _swapData->erase(iter);
    }

    CRYSTAL_DELETE_SAFE(_logData);
    CRYSTAL_DELETE_SAFE(_swapData);

//    CRYSTAL_TRACE("log file close:%s", logName.c_str());
}

void SpecifyLog::CloseAndReopen()
{
    if (LIKELY(_logFile))
    {
        _logFile->Close();
        _logFile->Reopen();
    }
}

void SpecifyLog::WriteLog(const LogLevelCfg &levelCfg, LogData *logData)
{
    // 1.外部判断,按理来说不应该在日志线程即将关闭或者关闭时写日志
    if(UNLIKELY(_isClose.load(std::memory_order_acquire)))
    {
        LogData::Delete_LogData(logData);
        return;
    }

    // 1.是否允许写日志 外部判断
    // if(!levelCfg._enable)
    //     return false;
        
    // 1.不可做影响log执行的其他事情
    auto beforeHookList = _beforeHook[levelCfg._level];
    if(beforeHookList)
    {
        for(auto iter:*beforeHookList)
            iter->Invoke(logData);
    }

    // 3.拷贝一次数据
    // LogData cache = *logData;

    // 3.打印到控制台
    if(levelCfg._enableConsole)
        _OutputToConsole(levelCfg, logData->_logInfo);

    // 4.将日志数据放入队列
    if(levelCfg._needWriteFile)
    {
        _logLck.Lock();
        _logData->push_back(logData);
        _logLck.Unlock();

        // 实时写日志
        if(UNLIKELY(levelCfg._enableRealTime))
            _wakeupFlush->Sinal();
    }
    else
    {
        LogData::Delete_LogData(logData);
    }

    // 6.日志后hook
    auto afterHookList = _afterHook[levelCfg._level];
    if(afterHookList)
    {
        for(auto iter:*afterHookList)
            iter->Invoke();
    }
}

void SpecifyLog::_OnThreadWriteLog()
{
    // 不需要写日志
    if(UNLIKELY(!_logFile))
        return;

    _logLck.Lock();
    if(_logData->empty())
    {
        // CRYSTAL_TRACE("%s have no log data to drop hardware.", _config->_logFileName.c_str());
        _logLck.Unlock();
        return;
    }

    // 只交换数据队列指针拷贝最少，最快，
    // 而且交换后_logDatas队列是空的相当于清空了数据 
    // 保证缓冲队列前几个都不为NULL, 碰到NULL表示结束
    // 由于主线程不会共享_logCaches所以是线程安全的（每个日志文件线程只有一个线程）
    // ,请保证外部其他线程不会调用本接口，需要立即写日志请调用flushall接口
    std::list<LogData *> *cache4RealLog = _logData;
    _logData = _swapData;
    _swapData = cache4RealLog;
    _logLck.Unlock();

    // 没有日志
    if(_swapData->empty())
    {
        // CRYSTAL_TRACE("%s have no swap data to drop hardware.", _config->_logFileName.c_str());
        return; 
    }

   // 2.写日志
   const Int64 maxFileSize = _config->_maxFileSize;
   for(auto iterLog = _swapData->begin(); iterLog != _swapData->end();)
   {
        // 1.跨天重新打开
        auto &logData = *iterLog;
        if(_logFile->IsDayPass(logData->_logTime))
        {
            // 跨天需要重新打开文件
            _logFile->Reopen(&logData->_logTime);
            _logFile->UpdateLastPassDayTime(&logData->_logTime);
        }

        // 2.文件过大转储到分立文件
        if(_logFile->IsTooLarge(maxFileSize))
            _logFile->PartitionFile(false, &logData->_logTime);

        // 3.写入文件
        _logFile->Write(logData->_logInfo.c_str(), logData->_logInfo.size());
        LogData::Delete_LogData(logData);

       iterLog = _swapData->erase(iterLog);
   }

    _logFile->Flush();
    // CRYSTAL_TRACE("log flush:%s", _logFile->GetFileName().c_str());
}

// void SpecifyLog::_OnThreadWork(LibThread *thread)
// {
//     const UInt64 intervalMs = static_cast<UInt64>(_config->_intervalMs);
//     CRYSTAL_TRACE("log file name thread start[%s] intervalMs[%llu]", _config->_logFileName.c_str(), intervalMs);
//     while (true)
//     {
//         if(UNLIKELY(thread->IsDestroy()))
//         {
//             _OnThreadWriteLog();
//             break;
//         }

//         // 写日志
//         _OnThreadWriteLog();

//         _wakeupLck.Lock();
//         _wakeupLck.TimeWait(intervalMs);
//         _wakeupLck.Unlock();

//         // CRYSTAL_TRACE("log file name thread wake up[%s]", _config->_logFileName.c_str());

//     }

//     CRYSTAL_TRACE("log thread close log file name:%s", _config->_logFileName.c_str());
// }

const LibString &SpecifyLog::GetLogName() const
{
    static const LibString emptyLog;
    return _logFile ? _logFile->GetFileName() : emptyLog;
}


void SpecifyLog::Flush()
{
    _wakeupFlush->Sinal();
}


void SpecifyLog::_WakupFlush()
{
    _wakeupFlush->Sinal();
}

void SpecifyLog::_OutputToConsole(const LogLevelCfg &levelCfg, const LibString &logInfo)
{
    SystemUtil::LockConsole();
    const Int32 oldColor = SystemUtil::GetConsoleColor();
    SystemUtil::SetConsoleColor(levelCfg._fgColor | levelCfg._bgColor);
    SystemUtil::OutputToConsole(logInfo);
    SystemUtil::SetConsoleColor(oldColor);
    SystemUtil::UnlockConsole();
}

UInt64 SpecifyLog::GetLogCount() const
{
    return static_cast<UInt64>(_logData->size());
}


void SpecifyLog::InstallBeforeHook(Int32 level, IDelegate<void, LogData *> *deleg)
{
    auto hookList = _beforeHook[level];
    if(UNLIKELY(!hookList))
    {
        hookList = new std::list<IDelegate<void, LogData *> *>;
        _beforeHook[level] = hookList;
    }

    hookList->push_back(deleg);
}

void SpecifyLog::InstallAfterHook(Int32 level, IDelegate<void> *deleg)
{   
    auto hookList = _afterHook[level];
    if(UNLIKELY(!hookList))
    {
        hookList = new std::list<IDelegate<void> *>;
        _afterHook[level] = hookList;
    }

    hookList->push_back(deleg);
}

void SpecifyLog::UnInstallAfterLogHookFunc(Int32 level, const IDelegate<void> *deleg)
{
    auto hookList = _afterHook[level];
    if(!hookList)
        return;

    for(auto iter = hookList->begin(); iter != hookList->end(); ++iter)
    {
        if(*iter == deleg)
        {
            (*iter)->Release();
            hookList->erase(iter);
            return;
        }
    }
}

void SpecifyLog::UnInstallBeforeLogHookFunc(Int32 level, const IDelegate<void, LogData *> *deleg)
{
    auto hookList = _beforeHook[level];
    if(!hookList)
        return;

    for(auto iter = hookList->begin(); iter != hookList->end(); ++iter)
    {
        if(*iter == deleg)
        {
            (*iter)->Release();
            hookList->erase(iter);
            return;
        }
    }
}

Int32 SpecifyLog::GetThreadRelationId() const
{
    return _config->_threadRelationId;
}


KERNEL_END
