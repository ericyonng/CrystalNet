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

KERNEL_BEGIN

// 日志文件配置
struct KERNEL_EXPORT LogFileDefine
{
    // 文件id
    Int32 FileId = 0;
    // 文件名
    LibString FileName;
    // 线程关联性id, 线程关联性id一样的log文件会被分在同一个线程, 利用多线程来处理日志
    Int32 ThreadRelationshipId = 0;
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
    // 是否产出日志
    bool IsEnableLog = true;

    // 日志路径配置
    LogPathCfg LogPath;
};

// 日志等级配置
struct KERNEL_EXPORT LogLevelInfoCfg
{
    bool Enable = true;
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

    static LogCfg *CreateNewObj(LogCfg &&cfg)
    {
        return new LogCfg(std::move(cfg));
    }
    
    void Release()
    {
        delete this;
    }
};

KERNEL_END

namespace YAML
{
    // LogFileDefine序列化反序列化
    template<>
   struct convert<KERNEL_NS::LogFileDefine>
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
            rhs.FileName = node["FileName"].as<std::string>();
            rhs.ThreadRelationshipId = node["ThreadRelationshipId"].as<Int32>();
            
            return true;
        }
    };

    // LogPathCfg
    template<>
    struct convert<KERNEL_NS::LogPathCfg>
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
            rhs.PartPath = node["PartPath"].as<std::string>();
            
            return true;
        }
    };

    // LogCommonCfg
    template<>
    struct convert<KERNEL_NS::LogCommonCfg>
    {
        static Node encode(const KERNEL_NS::LogCommonCfg& rhs)
        {
            Node finalNode;
            finalNode["LogFileDefineList"] = rhs.LogFileDefineList;
            finalNode["ExtName"] = rhs.ExtName.GetRaw();
            finalNode["LogTimerInterval"] = rhs.LogTimerInterval.ToString().GetRaw();
            finalNode["MaxFileSizeMB"] = rhs.MaxFileSizeMB;
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

            rhs.LogFileDefineList = node["LogFileDefineList"].as<std::vector<KERNEL_NS::LogFileDefine>>();
            rhs.ExtName = node["ExtName"].as<std::string>();
            rhs.LogTimerInterval = KERNEL_NS::TimeSlice(node["LogTimerInterval"].as<std::string>());
            rhs.MaxFileSizeMB = node["MaxFileSizeMB"].as<Int32>();
            rhs.IsEnableLog = node["IsEnableLog"].as<Int32>() != 0;
            rhs.LogPath = node["LogPath"].as<KERNEL_NS::LogPathCfg>();

            return true;
        }


        // LogLevelInfoCfg
        template<>
        struct convert<KERNEL_NS::LogLevelInfoCfg>
        {
            static Node encode(const KERNEL_NS::LogLevelInfoCfg& rhs)
            {
                Node finalNode;
                finalNode["Enable"] = rhs.Enable ? 1 : 0;
                finalNode["FileId"] = rhs.FileId;
                finalNode["EnableConsole"] = rhs.EnableConsole ? 1 : 0;

                // TODO:fgColor需要转成字符串
                finalNode["FgColor"] = rhs.FgColor;
            }
        };
    };
}

#endif
