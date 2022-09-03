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
 * Date: 2021-02-07 22:06:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/File/LibFile.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/DirectoryUtil.h>

KERNEL_BEGIN

LibFile::LibFile()
    :_fp(NULL)
    ,_fileSize(0)
    ,_useTimestampTailer(false)
    ,_openMode("ab+")
{
}

LibFile::~LibFile()
{
    _locker.Lock();
    if(_fp)
    {
        FileUtil::FlushFile(*_fp);
        FileUtil::CloseFile(*_fp);
    }
    _locker.Unlock();
}

bool LibFile::Init(const Byte8 *fileWholeName
    , bool isCreate
    , const Byte8 *openMode
    , bool useTimestampTailer
    , LibTime *nowTime)
{
    if (UNLIKELY(_fp))
    {
        printf("always open");
        ASSERT(!"always open");
        return false;
    }

    _createFileTime = nowTime ? *nowTime : LibTime::Now();
    _modifyFileTime = _createFileTime;
    _useTimestampTailer = useTimestampTailer;

    // 构建文件名
    LibString fileNameCache;
    _BuildFileName(fileWholeName, isCreate && _useTimestampTailer, fileNameCache, _extensionName);

    _fileName = DirectoryUtil::GetFileNameInPath(fileWholeName);
    _path = DirectoryUtil::GetFileDirInPath(fileNameCache);
    _openMode = openMode;

    return true;
}

bool LibFile::Open(const Byte8 *fileWholeName, bool *isFileExist, bool isCreate /*= false*/, const Byte8 *openMode /*= "ab+"*/, bool useTimestampTailer /*= false*/, LibTime *nowTime/* =NULL */)
{
    if(UNLIKELY(_fp))
    {
        printf("always open");
        ASSERT(!"always open");
        return false;
    }

    _createFileTime = nowTime ? *nowTime : LibTime::Now();
    _modifyFileTime = _createFileTime;
    _useTimestampTailer = useTimestampTailer;

    // 构建文件名
    LibString fileNameCache;
    _BuildFileName(fileWholeName, isCreate && _useTimestampTailer, fileNameCache, _extensionName);

    // 文件是否存在
    if (isFileExist)
        *isFileExist = FileUtil::IsFileExist(fileNameCache.c_str());

    _fp = FileUtil::OpenFile(fileNameCache.c_str(), isCreate, openMode);
    if(!_fp)
    {
        // 创建目录
        // CRYSTAL_TRACE("fileWholeName %s", fileWholeName);
        const auto &dir = DirectoryUtil::GetFileDirInPath(fileWholeName);
        // CRYSTAL_TRACE("WILL create dir %s", dir.c_str());
        // CRYSTAL_TRACE("fileNameCache %s", fileNameCache.c_str());
        if(DirectoryUtil::CreateDir(dir))
            _fp = FileUtil::OpenFile(fileNameCache.c_str(), isCreate, openMode);
    }

    // CRYSTAL_TRACE("LibFile _fp = %p", _fp);
    
    if(LIKELY(_fp))
    {
        _fileName = DirectoryUtil::GetFileNameInPath(fileWholeName);
        _path = DirectoryUtil::GetFileDirInPath(fileNameCache);
        _openMode = openMode;
    }

    _fileSize = static_cast<Int64>(FileUtil::GetFileSize(*_fp));
    ASSERT(_fileSize >= 0);

    return _fp != NULL;
}

bool LibFile::Reopen(LibTime *nowTime /* = NULL */)
{
    if(UNLIKELY(!Close()))
    {
        printf("close file fail!");
        return false;
    }

    LibString wholeName = _path + _fileName;
    return Open(wholeName.c_str(), NULL, true, _openMode.c_str(), _useTimestampTailer, nowTime);
}

bool LibFile::Flush()
{
    if(UNLIKELY(!_fp))
    {
        printf("file not open");
        return false;
    }

    return FileUtil::FlushFile(*_fp);
}

bool LibFile::Close()
{
    if(_fp)
    {
        FileUtil::FlushFile(*_fp);
        FileUtil::CloseFile(*_fp);
    }

    _fp = NULL;
    _fileSize = 0;
    return true;
}

Int64 LibFile::Write(const void *buffer, UInt64 writeDataLen)
{
//     if(UNLIKELY(!_fp))
//         return StatusDefs::Error;

    auto wrLen = FileUtil::WriteFile(*_fp, reinterpret_cast<const char *>(buffer), writeDataLen);
    if(wrLen != 0)
        _modifyFileTime.UpdateTime();

    _fileSize += wrLen;
    return wrLen;
}

Int64 LibFile::Read(LibString &outBuffer)
{
    if(UNLIKELY(!_fp))
        return Status::Error;

    return FileUtil::ReadFile(*_fp, outBuffer);
}

Int64 LibFile::Read(Int64 sizeLimit, LibString &outBuffer)
{
    if(UNLIKELY(!_fp))
        return Status::Error;

    return FileUtil::ReadFile(*_fp, outBuffer, sizeLimit);
}

Int64 LibFile::ReadOneLine(LibString &outBuffer)
{
    if(UNLIKELY(!_fp))
        return Status::Error;

    return static_cast<Int64>(FileUtil::ReadUtf8OneLine(*_fp, outBuffer));
}

void LibFile::_BuildFileName(const Byte8 *fileName, bool useTimeTail, LibString &fileNameOut, LibString &extensionNameOut) const
{
    fileNameOut = fileName;
    extensionNameOut = FileUtil::ExtractFileExtension(fileNameOut);

    if(useTimeTail)
        FileUtil::InsertFileTime(extensionNameOut, _createFileTime, fileNameOut);
}

void LibFile::_BuildFileName(LibString &fileNameOut) const
{
    fileNameOut = _path + _fileName;
    if(_useTimestampTailer)
        FileUtil::InsertFileTime(_extensionName, _createFileTime, fileNameOut);
}

KERNEL_END