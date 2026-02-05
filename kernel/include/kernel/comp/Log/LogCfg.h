// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-01-30 00:01:49
// Author: Eric Yonng
// Description:
#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_LOG_CFG_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_LOG_CFG_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/comp/LibString.h>
#include <vector>
#include <kernel/comp/TimeSlice.h>
#include <yaml-cpp/yaml.h>
#include <kernel/comp/File/ConsoleColorHelper.h>

KERNEL_BEGIN

// 日志文件配置
struct KERNEL_EXPORT LogFileDefine
{
    // 文件id
    Int32 FileId = 0;
    // 文件名
    // 如果是空的会在反序列化的时候被修正成当前程序名
    LibString FileName;
    // 线程关联性id, 线程关联性id一样的log文件会被分在同一个线程, 利用多线程来处理日志
    Int32 ThreadRelationshipId = 0;

    // 非yaml字段
    // 只要有一个关联的Level开启那么文件就开启
    bool IsEnable = false;
    // 只要有一个关联的LogLevel需要写文件那么就可以写文件
    bool NeedWriteFile = false;
    // 后缀名
    LibString ExtName = ".log";
};

// 日志路径:[是否使用程序名当日志的一级目录][路径]
struct KERNEL_EXPORT LogPathCfg
{
    bool IsUseProgramNameAsFirstLevelPath = true;

    // 路径:
    // 若使用程序名当一级目录则可以缺省指定路径名 若此时有指定路径名,程序名路径会追加在该指定路径成为二级目录
    // 若指定了绝对路径则必定使用给定的绝对路径 若此时又指定了程序名当目录 程序名路径追加在该指定路径成为二级目录
    LibString PartPath;
};

// 日志公共配置
struct KERNEL_EXPORT LogCommonCfg
{
    // 日志文件定义
    std::vector<LogFileDefine> LogFileDefineList;
    // 日志后缀名
    LibString ExtName = ".log";
    // 日志着盘时间间隔,定时着盘 格式:00:00:01.xxx, xxx部分是纳秒
    TimeSlice LogTimerInterval = TimeSlice::FromSeconds(1);
    // 日志单文件最大大小,超过会创建分立文件
    Int32 MaxFileSizeMB = 256;
    // 日志最大缓存限制,超过则即时着盘
    Int32 MaxLogCacheMB = 16;
    // 是否产出日志
    bool IsEnableLog = true;

    // 日志路径配置
    LogPathCfg LogPath;
};

// 日志等级配置
struct KERNEL_EXPORT LogLevelInfoCfg
{
    bool Enable = true;
    LibString LevelName;
    Int32 FileId = 0;
    bool EnableConsole = true;
    Int32 FgColor = 0;
    Int32 BgColor = 0;
    Int32 LevelId = 0;
    bool EnableRealTime = false;
    bool PRINT_STACK_TRACE_BACK = false;
    bool NEED_WRITE_FILE = true;
};

// 日志整体配置
struct KERNEL_EXPORT LogCfg
{
    LogCommonCfg LogCommon;
    std::vector<LogLevelInfoCfg> LogLevelList;

    // 手工加工部分:
    // 下表是LogLevelId => LogLevelCfg 
    std::vector<LogLevelInfoCfg *> LogLevelIndexRefCfg;
    // 最终日志路径
    LibString FinalLogPath;

    // 文件id下表索引关联 LogFileDefine
    std::vector<LogFileDefine *> FileIdRefFileDefine;
    // 文件id下标索引关联 LogLevelInfoCfg *列表,列表的下标是LogLevelId
    std::vector<std::vector<LogLevelInfoCfg *>> FileIdRefLevelInfoCfgs;

    // 线程关联性id与文件绑定关系
    std::map<Int32, std::vector<LogFileDefine *>> ThreadRelationIdRefFileDefines;

    // 最大文件id
    Int32 MaxLogFileId = 0;
    // 最大文件大小
    Int64 MaxLogSizeBytes = 0;
    // 最大缓存大小
    Int64 MaxLogCacheSizeBytes = 0;
    static LogCfg *CreateNewObj(LogCfg &&cfg)
    {
        return new LogCfg(std::move(cfg));
    }
    
    void Release()
    {
        delete this;
    }

    // 常用接口
public:
    const LogLevelInfoCfg *GetLevelCfg(Int32 logLevel) const;

    
    const LogFileDefine *GetLogFile(Int32 logLevel) const;
    
    Int64 GetMaxLogCacheBytes() const;
    
    const std::vector<LogFileDefine *> &GetLogFileCfgs() const;
    
    Int32 GetMaxLogFileId() const;
    
    bool IsEnableLog() const;
    
    const LibString &GetLogPath() const;
    
    const std::map<Int32, std::vector<LogFileDefine *>> &GetThreadRelationLogCfgs() const;
    
    const TimeSlice &GetIntervalMs() const;
};

ALWAYS_INLINE const LogLevelInfoCfg *LogCfg::GetLevelCfg(Int32 logLevel) const
{
    if(UNLIKELY(static_cast<Int32>(LogLevelIndexRefCfg.size()) <= logLevel))
    {
        return NULL;
    }

    return LogLevelIndexRefCfg[logLevel];
}
    
ALWAYS_INLINE const LogFileDefine *LogCfg::GetLogFile(Int32 logLevel) const
{
    auto logLevelCfg = GetLevelCfg(logLevel);
    if(UNLIKELY(!logLevelCfg))
        return NULL;

    return FileIdRefFileDefine[logLevelCfg->FileId];
}
    
ALWAYS_INLINE Int64 LogCfg::GetMaxLogCacheBytes() const
{
    return MaxLogCacheSizeBytes;
}
    
ALWAYS_INLINE const std::vector<LogFileDefine *> &LogCfg::GetLogFileCfgs() const
{
    return FileIdRefFileDefine;
}
    
ALWAYS_INLINE Int32 LogCfg::GetMaxLogFileId() const
{
    return MaxLogFileId;
}
    
ALWAYS_INLINE bool LogCfg::IsEnableLog() const
{
    return LogCommon.IsEnableLog;
}
    
ALWAYS_INLINE const LibString &LogCfg::GetLogPath() const
{
    return FinalLogPath;
}
    
ALWAYS_INLINE const std::map<Int32, std::vector<LogFileDefine *>> &LogCfg::GetThreadRelationLogCfgs() const
{
    return ThreadRelationIdRefFileDefines;
}
    
ALWAYS_INLINE const TimeSlice &LogCfg::GetIntervalMs() const
{
    return LogCommon.LogTimerInterval;
}
KERNEL_END

namespace YAML
{
    // LogFileDefine序列化反序列化
    template<>
   struct KERNEL_EXPORT convert<KERNEL_NS::LogFileDefine>
    {
        static Node encode(const KERNEL_NS::LogFileDefine& rhs)
        {
            Node node;
            node["FileId"] = rhs.FileId;
            node["FileName"] = rhs.FileName.GetRaw();
            node["ThreadRelationshipId"] = rhs.ThreadRelationshipId;
            return node;
        }

        static bool decode(const Node& node,  KERNEL_NS::LogFileDefine& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            rhs.FileId = node["FileId"].as<Int32>();
            auto &fileNameNode = node["FileName"];
            if(!fileNameNode.IsNull())
                rhs.FileName = fileNameNode.as<std::string>();
            rhs.ThreadRelationshipId = node["ThreadRelationshipId"].as<Int32>();
            
            return true;
        }
    };

    // LogPathCfg
    template<>
    struct KERNEL_EXPORT convert<KERNEL_NS::LogPathCfg>
    {
        static Node encode(const KERNEL_NS::LogPathCfg& rhs)
        {
            Node node;
            node["IsUseProgramNameAsFirstLevelPath"] = rhs.IsUseProgramNameAsFirstLevelPath ? 1 : 0;
            node["PartPath"] = rhs.PartPath.GetRaw();
            return node;
        }

        static bool decode(const Node& node, KERNEL_NS::LogPathCfg& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            rhs.IsUseProgramNameAsFirstLevelPath = node["IsUseProgramNameAsFirstLevelPath"].as<Int32>() != 0;

            if(!node["PartPath"].IsNull())
                rhs.PartPath = node["PartPath"].as<std::string>();
            
            return true;
        }
    };

    // LogCommonCfg
    template<>
    struct KERNEL_EXPORT convert<KERNEL_NS::LogCommonCfg>
    {
        static Node encode(const KERNEL_NS::LogCommonCfg& rhs)
        {
            Node finalNode;
            finalNode["LogFileDefineList"] = rhs.LogFileDefineList;
            finalNode["ExtName"] = rhs.ExtName.GetRaw();
            finalNode["LogTimerInterval"] = rhs.LogTimerInterval.ToString().GetRaw();
            finalNode["MaxFileSizeMB"] = rhs.MaxFileSizeMB;
            finalNode["MaxLogCacheMB"] = rhs.MaxLogCacheMB;
            finalNode["IsEnableLog"] = rhs.IsEnableLog ? 1 : 0;
            finalNode["LogPath"] = rhs.LogPath;

            return finalNode;
        }

        static bool decode(const Node& node, KERNEL_NS::LogCommonCfg& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            auto &listNode = node["LogFileDefineList"];
            if(listNode.IsSequence())
                rhs.LogFileDefineList = listNode.as<std::vector<KERNEL_NS::LogFileDefine>>();

            if(!node["ExtName"].IsNull())
                rhs.ExtName = node["ExtName"].as<std::string>();

            if(!node["LogTimerInterval"].IsNull())
                rhs.LogTimerInterval = KERNEL_NS::TimeSlice(node["LogTimerInterval"].as<std::string>());

            rhs.MaxFileSizeMB = node["MaxFileSizeMB"].as<Int32>();
            rhs.MaxLogCacheMB = node["MaxLogCacheMB"].as<Int32>();
            rhs.IsEnableLog = node["IsEnableLog"].as<Int32>() != 0;

            auto &pathNode = node["LogPath"];
            if(pathNode.IsMap())
                rhs.LogPath = pathNode.as<KERNEL_NS::LogPathCfg>();

            return true;
        }
    };

    // LogLevelInfoCfg
    template<>
    struct KERNEL_EXPORT convert<KERNEL_NS::LogLevelInfoCfg>
    {
        static Node encode(const KERNEL_NS::LogLevelInfoCfg& rhs)
        {
            Node finalNode;
            finalNode["Enable"] = rhs.Enable ? 1 : 0;
            finalNode["LevelName"] = rhs.LevelName.GetRaw();
            finalNode["FileId"] = rhs.FileId;
            finalNode["EnableConsole"] = rhs.EnableConsole ? 1 : 0;

            finalNode["FgColor"] = KERNEL_NS::ConsoleColorHelper::GetFrontColorStr(rhs.FgColor).GetRaw();
            finalNode["BgColor"] = KERNEL_NS::ConsoleColorHelper::GetBackColorStr(rhs.BgColor).GetRaw();
            finalNode["LevelId"] = rhs.LevelId;
            finalNode["EnableRealTime"] = rhs.EnableRealTime ? 1 : 0;
            finalNode["PRINT_STACK_TRACE_BACK"] = rhs.PRINT_STACK_TRACE_BACK ? 1 : 0;
            finalNode["NEED_WRITE_FILE"] = rhs.NEED_WRITE_FILE ? 1 : 0;

            return finalNode;
        }

        static bool decode(const Node& node, KERNEL_NS::LogLevelInfoCfg& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            rhs.Enable = node["Enable"].as<Int32>() != 0;
            rhs.LevelName = node["LevelName"].as<std::string>();
            rhs.FileId = node["FileId"].as<Int32>();
            rhs.EnableConsole = node["EnableConsole"].as<Int32>() != 0;

            auto &fgColorStrNode = node["FgColor"];
            rhs.FgColor = KERNEL_NS::ConsoleColorHelper::GetFrontColor("White");
            if(!fgColorStrNode.IsNull())
                rhs.FgColor = KERNEL_NS::ConsoleColorHelper::GetFrontColor(fgColorStrNode.as<std::string>());

            rhs.BgColor = 0;
            auto &backNode = node["BgColor"];
            if(!backNode.IsNull())
                rhs.BgColor = KERNEL_NS::ConsoleColorHelper::GetBackColor(backNode.as<std::string>());

            rhs.LevelId = node["LevelId"].as<Int32>();
            rhs.EnableRealTime = node["EnableRealTime"].as<Int32>() != 0;
            rhs.PRINT_STACK_TRACE_BACK = node["PRINT_STACK_TRACE_BACK"].as<Int32>() != 0;
            rhs.NEED_WRITE_FILE = node["NEED_WRITE_FILE"].as<Int32>() != 0;

            return true;
        }
    };

    // LogLevelInfoCfg
    template<>
    struct KERNEL_EXPORT convert<KERNEL_NS::LogCfg>
    {
        static Node encode(const KERNEL_NS::LogCfg& rhs);
        
        static bool decode(const Node& node, KERNEL_NS::LogCfg& rhs);
    };
}

#endif
