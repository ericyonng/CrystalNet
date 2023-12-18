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

KERNEL_BEGIN

class SpecifyLog;
class LogIniCfgMgr;
class Variant;
class LibThread;
class ConditionLocker;

class KERNEL_EXPORT LibLog : public ILog
{
public:
    LibLog();
    virtual ~LibLog();

public:
    virtual bool Init(const Byte8 *logConfigFile = "LogCfg.ini", const Byte8 *logCfgDir = NULL, const LibString &logContent = LibString(), const LibString &consoleConntent = LibString()) override;
    virtual void Start() override;
    virtual void Close() override;
    virtual void FlushAll() override;
    virtual const LibString &GetLogRootPath() const override;
    virtual bool IsStart() const override;
    virtual bool IsLogOpen() const override;
    virtual void ForceLogToDiskAll() override;

    virtual void UnInstallAfterLogHookFunc(Int32 level, const IDelegate<void> *delegate) override;
    virtual void UnInstallBeforeLogHookFunc(Int32 level, const IDelegate<void, LogData *> *delegate) override;

protected:
    virtual void _InstallAfterLogHookFunc(const LogLevelCfg *levelCfg, IDelegate<void> *delegate) override;
    virtual void _InstallBeforeLogHookFunc(const LogLevelCfg *levelCfg, IDelegate<void, LogData *> *delegate) override;

protected:
    virtual const LogLevelCfg *_GetLevelCfg(Int32 level) const override;
    virtual void _WriteLog(const LogLevelCfg *levelCfg, LogData *logData) override;
    void _OnLogThreadFlush(LibThread *t, Variant *params);
    void _OnLogFlush(std::vector<SpecifyLog *> &logs, Int32 threadRelationIdx, Int32 logCount);

    void StopAllLogThreadAndFallingDisk();
    void _OnAbnormalSignalHandler();

    UInt64 _GetDynamicInterval(UInt64 logCount) const;

private:
    std::atomic_bool _isInit;
    std::atomic_bool _isStart;
    std::atomic_bool _isFinish;
    LibString _rootDirName;
    LogIniCfgMgr *_logConfigMgr;
    std::atomic<Int64> _curCacheBytes;
    UInt64 _logTimerIntervalMs;

    std::vector<SpecifyLog *> _fileIdIdxRefLog;     

    // 线程锁,线程,flush时间间隔
    std::vector<LibThread *> _flushThreads;                         // 下标与线程相关性id绑定
    std::vector<ConditionLocker *> _flushLocks;                     // 锁
    std::vector<std::vector<SpecifyLog *> *> _threadRelationLogs;   // 下标与线程相关id绑定,目的在于一个线程执行多个日志的着盘

    std::atomic_bool _isForceLogDiskAll;
    std::vector<std::atomic_bool *> _isForceDiskFlag;
};

ALWAYS_INLINE UInt64 LibLog::_GetDynamicInterval(UInt64 logCount) const
{
    if(logCount < 10000)
        return _logTimerIntervalMs;
    
    if(logCount < 200000)
        return 10;
    
    return 0;
}

KERNEL_END

#endif
