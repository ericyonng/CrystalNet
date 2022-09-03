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
 * Date: 2021-02-17 19:08:19
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/source/comp/Log/LogIniCfgMgr.h>
#include <kernel/comp/File/LibIniFile.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/File/ConsoleConfigMgr.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Utils/FileUtil.h>

#undef LOG_INI_COMMON_SEG
#define LOG_INI_COMMON_SEG "Common"

KERNEL_BEGIN

LogIniCfgMgr::LogIniCfgMgr()
    :_iniFile(NULL)
    ,_maxLogCacheBytes(0)
    ,_maxFileId(0)
    ,_isEnableLog(true)
{

}

LogIniCfgMgr::~LogIniCfgMgr()
{
    Close();
}

bool LogIniCfgMgr::Init(const Byte8 *iniFileName)
{
    if(UNLIKELY(_iniFile))
    {
        CRYSTAL_TRACE("file[%s], LogIniCfgMgr already init", iniFileName);
        return false;
    }

    // 文件是否存在
    if(UNLIKELY(!FileUtil::IsFileExist(iniFileName)))
    {
        CRYSTAL_TRACE("file[%s], not exist when init LogIniCfgMgr", iniFileName);
        return false;
    }

    _iniFile = LibIniFile::New_LibIniFile();

    if(!_iniFile->Init(iniFileName, false))
    {
        CRYSTAL_TRACE("file[%s], init ini file fail when init LogIniCfgMgr", iniFileName);
        return false;
    }

    // 扫描日志等级id, 生成等级id与对应字符的映射字典
    {
        auto levelCfgs = _iniFile->GetSegmentCfgs("LogLevel");
        if(levelCfgs && !levelCfgs->empty())
        {
            // 遍历获取levelid最大值 _levelInfos size = maxid + 1
            const LibString sep = ",";
            for(auto iter = levelCfgs->begin(); iter != levelCfgs->end(); ++iter)
            {
                const auto &levelName = iter->first;
                const auto &levelCfgStr = iter->second;
                const auto &cfgCache = levelCfgStr.strip();
                auto cfgStrings = cfgCache.Split(sep);
                auto &cfgStr = cfgStrings[LogLevelValue::LEVEL_ID].strip();
                const Int32 levelId = StringUtil::StringToInt32(cfgStr.c_str());

                if(static_cast<Int32>(_levelInfos.size()) <= levelId)
                    _levelInfos.resize(levelId + 1);

                if(_levelInfos[levelId])
                {
                    CRYSTAL_TRACE("repeat levelid[%id], will replace old level[%s] with new level[%s]"
                    , levelId, _levelInfos[levelId]->_levelName.c_str(), levelName.c_str());

                    CRYSTAL_DELETE_SAFE(_levelInfos[levelId]);
                }

                auto newLevelInfo = new LogLevelInfo;
                newLevelInfo->_levelId = levelId;
                newLevelInfo->_levelName = levelName;
                _levelInfos[levelId] = newLevelInfo;
            }
        }
    }

    // 读取日志等级配置
    std::map<Int32, std::vector<LogLevelCfg *>> fileIdRefLogLeveCfgs;
    {
        auto colorMgr = ConsoleConfigMgr::GetInstance();
        auto &fgColor = colorMgr->GetFrontColor();
        auto &bgColor = colorMgr->GetBackColor();
        const LibString sep = ",";
        const LibString colorSep = "|";
        auto levelCfgs = _iniFile->GetSegmentCfgs("LogLevel");
        if(levelCfgs && !levelCfgs->empty())
        {
            const Int32 levelCount = static_cast<Int32>(_levelInfos.size());
            _levelIdxRefCfg.resize(levelCount);

            for(Int32 idx = 0; idx < levelCount; ++idx)
            {
                // 获取日志key名
                auto levelInfo = _levelInfos[idx];
                if(levelInfo)
                {
                    auto iterLevelCfg = levelCfgs->find(levelInfo->_levelName);
                    if(iterLevelCfg != levelCfgs->end())
                    {
                        const auto &levelCfgString = iterLevelCfg->second;

                        // 逗号切割 
                        const auto &cfgCache = levelCfgString.strip();
                        auto cfgStrings = cfgCache.Split(sep);
                        auto newLevelCfg = new LogLevelCfg;
                        newLevelCfg->_level = idx;
                        newLevelCfg->_levelName = levelInfo->_levelName;

                        // for(Int32 jdx = 0; jdx < cfgStrings.size(); ++ jdx)
                        //       CRYSTAL_TRACE("desc[%s] cfgStrings = jdx[%d] [%s]", desc, jdx, cfgStrings[jdx].c_str());

                        // [enable],[fileid],[enableConsole],[fgcolor]|[bgcolor]
                        newLevelCfg->_enable = (StringUtil::StringToInt32(cfgStrings[LogLevelValue::ENALBLE].strip().c_str()) != 0);
                        newLevelCfg->_fileId = (StringUtil::StringToInt32(cfgStrings[LogLevelValue::FILE_ID].strip().c_str()));
                        newLevelCfg->_enableConsole = (StringUtil::StringToInt32(cfgStrings[LogLevelValue::ENABLE_CONSOLE].strip().c_str()) != 0);
                        
                        // 默认颜色配置
                        newLevelCfg->_fgColor = fgColor._frontDefault;
                        newLevelCfg->_bgColor = bgColor._backDefault;

                        // CRYSTAL_TRACE("desc[%s] enable[%d]", desc, newLevelCfg->_enable);

                        // 颜色配置
                        if(cfgStrings.size() >= 4)
                        {
                            auto fgbgColorPairs = cfgStrings[LogLevelValue::FG_BG_COLOR].strip().Split(colorSep);
                            // 前景色
                            if(fgbgColorPairs.size() >= 1)
                                newLevelCfg->_fgColor = colorMgr->GetFrontColor(fgbgColorPairs[0].c_str());
                            // 背景色
                            if(fgbgColorPairs.size() >= 2)
                                newLevelCfg->_bgColor = colorMgr->GetBackColor(fgbgColorPairs[1].c_str());
                        }

                        // 收集日志配置对应的日志等级
                        auto iterLogLevelCfgs = fileIdRefLogLeveCfgs.find(newLevelCfg->_fileId);
                        if(iterLogLevelCfgs == fileIdRefLogLeveCfgs.end())
                        {
                            iterLogLevelCfgs = fileIdRefLogLeveCfgs.insert(std::make_pair(newLevelCfg->_fileId, std::vector<LogLevelCfg *>())).first;
                            iterLogLevelCfgs->second.resize(levelCount);
                        }
                        iterLogLevelCfgs->second[idx] = newLevelCfg;

                        // 开启实时着盘标记
                        newLevelCfg->_enableRealTime = (StringUtil::StringToInt32(cfgStrings[LogLevelValue::ENABLE_REAL_TIME].strip().c_str()) != 0);

                        // 开启打印堆栈
                        newLevelCfg->_printStackTraceBack = (StringUtil::StringToInt32(cfgStrings[LogLevelValue::PRINT_STACK_TRACE_BACK].c_str()) != 0);

                        _levelIdxRefCfg[idx] = newLevelCfg;
                    }

                }
            }
        }
    }

    // 读取公共配置
    {
        _maxLogCacheBytes = _iniFile->ReadInt("Common", "MaxLogCacheMB", 0) * 1024 * 1024;
        _isEnableLog = _iniFile->ReadInt("Common", "IsEnableLog", 0) != 0;

        LibString logPath;
        _iniFile->ReadStr(LOG_INI_COMMON_SEG, "LogPath",  "", logPath);
        if(logPath.empty())
        {// 缺乏配置按默认路径
            _logPath = SystemUtil::GetCurProgRootPath() + SystemUtil::GetCurProgramNameWithoutExt() + "_Log/";
        }
        else
        {
            // 切割
            auto pieces = logPath.Split(',', -1, true);
            if(pieces.empty() || (pieces[0].empty() && pieces[1].empty()))
            {// 该字段没有配置则按照默认的
                _logPath = SystemUtil::GetCurProgRootPath() + SystemUtil::GetCurProgramNameWithoutExt() + "_Log/";
            }
            else if(pieces[0].empty() && !pieces[1].empty())
            {// 使用程序名当日志的根目录
                _logPath = pieces[1] + SystemUtil::GetCurProgramNameWithoutExt() + "/";
            }
            else if(!pieces[0].empty() && pieces[1].empty())
            {
                if(StringUtil::StringToUInt32(pieces[0].c_str()) == 0)
                {// 不使用程序名指定
                    _logPath = SystemUtil::GetCurProgRootPath();
                }
                else
                {
                    _logPath = SystemUtil::GetCurProgRootPath() + SystemUtil::GetCurProgramNameWithoutExt() + "_Log/";
                }
            }
            else if(!pieces[0].empty() && !pieces[1].empty())
            {
                if(StringUtil::StringToUInt32(pieces[0].c_str()) == 0)
                {// 不使用程序名指定
                    _logPath = pieces[1];
                }
                else
                {
                    _logPath = pieces[1] + SystemUtil::GetCurProgramNameWithoutExt() + "_Log/";
                }
            }

            ASSERT(!_logPath.empty());
        }

        // 读取文件名
        LibString fileNameCfg;
        if(!_iniFile->ReadStr("Common", "FileName", NULL, fileNameCfg))
        {
            CRYSTAL_TRACE("Common FileName not found when LogIniCfgMgr init");
            return false;
        }

        // 读取拓展名,着盘间隔,单文件最大尺寸
        LibString extName;
        _iniFile->ReadStr("Common", "ExtName", ".log", extName);
        _intervalMs = static_cast<Int32>(_iniFile->ReadInt("Common", "LogTimerIntervalMs", 1000));
        const Int64 maxFileSize = _iniFile->ReadInt("Common", "MaxFileSizeMB", 256) * 1024 * 1024;

        // 文件名切分
        const LibString fileSep = "|";
        auto fileCfgs = fileNameCfg.Split(fileSep);
        
        const Int32 fileCount = static_cast<Int32>(fileCfgs.size());
        //_fileIdIdxRefCfg.resize(fileCount);

        const LibString idNameSep = ",";
        const auto &curProgName = SystemUtil::GetCurProgramNameWithoutExt();
        for(Int32 idx = 0; idx < fileCount; ++idx)
        {
            const auto &fileCfg = fileCfgs[idx];

            // 切分文件id和文件名
            auto fileIdNamePairs = fileCfg.Split(idNameSep, -1, false, true);
            // 文件名为空则使用当前程序名  
            if(fileIdNamePairs[1].empty())
                fileIdNamePairs[1] = curProgName;
            
            const Int32 fileId = StringUtil::StringToInt32(fileIdNamePairs[0].c_str());
            auto newLogCfg = new LogConfig;
            newLogCfg->_logFileId = fileId;

            // 关联的等级配置
            auto iterLevels = fileIdRefLogLeveCfgs.find(fileId);
            if(iterLevels != fileIdRefLogLeveCfgs.end())
                newLogCfg->_levelIdxRefCfg = iterLevels->second;

            // 只要有一个级别的日志开启则该日志文件即开启
            auto &levelIdxRefCfgs = newLogCfg->_levelIdxRefCfg;
            for(auto cfg:levelIdxRefCfgs )
            {
                if(cfg && cfg->_enable)
                {
                    newLogCfg->_isEnable = true;
                    break;
                }
            }

            // 剩余配置
            newLogCfg->_logFileName = fileIdNamePairs[1];
            newLogCfg->_threadRelationId = StringUtil::StringToInt32(fileIdNamePairs[2].c_str());
            newLogCfg->_extName = extName;
            newLogCfg->_intervalMs = _intervalMs;
            newLogCfg->_maxFileSize = maxFileSize;

            // CRYSTAL_TRACE("file name[%s] enable[%d]", newLogCfg->_logFileName.c_str(), newLogCfg->_isEnable);

            // 扩展空间
            if(fileId >= static_cast<Int32>(_fileIdIdxRefCfg.size()))
                _fileIdIdxRefCfg.resize(fileId + 1);

            // 最大的文件id
            if(_maxFileId < fileId)
                _maxFileId = fileId;

            _fileIdIdxRefCfg[fileId] = newLogCfg;

            // 日志与线程绑定关系
            auto iterRelation = _threadRelationIdRefLogCfgs.find(newLogCfg->_threadRelationId);
            if(iterRelation == _threadRelationIdRefLogCfgs.end())
                iterRelation = _threadRelationIdRefLogCfgs.insert(std::make_pair(newLogCfg->_threadRelationId, std::vector<LogConfig *>())).first;
            iterRelation->second.push_back(newLogCfg);
        }
    }
    

    return true;
}

void LogIniCfgMgr::Close()
{
    if(_iniFile)
        LibIniFile::Delete_LibIniFile(_iniFile);

    _iniFile = NULL;

    if(!_levelIdxRefCfg.empty())
        ContainerUtil::DelContainer(_levelIdxRefCfg);
    if(!_fileIdIdxRefCfg.empty())
        ContainerUtil::DelContainer(_fileIdIdxRefCfg);
    if(!_levelInfos.empty())
        ContainerUtil::DelContainer(_levelInfos);
}

KERNEL_END