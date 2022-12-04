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
 * Date: 2022-03-10 00:40:46
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/SockErrorMsgUtil.h>

KERNEL_BEGIN

std::unordered_map<Int32, LibString> SockErrorMsgUtil::_errInfo;

void SockErrorMsgUtil::Init()
{
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS

    // sock错误码
    _errInfo[WSAECONNRESET] = "WSAECONNRESET occur, from a stream socket, virtual circuit was reset by the remote side, should close the socket later.";
    _errInfo[WSAEFAULT] = "WSAEFAULT occur, the buffers, numberofbytessent, overlapped, completionroutine params is not totally contained in a valid part of the user address space.";
    _errInfo[WSAENETRESET] = "WSAENETRESET occur, from a stream socket, then connection has been broken due to keep-alive activity detecting afailure while the operation was in progress, should close socket later.";
    _errInfo[WSAENOBUFS] = "WSAENOBUFS occur, the windows socket provider reports a buffer deadlock, should close socket later.";
    _errInfo[WSAESHUTDOWN] = "WSAESHUTDOWN occur, the socket has been shut down , cant use wsasend on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.";
    _errInfo[WSAEWOULDBLOCK] = "WSAEWOULDBLOCK occur, overlapped sockets:there are too many outstanding overlapped io requests, please retry later. Nonoverlapped sockets: the socket is marked as nonblocking and the send operation cannot be completed immediately.";
    _errInfo[WSA_OPERATION_ABORTED] = "WSA_OPERATION_ABORTED occur, the overlapped operation has been canceld due to the closure of the socket, the execution of the SIO_FLUSH command in WSAIoctl, or the thred that initiated the overlapped request exited before the operation completed.";
    _errInfo[WSANOTINITIALISED] = "A successful WSAStartup call must occur before using this function.";
    _errInfo[WSAENETDOWN] = "The network subsystem has failed.";
    _errInfo[WSAEINPROGRESS] = "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
    _errInfo[WSAENOTSOCK] = "The descriptor s is not a socket.";

    #endif
}

KERNEL_END
