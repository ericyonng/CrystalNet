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
 * Date: 2021-02-17 16:16:26
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_SOURCE_COMP_LOG_SPECIFY_LOG_H__
#define __CRYSTAL_NET_KERNEL_SOURCE_COMP_LOG_SPECIFY_LOG_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>

#include <list>
#include <atomic>
#include <vector>
#include <kernel/comp/FileMonitor/FileMonitor.h>
#include <kernel/comp/FileMonitor/YamlDeserializer.h>
#include <kernel/comp/Log/LogCfg.h>

KERNEL_BEGIN

class LibLogFile;
class LibThread;
class ConditionLocker;
struct LogConfig;
struct LogLevelCfg;
struct LogData;

template <typename Rtn, typename... Args>
class IDelegate;

class SpecifyLog
{
    POOL_CREATE_OBJ_DEFAULT(SpecifyLog);
    friend class LibLog;
public:
    SpecifyLog();
    ~SpecifyLog();

public:
    Int32 Init(const LibString &rootDirName, FileMonitor<LogCfg, YamlDeserializer> *configMonitor, Int32 fileConfigId);
    void BindWakeupFlush(ConditionLocker *flusher);
    Int32 Start();
    void Close();
    void CloseAndReopen();
    void Flush();
    void WriteLog(const LogLevelCfg &levelCfg, LogData *logData);
    Int32 GetThreadRelationId() const;
    void ForceToDisk();

    // 在start之前安装hook
// // 线程不安全 原则上不可在多线程环境下使用
    void InstallBeforeHook(Int32 level, IDelegate<void, LogData *> *deleg);
    void InstallAfterHook(Int32 level, IDelegate<void> *deleg);
    void UnInstallAfterLogHookFunc(Int32 level, const IDelegate<void> *deleg);
    void UnInstallBeforeLogHookFunc(Int32 level, const IDelegate<void, LogData *> *deleg);

    UInt64 GetLogCount() const;

    bool IsClose() const;
    const LibString &GetLogName() const;

private:
    void _OnThreadWriteLog();
    void _WakupFlush();

private:
    void _OutputToConsole(const LogLevelCfg &levelCfg, const LibString &logInfo);

private:
    std::atomic_bool _isInit = {false};
    std::atomic_bool _isStart = {false};
    std::atomic_bool _isClose = {false};
    Int32 _fileConfigId = 0;
    FileMonitor<LogCfg, YamlDeserializer> *_fileMonitor = NULL;
    std::vector<std::list<IDelegate<void, LogData *> *> *> _beforeHook; // level 做下标索引
    std::vector<std::list<IDelegate<void> *> *> _afterHook;    // level 做下标索引
    LibLogFile *_logFile = NULL;
    
    // 锁
    SpinLock _logLck;
    ConditionLocker *_wakeupFlush = NULL;

    std::list<LogData *> *_logData = new std::list<LogData *>;
    std::list<LogData *> *_swapData = new std::list<LogData *>;
};

ALWAYS_INLINE void SpecifyLog::BindWakeupFlush(ConditionLocker *flusher)
{
    _wakeupFlush = flusher;
}

ALWAYS_INLINE void SpecifyLog::ForceToDisk()
{
    CloseAndReopen();
}

ALWAYS_INLINE bool SpecifyLog::IsClose() const
{
    return _isClose.load(std::memory_order_acquire);
}

KERNEL_END

#endif