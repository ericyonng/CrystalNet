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
 * Date: 2021-02-07 22:22:59
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/File/LibIniFile.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/File/IniFileDefs.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibIniFile);

LibIniFile::LibIniFile()
    :_isDirtied(false)
    ,_maxLine(0)
{

}

LibIniFile::~LibIniFile()
{
    _UpdateIni();
}

bool LibIniFile::Init(const Byte8 *path, bool createIfNotExist)
{
    // detect if file is existed
    if(path && (::strlen(path) > 0))
    {
        if(UNLIKELY(!FileUtil::IsFileExist(path)))
        {
            auto fp = FileUtil::OpenFile(path, createIfNotExist);
            if(UNLIKELY(!fp))
                return false;

            FileUtil::CloseFile(*fp);
        }

        _filePath = path;
    }

    return _Init();
}

void LibIniFile::Clear()
{
    _filePath.clear();
    _isDirtied = false;
    _maxLine = 0;
    _lineRefContent.clear();
    _segOrKeyRefLine.clear();
    _segmentRefKeyValues.clear();
    _segmentRefMaxValidLine.clear();
    _fromMemory.clear();
    _fileContent.clear();
}

void LibIniFile::Flush()
{
    if (!_filePath.empty())
    {// 文件存在时才更新
        if (!FileUtil::IsFileExist(_filePath.c_str()))
            return;
    }

    _UpdateIni(true);
}

const LibString &LibIniFile::GetPath() const
{
    return _filePath;
}

const LibString &LibIniFile::GetFileContent() const
{
    return _fileContent;
}

void LibIniFile::Lock()
{
    _lock.Lock();
}

void LibIniFile::Unlock()
{
    _lock.Unlock();
}

bool LibIniFile::ReadStr(const Byte8 *segmentName, const Byte8 *keyName, const Byte8 *defaultStr, LibString &strOut) const
{
    if(LIKELY(_ReadStr(segmentName, keyName, strOut)))
        return true;

    strOut = defaultStr;
    return false;
}

bool LibIniFile::ReadStr(const Byte8 *segmentName, const Byte8 *keyName, LibString &strOut) const
{
    return _ReadStr(segmentName, keyName, strOut);
}

Int64 LibIniFile::ReadInt(const Byte8 *segmentName, const Byte8 *keyName, Int64 defaultInt) const
{
    LibString cache;
    if(_ReadStr(segmentName, keyName, cache) && !cache.empty())
        return StringUtil::StringToInt64(cache.c_str());

    return defaultInt;
}

UInt64 LibIniFile::ReadUInt(const Byte8 *segmentName, const Byte8 *keyName, UInt64 defaultInt) const
{
    LibString cache;
    if(_ReadStr(segmentName, keyName, cache) && !cache.empty())
        return StringUtil::StringToUInt64(cache.c_str());

    return defaultInt;
}

bool LibIniFile::CheckReadInt(const Byte8 *segmentName, const Byte8 *keyName, Int64 &result) const
{
    LibString cache;
    if(_ReadStr(segmentName, keyName, cache) && !cache.empty())
    {
        result = StringUtil::StringToInt64(cache.c_str());
        return true;
    } 

    return false;
}

bool LibIniFile::CheckReadUInt(const Byte8 *segmentName, const Byte8 *keyName, UInt64 &result) const
{
    LibString cache;
    if(_ReadStr(segmentName, keyName, cache) && !cache.empty())
    {
        result = StringUtil::StringToUInt64(cache.c_str());
        return true;
    }

    return false;
}

bool LibIniFile::CheckReadNumber(const Byte8 *segmentName, const Byte8 *keyName, Int64 &result) const
{
    LibString cache;
    if(_ReadStr(segmentName, keyName, cache) && !cache.empty())
    {
        result = StringUtil::StringToInt64(cache.c_str());
        return true;
    }

    return false;
}

bool LibIniFile::CheckReadNumber(const Byte8 *segmentName, const Byte8 *keyName, UInt64 &result) const
{
    LibString cache;
    if(_ReadStr(segmentName, keyName, cache) && !cache.empty())
    {
        result = StringUtil::StringToUInt64(cache.c_str());
        return true;
    }

    return false;
}

bool LibIniFile::CheckReadNumber(const Byte8 *segmentName, const Byte8 *keyName, Int32 &result) const
{
    LibString cache;
    if(_ReadStr(segmentName, keyName, cache) && !cache.empty())
    {
        result = StringUtil::StringToInt32(cache.c_str());
        return true;
    }

    return false;
}

bool LibIniFile::CheckReadNumber(const Byte8 *segmentName, const Byte8 *keyName, UInt32 &result) const
{
    LibString cache;
    if(_ReadStr(segmentName, keyName, cache) && !cache.empty())
    {
        result = StringUtil::StringToUInt32(cache.c_str());
        return true;
    }

    return false;
}

bool LibIniFile::CheckReadNumber(const Byte8 *segmentName, const Byte8 *keyName, Double &result) const
{
    LibString cache;
    if(_ReadStr(segmentName, keyName, cache) && !cache.empty())
    {
        result = StringUtil::StringToDouble(cache.c_str());
        return true;
    }

    return false;
}

bool LibIniFile::WriteStr(const Byte8 *segmentName, const Byte8 *keyName, const Byte8 *wrStr)
{
    return _WriteStr(segmentName, keyName, wrStr);
}

bool LibIniFile::WriteNumber(const Byte8 *segmentName, const Byte8 *keyName, UInt64 number)
{
    const auto &value = StringUtil::Num2Str(number);
    return _WriteStr(segmentName, keyName, value.c_str());
}

bool LibIniFile::WriteNumber(const Byte8 *segmentName, const Byte8 *keyName, Int64 number)
{
    const auto &value = StringUtil::Num2Str(number);
    return _WriteStr(segmentName, keyName, value.c_str());
}

bool LibIniFile::WriteNumber(const Byte8 *segmentName, const Byte8 *keyName, Double number)
{
    const auto &value = StringUtil::Num2Str(number);
    return _WriteStr(segmentName, keyName, value.c_str());
}

bool LibIniFile::WriteNumber(const Byte8 *segmentName, const Byte8 *keyName, Int32 number)
{
    const auto &value = StringUtil::Num2Str(number);
    return _WriteStr(segmentName, keyName, value.c_str());
}

bool LibIniFile::WriteNumber(const Byte8 *segmentName, const Byte8 *keyName, UInt32 number)
{
    const auto &value = StringUtil::Num2Str(number);
    return _WriteStr(segmentName, keyName, value.c_str());
}

bool LibIniFile::HasCfgs(const Byte8 *segmentName) const
{
    return  _segmentRefKeyValues.find(segmentName) != _segmentRefKeyValues.end();
}

bool LibIniFile::HasCfgs() const
{
    return !_segmentRefKeyValues.empty();
}

bool LibIniFile::ChangeLineBetweenSegs()
{
    // 打开并清空文件
    if(!_filePath.empty())
    {
        auto fp = FileUtil::OpenFile(_filePath.c_str(), false, "w");
        if(!fp)
            return false;

        LibString lineData;
        LibString seg;
        for(auto iterLineData = _lineRefContent.begin(); iterLineData != _lineRefContent.end(); ++iterLineData)
        {
            lineData.clear();
            lineData = iterLineData->second + LibString(IniFileDefs::_changeLineFlag);
            if(IniFileMethods::IsSegment(lineData, seg))
                FileUtil::WriteFile(*fp, LibString(IniFileDefs::_changeLineFlag));
            FileUtil::WriteFile(*fp, lineData);
        }

        FileUtil::FlushFile(*fp);
        FileUtil::CloseFile(*fp);
    }
    else
    {
        _fromMemory.clear();
        LibString lineData;
        LibString seg;
        for(auto iterLineData = _lineRefContent.begin(); iterLineData != _lineRefContent.end(); ++iterLineData)
        {
            lineData.clear();
            lineData = iterLineData->second + LibString(IniFileDefs::_changeLineFlag);
            if(IniFileMethods::IsSegment(lineData, seg))
                _fromMemory.AppendFormat("%s", LibString(IniFileDefs::_changeLineFlag).c_str());
            _fromMemory.AppendFormat("%s", lineData.c_str());
        }
    }

    // 重新load配置
    _isDirtied = false;
    return _LoadAllCfgs();
}

bool LibIniFile::WriteFileHeaderAnnotation(const LibString &content)
{
    // 打开并清空文件
    if(!_filePath.empty())
    {
        auto fp = FileUtil::OpenFile(_filePath.c_str(), false, "w");
        if(!fp)
            return false;

        FileUtil::CloseFile(*fp);
    }
    
    // 1.获取第一个segment的行号在segment前插入
    const Int32 firstSegLine = _GetSegmentLineByLoop(1);
    // 2.分离出多行数据
    auto multiLineContent = content.Split(IniFileDefs::_changeLineFlag);
    // 3.每一行添加注释符
    const Int32 contentLine = static_cast<Int32>(multiLineContent.size());
    for(Int32 i = 0; i < contentLine; ++i)
        multiLineContent[i] = LibString(IniFileDefs::_annotationFlag) + multiLineContent[i];

    // 4.多行内容拼接成一个string对象
    LibString willWriteContent;
    for(Int32 i = 0; i < contentLine; ++i)
        willWriteContent << multiLineContent[i] << LibString(IniFileDefs::_changeLineFlag);

    return _InsertNewLineData(firstSegLine, willWriteContent);
}

const std::unordered_map<LibString, LibString> *LibIniFile::GetSegmentCfgs(const Byte8 *segmentName) const
{
    auto iterKeyValue = _segmentRefKeyValues.find(segmentName);
    if(iterKeyValue == _segmentRefKeyValues.end())
        return NULL;

    return &(iterKeyValue->second);
}

bool LibIniFile::_Init()
{
    // 读取所有配置内容
    if(!_LoadAllCfgs())
        return false;

    return true;
}

bool LibIniFile::_LoadAllCfgs()
{
    // 刷新文件
    _UpdateIni();

    if(!_filePath.empty())
    {
        auto fp = FileUtil::OpenFile(_filePath.c_str());
        if(!fp)
            return false;

        // 解析seg,key value
        // 读取每一行，遇到[表示新的segment
        // 一行一行的读取，读到读不出来为止
        _maxLine = 0;
        _lineRefContent.clear();
        _segOrKeyRefLine.clear();
        _segmentRefKeyValues.clear();
        _segmentRefMaxValidLine.clear();

        std::unordered_map<LibString, LibString> *curKeyValues = NULL;
        LibString curSegment;
        while(true)
        {
            LibString lineData;
            auto cnt = FileUtil::ReadUtf8OneLine(*fp, lineData);
            if(cnt || !FileUtil::IsEnd(*fp))
                _lineRefContent.insert(std::make_pair(++_maxLine, lineData));
            if(!cnt)
            {
                if(FileUtil::IsEnd(*fp))
                    break;
                    
                continue;
            }

            // 粗提取
            Int32 contentType = IniFileDefs::ContentType::Invalid;
            LibString validContent;
            auto hasValidData = IniFileMethods::ExtractValidRawData(lineData, contentType, validContent);
            if(hasValidData)
                _OnReadValidData(validContent, contentType, _maxLine, curSegment, curKeyValues);
        }

        // 更新配置文件内存缓存
        _UpdateFileMemCache(*fp);
        FileUtil::CloseFile(*fp);
    }
    else
    {
        // 解析seg,key value
        // 读取每一行，遇到[表示新的segment
        // 一行一行的读取，读到读不出来为止
        _maxLine = 0;
        _lineRefContent.clear();
        _segOrKeyRefLine.clear();
        _segmentRefKeyValues.clear();
        _segmentRefMaxValidLine.clear();

        if(!_fromMemory.empty())
        {
            const auto &multilineContent = _fromMemory.Split(LibString(IniFileDefs::_changeLineFlag));

            std::unordered_map<LibString, LibString> *curKeyValues = NULL;
            LibString curSegment;
            for(auto &lineCache : multilineContent)
            {
                _lineRefContent.insert(std::make_pair(++_maxLine, lineCache));

                if(lineCache.empty())
                    continue;

                // 粗提取
                Int32 contentType = IniFileDefs::ContentType::Invalid;
                LibString validContent;
                auto hasValidData = IniFileMethods::ExtractValidRawData(lineCache, contentType, validContent);
                if(hasValidData)
                    _OnReadValidData(validContent, contentType, _maxLine, curSegment, curKeyValues);
            }

            // 更新配置文件内存缓存
            _UpdateFileMemCache(_fromMemory);
        }
    }

    return true;
}

bool LibIniFile::_ReadStr(const Byte8 *segmentName, const Byte8 *keyName, LibString &strOut) const
{
    auto iterKeyValue = _segmentRefKeyValues.find(segmentName);
    if(iterKeyValue == _segmentRefKeyValues.end())
        return false;

    auto &keyValue = iterKeyValue->second;
    auto iterValue = keyValue.find(keyName);
    if(iterValue == keyValue.end())
        return false;

    strOut = iterValue->second;
    return true;
}

bool LibIniFile::_WriteStr(const Byte8 *segmentName, const Byte8 *keyName, const Byte8 *wrStr)
{
    // 寻找段 没有便创建段，并追加行号
    auto iterKeyValue = _segmentRefKeyValues.find(segmentName);
    if(iterKeyValue == _segmentRefKeyValues.end())
    {
        iterKeyValue = _segmentRefKeyValues.insert(std::make_pair(segmentName, std::unordered_map<LibString, LibString>())).first;
        _segOrKeyRefLine.insert(std::make_pair(segmentName, ++_maxLine));

        LibString segContent = LibString(IniFileDefs::_leftSegmentFlag);
        segContent << segmentName << LibString(IniFileDefs::_rightSegmentFlag);
        _lineRefContent.insert(std::make_pair(_maxLine, segContent));
        _isDirtied = true;
    }

    // 寻找键 没有便创建键值对 并追加行号 并更新数据
    auto &keyValue = iterKeyValue->second;
    auto iterValue = keyValue.find(keyName);
    if(iterValue == keyValue.end())
    {
        const auto &segStr = iterKeyValue->first;
        Int32 segMaxValidLine = _GetSegmentKeyValueMaxValidLine(segStr);
        if(segMaxValidLine < 0)
        {// 段不存在
            CRYSTAL_TRACE("_GetSegmentKeyValueMaxValidLine fail seg[%s] not found", segStr.c_str());
            return false;
        }

        // 更新keyvalue
        keyValue.insert(std::make_pair(keyName, wrStr));
        LibString keyValueContent = keyName;
        keyValueContent << LibString(IniFileDefs::_keyValueJoinerFlag) << wrStr;

        // 为key创建段中唯一索引
        LibString segKey = segStr + LibString(IniFileDefs::_segKeyJoinerFlag) + keyName;

        // 插入新的行数据
        return _InsertNewLineData(++segMaxValidLine, segStr, keyName, wrStr);
    }
    else
    {
        if(iterValue->second != wrStr)
        {
            // 新值
            iterValue->second = wrStr;

            // 新的键值对所在行
            LibString segKey = iterKeyValue->first + LibString(IniFileDefs::_segKeyJoinerFlag) + keyName;
            auto iterLine = _segOrKeyRefLine.find(segKey);

            auto iterContent = _lineRefContent.find(iterLine->second);
            auto &content = iterContent->second;

            // 分离
            const auto &splitStr = content.Split(IniFileDefs::_annotationFlag, 1);

            LibString comments;
            if(splitStr.size() >= 2)
            {// 含有注释
                comments << LibString(IniFileDefs::_annotationFlag);
                comments << splitStr[1];
            }

            content = keyName;
            content << LibString(IniFileDefs::_keyValueJoinerFlag) << iterValue->second;
            if(!comments.empty())
                content << IniFileDefs::_annotationSpaceStr << comments;

            _isDirtied = true;
        }
    }

    // 更新文件
    _UpdateIni(true);

    return true;
}

bool LibIniFile::_InsertNewLineData(Int32 line, const LibString &segment, const LibString &key,  const LibString &value)
{
    // 段不存在不可插入
    auto iterKeyValue = _segmentRefKeyValues.find(segment);
    if(iterKeyValue == _segmentRefKeyValues.end())
    {
        CRYSTAL_TRACE("segment[%s] not found _InsertNewLineData fail.", segment.c_str());
        return false;
    }

    LibString keyValue;
    IniFileMethods::MakeKeyValuePairStr(key, value, keyValue);
    _InsertNewLineData(line, keyValue);
    return true;
}

bool LibIniFile::_InsertNewLineData(Int32 line, const LibString &content)
{
    auto iterContent = _lineRefContent.find(line);
    if(iterContent != _lineRefContent.end())
    {// 已存在则需要挪动行数据
        LibString swapStr, frontStr;
        frontStr = iterContent->second;
        for(++iterContent; iterContent != _lineRefContent.end(); ++iterContent)
        {
            swapStr = iterContent->second;
            iterContent->second = frontStr;
            frontStr = swapStr;
        }

        // 最后一个节点挪到新的行
        ++_maxLine;
        _lineRefContent.insert(std::make_pair(_maxLine, frontStr));
        auto iterToModify = _lineRefContent.find(line);
        iterToModify->second = content;
    }
    else
    {// 不存在则不用挪直接插入
        _lineRefContent.insert(std::make_pair(line, content));
    }

    // 更新最大行号
    if(_maxLine < line)
        _maxLine = line;

    _isDirtied = true;
    _UpdateIni(true);

    // 重新加载配置
    return _LoadAllCfgs();
}

void LibIniFile::_UpdateIni(bool isUpdateMemCache)
{
    if(!_isDirtied)
        return;

    _isDirtied = false;

    // 打开并清空文件
    if(LIKELY(!_filePath.empty()))
    {
        auto fp = FileUtil::OpenFile(_filePath.c_str(), false, "w");
        LibString lineData;
        for(auto iterLineData = _lineRefContent.begin(); iterLineData != _lineRefContent.end(); ++iterLineData)
        {
            lineData.clear();
            lineData = iterLineData->second + LibString(IniFileDefs::_changeLineFlag);
            FileUtil::WriteFile(*fp, lineData);
        }

        FileUtil::FlushFile(*fp);

        // 更新内存缓存
        if (isUpdateMemCache)
            _UpdateFileMemCache(*fp);

        FileUtil::CloseFile(*fp);
    }
    else
    {
        _fromMemory.clear();
        LibString lineData;
        for(auto iterLineData = _lineRefContent.begin(); iterLineData != _lineRefContent.end(); ++iterLineData)
        {
            lineData.clear();
            lineData = iterLineData->second + LibString(IniFileDefs::_changeLineFlag);
            _fromMemory.AppendFormat("%s", lineData.c_str());
        }

        // 更新内存缓存
        if (isUpdateMemCache)
            _UpdateFileMemCache(_fromMemory);
    }
}

UInt64 LibIniFile::_UpdateFileMemCache(FILE &fp)
{
    // 缓存文件内容
    _fileContent.clear();
    FileUtil::ResetFileCursor(fp);
    return FileUtil::ReadFile(fp, _fileContent);
}

UInt64 LibIniFile::_UpdateFileMemCache(const LibString &content)
{
    _fileContent = content;
    return _fileContent.size();
}

void LibIniFile::_OnReadValidData(const LibString &validContent
                                  , Int32 contentType
                                  , Int32 line
                                  , LibString &curSegment
                                  , std::unordered_map<LibString, LibString> *&curKeyValues)
{
    if(contentType == IniFileDefs::ContentType::Segment)
    {// 是段
        curSegment = validContent;
        auto iterKeyValues = _segmentRefKeyValues.find(curSegment);
        if(iterKeyValues == _segmentRefKeyValues.end())
            iterKeyValues = _segmentRefKeyValues.insert(std::make_pair(curSegment, std::unordered_map<LibString, LibString>())).first;
        curKeyValues = &(iterKeyValues->second);

        // 记录段所在的行号
        auto iterLine = _segOrKeyRefLine.find(curSegment);
        if(iterLine == _segOrKeyRefLine.end())
        {
            _segOrKeyRefLine.insert(std::make_pair(curSegment, line));
        }
    }
    else if(contentType == IniFileDefs::ContentType::KeyValue)
    {// 是键值对
        if(!curSegment.empty())
        {// 需要先有段再有键值对
            LibString key;
            LibString value;
            if(IniFileMethods::SpiltKeyValue(validContent, key, value))
            {
                auto iterValue = curKeyValues->find(key);
                if(iterValue == curKeyValues->end())
                {
                    curKeyValues->insert(std::make_pair(key, value));

                    // 记录键值对所在的行号
                    LibString segKey = curSegment + LibString(IniFileDefs::_segKeyJoinerFlag) + key;
                    auto iterLine = _segOrKeyRefLine.find(segKey);
                    if(iterLine != _segOrKeyRefLine.end())
                        perror("segOrKey repeated");

                    _segOrKeyRefLine.insert(std::make_pair(segKey, line));
                }

                // 更新键值对最大有效行号
                auto iterMaxValidLine = _segmentRefMaxValidLine.find(curSegment);
                if(iterMaxValidLine == _segmentRefMaxValidLine.end())
                    iterMaxValidLine = _segmentRefMaxValidLine.insert(std::make_pair(curSegment, 0)).first;

                if(iterMaxValidLine->second < line)
                    iterMaxValidLine->second = line;
            }
        }
    }
}

Int32 LibIniFile::_GetSegmentKeyValueMaxValidLine(const LibString &segment) const
{
    // 若该段是空的则以segment所在行作为最大行号
    auto iterLine = _segmentRefMaxValidLine.find(segment);
    if(iterLine == _segmentRefMaxValidLine.end())
    {
        auto iterSegLine = _segOrKeyRefLine.find(segment);
        if(iterSegLine == _segOrKeyRefLine.end())
        {// 该段不存在
            return -1;
        }

        return iterSegLine->second;
    }

    return iterLine->second;
}

Int32 LibIniFile::_GetSegmentLineByLoop(Int32 index) const
{
    LibString lineData;
    LibString seg;
    Int32 realLine = 1; // 找不到任何segment则从第1行插入
    Int32 loopCnt = 0;
    for(auto iterLineData = _lineRefContent.begin(); iterLineData != _lineRefContent.end(); ++iterLineData)
    {
        lineData.clear();
        lineData = iterLineData->second;
        if(IniFileMethods::IsSegment(lineData, seg))
        {
            ++loopCnt;
            realLine = iterLineData->first;

            if(loopCnt >= index)
                break;
        }
    }

    return realLine;
}

KERNEL_END