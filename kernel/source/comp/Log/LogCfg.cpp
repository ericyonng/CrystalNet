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
// Date: 2026-02-05 01:02:02
// Author: Eric Yonng
// Description:


#include <pch.h>
#include <kernel/comp/Log/LogCfg.h>
#include <kernel/comp/Utils/SystemUtil.h>

namespace YAML
{
    
    // LogLevelInfoCfg
    Node convert<KERNEL_NS::LogCfg>::encode(const KERNEL_NS::LogCfg& rhs)
    {
        Node finalNode;
        finalNode["LogCommon"] = rhs.LogCommon;
        finalNode["LogLevelList"] = rhs.LogLevelList;
        return finalNode;
    }

    bool convert<KERNEL_NS::LogCfg>::decode(const Node& node, KERNEL_NS::LogCfg& rhs)
    {
        if(!node.IsMap())
        {
            return false;
        }

        auto &commonNode = node["LogCommon"];
        if(commonNode.IsMap())
            rhs.LogCommon = commonNode.as<KERNEL_NS::LogCommonCfg>();

        rhs.MaxLogSizeBytes = static_cast<Int64>(rhs.LogCommon.MaxFileSizeMB) * 1024 * 1024;
        rhs.MaxLogCacheSizeBytes = static_cast<Int64>(rhs.LogCommon.MaxLogCacheMB) * 1024 * 1024;

        auto &listNode = node["LogLevelList"];
        if(listNode.IsSequence())
            rhs.LogLevelList = listNode.as<std::vector<KERNEL_NS::LogLevelInfoCfg>>();

        // 加工 LogLevelList让LevelId充当索引
        std::vector<std::vector<KERNEL_NS::LogLevelInfoCfg *>> fileIdRefLogLevels;
        if(!rhs.LogLevelList.empty())
        {
            Int32 maxLevelId = 0;
            Int32 maxFileId = 0;
            for(auto &levelInfo : rhs.LogLevelList)
            {
                if(levelInfo.LevelId > maxLevelId)
                {
                    maxLevelId = levelInfo.LevelId;
                }

                if(maxFileId < levelInfo.FileId)
                    maxFileId = levelInfo.FileId;
            }

            fileIdRefLogLevels.resize(maxFileId + 1);

            rhs.LogLevelIndexRefCfg.resize(maxLevelId + 1);
            const Int32 count = static_cast<Int32>(rhs.LogLevelList.size());
            for(Int32 i = 0; i < count; ++i)
            {
                auto &levelInfo = rhs.LogLevelList[i];

                if(rhs.LogLevelIndexRefCfg[levelInfo.LevelId])
                {
                    CRYSTAL_TRACE("repeat levelid[%id], will replace old level[%s] with new level[%s]"
                            , levelInfo.LevelId, rhs.LogLevelIndexRefCfg[levelInfo.LevelId]->LevelName.c_str()
                            , levelInfo.LevelName.c_str())
                }

                rhs.LogLevelIndexRefCfg[levelInfo.LevelId] = &levelInfo;

                // 与文件id关联
                auto &logLevels = fileIdRefLogLevels[levelInfo.FileId];
                if((levelInfo.LevelId + 1) > static_cast<Int32>(logLevels.size()))
                    logLevels.resize(levelInfo.LevelId + 1);
                logLevels[levelInfo.LevelId] = &levelInfo;
            }
        }

        // 日志路径
        auto &logPath = rhs.LogCommon.LogPath;
        if(logPath.PartPath.empty())
        {
            // 使用程序所在路径, 且日志目录名为程序名
            if(logPath.IsUseProgramNameAsFirstLevelPath)
            {
                rhs.FinalLogPath = KERNEL_NS::SystemUtil::GetCurProgRootPath() +  KERNEL_NS::SystemUtil::GetCurProgramNameWithoutExt() + "_Log/";
            }
            // 使用程序所在路径直接做日志路径
            else
            {
                rhs.FinalLogPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
            }
        }
        // 使用指定路径 + 程序名作为日志目录
        else if(!logPath.PartPath.empty())
        {
            if(logPath.IsUseProgramNameAsFirstLevelPath)
            {
                rhs.FinalLogPath = logPath.PartPath + KERNEL_NS::SystemUtil::GetCurProgramNameWithoutExt() + "_Log/";
            }
            else
            {
                rhs.FinalLogPath = logPath.PartPath;
            }
        }

        if(UNLIKELY(rhs.FinalLogPath.empty()))
        {
            CRYSTAL_TRACE("LogPath log path is empty");
        }

        // 文件部分
        {
            const auto &curProgName = KERNEL_NS::SystemUtil::GetCurProgramNameWithoutExt();

            rhs.FileIdRefLevelInfoCfgs = fileIdRefLogLevels;

            const Int32 count = static_cast<Int32>(rhs.LogCommon.LogFileDefineList.size());
            Int32 maxFileId = 0;
            for(Int32 i = 0; i < count; ++i)
            {
                auto &logFileDefine = rhs.LogCommon.LogFileDefineList[i];
                if(logFileDefine.FileName.empty())
                {
                    logFileDefine.FileName = curProgName;
                }

                logFileDefine.ExtName = rhs.LogCommon.ExtName;

                if(maxFileId < logFileDefine.FileId)
                    maxFileId = logFileDefine.FileId;

                if(static_cast<Int32>(fileIdRefLogLevels.size()) > logFileDefine.FileId)
                {
                    auto &fileLogLevels = fileIdRefLogLevels[logFileDefine.FileId];
                    for(auto levelInfo : fileLogLevels)
                    {
                        if(!levelInfo)
                            continue;

                        if(levelInfo->Enable)
                            logFileDefine.IsEnable = true;
                        if(levelInfo->NEED_WRITE_FILE)
                            logFileDefine.NeedWriteFile = true;
                    }
                }
            }

            rhs.MaxLogFileId = maxFileId;

            // 文件id与配置映射
            rhs.FileIdRefFileDefine.resize(maxFileId + 1);
            for(Int32 i = 0; i < count; ++i)
            {
                auto &fileDefine = rhs.LogCommon.LogFileDefineList[i];
                rhs.FileIdRefFileDefine[fileDefine.FileId] = &fileDefine;

                auto iterThread = rhs.ThreadRelationIdRefFileDefines.find(fileDefine.ThreadRelationshipId);
                if(iterThread == rhs.ThreadRelationIdRefFileDefines.end())
                    iterThread = rhs.ThreadRelationIdRefFileDefines.insert(std::make_pair(fileDefine.ThreadRelationshipId, std::vector<KERNEL_NS::LogFileDefine *>())).first;

                iterThread->second.push_back(&fileDefine);
            }
        }

        return true;
    }
}