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
#include <kernel/comp/Utils/DirectoryUtil.h>

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

bool LogIniCfgMgr::Init(const Byte8 *iniFileName, const LibString &fromMemory, const LibString &consoleConfigContent)
{
    if(UNLIKELY(_iniFile))
    {
        CRYSTAL_TRACE("file[%s], LogIniCfgMgr already init", iniFileName);
        return false;
    }

    // ??????????????????
    if(fromMemory.empty())
    {
        if(UNLIKELY(!FileUtil::IsFileExist(iniFileName)))
        {
            CRYSTAL_TRACE("file[%s], not exist when init LogIniCfgMgr", iniFileName);
            return false;
        }
    }

    _iniFile = LibIniFile::New_LibIniFile();

    if(!fromMemory.empty())
        _iniFile->SetMemoryIniContent(fromMemory);

    if(!_iniFile->Init(fromMemory.empty() ? iniFileName : NULL, false))
    {
        CRYSTAL_TRACE("file[%s], init ini file fail when init LogIniCfgMgr", iniFileName);
        return false;
    }

    // ??????????????????id, ????????????id??????????????????????????????
    {
        auto levelCfgs = _iniFile->GetSegmentCfgs("LogLevel");
        if(levelCfgs && !levelCfgs->empty())
        {
            // ????????????levelid????????? _levelInfos size = maxid + 1
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

    // ????????????????????????
    std::map<Int32, std::vector<LogLevelCfg *>> fileIdRefLogLeveCfgs;
    {
        // ??????????????????????????????
        LibString path;
        SystemUtil::GetProgramPath(true, path);
        auto colorMgr = ConsoleConfigMgr::GetInstance();
        colorMgr->Init("ConsoleConfig.ini", (DirectoryUtil::GetFileDirInPath(path) + ROOT_DIR_INI_SUB_DIR).c_str(), consoleConfigContent);

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
                // ????????????key???
                auto levelInfo = _levelInfos[idx];
                if(levelInfo)
                {
                    auto iterLevelCfg = levelCfgs->find(levelInfo->_levelName);
                    if(iterLevelCfg != levelCfgs->end())
                    {
                        const auto &levelCfgString = iterLevelCfg->second;

                        // ???????????? 
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
                        
                        // ??????????????????
                        newLevelCfg->_fgColor = fgColor._frontDefault;
                        newLevelCfg->_bgColor = bgColor._backDefault;

                        // CRYSTAL_TRACE("desc[%s] enable[%d]", desc, newLevelCfg->_enable);

                        // ????????????
                        if(cfgStrings.size() >= 4)
                        {
                            auto fgbgColorPairs = cfgStrings[LogLevelValue::FG_BG_COLOR].strip().Split(colorSep);
                            // ?????????
                            if(fgbgColorPairs.size() >= 1)
                                newLevelCfg->_fgColor = colorMgr->GetFrontColor(fgbgColorPairs[0].c_str());
                            // ?????????
                            if(fgbgColorPairs.size() >= 2)
                                newLevelCfg->_bgColor = colorMgr->GetBackColor(fgbgColorPairs[1].c_str());
                        }

                        // ???????????????????????????????????????
                        auto iterLogLevelCfgs = fileIdRefLogLeveCfgs.find(newLevelCfg->_fileId);
                        if(iterLogLevelCfgs == fileIdRefLogLeveCfgs.end())
                        {
                            iterLogLevelCfgs = fileIdRefLogLeveCfgs.insert(std::make_pair(newLevelCfg->_fileId, std::vector<LogLevelCfg *>())).first;
                            iterLogLevelCfgs->second.resize(levelCount);
                        }
                        iterLogLevelCfgs->second[idx] = newLevelCfg;

                        // ????????????????????????
                        newLevelCfg->_enableRealTime = (StringUtil::StringToInt32(cfgStrings[LogLevelValue::ENABLE_REAL_TIME].strip().c_str()) != 0);

                        // ??????????????????
                        newLevelCfg->_printStackTraceBack = (StringUtil::StringToInt32(cfgStrings[LogLevelValue::PRINT_STACK_TRACE_BACK].c_str()) != 0);

                        // ????????????????????????
                        newLevelCfg->_needWriteFile = (StringUtil::StringToInt32(cfgStrings[LogLevelValue::NEED_WRITE_FILE].strip().c_str()) != 0);

                        _levelIdxRefCfg[idx] = newLevelCfg;
                    }

                }
            }
        }
    }

    // ??????????????????
    {
        _maxLogCacheBytes = _iniFile->ReadInt("Common", "MaxLogCacheMB", 0) * 1024 * 1024;
        _isEnableLog = _iniFile->ReadInt("Common", "IsEnableLog", 0) != 0;

        LibString logPath;
        _iniFile->ReadStr(LOG_INI_COMMON_SEG, "LogPath",  "", logPath);
        if(logPath.empty())
        {// ???????????????????????????
            _logPath = SystemUtil::GetCurProgRootPath() + SystemUtil::GetCurProgramNameWithoutExt() + "_Log/";
        }
        else
        {
            // ??????
            auto pieces = logPath.Split(',', -1, true);
            if(pieces.empty() || (pieces[0].empty() && pieces[1].empty()))
            {// ???????????????????????????????????????
                _logPath = SystemUtil::GetCurProgRootPath() + SystemUtil::GetCurProgramNameWithoutExt() + "_Log/";
            }
            else if(pieces[0].empty() && !pieces[1].empty())
            {// ????????????????????????????????????
                _logPath = pieces[1] + SystemUtil::GetCurProgramNameWithoutExt() + "/";
            }
            else if(!pieces[0].empty() && pieces[1].empty())
            {
                if(StringUtil::StringToUInt32(pieces[0].c_str()) == 0)
                {// ????????????????????????
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
                {// ????????????????????????
                    _logPath = pieces[1];
                }
                else
                {
                    _logPath = pieces[1] + SystemUtil::GetCurProgramNameWithoutExt() + "_Log/";
                }
            }

            ASSERT(!_logPath.empty());
            if(UNLIKELY(_logPath.empty()))
            {
                CRYSTAL_TRACE("LogPath log path is empty");
                return false;
            }
        }

        // ???????????????
        LibString fileNameCfg;
        if(!_iniFile->ReadStr("Common", "FileName", NULL, fileNameCfg))
        {
            CRYSTAL_TRACE("Common FileName not found when LogIniCfgMgr init");
            return false;
        }

        // ???????????????,????????????,?????????????????????
        LibString extName;
        _iniFile->ReadStr("Common", "ExtName", ".log", extName);
        _intervalMs = static_cast<Int32>(_iniFile->ReadInt("Common", "LogTimerIntervalMs", 1000));
        const Int64 maxFileSize = _iniFile->ReadInt("Common", "MaxFileSizeMB", 256) * 1024 * 1024;

        // ???????????????
        const LibString fileSep = "|";
        auto fileCfgs = fileNameCfg.Split(fileSep);
        
        const Int32 fileCount = static_cast<Int32>(fileCfgs.size());
        //_fileIdIdxRefCfg.resize(fileCount);

        const LibString idNameSep = ",";
        const auto &curProgName = SystemUtil::GetCurProgramNameWithoutExt();
        for(Int32 idx = 0; idx < fileCount; ++idx)
        {
            const auto &fileCfg = fileCfgs[idx];

            // ????????????id????????????
            auto fileIdNamePairs = fileCfg.Split(idNameSep, -1, false, true);
            // ???????????????????????????????????????  
            if(fileIdNamePairs[1].empty())
                fileIdNamePairs[1] = curProgName;
            
            const Int32 fileId = StringUtil::StringToInt32(fileIdNamePairs[0].c_str());
            auto newLogCfg = new LogConfig;
            newLogCfg->_logFileId = fileId;

            // ?????????????????????
            auto iterLevels = fileIdRefLogLeveCfgs.find(fileId);
            if(iterLevels != fileIdRefLogLeveCfgs.end())
                newLogCfg->_levelIdxRefCfg = iterLevels->second;

            // ???????????????????????????????????????????????????????????????
            auto &levelIdxRefCfgs = newLogCfg->_levelIdxRefCfg;
            for(auto cfg:levelIdxRefCfgs )
            {
                if(cfg && cfg->_enable)
                {
                    newLogCfg->_isEnable = true;
                    break;
                }
            }

            // ?????????????????????????????????????????????????????????????????????
            for(auto cfg:levelIdxRefCfgs )
            {
                if(cfg && cfg->_needWriteFile)
                {
                    newLogCfg->_needWriteFile = true;
                    break;
                }
            }

            // ????????????
            newLogCfg->_logFileName = fileIdNamePairs[1];
            newLogCfg->_threadRelationId = StringUtil::StringToInt32(fileIdNamePairs[2].c_str());
            newLogCfg->_extName = extName;
            newLogCfg->_intervalMs = _intervalMs;
            newLogCfg->_maxFileSize = maxFileSize;

            // CRYSTAL_TRACE("file name[%s] enable[%d]", newLogCfg->_logFileName.c_str(), newLogCfg->_isEnable);

            // ????????????
            if(fileId >= static_cast<Int32>(_fileIdIdxRefCfg.size()))
                _fileIdIdxRefCfg.resize(fileId + 1);

            // ???????????????id
            if(_maxFileId < fileId)
                _maxFileId = fileId;

            _fileIdIdxRefCfg[fileId] = newLogCfg;

            // ???????????????????????????
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