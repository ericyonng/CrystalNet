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
#include <kernel/comp/Log/LibLog.h>
#include <kernel/comp/SmartPtr.h>

KERNEL_BEGIN

// ILog *ILog::GetDefaultInstance()
// {
//     static SmartPtr<LibLog, _Build::MT> s_log = new LibLog;
//     // CRYSTAL_TRACE("ENTRY LOG create new log");
//     return s_log.Cast<ILog>();
// }

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