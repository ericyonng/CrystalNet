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
 * Date: 2021-02-07 23:54:40
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_LIB_LOG_FILE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_LIB_LOG_FILE_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/File/LibFile.h>

KERNEL_BEGIN

class KERNEL_EXPORT LibLogFile : public LibFile
{
    POOL_CREATE_OBJ_DEFAULT_P1(LibFile, LibLogFile);
public:
    LibLogFile();
    virtual ~LibLogFile();

public:
    bool IsDayPass(const LibTime &nowTime) const;
    void UpdateLastPassDayTime(LibTime *nowTime = NULL);

    // 分割文件
    bool IsTooLarge(Int64 limitSize) const;

    // isSysFirstCreate=true:系统第一次创建不会分文件
    void PartitionFile(bool isSysFirstCreate = false, LibTime *nowTime = NULL);    

protected:
    Int32 _partNo;
    LibTime  _lastPassDayTime;
};

inline bool LibLogFile::IsTooLarge(Int64 limitSize) const
{
    if(limitSize <= 0)
        return false;

    return _fileSize >= limitSize;
}

inline bool LibLogFile::IsDayPass(const LibTime &nowTime) const
{
    return _lastPassDayTime.GetZeroTime() != nowTime.GetZeroTime();
}

inline void LibLogFile::UpdateLastPassDayTime(LibTime *nowTime)
{
    if(!nowTime)
    {
        _lastPassDayTime.UpdateTime();
        return;
    }

    _lastPassDayTime = *nowTime;
}

KERNEL_END

#endif
