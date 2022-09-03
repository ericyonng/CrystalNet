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
 * Date: 2021-02-17 19:07:49
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_SOURCE_COMP_LOG_LOG_INI_CFG_MGR_H__
#define __CRYSTAL_NET_KERNEL_SOURCE_COMP_LOG_LOG_INI_CFG_MGR_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Log/LogDefs.h>

KERNEL_BEGIN

class LibIniFile;
struct LogLevelCfg;
struct LogConfig;

class LogIniCfgMgr
{
public:
    LogIniCfgMgr();
    virtual ~LogIniCfgMgr();

    // 默认在当前程序下的ini目录
    virtual bool Init(const Byte8 *iniFileName);
    void Close();

public:
    const LogLevelCfg *GetLevelCfg(Int32 logLevel) const;
    const LogConfig *GetLogCfg(Int32 logLevel) const;
    Int64 GetMaxLogCacheBytes() const;
    const std::vector<LogConfig *> &GetLogFileCfgs() const;
    const Int32 GetMaxLogFileId() const;
    const bool IsEnableLog() const;
    const LibString &GetLogPath() const;
    const std::map<Int32, std::vector<LogConfig *>> &GetThreadRelationLogCfgs() const;
    const Int32 GetIntervalMs() const;


public:
    LibIniFile *_iniFile;

    std::vector<LogLevelInfo *> _levelInfos;    // levelid做下标索引
    std::vector<LogLevelCfg *> _levelIdxRefCfg; // level做下表索引
    std::vector<LogConfig *> _fileIdIdxRefCfg;  // 文件id做下标索引
    std::map<Int32, std::vector<LogConfig *>> _threadRelationIdRefLogCfgs;      // 日志与线程相关性dict
    Int64 _maxLogCacheBytes;                    // 最大日志缓存大小
    Int32 _maxFileId;                           // 最大文件id
    bool _isEnableLog;                          // 是否产出日志
    LibString _logPath;                         // 日志路径
    Int32 _intervalMs;                          // 着盘间隔ms
};

inline const LogLevelCfg *LogIniCfgMgr::GetLevelCfg(Int32 logLevel) const
{
    return _levelIdxRefCfg[logLevel];
}

inline const LogConfig *LogIniCfgMgr::GetLogCfg(Int32 logLevel) const
{
    auto levelCfg = GetLevelCfg(logLevel);
    return _fileIdIdxRefCfg[levelCfg->_fileId];
}

inline Int64 LogIniCfgMgr::GetMaxLogCacheBytes() const
{
    return _maxLogCacheBytes;
}

inline const std::vector<LogConfig *> &LogIniCfgMgr::GetLogFileCfgs() const
{
    return _fileIdIdxRefCfg;
}

inline const Int32 LogIniCfgMgr::GetMaxLogFileId() const
{
    return _maxFileId;
}

inline const bool LogIniCfgMgr::IsEnableLog() const
{
    return _isEnableLog;
}

inline const LibString &LogIniCfgMgr::GetLogPath() const
{
    return _logPath;
}

inline const std::map<Int32, std::vector<LogConfig *>> &LogIniCfgMgr::GetThreadRelationLogCfgs() const
{
    return _threadRelationIdRefLogCfgs;
}

inline const Int32 LogIniCfgMgr::GetIntervalMs() const
{
    return _intervalMs;
}

KERNEL_END

#endif
