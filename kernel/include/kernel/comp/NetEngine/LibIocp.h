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
 * Date: 2021-03-22 17:00:01
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_LIB_IOCP_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_LIB_IOCP_H__

#pragma once

#include <kernel/common/macro.h>
#include <kernel/common/BaseType.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #include <WinSock2.h>
KERNEL_BEGIN

struct IoData;
struct IoEvent;

class LibIocp
{
public:
    LibIocp();
    virtual ~LibIocp();

    /* 初始化与注册 */
    // init/reg
    /*
    *   brief:
    *       1. - Create 创建端口
    *       2. - Destroy 销毁端口
    *       3. - reg 关联端口 reg时候会把自定义的地址数据ulong_ptr传入在wait时候传回，
    *                         便于确认wait传回的是哪一个绑定在完成端口的数据
    *       4. - reg 关联端口，并传入自定义数据作为ulong_ptr的数据
    *       5. - Init 加载accept函数等，
    *                         因为跨dll调用是有损耗的所以要自己提前加载到内存中
    */
public:
    // @param(family):AF_INET/AF_INET6
    // @param(type): 数据类型 SOCK_STREAM/SOCK_DGRAM等
    // @param(protocolType): 协议类型 IPPROTO_TCP/IPPROTO_UDP等
    Int32 Create();
    void Destroy();
    Int32 Reg(SOCKET sockfd);
    Int32 Reg(SOCKET sockfd, void *ptr);        // 第二个参数是完成时回传的completekey
    Int32 Reg(SOCKET sockfd, UInt64 sessionId); // 第二个参数是完成时回传的completekey
    

    /* iocp的操作 */
    //
    /*
    *   brief:
    *       1. - PostAccept accept时候会把重叠体传入，在wait时候传回
    *                       传入重叠体结构便于区分绑定在完成端口上的socket中的子操作，
    *                       比如绑定的是监听socket但是可能被多个客户端连接，
    *                       为了区分是哪个客户端需要多传入一个重叠体结构以便区分
    *       2. - PostRecv 投递接收数据
    *       3. - PostSend 投递发送数据
    *       4. - PostQuit 投递退出
    *       5. - WaitForCompletion 等待数据io端口完成，
    *                              getqueue是线程安全，
    *                              它会一个一个消息的从完成队列中取出不需要担心线程安全问题
    */
public:
    Int32 PostQuit(UInt64 wakeupSessionId);
    Int32 WaitForCompletion(IoEvent &ioEvent, Int32 &errorCode, ULong millisec = INFINITE);
    

private:
    HANDLE _completionPort;
};

KERNEL_END

#endif

#endif
