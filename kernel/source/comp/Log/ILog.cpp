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
 * Date: 2021-02-17 15:37:34
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Log/ILog.h>

KERNEL_BEGIN

// ILog *ILog::GetDefaultInstance()
// {
//     static SmartPtr<LibLog, _Build::MT> s_log = new LibLog;
//     // CRYSTAL_TRACE("ENTRY LOG create new log");
//     return s_log.Cast<ILog>();
// }

void ILog::Info(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::Info, fmt, va, finalSize);
    va_end(va);
}

void ILog::Debug(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::Debug, fmt, va, finalSize);
    va_end(va);
}

void ILog::Warn(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::Warn, fmt, va, finalSize);
    va_end(va);
}

void ILog::Error(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common1(tag, LogLevel::Error, fileName, funcName, codeLine, fmt, va, finalSize);
    va_end(va);
}

void ILog::Monitor(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common4(LogLevel::Monitor, fmt, va, finalSize);
    va_end(va);
}

void ILog::Crash(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common4(LogLevel::Crash, fmt, va, finalSize);
    va_end(va);
}

void ILog::Net(const Byte8 *tag, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common2(tag, LogLevel::Net, fmt, va, finalSize);
    va_end(va);
}

void ILog::NetDebug(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::NetDebug, fmt, va, finalSize);
    va_end(va);
}

void ILog::NetWarn(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::NetWarn, fmt, va, finalSize);
    va_end(va);
}

void ILog::NetInfo(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::NetInfo, fmt, va, finalSize);
    va_end(va);
}

void ILog::NetError(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common1(tag, LogLevel::NetError, fileName, funcName, codeLine, fmt, va, finalSize);
    va_end(va);
}

void ILog::NetTrace(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::NetTrace, fmt, va, finalSize);
    va_end(va);
}

void ILog::Sys(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::Sys, fmt, va, finalSize);
    va_end(va);
}

void ILog::MemMonitor(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common4(LogLevel::MemMonitor, fmt, va, finalSize);
    va_end(va);
}

void ILog::Custom(const char *fmt, ...)
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
void ILog::Trace(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    _Common5(tag, codeLine, LogLevel::Trace, fmt, va, finalSize);
    va_end(va);
}

void ILog::_Common1(const Byte8 *tag, Int32 levelId, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, va_list va, UInt64 formatFinalSize)
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

    const auto tid = KERNEL_NS::SystemUtil::GetCurrentThreadId();
    // 构建日志数据
    LogData *newLogData = LogData::New_LogData();
    newLogData->_logTime.UpdateTime();
    auto &logInfo = newLogData->_logInfo;
    logInfo.AppendFormat("%s<%s>[%s][%s][%s][line:%d][tid:%llu]: "
                                   , newLogData->_logTime.ToString().c_str()
                                   , levelCfg->_levelName.c_str()
                                   , (tag ? tag : "")
                                   , fileName
                                   , funcName
                                   , codeLine
                                   , tid);

    logInfo.AppendFormatWithVaList(formatFinalSize, fmt, va)
            .AppendEnd();


    _WriteLog(levelCfg, newLogData);
}

void ILog::_Common2(const Byte8 *tag, Int32 levelId, const char *fmt, va_list va, UInt64 formatFinalSize)
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
    logInfo.AppendFormat("%s<%s>[%s][tid:%llu]: "
                        , newLogData->_logTime.ToString().c_str()
                        , levelCfg->_levelName.c_str()
                        , (tag ? tag : "")
                        , tid);

    logInfo.AppendFormatWithVaList(formatFinalSize, fmt, va)
            .AppendEnd();

    _WriteLog(levelCfg, newLogData);
}

void ILog::_Common3(Int32 levelId, const char *fmt, va_list va, UInt64 formatFinalSize)
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

    logInfo.AppendFormat("[tid:%llu] ", tid);
    logInfo.AppendFormatWithVaList(formatFinalSize, fmt, va)
            .AppendEnd();

    _WriteLog(levelCfg, newLogData);
}

void ILog::_Common4(Int32 levelId, const char *fmt, va_list va, UInt64 formatFinalSize)
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
    logInfo.AppendFormat("%s<%s>[tid:%llu]: "
                        , newLogData->_logTime.ToString().c_str()
                        , levelCfg->_levelName.c_str()
                        , tid);

    logInfo.AppendFormatWithVaList(formatFinalSize, fmt, va)
            .AppendEnd();

    _WriteLog(levelCfg, newLogData);
}

void ILog::_Common5(const Byte8 *tag, Int32 codeLine, Int32 levelId, const char *fmt, va_list va, UInt64 formatFinalSize)
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
    logInfo.AppendFormat("%s<%s>[%s][line:%d][tid:%llu]: "
                        , newLogData->_logTime.ToString().c_str()
                        , levelCfg->_levelName.c_str()
                        , (tag ? tag : "")
                        , codeLine
                        , tid);

    logInfo.AppendFormatWithVaList(formatFinalSize, fmt, va)
            .AppendEnd();

    _WriteLog(levelCfg, newLogData);
}

KERNEL_END