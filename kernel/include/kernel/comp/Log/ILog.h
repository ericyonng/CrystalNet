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
 * Date: 2021-02-09 22:58:49
 * Author: Eric Yonng
 * Description: 
 *             1.定时着盘
 *             2.超过内存限制着盘
 *             3.扩展日志方便 :
 *                          a.继承ILog, 添加需要扩展的接口,
 *                          b.配置中Level添加一行扩展的等级配置,并添加对应的文件名以及文件id等
 *                          c.日志配置中添加相应等级的配置即可,且保证等级名称与程序中等级名称一致即可
 *                              如：
 *                                  [enable],[fileid],[enableConsole],[fgcolor]|[bgcolor],[LevelId]
 *                                  Custom = 1,3,1,White|Black,7
 * 
 *             4.tag:默认类型名
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_ILOG_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_ILOG_H__

#pragma once

#include <kernel/kernel_export.h>
#include <stdarg.h>
#include <utility>

#include <kernel/comp/LibString.h>
#include <kernel/comp/Log/LogDefs.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Log/LogMacro.h>

KERNEL_BEGIN

class KERNEL_EXPORT ILog
{
    friend class LogTool;
    
public:
    ILog(){}
    virtual ~ILog(){}
    
    // static ILog *GetDefaultInstance();

    virtual bool Init(const Byte8 *logConfigFile = "LogCfg.ini", const Byte8 *logCfgDir = NULL, const LibString &logContent = LibString(), const LibString &consoleConntent = LibString()) = 0;
    virtual void Start() = 0;
    virtual void Close() = 0;
    virtual void FlushAll() = 0;
    virtual const LibString &GetLogRootPath() const = 0;
    virtual bool IsStart() const = 0;
    virtual void ForceLogToDiskAll() = 0;


public:
    // // 进程日志 文件名使用进程名 使用通用日志宏
    void Info(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(6, 7);
    void Debug(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(6, 7);
    void Warn(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(6, 7);
    void Error(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(6, 7);
    template<typename... Args>
    void Info2(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, Args&&... args);
    template<typename... Args>
    void Debug2(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, Args&&... args);
    template<typename... Args>
    void Warn2(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, Args&&... args);
    template<typename... Args>
    void Error2(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, Args&&... args);
    
    void Monitor(const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(2, 3);

    // 崩溃日志 文件名字:Crash
    void Crash(const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(2, 3);
    
    // 网络日志 文件名: Net
    void Net(const Byte8 *tag, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(3, 4);
    void NetDebug(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(6, 7);
    void NetWarn(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(6, 7);
    void NetInfo(const Byte8 *tag, const char *fileName,  const char *funcName, Int32 codeLine, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(6, 7);
    void NetError(const Byte8 *tag, const char *fileName,  const char *funcName, Int32 codeLine, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(6, 7);
    void NetTrace(const Byte8 *tag, const char *fileName,  const char *funcName, Int32 codeLine, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(6, 7);

    // 系统日志
    void Sys(const Byte8 *tag, const char *fileName,  const char *funcName, Int32 codeLine, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(6, 7);
    // 内存监视器
    void MemMonitor(const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(2, 3);

    // 自定义 文件名:Custom
    void Custom(const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(2, 3);

    // 追踪日志
    void Trace(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...) LIB_KERNEL_FORMAT_CHECK(6, 7);

    // sql
    template<typename... Args>
    void FailSql(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, Args&&... args);
    template<typename... Args>
    void DumpSql(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, Args&&... args);

    // // hook函数安装与卸载 // 线程不安全 原则上不可在多线程环境下使用 必须在单线程情况下设置与卸载
public:
    template<typename ObjType>
    const IDelegate<void> * InstallAfterLogHookFunc(Int32 level, ObjType *obj, void (ObjType::*cb)());
    const IDelegate<void> * InstallAfterLogHookFunc(Int32 level, void (*cb)());
    template<typename ObjType>
    const IDelegate<void, LogData *> * InstallBeforeLogHookFunc(Int32 level, ObjType *obj, void (ObjType::*cb)(LogData *logData));
    const IDelegate<void, LogData *> * InstallBeforeLogHookFunc(Int32 level, void(*cb)(LogData *logData));
    
    virtual void UnInstallAfterLogHookFunc(Int32 level, const IDelegate<void> *delegate) = 0;
    virtual void UnInstallBeforeLogHookFunc(Int32 level, const IDelegate<void, LogData *> *delegate) = 0;

protected:
    virtual void _InstallAfterLogHookFunc(const LogLevelCfg *levelCfg, IDelegate<void> *delegate) = 0;
    virtual void _InstallBeforeLogHookFunc(const LogLevelCfg *levelCfg, IDelegate<void, LogData *> *delegate) = 0;
    

public:
    bool IsEnable(Int32 level) const;
    virtual bool IsLogOpen() const = 0;

protected:
    virtual const LogLevelCfg *_GetLevelCfg(Int32 level) const = 0;
    virtual void _WriteLog(const LogLevelCfg *levelCfg, LogData *logData) = 0;

    // 继承后可调用的写日志模版
    void _Common1(const Byte8 *tag, Int32 levelId, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, va_list va, UInt64 formatFinalSize);
    void _Common2(const Byte8 *tag, Int32 levelId, const char *fmt, va_list va, UInt64 formatFinalSize);
    // 不带前缀单纯输出数据
    void _Common3(Int32 levelId, const char *fmt, va_list va, UInt64 formatFinalSize);
    // 带log time 与日志级别
    void _Common4(Int32 levelId, const char *fmt, va_list va, UInt64 formatFinalSize);

    void _Common5(const Byte8 *tag, Int32 codeLine, Int32 levelId, const char *fmt, va_list va, UInt64 formatFinalSize);
    template<typename... Args>
    void _Common6(const Byte8 *tag, Int32 codeLine, Int32 levelId, Args&&... args)
    {
        if(UNLIKELY(!IsLogOpen()))
            return;
    
        auto levelCfg = _GetLevelCfg(levelId);
        if(UNLIKELY(!levelCfg))
        {
            CRYSTAL_TRACE("log level[%d] cfg not found", levelId);
            return;
        }

        // 是否需要输出日志
        if(UNLIKELY(!levelCfg->_enable))
            return;

        // 构建日志数据
        const auto tid = KERNEL_NS::SystemUtil::GetCurrentThreadId();
        LogData *newLogData = LogData::New_LogData();
        newLogData->_logTime.UpdateTime();
        auto &logInfo = newLogData->_logInfo;
        logInfo.AppendFormat("%s<%s>[%s][line:%d][tid:%llu]: ", newLogData->_logTime.ToString().c_str(), levelCfg->_levelName.c_str(), (tag ? tag : ""), codeLine, tid);
        logInfo.Append(std::forward<Args>(args)...);
        logInfo.AppendEnd();

        _WriteLog(levelCfg, newLogData);
    }
};

template<typename... Args>
ALWAYS_INLINE void ILog::Info2(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, Args&&... args)
{
    _Common6(tag, codeLine, LogLevel::Info, std::forward<Args>(args)...);
}

template<typename... Args>
ALWAYS_INLINE void ILog::Debug2(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, Args&&... args)
{
    _Common6(tag, codeLine, LogLevel::Debug, std::forward<Args>(args)...);
}

template<typename... Args>
ALWAYS_INLINE void ILog::Warn2(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, Args&&... args)
{
    _Common6(tag, codeLine, LogLevel::Warn, std::forward<Args>(args)...);
}

template<typename... Args>
ALWAYS_INLINE void ILog::Error2(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, Args&&... args)
{
    _Common6(tag, codeLine, LogLevel::Error, std::forward<Args>(args)...);
}

template<typename... Args>
ALWAYS_INLINE void ILog::FailSql(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, Args&&... args)
{
    _Common6(tag, codeLine, LogLevel::FailSql, std::forward<Args>(args)...);
}

template<typename... Args>
ALWAYS_INLINE void ILog::DumpSql(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, Args&&... args)
{
    _Common6(tag, codeLine, LogLevel::DumpSql, std::forward<Args>(args)...);
}

template<typename ObjType>
ALWAYS_INLINE const IDelegate<void> *ILog::InstallAfterLogHookFunc(Int32 level, ObjType *obj, void (ObjType::*cb)())
{
    if(UNLIKELY(!IsLogOpen()))
        return NULL;
 
    auto levelCfg = _GetLevelCfg(level);
    if(UNLIKELY(!levelCfg || !levelCfg->_enable))
        return NULL;

    if(UNLIKELY(IsStart()))
    {
        CRYSTAL_TRACE("log has already start InstallAfterLogHookFunc");
        return NULL;
    }

    auto newDelegate = DelegateFactory::Create(obj, cb);
    _InstallAfterLogHookFunc(levelCfg, newDelegate);
    return newDelegate;
}

ALWAYS_INLINE const IDelegate<void> *ILog::InstallAfterLogHookFunc(Int32 level, void (*cb)())
{
    if(UNLIKELY(!IsLogOpen()))
        return NULL;
 
    auto levelCfg = _GetLevelCfg(level);
    if(UNLIKELY(!levelCfg || !levelCfg->_enable))
        return NULL;

    if(UNLIKELY(IsStart()))
    {
        CRYSTAL_TRACE("log has already start InstallAfterLogHookFunc");
        return NULL;
    }

    auto newDelegate = DelegateFactory::Create(cb);
    _InstallAfterLogHookFunc(levelCfg, newDelegate);
    return newDelegate;
}

template<typename ObjType>
ALWAYS_INLINE const IDelegate<void, LogData *> *ILog::InstallBeforeLogHookFunc(Int32 level, ObjType *obj, void (ObjType::*cb)(LogData *logData))
{
    if(UNLIKELY(!IsLogOpen()))
        return NULL;
 
    auto levelCfg = _GetLevelCfg(level);
    if(UNLIKELY(!levelCfg || !levelCfg->_enable))
        return NULL;

    if(UNLIKELY(IsStart()))
    {
        CRYSTAL_TRACE("log has already start InstallBeforeLogHookFunc");
        return NULL;
    }

    auto newDelegate = DelegateFactory::Create(obj, cb);
    _InstallBeforeLogHookFunc(levelCfg, newDelegate);
    return newDelegate;
}

ALWAYS_INLINE const IDelegate<void, LogData *> *ILog::InstallBeforeLogHookFunc(Int32 level, void(*cb)(LogData *logData))
{
    if(UNLIKELY(!IsLogOpen()))
        return NULL;
 
    auto levelCfg = _GetLevelCfg(level);
    if(UNLIKELY(!levelCfg || !levelCfg->_enable))
        return NULL;

    if(UNLIKELY(IsStart()))
    {
        CRYSTAL_TRACE("log has already start InstallBeforeLogHookFunc");
        return NULL;
    }
    
    auto newDelegate = DelegateFactory::Create(cb);
    _InstallBeforeLogHookFunc(levelCfg, newDelegate);
    return newDelegate;
}

ALWAYS_INLINE bool ILog::IsEnable(Int32 level) const
{
    if(UNLIKELY(!IsLogOpen() || !IsStart()))
        return false;

    auto levelCfg = _GetLevelCfg(level);
    return levelCfg ? levelCfg->_enable : false;
}

KERNEL_END

#endif
