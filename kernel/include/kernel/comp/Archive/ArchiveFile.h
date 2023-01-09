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
 * Date: 2023-01-09 22:12:28
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ARCHIVE_ARCHIVE_FILE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ARCHIVE_ARCHIVE_FILE_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class KERNEL_EXPORT ArchiveFile
{
    POOL_CREATE_OBJ_DEFAULT(ArchiveFile);
    
public:
    ArchiveFile();
    ~ArchiveFile();
    ArchiveFile(const ArchiveFile&) = delete;
    ArchiveFile(ArchiveFile &&other);

    bool ExtractToMem(const LibString &archivePathName);
    void Clear();
    bool IsInit() const;

    const LibString &GetFilePath() const;

    const std::unordered_map<LibString, std::pair<Byte8 *, UInt64>> &GetContent() const;

private:
    bool _init;
    LibString _path;

    std::vector<Byte8 *> _contents;

    // pair: content, contentSize
    std::unordered_map<LibString, std::pair<Byte8 *, UInt64>> _fileRefContent;
};

ALWAYS_INLINE bool ArchiveFile::IsInit() const
{
    return _init;
}

ALWAYS_INLINE const LibString &ArchiveFile::GetFilePath() const
{
    return _path;
}

ALWAYS_INLINE const std::unordered_map<LibString, std::pair<Byte8 *, UInt64>> &ArchiveFile::GetContent() const
{
    return _fileRefContent;
}


KERNEL_END

#endif
