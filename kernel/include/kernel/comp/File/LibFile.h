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
 * Date: 2021-02-07 21:00:02
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_LIB_FILE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_LIB_FILE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/memory/ObjPoolMacro.h>

#include <kernel/comp/LibTime.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Lock/Impl/Locker.h>

#include <stdio.h>

KERNEL_BEGIN

struct KERNEL_EXPORT InitFileInfo
{
    LibString _fileWholeName;
    bool _isCreate;
    LibString _openMode;
    bool _useTimestampTailer;
    LibTime _nowTime;
    
};

class KERNEL_EXPORT LibFile
{
    POOL_CREATE_OBJ_DEFAULT(LibFile);

public:
    LibFile();
    virtual ~LibFile();

public:
    virtual bool Init(const Byte8 *fileWholeName
        , bool isCreate = false
        , const Byte8 *openMode = "ab+"
        , bool useTimestampTailer = false
        , LibTime *nowTime = NULL);

    virtual bool Open(const Byte8 *fileWholeName
    , bool *IsFileExist = NULL
    , bool isCreate = false
    , const Byte8 *openMode = "ab+"
    , bool useTimestampTailer = false
    , LibTime *nowTime = NULL);

    virtual bool Reopen(LibTime *nowTime = NULL);
    virtual bool Flush();
    virtual bool Close();

    virtual Int64 Write(const void *buffer, UInt64 writeDataLen);
    virtual Int64 Read(LibString &outBuffer);
    virtual Int64 Read(Int64 sizeLimit, LibString &outBuffer);
    virtual Int64 ReadOneLine(LibString &outBuffer);

    void Lock();
    void UnLock();

    bool IsOpen() const;
    const LibString &GetPath() const;
    const LibString &GetFileName() const;
    Int64 GetSize() const;
    void GetCurrentFileName(LibString &curName) const;
    FILE *GetFp();

public:
    operator bool() const;
    operator FILE *();
    operator const FILE *() const;

protected:
    void _BuildFileName(const Byte8 *fileName
    , bool useTimeTail
    , LibString &fileNameOut
    , LibString &extensionNameOut) const;

    void _BuildFileName(LibString &fileNameOut) const;

protected:
    FILE *_fp = NULL;
    Int64 _fileSize;
    LibTime _createFileTime;
    LibTime _modifyFileTime;
    bool _useTimestampTailer = false;
    LibString _path;
    LibString _fileName;
    LibString _extensionName;       // 扩展名
    LibString _openMode = "ab+";
    Locker  _locker;
};

ALWAYS_INLINE void LibFile::Lock()
{
    _locker.Lock();
}

ALWAYS_INLINE void LibFile::UnLock()
{
    _locker.Unlock();
}

ALWAYS_INLINE bool LibFile::IsOpen() const
{
    return _fp != NULL;
}

ALWAYS_INLINE const LibString &LibFile::GetPath() const
{
    return _path;
}

ALWAYS_INLINE const LibString &LibFile::GetFileName() const
{
    return _fileName;
}

ALWAYS_INLINE Int64 LibFile::GetSize() const
{
    return _fileSize;
}

ALWAYS_INLINE void LibFile::GetCurrentFileName(LibString &curName) const
{
    _BuildFileName(curName);
}

ALWAYS_INLINE FILE *LibFile::GetFp()
{
    return _fp;
}

ALWAYS_INLINE LibFile::operator bool() const
{
    return _fp != NULL;
}

ALWAYS_INLINE LibFile::operator FILE *()
{
    return _fp;
}

ALWAYS_INLINE LibFile::operator const FILE *() const
{
    return _fp;
}

KERNEL_END

#endif
