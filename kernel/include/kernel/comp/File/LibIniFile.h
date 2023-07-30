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
 * Date: 2021-02-07 22:15:32
 * Author: Eric Yonng
 * Description: 支持UTF8, ini 配置文件读取配置后即关闭配置文件,配置只存在内存中, 支持来源于内存的Ini,支持无ini文件
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_LIB_INI_FILE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_LIB_INI_FILE_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/memory/memory.h>

KERNEL_BEGIN

class KERNEL_EXPORT LibIniFile
{
    POOL_CREATE_OBJ_DEFAULT(LibIniFile);
    
public:
    LibIniFile();
    virtual ~LibIniFile();

public:
    bool Init(const Byte8 *path, bool createIfNotExist = true);

    // 使用内存配置表 此时需在Init前调用，并且init时候path传NULL
    void SetMemoryIniContent(const KERNEL_NS::LibString &fromMemory);
    const KERNEL_NS::LibString &GetMemoryIniContent() const;
    void Clear();
    void Flush();
    const LibString &GetPath() const;
    const LibString &GetFileContent() const;

    void Lock();
    void Unlock();

    bool ReadStr(const Byte8 *segmentName, const Byte8 *keyName, const Byte8 *defaultStr, LibString &strOut) const;
    bool ReadStr(const Byte8 *segmentName, const Byte8 *keyName, LibString &strOut) const;
    
    Int64 ReadInt(const Byte8 *segmentName, const Byte8 *keyName, Int64 defaultInt) const;
    UInt64 ReadUInt(const Byte8 *segmentName, const Byte8 *keyName, UInt64 defaultInt) const;
    bool CheckReadInt(const Byte8 *segmentName, const Byte8 *keyName, Int64 &result) const;
    bool CheckReadUInt(const Byte8 *segmentName, const Byte8 *keyName, UInt64 &result) const;

    bool CheckReadNumber(const Byte8 *segmentName, const Byte8 *keyName, Int64 &result) const;
    bool CheckReadNumber(const Byte8 *segmentName, const Byte8 *keyName, UInt64 &result) const;
    bool CheckReadNumber(const Byte8 *segmentName, const Byte8 *keyName, Int32 &result) const;
    bool CheckReadNumber(const Byte8 *segmentName, const Byte8 *keyName, UInt32 &result) const;
    bool CheckReadNumber(const Byte8 *segmentName, const Byte8 *keyName, Double &result) const;

    bool WriteStr(const Byte8 *segmentName, const Byte8 *keyName, const Byte8 *wrStr);
    bool WriteNumber(const Byte8 *segmentName, const Byte8 *keyName, UInt64 number);
    bool WriteNumber(const Byte8 *segmentName, const Byte8 *keyName, Int64 number);
    bool WriteNumber(const Byte8 *segmentName, const Byte8 *keyName, Double number);

    bool HasCfgs(const Byte8 *segmentName) const;
    bool HasCfgs() const;
    bool ChangeLineBetweenSegs();
    bool WriteFileHeaderAnnotation(const LibString &content);   // 内容只会在第一个segment的位置插入

    // 段下所有配置
    const std::unordered_map<LibString, LibString> *GetSegmentCfgs(const Byte8 *segmentName) const;

private:
    bool _Init();
    bool _LoadAllCfgs();

    bool _ReadStr(const Byte8 *segmentName, const Byte8 *keyName, LibString &strOut) const;
    bool _WriteStr(const Byte8 *segmentName, const Byte8 *keyName, const Byte8 *wrStr);

    // 插入新行数据
    bool _InsertNewLineData(Int32 line, const LibString &segment, const LibString &key, const LibString &value); // 会重新加载配置，容器迭代器全部失效
    bool _InsertNewLineData(Int32 line, const LibString &content); // 向某一行插入任意数据
    // 更新配置
    void _UpdateIni(bool isUpdateMemCache = false);

    // 更新内存缓存
    UInt64 _UpdateFileMemCache(FILE &fp);
    UInt64 _UpdateFileMemCache(const LibString &content);

    // 读取到有效的数据
    void _OnReadValidData(const LibString &validContent
                          , Int32 contentType
                          , Int32 line
                          , LibString &curSegment
                          , std::unordered_map<LibString, LibString> *&curKeyValues);

    // 段的所包含的键值对的最大行
    Int32 _GetSegmentKeyValueMaxValidLine(const LibString &segment) const;  // 返回-1该段不存在
    Int32 _GetSegmentLineByLoop(Int32 index) const; // 获取第index个segment的行号

private:
    Locker _lock;
    LibString  _filePath;
    LibString _fromMemory;
    LibString _fileContent;                                                         // 配置文件内容

    bool _isDirtied;
    Int32 _maxLine;                                                                 // 行号最小值1
    std:: map<Int32,     LibString> _lineRefContent;                                // 每一行的元数据,行号最小值从1开始
    std:: unordered_map<LibString, Int32> _segOrKeyRefLine;                                   // seg对应的行号或者seg-key所在的行号 例如seg, seg-key
    std:: unordered_map<LibString, std::unordered_map<LibString, LibString>> _segmentRefKeyValues;
    std:: unordered_map<LibString, Int32> _segmentRefMaxValidLine;                            // key：段， value:该段有效的最大行号
};

ALWAYS_INLINE void LibIniFile::SetMemoryIniContent(const KERNEL_NS::LibString &fromMemory)
{
    _fromMemory = fromMemory;
}

ALWAYS_INLINE const KERNEL_NS::LibString &LibIniFile::GetMemoryIniContent() const
{
    return _fromMemory;
}

KERNEL_END

#endif
