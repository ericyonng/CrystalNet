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
 * Date: 2022-01-26 10:42:08
 * Author: Eric Yonng
 * Description: iocp的重叠结构
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_LIB_OVERLAPPED_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_LIB_OVERLAPPED_H__

#include <kernel/common/macro.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

#include <kernel/comp/memory/ObjPoolMacro.h>

KERNEL_BEGIN

class OverLappedOpcode
{
public:
    enum 
    {
        None = 0,
        Accept = 1,
        Connect = 2,
        Recv = 3,
        Send = 4,
        End,
    };

    static const Byte8 *ToString(Int32 opcode)
    {
        switch (opcode)
        {
        case OverLappedOpcode::Accept: return "Accept";
        case OverLappedOpcode::Connect: return "Connect";
        case OverLappedOpcode::Recv: return "Recv";
        case OverLappedOpcode::Send: return "Send";
        default:
            break;
        }

        return "Unknown";
    };
};

struct LibOverlapped : public OVERLAPPED
{
    POOL_CREATE_OBJ_DEFAULT(LibOverlapped);

    LibOverlapped();

    Int32 _opcode;          // OverLappedOpcode
    UInt64 _sessionId;      // 会话id
    SOCKET _sock;           // 套接字
};

KERNEL_END

#endif

#endif
