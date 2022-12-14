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

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Log/LogDefs.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>

// 函数与行号便利宏
#undef _FUNC_LINE_ARGS_
#define _FUNC_LINE_ARGS_ __FILE__, __FUNCTION__, __LINE__

// // 获取tag

// 类实例获取tag
#undef LOG_OBJ_TAG
#define LOG_OBJ_TAG() KERNEL_NS::RttiUtil::GetByType<decltype(*this)>()
// 非类实例获取tag
#undef LOG_NON_OBJ_TAG
#define LOG_NON_OBJ_TAG(classType) KERNEL_NS::RttiUtil::GetByType<classType>()


// 带修饰的log宏
#undef LOGFMT_DETAIL
#define LOGFMT_DETAIL(tag, x) tag, _FUNC_LINE_ARGS_, x

// // 带tag宏,除了Net接口特殊外都可用

// 类实例tag
#undef LOGFMT_OBJ_TAG
#define LOGFMT_OBJ_TAG(x) LOGFMT_DETAIL(LOG_OBJ_TAG(), x)

// 非类实例tag
#undef LOGFMT_NON_OBJ_TAG
#define LOGFMT_NON_OBJ_TAG(classType, x) LOGFMT_DETAIL(LOG_NON_OBJ_TAG(classType), x)


KERNEL_BEGIN

class KERNEL_EXPORT ILog
{
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

};

inline void ILog::Info(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::Info, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::Debug(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::Debug, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::Warn(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::Warn, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::Error(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common1(tag, LogLevel::Error, fileName, funcName, codeLine, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::Monitor(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common4(LogLevel::Monitor, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::Crash(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common4(LogLevel::Crash, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::Net(const Byte8 *tag, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common2(tag, LogLevel::Net, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::NetDebug(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::NetDebug, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::NetWarn(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::NetWarn, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::NetInfo(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::NetInfo, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::NetError(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common1(tag, LogLevel::NetError, fileName, funcName, codeLine, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::NetTrace(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::NetTrace, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::Sys(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::Sys, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::MemMonitor(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common4(LogLevel::MemMonitor, fmt, va, finalSize);
    va_end(va);
}

inline void ILog::Custom(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common3(LogLevel::Custom, fmt, va, finalSize);
    va_end(va);
}

// 追踪日志
inline void ILog::Trace(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::Trace, fmt, va, finalSize);
    va_end(va);
}

template<typename ObjType>
inline const IDelegate<void> *ILog::InstallAfterLogHookFunc(Int32 level, ObjType *obj, void (ObjType::*cb)())
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

inline const IDelegate<void> *ILog::InstallAfterLogHookFunc(Int32 level, void (*cb)())
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
inline const IDelegate<void, LogData *> *ILog::InstallBeforeLogHookFunc(Int32 level, ObjType *obj, void (ObjType::*cb)(LogData *logData))
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

inline const IDelegate<void, LogData *> *ILog::InstallBeforeLogHookFunc(Int32 level, void(*cb)(LogData *logData))
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

inline bool ILog::IsEnable(Int32 level) const
{
    auto levelCfg = _GetLevelCfg(level);
    if(UNLIKELY(!levelCfg || !levelCfg->_enable))
        return false;

    return true;
}

inline void ILog::_Common1(const Byte8 *tag, Int32 levelId, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, va_list va, UInt64 formatFinalSize)
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
    LogData *newLogData = LogData::New_LogData();
    newLogData->_logTime.UpdateTime();
    auto &logInfo = newLogData->_logInfo;
    logInfo.AppendFormat("%s<%s>[%s][%s][%s][line:%d]: "
                                   , newLogData->_logTime.ToString().c_str()
                                   , levelCfg->_levelName.c_str()
                                   , (tag ? tag : "")
                                   , fileName
                                   , funcName
                                   , codeLine);

    logInfo.AppendFormatWithVaList(formatFinalSize, fmt, va)
            .AppendEnd();


    _WriteLog(levelCfg, newLogData);
}

inline void ILog::_Common2(const Byte8 *tag, Int32 levelId, const char *fmt, va_list va, UInt64 formatFinalSize)
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
    LogData *newLogData = LogData::New_LogData();
    newLogData->_logTime.UpdateTime();
    auto &logInfo = newLogData->_logInfo;
    logInfo.AppendFormat("%s<%s>[%s]: "
                        , newLogData->_logTime.ToString().c_str()
                        , levelCfg->_levelName.c_str()
                        , (tag ? tag : ""));

    logInfo.AppendFormatWithVaList(formatFinalSize, fmt, va)
            .AppendEnd();

    _WriteLog(levelCfg, newLogData);
}

inline void ILog::_Common3(Int32 levelId, const char *fmt, va_list va, UInt64 formatFinalSize)
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
    LogData *newLogData = LogData::New_LogData();
    newLogData->_logTime.UpdateTime();
    auto &logInfo = newLogData->_logInfo;

    logInfo.AppendFormatWithVaList(formatFinalSize, fmt, va)
            .AppendEnd();

    _WriteLog(levelCfg, newLogData);
}

inline void ILog::_Common4(Int32 levelId, const char *fmt, va_list va, UInt64 formatFinalSize)
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
    LogData *newLogData = LogData::New_LogData();
    newLogData->_logTime.UpdateTime();

    auto &logInfo = newLogData->_logInfo;
    logInfo.AppendFormat("%s<%s>: "
                        , newLogData->_logTime.ToString().c_str()
                        , levelCfg->_levelName.c_str());

    logInfo.AppendFormatWithVaList(formatFinalSize, fmt, va)
            .AppendEnd();

    _WriteLog(levelCfg, newLogData);
}

inline void ILog::_Common5(const Byte8 *tag, Int32 codeLine, Int32 levelId, const char *fmt, va_list va, UInt64 formatFinalSize)
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
    LogData *newLogData = LogData::New_LogData();
    newLogData->_logTime.UpdateTime();
    auto &logInfo = newLogData->_logInfo;
    logInfo.AppendFormat("%s<%s>[%s][line:%d]: "
                        , newLogData->_logTime.ToString().c_str()
                        , levelCfg->_levelName.c_str()
                        , (tag ? tag : "")
                        , codeLine);

    logInfo.AppendFormatWithVaList(formatFinalSize, fmt, va)
            .AppendEnd();

    _WriteLog(levelCfg, newLogData);
}

    
KERNEL_END

#endif
