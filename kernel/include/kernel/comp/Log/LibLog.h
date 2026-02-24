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
 * Date: 2021-02-17 15:58:54
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_LIB_LOG_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_LIB_LOG_H__

#pragma once

#include <kernel/comp/Log/ILog.h>
#include <atomic>
#include <vector>
#include <kernel/comp/FileMonitor/FileMonitor.h>
#include <kernel/comp/FileMonitor/YamlDeserializer.h>
#include <kernel/comp/Log/LogCfg.h>
#include <kernel/comp/Coroutines/CoTask.h>

KERNEL_BEGIN

class SpecifyLog;
class LogIniCfgMgr;
class Variant;
class LibThread;
class ConditionLocker;
class CoLocker;

class KERNEL_EXPORT LibLog : public ILog
{
public:
    LibLog();
    virtual ~LibLog();

public:
    virtual bool Init(const Byte8 *logConfigFile = "Log.yaml", const Byte8 *logCfgDir = NULL, YamlMemory *yamlMemory = NULL) override;
    virtual void Start() override;
    virtual void Close() override;
    virtual void FlushAll() override;
    virtual const LibString &GetLogRootPath() const override;
    virtual bool IsStart() const override;
    virtual bool IsLogOpen() const override;
    virtual void ForceLogToDiskAll() override;

    virtual void UnInstallAfterLogHookFunc(Int32 level, const IDelegate<void> *delegate) override;
    virtual void UnInstallBeforeLogHookFunc(Int32 level, const IDelegate<void, LogData *> *delegate) override;

    virtual const void *GetFileMonitor() const override;
    virtual SmartPtr<LogCfg, AutoDelMethods::Release> GetLogCfg() const override;

protected:
    virtual void _InstallAfterLogHookFunc(const LogLevelInfoCfg *levelCfg, IDelegate<void> *delegate) override;
    virtual void _InstallBeforeLogHookFunc(const LogLevelInfoCfg *levelCfg, IDelegate<void, LogData *> *delegate) override;

protected:
    virtual void _WriteLog(const LogLevelInfoCfg *levelCfg, LogData *logData) override;
    CoTask<> _OnLogThreadFlush(Variant *params);
    void _OnLogFlush(std::vector<SpecifyLog *> &logs, Int32 threadRelationIdx, Int32 logCount);

    void StopAllLogThreadAndFallingDisk();
    void _OnAbnormalSignalHandler();

    UInt64 _GetDynamicInterval(UInt64 logCount) const;

private:
    std::atomic_bool _isInit;
    std::atomic_bool _isStart;
    std::atomic_bool _isFinish;
    LibString _rootDirName;
    // LogIniCfgMgr *_logConfigMgr;
    FileMonitor<LogCfg, YamlDeserializer> *_fileMonitor;
    
    std::atomic<Int64> _curCacheBytes;
    // UInt64 _logTimerIntervalMs;

    std::vector<SpecifyLog *> _fileIdIdxRefLog;     

    // 线程锁,线程,flush时间间隔
    std::vector<CoLocker *> _flushLocks;                            // 锁(下标是线程关联性id对线程池数量取模)
    std::vector<std::vector<SpecifyLog *> *> _threadRelationLogs;   // 下标与线程相关id绑定,目的在于一个线程执行多个日志的着盘

    std::atomic<Int32> _workingNum;                                 // 工作的线程数量
    
    std::atomic_bool _isForceLogDiskAll;
    std::vector<std::atomic_bool *> _isForceDiskFlag;
};

ALWAYS_INLINE UInt64 LibLog::_GetDynamicInterval(UInt64 logCount) const
{
    if(logCount < 10000)
        return _fileMonitor->Current()->LogCommon.LogTimerInterval.GetTotalMilliSeconds();
    
    if(logCount < 200000)
        return 10;
    
    return 0;
}

KERNEL_END

#endif
