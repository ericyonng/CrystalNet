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
 * Date: 2022-12-25 12:31:23
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_PIPELINE_IPIPE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_PIPELINE_IPIPE_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>

KERNEL_BEGIN

class KERNEL_EXPORT IPipe
{
    POOL_CREATE_OBJ_DEFAULT(IPipe);

public:
    IPipe(Int32 pipeType) :_pipeType(pipeType) {}
    virtual ~IPipe() {}

    virtual void Release() = 0;
    virtual void SetPipeName(const LibString &name) = 0;
    virtual LibString GetPipeName() const = 0;
    virtual Int32 Open() = 0;
    virtual bool Write(const Byte8 *buffer, Int64 &sz) = 0;
    virtual bool Read(Byte8 *buffer, Int64 &count) = 0;
    virtual void Flush() {}
    virtual void Close() = 0;

    Int32 GetPipeType() const { return _pipeType; }

private:
    Int32 _pipeType;
};

KERNEL_END

#endif
