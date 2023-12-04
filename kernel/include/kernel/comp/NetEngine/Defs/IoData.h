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
 * Date: 2021-03-22 20:10:53
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_IO_DATA_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_IO_DATA_H__

#pragma once

#include <kernel/common/macro.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include <kernel/comp/memory/ObjPoolMacro.h>
 #include <ws2def.h>
 #include <minwinbase.h>
 #include <string.h>
#endif

KERNEL_BEGIN

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

template<typename BuildType>
class LibStream;
// template class LibStream<_Build::TL>;

struct IoData
{
    POOL_CREATE_OBJ_DEFAULT(IoData);

    void Reset();

    // 重叠体
    OVERLAPPED _overlapped{0};              // 使用重叠体可以关联到iodatabase,在投递accept时候传入
    Int32 _ioType = 0;                      // IoEventType
    SOCKET _sock = INVALID_SOCKET;          // sock
    SOCKET _listenSock = INVALID_SOCKET;    // 监听端口socket
    UInt64 _sessionId = 0;                  // 会话唯一id
    UInt64 _handledBytes = 0;               // 处理的数据量

    // 没必要每个客户端指定一个缓冲，太大了，_wsaBuff中若有缓冲区请自行释放
    // 因为iocp取数据 时候数据是从队列中先进先出的方式被拷贝出来，
    // 每个iocp中的线程是互斥的执行的 
    LibStream<_Build::TL> *_tlStream = NULL;      // 缓冲对象
    WSABUF _wsaBuff{0};                 // 数据缓冲结构 buf的大小:sizeof(KernelSockAddrInRaw)
};

ALWAYS_INLINE void IoData::Reset()
{
    ::memset(&_overlapped, 0, sizeof(_overlapped));
    _ioType = 0;
    _sock = INVALID_SOCKET;
    _listenSock = INVALID_SOCKET;
    _sessionId = 0;
    _handledBytes = 0;
    _tlStream = NULL;
    ::memset(&_wsaBuff, 0, sizeof(_wsaBuff));
}

#endif


KERNEL_END

#endif

