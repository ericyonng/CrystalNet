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
 * Date: 2022-12-25 13:44:00
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_PIPELINE_FILE_PIPE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_PIPELINE_FILE_PIPE_H__

#pragma once

#include <kernel/comp/Pipline/IPipe.h>
#include <kernel/comp/LibString.h>
#include <functional>

KERNEL_BEGIN

template <typename Rtn, typename... Args>
class IDelegate;

class KERNEL_EXPORT FilePipe : public IPipe
{
    POOL_CREATE_OBJ_DEFAULT_P1(IPipe, FilePipe);

public:
    FilePipe();
    ~FilePipe();

    virtual void Release();
    void SetRelease(std::function<void()> &&func);

    bool Init(const LibString &file);

    virtual void SetPipeName(const LibString &name);
    virtual LibString GetPipeName() const;
    virtual Int32 Open();
    virtual bool Write(const Byte8 *buffer, Int64 &sz);
    virtual bool Read(Byte8 *buffer, Int64 &count);
    virtual void Flush();
    virtual void Close();

    Int64 GetWriteBytes() const;
    Int64 GetReadBytes() const;

private:
    LibString _file;
    FILE *_fp;
    Int64 _writePos;
    Int64 _readPos;

    IDelegate<void> *_release;
};

ALWAYS_INLINE Int64 FilePipe::GetWriteBytes() const
{
    return _writePos;
}

ALWAYS_INLINE Int64 FilePipe::GetReadBytes() const
{
    return _readPos;
}

KERNEL_END

#endif