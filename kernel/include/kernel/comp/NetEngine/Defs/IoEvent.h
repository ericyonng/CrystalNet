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
 * Author: Eric Yonng
 * Date: 2021-03-23 17:46:36
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_IO_EVENT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_IO_EVENT_H__

#pragma once

#include <kernel/common/macro.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

struct IoData;

class IoEventType
{
public:
    /* IO操作类型 */
    enum IO_TYPE:Int32
    {
        IO_ACCEPT = 10,
        IO_RECV,
        IO_SEND,
        IO_CONNECT,
    };

    static const Byte8 *ToString(Int32 ioType)
    {
        switch (ioType)
        {
        case IO_TYPE::IO_ACCEPT: return "IO_ACCEPT";
        case IO_TYPE::IO_RECV: return "IO_RECV";
        case IO_TYPE::IO_SEND: return "IO_SEND";
        case IO_TYPE::IO_CONNECT: return "IO_CONNECT";
            break;
        default:
            break;
        }

        return "IO_UNKNOWN";
    }
};


struct IoEvent
{
    POOL_CREATE_OBJ_DEFAULT(IoEvent);
    IoEvent();

    LibString ToString() const;
    UInt64 _sessionId;  // 会话id
    IoData *_ioData;    // 网络传输数据 overlapped结构
    Int64 _bytesTrans;  // 传输的字节数
};

ALWAYS_INLINE IoEvent::IoEvent()
:_sessionId(0)
,_ioData(NULL)
,_bytesTrans(0)
{

}

KERNEL_END

#endif

#endif
