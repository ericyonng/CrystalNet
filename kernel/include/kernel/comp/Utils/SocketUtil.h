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
 * Date: 2021-03-21 23:09:44
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_SOCKET_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_SOCKET_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>

#if CRYSTAL_TARGET_PLATFORM_LINUX
 #include <sys/socket.h>
 #include <errno.h>
 #include <arpa/inet.h>
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #include <WinSock2.h>
    #include <ws2ipdef.h>   // ipv6等
    #include <ws2tcpip.h>
#endif

#include <kernel/comp/Log/log.h>
#include <kernel/common/BaseType.h>
#include <kernel/common/LibSockLen.h>
#include <kernel/comp/Utils/Defs/Socket.h>

KERNEL_BEGIN

struct BriefAddrInfo;
struct BriefSockAddr;
class KernelSockAddrIn;
struct IoData;

// 网络模型
class KERNEL_EXPORT NetIoModule
{
public:
    enum Type
    {
        EPOLL = 0,
        IOCP = 1,

        EPOLL_LT = 2,
        EPOLL_ET = 3,
    };
};

class KERNEL_EXPORT SocketUtil
{
public:
    static int InitSocketEnv();
    static int ClearSocketEnv();
    static Int32 SetNoBlock(SOCKET sock);
    static Int32 SetBlock(SOCKET sock);
    static Int32 SetCloseOnExec(SOCKET sock);
    // linix sock一旦处于linsten状态则再次绑定同一端口，会失败,此时若要绑定同一端口那么需要第一次bind sock时sock是reuseport状态
    static Int32 MakeReUseAddr(SOCKET sock, bool isEnable = true);
    // windows下等同于MakeReUseAddr
    static Int32 MakeReUsePort(SOCKET sock, bool isEnable = true);
    static Int32 MakeNoDelay(SOCKET sock, bool isNodelay);
    
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    // iocp accept成功时调用
    static Int32 UpdateAcceptContext(SOCKET &listenSock, SOCKET &sock);
    // iocp connect成功时调用
    static Int32 UpdateConnectContext(SOCKET &sock);
    #endif

    static Int32 DestroySocket(SOCKET &sock);
    static Int32 DestroySocket2(SOCKET sock);
    static bool IsReuseAddr(SOCKET sock);
    static bool IsReusePort(SOCKET sock);
    static bool IsNoDelay(SOCKET sock);
    static bool IsNonBlocking(SOCKET sock);
    static bool IsCloseOnExec(SOCKET sock);

    static UInt64 GetSendBufSize(SOCKET sock);
    static Int32 SetSendBufSize(SOCKET sock, UInt64 size);
    static UInt64 GetRecvBufSize(SOCKET sock);
    static Int32 SetRecvBufSize(SOCKET sock, UInt64 size);

    static bool IsIpv4(const LibString &ip);
    static bool IsIpv6(const LibString &ip);
    static bool IsIp(const LibString &ip);

    // 获取对象地址信息 0表示成功其他值为错误信息 获取远程客户端地址信息 需要在accept成功完成后才可以调用 windows 下lasterror是WSAGetLastError获得,linux下lasterror是errno获得
    static Int32 GetPeerAddr(UInt16 family, UInt64 sSocket, Int32 sizeIp, Byte8 *&ip, UInt16 &port, Int32 &lastError);
    // 对监听端口而言只有bind之后才可以调用,windows 下lasterror是WSAGetLastError获得,linux下lasterror是errno获得
    template<typename SockAddrInType>
    static Int32 GetSockName(SOCKET sock, SockAddrInType &sockAddr, Int32 &lastError);

    // 转换为网络字节序 family:AF_INET(udp,tcp)
    static bool FillTcpAddrInfo(const Byte8 *ip, UInt16 port, UInt16 family, sockaddr_in &addrObj);
    static bool FillTcpAddrInfo(const Byte8 *ip, UInt16 port, UInt16 family, sockaddr_in6 &addrObj);
    // 转换为主机信息
    static bool GetAddrInfoFromNetInfo(sockaddr_in &addrObj, UInt64 szip, Byte8 *&ip, UInt16 &port);
    static bool GetAddrInfoFromNetInfo(sockaddr_in6 &addrObj, UInt64 szip, Byte8 *&ip, UInt16 &port);
    // 套接字等待超时
    static bool IsDetectTimeOut(
        SOCKET &sock
        , timeval timeout
        , bool enableReadableDetect = true
        , bool enableWriteableDetect = false
        , int *errOut = NULL
        , bool isInfiniteWaiting = false);
    // 设置sock缓冲区大小
    static Int32 SetSocketCacheSize(SOCKET &sock, SocketDefs::SOCKET_CACHE_TYPE eType, Int64 cacheSize, Int32 &lastErr);
    // 获取sock缓冲区大小
    static Int32 GetSocketCacheSize(SOCKET &sock, SocketDefs::SOCKET_CACHE_TYPE eType, Int64 &cacheSize, Int32 &lastErr);

    // 网络速度计
    static LibString ToFmtSpeedPerSec(Int64 bytesps);
    // 是否无效套接字
    static bool IsValidSock(SOCKET sock);

    // 套接字api函数
    // sock api
    static Int32 CreateSock(SOCKET &newSock
                            , Int32 protoFamily = AF_INET                // ipv4 default
                            , Int32 type = SOCK_STREAM                  // 默认字节流, udp是 SOCK_DGRAM 报文
                            , Int32 protocol = IPPROTO_TCP             // 默认tcp协议
                            , bool noBlock = true                      // 非阻塞
                             , bool isNodelay = true                    // 禁用nagle算法保证低延迟
                             , bool reuseAddr = true                    // 端口重用
                             , bool closeOnExec = true                  // 程序关闭后自动close掉

    );
    static Int32 CreateSock2(SOCKET &newSock, Int32 af = AF_INET, Int32 type = SOCK_STREAM, Int32 protocol = IPPROTO_TCP);
    static Int32 Bind(SOCKET sock, const BriefAddrInfo &briefAddr
                      , Int32 protoFamily = AF_INET     // ipv4 default
                      , KernelSockAddrIn *localAddr = NULL
    );
    // 多线程或者多进程间事件通知
    static Int32 CreateEventFd();
    static Int32 Bind(SOCKET sock, const LibString &ip, UInt16 port, Int32 protoFamily = AF_INET, KernelSockAddrIn *localAddr = NULL);
    static Int32 Listen(SOCKET sock, Int32 unconnectQueueLen = SOMAXCONN);   // 默认排队数量

    static Int32 SetSockOp(SOCKET sock, bool noBlock = true, bool noDelay = true, bool reuseAddr = true, bool closeOnExec = true);
    static Int32 SetSockOp(SOCKET handle, Int32 level, Int32 optname, const void *optval, UInt64 len);
    static Int32 GetSockOp(SOCKET sock, Int32 level, Int32 optname, void *optval, LibSockLen *len);

    static Int32 UpdateLocalAddr(SOCKET sock, BriefSockAddr &addr);
    static Int32 UpdateRemoteAddr(UInt16 family, SOCKET sock, BriefSockAddr &addr);

    

    // // 套接字接口
    // @return(写入字节数)
    template<NetIoModule::Type ioModule>
    static Int32 Write(SOCKET sock, const Byte8 *buffer, UInt64 bufferSize, Int32 &err);
    template<NetIoModule::Type ioModule>
    static Int32 Read(SOCKET sock, Byte8 *buffer, UInt64 bufferSize, Int32 &err);
    static Int32 Connect(SOCKET sock, const KernelSockAddrIn *sin);

    static Int32 SyncSend(SOCKET handle, const void *buf, Int64 len, int flags, Int64 &handledBytes);
    

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    // // iocp接口
    /*
    *   brief:
    *       1. - PostAccept accept时候会把重叠体传入，在wait时候传回
    *                       传入重叠体结构便于区分绑定在完成端口上的socket中的子操作，
    *                       比如绑定的是监听socket但是可能被多个客户端连接，
    *                       为了区分是哪个客户端需要多传入一个重叠体结构以便区分
    *       2. - PostRecv 投递接收数据
    *       3. - PostSend 投递发送数据
    *       4. - return SockError_FatalError socket 需要关闭, 其他情况请自行处理
    */
public:
    // post线程安全
    static Int32 PostAccept(UInt16 family, IoData *ioData);
    static Int32 PostRecv(IoData *ioData);
    static Int32 PostSend(IoData *ioData);
    static Int32 PostConnect(IoData *ioData, UInt16 family, const LibString &ip, UInt16 port);
    
    #endif

    static Int32 ShutdownRead(SOCKET sock);
    static Int32 ShutdownWrite(SOCKET sock);
    static Int32 ShutdownReadWrite(SOCKET sock);
    
private:
    static bool _isInitEnv;
};

ALWAYS_INLINE bool SocketUtil::IsIp(const LibString &ip)
{
    return IsIpv4(ip) || IsIpv6(ip);
}

template<typename SockAddrInType>
inline Int32 SocketUtil::GetSockName(SOCKET sock, SockAddrInType &sockAddr, Int32 &lastError)
{
    if(!sock || sock == INVALID_SOCKET)
        return Status::Socket_ParamError;
    
    Int32 addrSize = static_cast<Int32>(sizeof(sockAddr));
    if(::getsockname(sock, (sockaddr *)(&sockAddr), reinterpret_cast<socklen_t *>(&addrSize)) != 0)
    {
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
        lastError = WSAGetLastError();
#else
        lastError = errno;
#endif
        return Status::IPUtil_GetSockNameFailed;
    }

    return Status::Success;
}

inline bool SocketUtil::GetAddrInfoFromNetInfo(sockaddr_in &addrObj, UInt64 szip, Byte8 *&ip, UInt16 &port)
{
    if(::inet_ntop(addrObj.sin_family, &addrObj.sin_addr.s_addr, ip, szip) == NULL)
        return false;

    port = ntohs(addrObj.sin_port);

    return true;
}

inline bool SocketUtil::GetAddrInfoFromNetInfo(sockaddr_in6 &addrObj, UInt64 szip, Byte8 *&ip, UInt16 &port)
{
    if(::inet_ntop(addrObj.sin6_family, &addrObj.sin6_addr, ip, szip) == NULL)
        return false;

    port = ntohs(addrObj.sin6_port);

    return true;
}

inline bool SocketUtil::IsValidSock(SOCKET sock)
{
    return sock != INVALID_SOCKET;
}

inline Int32 SocketUtil::Bind(SOCKET sock, const BriefAddrInfo &briefAddr
                      , Int32 protoFamily     // ipv4 default
                      , KernelSockAddrIn *localAddr
    )
{
    return Bind(sock, briefAddr._ip, briefAddr._port, protoFamily, localAddr);
}

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
template<>
ALWAYS_INLINE Int32 SocketUtil::Write<NetIoModule::EPOLL_LT>(SOCKET sock, const Byte8 *buffer, UInt64 bufferSize, Int32 &err)
{
    Int32 ret = ::send(sock, buffer, bufferSize, 0);
    err = errno;
    if(ret <= 0)
    {
        if(g_Log->IsEnable(LogLevel::Trace))
            g_Log->Trace(LOGFMT_NON_OBJ_TAG(SocketUtil, "send ret -1 sock = [%d], buffer = [%p], bufferSize = [%llu] err = [%d]"), sock, buffer, bufferSize, err);    
        return Status::SockInterrupt;  
    }

    return ret;
}

template<>
ALWAYS_INLINE Int32 SocketUtil::Write<NetIoModule::EPOLL_ET>(SOCKET sock, const Byte8 *buffer, UInt64 bufferSize, Int32 &err)
{
    Int32 ret = 0;
    while ((ret = ::send(sock,
                         buffer, 
                         bufferSize, 
                         0)) < 0 && errno == EINTR);

    if(ret == -1)
    {
        err = errno;
        if(g_Log->IsEnable(LogLevel::Trace))
            g_Log->Trace(LOGFMT_NON_OBJ_TAG(SocketUtil, "send ret -1 sock = [%d], buffer = [%p], bufferSize = [%llu] err = [%d]")
                                , sock, buffer, bufferSize, err);

        // TODO:外部需要处理EWOULDBLOCK,EAGAIN,若非前两者错误,则视为sock错误需要断开
        return Status::SockInterrupt;
    }

    return ret;
}

template<>
ALWAYS_INLINE Int32 SocketUtil::Read<NetIoModule::EPOLL_LT>(SOCKET sock, Byte8 *buffer, UInt64 bufferSize, Int32 &err)
{
    Int32 ret = ::recv(sock, buffer, bufferSize, 0);
    if(ret <= 0)
    {
        err = errno;
        if(g_Log->IsEnable(LogLevel::Trace))
            g_Log->Trace(LOGFMT_NON_OBJ_TAG(SocketUtil, "recv fail sock = [%d], buffer = [%p], bufferSize = [%llu], errno = [%d]"), sock, buffer, bufferSize, err);
        return Status::SockInterrupt;
    }

    return ret;
}

template<>
ALWAYS_INLINE Int32 SocketUtil::Read<NetIoModule::EPOLL_ET>(SOCKET sock, Byte8 *buffer, UInt64 bufferSize, Int32 &err)
{
    Int32 ret;
    while ((ret = ::recv(sock,
                         buffer, 
                         bufferSize, 
                         0)) < 0 && errno == EINTR);
    if (ret == -1)
    {
        err = errno;
        // TODO:外部需要处理EWOULDBLOCK, EAGAIN,等
        if(g_Log->IsEnable(LogLevel::Trace))
            g_Log->Trace(LOGFMT_NON_OBJ_TAG(SocketUtil, "recv fail sock = [%d], buffer = [%p], bufferSize = [%llu], errno = [%d]"), sock, buffer, bufferSize, err);

        return Status::SockInterrupt;
    }

    return ret;
}

#else

// TODO:windows下的sock接口

#endif

ALWAYS_INLINE Int32 SocketUtil::DestroySocket2(SOCKET sock)
{
    return DestroySocket(sock);
}
KERNEL_END

#endif

