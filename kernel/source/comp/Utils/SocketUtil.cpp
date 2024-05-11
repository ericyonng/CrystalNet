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
 * Date: 2021-03-21 23:21:55
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/common/Buffer.h>
#include <kernel/comp/Utils/SocketUtil.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/NetEngine/BriefSockAddr.h>
#include <kernel/comp/NetEngine/Defs/sockaddr_in.h>
#include <kernel/comp/NetEngine/Defs/NetDefs.h>
#include <kernel/comp/NetEngine/Defs/IoData.h>
#include <kernel/comp/Utils/SockErrorMsgUtil.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/NetEngine/Defs/IoEvent.h>
#include <kernel/comp/Utils/Defs/LibGuidDefs.h>
#include <kernel/common/statics.h>
#include <kernel/common/status.h>
#include <kernel/comp/Utils/Defs/Math.h>

// extern "C"
// {
//     static void SigpipeHandler(Int32 signalNo)
//     {
//         g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "signal no[%d] raise"), signalNo);
//     }
// }


#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 typedef size_t ssize_t;

 #include <WinSock2.h>
 #include <ws2def.h>
 #include <ws2tcpip.h>
 #include<MSWSock.h>
 
#endif

#if CRYSTAL_TARGET_PLATFORM_LINUX
 #include <fcntl.h>
 #include <linux/version.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netinet/tcp.h>
 #include <arpa/inet.h>
#endif

KERNEL_BEGIN

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
static LPFN_ACCEPTEX _fnAcceptEx;   // acceptex
static LPFN_CONNECTEX _fnConnectEx; // connect
static LPFN_GETACCEPTEXSOCKADDRS _fnGetAcceptClientAddrIn;  // get client addr
#endif

bool SocketUtil::_isInitEnv = false;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
static bool __InitIocpEnv()
{
    DWORD bytesReturn = 0;
    GUID acceptExGuid = WSAID_ACCEPTEX;
    
    // 为了加载函数而临时创建,结束会被释放掉
    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == INVALID_SOCKET)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "create sock fail family"));
        return false;
    }

    // get acceptex
    Int32 st = ::WSAIoctl(sock,
                            SIO_GET_EXTENSION_FUNCTION_POINTER,
                            &acceptExGuid,
                            sizeof(acceptExGuid),
                            &(_fnAcceptEx),
                            sizeof(LPFN_ACCEPTEX),
                            &bytesReturn,
                            nullptr,
                            nullptr);
    if (st != 0)
    {
        auto error = ::WSAGetLastError();
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "WSAIoctl get acceptex function fail st:%d, error:%d, sock:%d"), st, error, sock);
        SocketUtil::DestroySocket(sock);
        return false;
    }

    // get connectex
    GUID connectExGuid = WSAID_CONNECTEX;
    st = ::WSAIoctl(sock,
                        SIO_GET_EXTENSION_FUNCTION_POINTER,
                        &connectExGuid,
                        sizeof(connectExGuid),
                        &(_fnConnectEx),
                        sizeof(LPFN_CONNECTEX),
                        &bytesReturn,
                        nullptr,
                        nullptr);
    if (st != 0)
    {
        auto error = ::WSAGetLastError();
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "WSAIoctl get connectex function fail st:%d, error:%d, sock:%d"), st, error, sock);
        SocketUtil::DestroySocket(sock);
        return false;
    }

    // 加载_fnGetAcceptClientAddrIn
    GUID getAcceptExSockAddrsGuid = WSAID_GETACCEPTEXSOCKADDRS;
    st = ::WSAIoctl(sock,
                        SIO_GET_EXTENSION_FUNCTION_POINTER,
                        &getAcceptExSockAddrsGuid,
                        sizeof(getAcceptExSockAddrsGuid),
                        &(_fnGetAcceptClientAddrIn),
                        sizeof(LPFN_GETACCEPTEXSOCKADDRS),
                        &bytesReturn,
                        nullptr,
                        nullptr);
    if (st != 0)
    {
        auto error = ::WSAGetLastError();
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "WSAIoctl get getacceptclientaddr function fail st:%d, error:%d, sock:%d"), st, error, sock);
        SocketUtil::DestroySocket(sock);
        return false;
    }

    SocketUtil::DestroySocket(sock);

    return true;
}
#endif

int SocketUtil::InitSocketEnv()
{
    if(_isInitEnv)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "has init sock before."));
        return Status::Success;
    }

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    WSADATA wsaData;
    memset(&wsaData, 0, sizeof(WSADATA));
    auto result = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        WSACleanup();
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "WSAStartup fail LOBYTE wVersion not 2 or HIBYTE wVersion not 2  result[%d]"), result);
        return Status::Error;
    }

    if(result != NO_ERROR)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "WSAStartup fail result[%d]"), result);
        return Status::Error;
    }

    // iocp相关接口初始化
    if(!__InitIocpEnv())
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "init iocp env fail"));
        return Status::Error;
    }

#endif

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    // if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    //  return (1);
    // 忽略异常信号，默认情况会导致进程终止
    // TODO:Linux下 异常情况下应该打印日志 统一走SignalHandleUtil 信号处理
    // signal(SIGPIPE, SigpipeHandler);
#endif

    _isInitEnv = true;
    
    return Status::Success;
}

int SocketUtil::ClearSocketEnv()
{
   if(!_isInitEnv)
   {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "has already clear sock env."));
        return Status::Success;
   }

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    int result = NO_ERROR;
    result = ::WSACleanup();
//     if(result == SOCKET_ERROR) 
//     {
//         if(WSACancelBlockingCall() == 0) //winsock 1.0
//             result = WSACleanup();
//     }

    if(result != NO_ERROR)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "WSACleanup fail."));
        return Status::Error;
    }
#endif

    _isInitEnv = false;
    return Status::Success;
}

Int32 SocketUtil::SetNoBlock(SOCKET sock)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    ULong ul = 1;
    if(SOCKET_ERROR == ioctlsocket(sock, FIONBIO, &ul))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "ioctlsocket fail."));
        return Status::Socket_SetNoBlockParamError;
    }
#else
    int flags;
    if((flags = fcntl(sock, F_GETFL, NULL)) < 0) 
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "fcntl F_GETFL fail flags[%d]."), flags);
        return Status::Socket_SetNoBlockParamError;
    }

    if(!(flags & O_NONBLOCK)) {
        if(fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "fcntl F_SETFL fail flags[%d]."), flags);
            return Status::Socket_SetNoBlockParamError;
        }
    }
#endif

    return Status::Success;
}

Int32 SocketUtil::SetBlock(SOCKET sock)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    ULong ul = 0;
    if(0 != ioctlsocket(sock, FIONBIO, &ul))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "ioctlsocket FIONBIO fail ul[%d]."), ul);
        return Status::Socket_SetBlockParamError;
    }

    return Status::Success;
#else
    int flags;
    if((flags = fcntl(sock, F_GETFL, NULL)) < 0) 
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "fcntl F_GETFL fail flags[%d]."), flags);

        return Status::Socket_SetBlockParamError;
    }

    flags &= ~O_NONBLOCK;
    if(fcntl(sock, F_SETFL, flags) == -1) 
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "fcntl F_SETFL fail flags[%d]."), flags);
        return Status::Socket_SetBlockParamError;
    }

    return Status::Success;
#endif

}

Int32 SocketUtil::SetCloseOnExec(SOCKET sock)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    // close-on-exec
    Int32 flags = ::fcntl(sock, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    Int32 ret = ::fcntl(sock, F_SETFD, flags);
    if (ret == -1)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "fcntl F_SETFD fail flags[%d] ret[%d]."), flags, ret);
        return Status::Socket_FcntlError;
    }
#endif

    return Status::Success;
}

Int32 SocketUtil::MakeReUseAddr(SOCKET sock, bool isEnable)
{
    int flag = isEnable ? 1 : 0;
    return SetSockOp(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(Int32));
}

Int32 SocketUtil::MakeReUsePort(SOCKET sock, bool isEnable)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    // linux 3.9才有SO_REUSEPORT,其他版本则无效处理
    int flag = isEnable ? 1 : 0;
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
        return SetSockOp(sock, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(Int32));
    #else
        return Status::Failed;
    #endif
#else // CRYSTAL_TARGET_PLATFORM_WINDOWS
    return MakeReUseAddr(sock);
#endif // CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
}

Int32 SocketUtil::MakeNoDelay(SOCKET sock, bool isNodelay)
{
    Int32 flag = isNodelay ? 1 : 0;
    return SetSockOp(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(Int32));
}

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
Int32 SocketUtil::UpdateAcceptContext(SOCKET &listenSock, SOCKET &sock)
{
    // 在acceptex调用完成后 设置SO_UPDATE_ACCEPT_CONTEXT 可让sock继承listenSock的属性
    return SetSockOp(sock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, &listenSock, sizeof(SOCKET));
}

Int32 SocketUtil::UpdateConnectContext(SOCKET &sock)
{
    return SetSockOp(sock, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
}
#endif

Int32 SocketUtil::DestroySocket(SOCKET &sock)
{
    Int32 ret = 0;
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if(LIKELY(sock != INVALID_SOCKET))
        ret = ::closesocket(sock);
#else
    if(LIKELY(sock != INVALID_SOCKET))
        ret = ::close(sock);
#endif
    g_Log->NetDebug(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "close ret = [%d], sock=[%d]"), ret, sock);
    sock = INVALID_SOCKET;
    if(UNLIKELY(ret < 0))
        g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "close sock fail ret[%d, %s]."), ret, SockErrorMsgUtil::GetString(ret).c_str());

    return ret == 0 ? Status::Success : Status::Socket_Error;
}

bool SocketUtil::IsReuseAddr(SOCKET sock)
{
    Int32 flag = 0;
    LibSockLen len = sizeof(Int32);

    Int32 err = GetSockOp(sock, SOL_SOCKET, SO_REUSEADDR, &flag, &len);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "get sock op fail err:%d"), err);
        return false;
    }

    return flag != 0;
}

bool SocketUtil::IsReusePort(SOCKET sock)
{
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        return IsReuseAddr(sock);
    #endif

    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        // linux 下内核版本3.9.0以上才有
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
            Int32 flag = 0;
            LibSockLen len = sizeof(Int32);

            Int32 err = GetSockOp(sock, SOL_SOCKET, SO_REUSEPORT, &flag, &len);
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "get sock op fail err:%d"), err);
                return false;
            }
            return flag != 0;
        #else
            return false;
        #endif
    #endif
}

bool SocketUtil::IsNoDelay(SOCKET sock)
{
    Int32 flag = 0;
    LibSockLen len = sizeof(Int32);
    Int32 err = GetSockOp(sock, IPPROTO_TCP, TCP_NODELAY, &flag, &len);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "get sock op fail sock:%d, err:%d"), sock, err);
        return false;
    }

    return flag != 0;
}

bool SocketUtil::IsNonBlocking(SOCKET sock)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    
    g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "windows cant get socket block status."));
    return false;
    
#else
    Int32 flags;
    if((flags = ::fcntl(sock, F_GETFL, NULL)) < 0) 
    {
        Int32 err = errno;
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "fcntl F_GETFL fail flags[%d] err:%d, %s.")
                , flags, err, SystemUtil::GetErrString(err).c_str());
        return false;
    }

    return flags & O_NONBLOCK;
#endif
}

bool SocketUtil::IsCloseOnExec(SOCKET sock)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    // close-on-exec
    Int32 flags = ::fcntl(sock, F_GETFD, 0);
    if(flags < 0)
    {
        Int32 err = errno;
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "fcntl F_GETFL fail flags[%d] err:%d, %s.")
                , flags, err, SystemUtil::GetErrString(err).c_str());
        return false;
    }

    return flags & FD_CLOEXEC;
#else
    g_Log->Warn(LOGFMT_NON_OBJ_TAG(SocketUtil, "windows have no close on exe feature"));
    return false;
#endif
}

UInt64 SocketUtil::GetSendBufSize(SOCKET sock)
{
    Int32 rcvBufSize = 0;
    LibSockLen len = sizeof(Int32);
    if (GetSockOp(sock
                  , SOL_SOCKET
                  , SO_RCVBUF
                  ,reinterpret_cast<void *>(&rcvBufSize)
                  , &len) != Status::Success)
        return 0;

    return static_cast<size_t>(rcvBufSize);
}

Int32 SocketUtil::SetSendBufSize(SOCKET sock, UInt64 size)
{
    if (UNLIKELY(size == 0))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "size is zero sock:%d"), sock);
        return Status::Failed;
    }

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (::setsockopt(sock, SOL_SOCKET, SO_SNDBUF, 
        reinterpret_cast<const char *>(&size), sizeof(Int32)) != 0)
    {
        Int32 err = errno;
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "setsockopt SO_SNDBUF fail size:%llu errno:%d,%s sock:%d")
                    ,size, err, SystemUtil::GetErrString(err).c_str(), sock);
        return Status::Failed;
    }

    return Status::Success;
#else // CRYSTAL_TARGET_PLATFORM_WINDOWS
    if (::setsockopt(sock, SOL_SOCKET, SO_SNDBUF, 
        reinterpret_cast<const char *>(&size), sizeof(Int32)) == SOCKET_ERROR)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "setsockopt SO_SNDBUF fail size:%llu sock:%d")
                    ,size, sock);
        return Status::Failed;
    }

    return Status::Success;
#endif // CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
}

UInt64 SocketUtil::GetRecvBufSize(SOCKET sock)
{
    Int32 rcvBufSize = 0;
    LibSockLen len = sizeof(Int32);
    if (GetSockOp(sock
                  , SOL_SOCKET
                  , SO_RCVBUF
                  , reinterpret_cast<void *>(&rcvBufSize)
                  , &len) != Status::Success)
        return 0;

    return static_cast<size_t>(rcvBufSize);
}

Int32 SocketUtil::SetRecvBufSize(SOCKET sock, UInt64 size)
{
    if (UNLIKELY(size == 0))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "SetRecvBufSize size is zero sock:%d"), sock);
        return Status::Failed;
    }

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (::setsockopt(sock, SOL_SOCKET, 
        SO_RCVBUF, reinterpret_cast<char *>(&size), sizeof(Int32)) != 0)
    {
        Int32 err = errno;
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "setsockopt SO_RCVBUF fail size:%llu errno:%d,%s sock:%d")
                    ,size, err, SystemUtil::GetErrString(err).c_str(), sock);
        return Status::Failed;
    }

    return Status::Success;
#else // CRYSTAL_TARGET_PLATFORM_WINDOWS
    if (::setsockopt(sock, SOL_SOCKET, 
        SO_RCVBUF, reinterpret_cast<char *>(&size), sizeof(Int32)) == SOCKET_ERROR)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "setsockopt SO_RCVBUF fail size:%llu sock:%d")
                    ,size, sock);
        return Status::Failed;
    }

    return Status::Success;
#endif //CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
}

bool SocketUtil::IsIpv4(const LibString &ip)
{
    if (UNLIKELY(ip.empty()))
        return false;

    struct sockaddr_in in4; 
    return ::inet_pton(AF_INET, ip.c_str(), &in4.sin_addr) == 1;

    // typedef std::vector<LibString> _Parts;
    // const _Parts parts = ip.Split(".");
    // if (parts.size() != 4)
    //     return false;

    // for (_Parts::const_iterator it = parts.begin();
    //      it != parts.end();
    //      ++it)
    // {
    //     const LibString &part = *it;
    //     if (part.empty())
    //         return false;

    //     for (LibString::size_type i = 0;
    //          i < part.length();
    //          ++i)
    //         if (!('0' <= part[i] && part[i] <= '9'))
    //             return false;
    // }

    // return true;
}

bool SocketUtil::IsIpv6(const LibString &ip)
{
    // 普通ipv6格式:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
    if(UNLIKELY(ip.empty()))
        return false;

    struct sockaddr_in6 in6; 
    return ::inet_pton(AF_INET6, ip.c_str(), &in6.sin6_addr) == 1;

    // typedef std::vector<LibString> _Parts;
    // const _Parts parts = ip.Split(":");
    // // 至少得是 xxxx::xxxx
    // if(parts.size() < 3)
    //     return false;
    
    // // ::出现最多只有一次,即空字符串最多只有一次
    // Int32 emptyLimit = 1;
    // if(parts[0].empty())
    //     emptyLimit += 1;
    // if(parts[parts.size()-1].empty())
    //     emptyLimit += 1;

    // Int32 countEmpty = 0;
    // for(auto &slice:parts)
    // {
    //     if(slice.empty())
    //     {
    //         ++countEmpty;
    //         continue;
    //     }

    //     // 必须是16进制且最多16位，2字节
    //     if(slice.size() > 4)
    //         return false;
        
    //     for (LibString::size_type i = 0;
    //          i < slice.length();
    //          ++i)
    //     {
    //         if(!StringUtil::IsHex(slice[i]))
    //             return false;
    //     }
    // }

    // // 表示::有多个
    // if(countEmpty > emptyLimit)
    //     return false;

    // return true;
}

Int32 SocketUtil::GetPeerAddr(UInt16 family, UInt64 sSocket, Int32 sizeIp, Byte8 *&ip, UInt16 &port, Int32 &lastError)
{
    if(!sSocket || sSocket == static_cast<UInt64>(INVALID_SOCKET))
        return Status::Socket_ParamError;

    //定义
    const bool isIpv4 = family == AF_INET;
    KernelSockAddrIn addr_in(isIpv4);
    MemUtil::Zeroset(addr_in._data);
    socklen_t  len = 0;
    if(addr_in._isIpv4)
    {
        len = sizeof(addr_in._data._sinV4);
    }
    else
    {
        len = sizeof(addr_in._data._sinV6);
    }

    // 获取客户端地址
    if(getpeername(sSocket, (struct sockaddr*)&addr_in._data, &len) != 0)
    {
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
        lastError = WSAGetLastError();
#else
        lastError = errno;
#endif
        return Status::IPUtil_GetPeerNameFailed;
    }

    if(addr_in._isIpv4)
    {
        auto &sinv4 = addr_in._data._sinV4;
        if(::inet_ntop(family, &sinv4.sin_addr.s_addr, ip, sizeIp) == NULL)
        {
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            lastError = WSAGetLastError();
    #else
            lastError = errno;
    #endif
            return Status::Error;
        }

        port = ntohs(sinv4.sin_port);
    }
    else
    {
        auto &sinv6 = addr_in._data._sinV6;
        if(::inet_ntop(family, &addr_in._data._sinV6.sin6_addr, ip, sizeIp) == NULL)
        {
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            lastError = WSAGetLastError();
    #else
            lastError = errno;
    #endif
            return Status::Error;
        }
        port = ntohs(sinv6.sin6_port);
    }

    return Status::Success;
}

bool SocketUtil::FillTcpAddrInfo(const Byte8 *ip, UInt16 port, UInt16 family, sockaddr_in &addrObj)
{
    if(!ip || strlen(ip) == 0)
        return false;

    ::memset(&addrObj, 0, sizeof(addrObj));
    addrObj.sin_family = family;
    if(::inet_pton(addrObj.sin_family, ip, &addrObj.sin_addr.s_addr) != 1)
        return false;

    addrObj.sin_port = htons(port);

    return true;
}

bool SocketUtil::FillTcpAddrInfo(const Byte8 *ip, UInt16 port, UInt16 family, sockaddr_in6 &addrObj)
{
    if(!ip || strlen(ip) == 0)
        return false;

    ::memset(&addrObj, 0, sizeof(addrObj));
    addrObj.sin6_family = family;
    if(::inet_pton(addrObj.sin6_family, ip, &addrObj.sin6_addr) != 1)
        return false;

    addrObj.sin6_port = htons(port);

    return true;
}

bool SocketUtil::IsDetectTimeOut(
    SOCKET &sock
    , timeval timeout
    , bool enableReadableDetect
    , bool enableWriteableDetect
    , int *errOut
    , bool isInfiniteWaiting)
{
    // 清理
    fd_set *readableDetect = NULL;
    fd_set *writableDetect = NULL;
    fd_set  readableSet;
    fd_set  writableSet;
    FD_ZERO(&readableSet);
    FD_ZERO(&writableSet);

    if(enableReadableDetect)
    {
        readableDetect = &readableSet;
        FD_SET(sock, &readableSet);
    }

    if(enableWriteableDetect)
    {
        writableDetect = &writableSet;
        FD_SET(sock, &writableSet);
    }

    int ret = SOCKET_ERROR;
    if(isInfiniteWaiting) { //永久阻塞
        // 0表示超时，否则返回SOCKET_ERROR 当返回为-1时，所有描述符集清0。 
        // 当返回为正数时，表示已经准备好的描述符数。
        ret = select(static_cast<Int32>(sock + 1), readableDetect, writableDetect, NULL, NULL); 
    }
    else {
        // 0表示超时，否则返回SOCKET_ERROR 当返回为-1时，所有描述符集清0。
        // 当返回为正数时，表示已经准备好的描述符数。
        ret = select(static_cast<Int32>(sock + 1), readableDetect, writableDetect, NULL, &timeout); 
    }

    // select结果带出去
    if(errOut) 
        *errOut = ret;

    if(ret == SOCKET_ERROR) {
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        perror("select fail");
#else
        auto lastErr = GetLastError();
        LibString err;
        err.AppendFormat("\nselect fail lastErr[%lu]\n", lastErr);
        SystemUtil::LockConsole();
        SystemUtil::OutputToConsole(err);
        SystemUtil::UnlockConsole();
#endif
        FD_CLR(sock, &readableSet);
        FD_CLR(sock, &writableSet);

        return true;
    }

    FD_CLR(sock, &readableSet);
    FD_CLR(sock, &writableSet);

    // 没有超时
    if(ret > 0)
        return false;

    return true;
}

Int32 SocketUtil::SetSocketCacheSize(SOCKET &sock, SocketDefs::SOCKET_CACHE_TYPE eType, Int64 cacheSize, Int32 &lastErr)
{
    if(UNLIKELY(!IS_VALID_SOCKET_IDD(static_cast<UInt64>(sock))))
    {
         g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "invalid sock[%d]."), sock);
        return Status::Socket_InvalidSocket;
    }

    switch(eType)
    {
        case SocketDefs:: SOCKET_CACHE_TYPE_RECV:
        {
            Int32 ret = SetSockOp(sock, SOL_SOCKET, SO_RCVBUF, &cacheSize, sizeof(cacheSize));
            if(ret != Status::Success)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "setsockopt SO_RCVBUF fail sock[%d] ret[%d]."), sock, ret);
                return Status::Socket_SetSockOptFailed;
            }
        }
        break;
        case SocketDefs::SOCKET_CACHE_TYPE_SEND:
        {
            auto ret = SetSockOp(sock, SOL_SOCKET, SO_SNDBUF, &cacheSize, sizeof(cacheSize));
            if(ret != Status::Success)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "setsockopt SO_SNDBUF fail sock[%d] ret[%d]."), sock, ret);
                return Status::Socket_SetSockOptFailed;
            }
        }
        break;
        default:
        {
            return Status::Socket_ParamError;
        }
    }

    return Status::Success;
}

Int32 SocketUtil::GetSocketCacheSize(SOCKET &sock, SocketDefs::SOCKET_CACHE_TYPE eType, Int64 &cacheSize, Int32 &lastErr)
{
    if(UNLIKELY(!IS_VALID_SOCKET_IDD(static_cast<UInt64>(sock))))
    {
         g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "invalid sock[%d]."), sock);
        return Status::Socket_InvalidSocket;
    }

    switch(eType)
    {
        case SocketDefs::SOCKET_CACHE_TYPE_RECV:
        {
            LibSockLen Len = sizeof(cacheSize);
            Int32 ret = GetSockOp(sock, SOL_SOCKET, SO_RCVBUF, &cacheSize, &Len);
            if(ret != Status::Success)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "getsockopt SO_RCVBUF fail sock[%d] nRet[%d]."), sock, ret);
                return Status::Socket_GetsockoptFailed;
            }
        }
        break;
        case SocketDefs::SOCKET_CACHE_TYPE_SEND:
        {
            LibSockLen Len = sizeof(cacheSize);
            Int32 ret = GetSockOp(sock, SOL_SOCKET, SO_SNDBUF, &cacheSize, &Len);
            if(ret != Status::Success)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "getsockopt SO_SNDBUF fail sock[%d] nRet[%d]."), sock, ret);
                return Status::Socket_GetsockoptFailed;
            }
        }
        break;
        default:
        {
            return Status::Socket_Unknown;
        }
    }

    return Status::Success;
}

LibString SocketUtil::ToFmtSpeedPerSec(Int64 bytesps)
{
    // 小于1MB大于1KB的使用KB/s
    // 大于1MB小于1GB的使用MB/s
    // 大于1GB的使用GB/s 
    // 精度保留3位小数
    LibString info;
    if(bytesps < __LIB_DATA_1KB__)
    {// 小于1KB的使用B/s
        info.AppendFormat("%lld B/s", bytesps);
        return info;
    }
    else if(bytesps >= __LIB_DATA_1KB__ && bytesps < __LIB_DATA_1MB__)
    {
        double speedData = static_cast<double>(bytesps);
        speedData = speedData / __LIB_DATA_1KB__;
        info.AppendFormat("%.3lf KB/s", speedData);
        return info;
    }
    else if(bytesps >= __LIB_DATA_1MB__ && bytesps < __LIB_DATA_1GB__)
    {
        double speedData = static_cast<double>(bytesps);
        speedData = speedData / __LIB_DATA_1MB__;
        info.AppendFormat("%.3lf MB/s", speedData);
        return info;
    }
    else if(bytesps >= __LIB_DATA_1GB__)
    {
        double speedData = static_cast<double>(bytesps);
        speedData = speedData / __LIB_DATA_1GB__;
        info.AppendFormat("%.3lf GB/s", speedData);
        return info;
    }

    return "";
}


Int32 SocketUtil::CreateSock(SOCKET &newSock
                             , Int32 protoFamily /*= AF_INET ipv4 default */
                             , Int32 type /*= __FS_TCP_SOCKTYPE__  默认字节流, udp是 SOCK_DGRAM 报文 */
                             , Int32 protocol /*= IPPROTO_TCP 默认tcp协议 */
                             , bool noBlock /* = true */                // 非阻塞
                             , bool isNodelay /* = true */              // 禁用nagle算法保证低延迟
                             , bool reuseAddr /* = true */              // 端口重用  
                             , bool closeOnExec /* = true */            // 程序关闭后自动close掉
                            
)
{

#if CRYSTAL_TARGET_PLATFORM_LINUX
    newSock = ::socket(protoFamily, type, protocol);
#else
    newSock = ::WSASocket(protoFamily, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
#endif
    if(UNLIKELY(newSock == INVALID_SOCKET))
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "create sock fail protoFamily[%d], type[%d], protocol[%d]")
        , protoFamily, type, protocol);
        return Status::Failed;
    }

    auto err = SetSockOp(newSock, noBlock, isNodelay, reuseAddr, closeOnExec);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "set sock op fail newSock[%d], err[%d]"), newSock, err);
        DestroySocket(newSock);
        return err;
    }

    return Status::Success;
}

Int32 SocketUtil::CreateSock2(SOCKET &newSock, Int32 af, Int32 type, Int32 protocol)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    newSock = ::socket(af, type, protocol);
#else
    newSock = ::WSASocket(af, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
#endif

    if(UNLIKELY(newSock == INVALID_SOCKET))
    {
        Int32 err;
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            err = ::WSAGetLastError();
            g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "create sock fail af:%d, type:%d, protocol:%d, err:%d")
                                , af, type, protocol, err);
        #endif

        #if CRYSTAL_TARGET_PLATFORM_LINUX
            err = errno;
            const auto &errStr = SystemUtil::GetErrString(err);
            g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "create sock fail af:%d, type:%d, protocol:%d, err:%d, %s")
                                , af, type, protocol, err, errStr.c_str());
        #endif

        return Status::Socket_Error;
    }

    return Status::Success;
}

Int32 SocketUtil::Bind(SOCKET sock, const LibString &ip, UInt16 port, Int32 protoFamily, KernelSockAddrIn *localAddr)
{
    // TODO:读取配置表
    // 1.bind 绑定用于接受客户端连接的网络端口
    KernelSockAddrIn sin(protoFamily == AF_INET);
    if(sin._isIpv4)
    {
        auto &sin_v4 = sin._data._sinV4;
        MemUtil::Zeroset(sin_v4);
        sin_v4.sin_family = protoFamily;
        // host to net unsigned short
        sin_v4.sin_port = htons(port);
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            if(ip.empty())
            {
                sin_v4.sin_addr.s_addr = INADDR_ANY;
            }
            else
            {
                // 比较新的函数对比 inet_addr
                ::inet_pton(sin_v4.sin_family, ip.c_str(), &(sin_v4.sin_addr));
            }
        #else
            if(ip.empty()) 
            {
                sin_v4.sin_addr.s_addr = INADDR_ANY;
            }
            else 
            {
                ::inet_pton(sin_v4.sin_family, ip.c_str(), &(sin_v4.sin_addr.s_addr));
            }
        #endif

        #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
            Int32 ret = ::bind(sock, (sockaddr *)&sin_v4, sizeof(sin_v4));
            if(-1 == ret)
            {
                Int32 lastErr = errno;
                g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "bind fail sock = [%d], ip[%s:%hu] protoFamily[%d] errno=[%d], errstring[%s]")
                        , sock, ip.c_str(), port, protoFamily, lastErr, SystemUtil::GetErrString(lastErr).c_str());
                return Status::BindFail;
            }
        #else
            Int32 ret = ::bind(sock, (sockaddr *)&sin_v4, sizeof(sin_v4));
            if(SOCKET_ERROR == ret)
            {
                Int32 lastErr = ::WSAGetLastError();
                g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "bind fail sock = [%d], ip[%s:%hu] protoFamily[%d] errno=[%d], errstring[%s]")
                        , sock, ip.c_str(), port, protoFamily, lastErr, SystemUtil::GetErrString(lastErr).c_str());
                return Status::BindFail;
            }
        #endif
    }
    else
    {
        auto &sin_v6 = sin._data._sinV6;
        MemUtil::Zeroset(sin_v6);
        sin_v6.sin6_family = AF_INET6;
        sin_v6.sin6_port = htons(port);
        if(ip.empty()) 
        {
            sin_v6.sin6_addr = in6addr_any;
        }
        else 
        {
            ::inet_pton(sin_v6.sin6_family, ip.c_str(), &(sin_v6.sin6_addr));
        }

        #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
            Int32 ret = ::bind(sock, (sockaddr *)&sin_v6, sizeof(sin_v6));
            if(-1 == ret)
            {
                Int32 lastErr = errno;
                g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "bind fail sock = [%d], ip[%s:%hu] protoFamily[%d] errno=[%d], errstring[%s]")
                        , sock, ip.c_str(), port, protoFamily, lastErr, SystemUtil::GetErrString(lastErr).c_str());
                return Status::BindFail;
            }
        #else
            Int32 ret = ::bind(sock, (sockaddr *)&sin_v6, sizeof(sin_v6));
            if(SOCKET_ERROR == ret)
            {
                Int32 lastErr = ::WSAGetLastError();
                g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "bind fail sock = [%d], ip[%s:%hu] protoFamily[%d] errno=[%d], errstring[%s]")
                        , sock, ip.c_str(), port, protoFamily, lastErr, SystemUtil::GetErrString(lastErr).c_str());
                return Status::BindFail;
            }
        #endif
    }

    if(localAddr)
        *localAddr = sin;

    g_Log->NetTrace(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "bind suc sock = [%d], ip[%s:%hu] protoFamily[%d, %s]."), sock, ip.c_str(), port, protoFamily, FamilyType::ToString(protoFamily));

    return Status::Success;
}

Int32 SocketUtil::CreateEventFd()
{
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        Int32 evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (evtfd < 0)
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "eventfd fail errno=[%d], errstr[%s]"), errno, SystemUtil::GetErrString(errno).c_str());
        }

        return evtfd;

    #else
        return 0;
    #endif
}

Int32 SocketUtil::Listen(SOCKET sock, Int32 unconnectQueueLen /*= SOMAXCONN*/)
{
    if (unconnectQueueLen <= 0)
        unconnectQueueLen = SOMAXCONN;

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (::listen(sock, unconnectQueueLen) == -1)
    {
        Int32 err = errno;
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "listen fail sock:%d, err:%d, %s"), sock, err, SystemUtil::GetErrString(err).c_str());
        return Status::ListenFail;
    }
#else // CRYSTAL_TARGET_PLATFORM_WINDOWS
    if (::listen(sock, unconnectQueueLen) == SOCKET_ERROR)
    {
        Int32 err = ::WSAGetLastError();
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "listen fail sock:%d, err:%d"), sock, err);
        return Status::ListenFail;
    }
#endif // CRYSTAL_TARGET_PLATFORM_NON_WINDOWS


    return Status::Success;
}

Int32 SocketUtil::SetSockOp(SOCKET sock, bool noBlock, bool noDelay, bool reuseAddr, bool closeOnExec)
{
    if(LIKELY(noBlock))
    {
        auto err = SetNoBlock(sock);
        if(UNLIKELY(err != Status::Success))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "set no block fail new sock[%d], err[%d]"), sock, err);
            return err;
        }
    }

    if(LIKELY(noDelay))
    {
        auto err = MakeNoDelay(sock, true);
        if(UNLIKELY(err != Status::Success))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "MakeNoDelayk fail new sock[%d], err[%d]"), sock, err);
            return err;
        }
    }

    if(LIKELY(reuseAddr))
    {
        auto err = MakeReUseAddr(sock);
        if(UNLIKELY(err != Status::Success))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "MakeReUseAddr fail new sock[%d], err[%d]"), sock, err);
            return err;
        }
    }

    if(LIKELY(closeOnExec))
    {
        auto err = SetCloseOnExec(sock);
        if(UNLIKELY(err != Status::Success))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "MakeReUseAddr fail new sock[%d], err[%d]"), sock, err);
            return err;
        }
    }

    return Status::Success;
}

Int32 SocketUtil::SetSockOp(SOCKET handle, Int32 level, Int32 optname, const void *optval, UInt64 len)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    if (::setsockopt(handle, level, optname, 
        reinterpret_cast<const char *>(optval), len) != 0)
    {
        Int32 err = errno;
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "setsockopt fail handle:%d, level:%d, optname:%d, err:%d, %s")
                    , handle, level, optname, err, SystemUtil::GetErrString(err).c_str());
        return Status::Failed;
    }

    return Status::Success;
#else
    if (::setsockopt(handle, level, optname, 
        reinterpret_cast<const char *>(optval), static_cast<Int32>(len)) == SOCKET_ERROR)
    {
        Int32 err = ::WSAGetLastError();
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "setsockopt fail handle:%d, level:%d, optname:%d, err:%d")
                    , handle, level, optname, err);
        return Status::Failed;
    }

    return Status::Success;
#endif
}

Int32 SocketUtil::GetSockOp(SOCKET sock, Int32 level, Int32 optname, void *optval, LibSockLen *len)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    if (::getsockopt(sock, level, optname, 
        reinterpret_cast<Byte8 *>(optval), len) != 0)
    {
        Int32 err = errno;
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "GetSockOp::getsockopt fail sock[%d], level=[%d], optname=[%d], err:%d,%s")
                    , sock, level, optname, err, SystemUtil::GetErrString(err).c_str());
        return Status::Socket_Error;
    }

    return Status::Success;
#else // windows
    if (::getsockopt(sock, level, optname, 
        reinterpret_cast<Byte8 *>(optval), len) == SOCKET_ERROR)
    {
        Int32 err = ::WSAGetLastError();
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "GetSockOp::getsockopt fail sock[%d], level=[%d], optname=[%d], last err:%d")
                    , sock, level, optname, err);
        return Status::Socket_Error;
    }

    return Status::Success;
#endif // windows
}

Int32 SocketUtil::UpdateLocalAddr(SOCKET sock, BriefSockAddr &addr)
{
    // 本地地址
    Int32 lastErr = 0;
    Int32 errCode = Status::Success;
    if(addr._addr._isIpv4)
    {
        errCode = SocketUtil::GetSockName(sock, addr._addr._data._sinV4, lastErr);
    }
    else
    {
        errCode = SocketUtil::GetSockName(sock, addr._addr._data._sinV6, lastErr);
    }
    if(errCode != Status::Success)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "_sock[%d] GetSockName fail errCode = [%d]!")
                                , sock, errCode);
        return errCode;
    }

    BUFFER256 ip = {0};
    Byte8 *ptr = ip;
    const auto isIpv4 = addr._addr._isIpv4;
    if(isIpv4)
    {
        if(!SocketUtil::GetAddrInfoFromNetInfo(addr._addr._data._sinV4, sizeof(ip), ptr, addr._port))
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "_sock[%d], GetAddrInfoFromNetInfo fail"), sock);
            return Status::Failed;
        }
    }
    else
    {
        if(!SocketUtil::GetAddrInfoFromNetInfo(addr._addr._data._sinV6, sizeof(ip), ptr, addr._port))
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "_sock[%d], GetAddrInfoFromNetInfo fail"), sock);
            return Status::Failed;
        }
    }

    addr._ip = ip;
    if(isIpv4)
    {
        addr._ipNetByteOrder[0] = addr._addr._data._sinV4.sin_addr.s_addr;
    }
    else
    {
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        addr._ipNetByteOrder[0] =  *reinterpret_cast<const UInt64 *>(addr._addr._data._sinV6.sin6_addr.u.Byte);
        addr._ipNetByteOrder[1] =  *reinterpret_cast<const UInt64 *>(&(addr._addr._data._sinV6.sin6_addr.u.Byte[8]));
        #endif

        #if CRYSTAL_TARGET_PLATFORM_LINUX
        /*
            struct sockaddr_in6 {
               sa_family_t     sin6_family;   // AF_INET6 
               in_port_t       sin6_port;     // port number 
               uint32_t        sin6_flowinfo; // IPv6 flow information 
               struct in6_addr sin6_addr;     // IPv6 address 
               uint32_t        sin6_scope_id; // Scope ID (new in 2.4) 
           };
           struct in6_addr {
               unsigned char   s6_addr[16];   // IPv6 address 
           };
        */

       auto order0 = &(addr._addr._data._sinV6.sin6_addr.s6_addr[0]);
       auto order1 = &(addr._addr._data._sinV6.sin6_addr.s6_addr[8]);
        addr._ipNetByteOrder[0] =  *reinterpret_cast<const UInt64 *>(order0);
        addr._ipNetByteOrder[1] =  *reinterpret_cast<const UInt64 *>(order1);
        #endif

    }
    addr._ipAndPort.AppendFormat("%s:%hu", ip, addr._port);
    addr._isUpdate = true;

    // g_Log->Debug(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "sock[%d] local addr info[%s]")
    // ,sock, addr.ToString().c_str());

    return Status::Success;
}

Int32 SocketUtil::UpdateRemoteAddr(UInt16 family, SOCKET sock, BriefSockAddr &addr)
{
    // 远程地址
    BUFFER256 ip = {0};
    Byte8 *ptr = ip;
    Int32 lastErr = 0;
    Int32 errCode = SocketUtil::GetPeerAddr(family, sock, sizeof(ip), ptr, addr._port, lastErr);
    if(errCode != Status::Success)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "sock[%d] get peer addr fail lastErr:%d!"), sock, lastErr);
        return errCode;
    }

    addr._ip = ip;
    lastErr = ::inet_pton(family, ip, &addr._ipNetByteOrder);
    if(lastErr != 1)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "inet_pton fail lastErr[%d], family[%hu], ip[%s]"), lastErr, family, ip);
        return Status::Failed;
    }
    addr._isUpdate = true;
    if(addr._addr._isIpv4)
    {
        auto &sinv4 = addr._addr._data._sinV4;
        sinv4.sin_family = family;
        sinv4.sin_addr.s_addr = 
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            static_cast<ULong>(addr._ipNetByteOrder[0]);
        #else
            addr._ipNetByteOrder[0];
        #endif
        sinv4.sin_port = htons(addr._port);
    }
    else
    {
        auto &sinv6 = addr._addr._data._sinV6;
        sinv6.sin6_family = family;
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            MemUtil::Memcpy(sinv6.sin6_addr.u.Byte, addr._ipNetByteOrder, sizeof(sinv6.sin6_addr.u.Byte));
        #else
            MemUtil::Memcpy(sinv6.sin6_addr.__in6_u.__u6_addr8, addr._ipNetByteOrder, sizeof(sinv6.sin6_addr.__in6_u.__u6_addr8));
        #endif
        sinv6.sin6_port = htons(addr._port);
    }
    addr._ipAndPort.AppendFormat("%s:%hu", ip, addr._port);

    // g_Log->NetDebug(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "sock[%d] _family[%hu] remote addr info [%s]")
    // ,sock, family, addr.ToString().c_str());    

    return Status::Success;
}

Int32 SocketUtil::Connect(SOCKET sock, const KernelSockAddrIn *sin)
{
    Int32 err = Status::Success;
    UInt64 len = sin->_isIpv4 ? sizeof(sin->_data._sinV4) : sizeof(sin->_data._sinV6);

    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        if (::connect(sock, reinterpret_cast<const struct sockaddr *>(&sin->_data), len) == -1)
        {
            // 网络阻塞 需要异步等待完成
            if (errno == EINPROGRESS)
                err = Status::SockError_EWOULDBLOCK;
            else
                err = Status::Socket_Error;

            Int32 lastErr = errno;
            g_Log->NetTrace(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "connect perhaps fail lastErr = [%d, %s], err = [%d], sock = [%d]")
                                        , lastErr, SystemUtil::GetErrString(lastErr).c_str(), err, sock);
        }
    #endif

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        if (::connect(sock, reinterpret_cast<const struct sockaddr *>(&sin->_data), static_cast<Int32>(len)) == SOCKET_ERROR)
        {
            // 网络阻塞 需要异步等待完成
            Int32 lastErr = ::WSAGetLastError();
            if (lastErr == WSAEWOULDBLOCK)
                err = Status::SockError_EWOULDBLOCK;
            else
                err = Status::Socket_Error;

            g_Log->NetTrace(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "connect perhaps fail lastErr = [%d, %s], err = [%d], sock = [%d]")
                                        , lastErr, SystemUtil::GetErrString(lastErr).c_str(), err, sock);
        }
    #endif
    

    return err;
}

Int32 SocketUtil::SyncSend(SOCKET handle, const void *buf, Int64 len, int flags, Int64 &handledBytes)
{
#if CRYSTAL_TARGET_PLATFORM_NON_IPHONE
    int ret = 0;
#else // iPhone
    ssize_t ret = 0;
#endif

    while ((ret = ::send(handle,
                         reinterpret_cast<const char *>(buf), 
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
                         static_cast<Int32>(len), 
#else
                         len, 
#endif
                         flags)) < 0 && errno == EINTR);

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (ret == -1)
    {
        // 缓冲区满不可写
        if(errno == EWOULDBLOCK || errno == EAGAIN)
        {
            // g_Log->NetDebug(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "sync send SockError_EAGAIN_OR_EWOULDBLOCK error errno = [%d, %s]"), errno, SystemUtil::GetErrString(errno).c_str());
            return Status::SockError_EAGAIN_OR_EWOULDBLOCK;
        }

        g_Log->NetDebug(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "do send unknown error errno = [%d, %s]"), errno, SystemUtil::GetErrString(errno).c_str());
        return Status::SockError_UnknownError;
    }

    handledBytes += ret;
    return Status::Success;
#else // CRYSTAL_TARGET_PLATFORM_WIN32
    if (ret == SOCKET_ERROR)
    {
        Int32 lastErr = ::WSAGetLastError();
        if (::WSAGetLastError() == WSAEWOULDBLOCK)
        {
            g_Log->NetDebug(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "sync send SockError_EAGAIN_OR_EWOULDBLOCK error"));
            return Status::SockError_EAGAIN_OR_EWOULDBLOCK;
        }

        g_Log->NetError(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "sync send fatal error lasterr:%d, %s"), lastErr, SockErrorMsgUtil::GetString(lastErr).c_str());
        return Status::SockError_UnknownError;
    }

    handledBytes += ret;
    return Status::Success;
#endif
}

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

Int32 SocketUtil::PostAccept(UInt16 family, IoData *ioData)
{
    static void *acceptBuff[(sizeof(KernelSockAddrInRaw) + 16) * 2] = {NULL};

    const bool isIpV4 = family == AF_INET;
    static const UInt64 specifyAddrSize = static_cast<UInt64>((isIpV4 ? (sizeof(sockaddr_in) + 16) : sizeof(sockaddr_in6)) + 16);
    ioData->_ioType = IoEventType::IO_ACCEPT;
    if(!_fnAcceptEx(ioData->_listenSock
                    , ioData->_sock
                    , acceptBuff
                    , 0
                    , static_cast<DWORD>(specifyAddrSize)     // msdn指定参数
                    , static_cast<DWORD>(specifyAddrSize)     // msdn指定参数
                    , NULL
                    , &ioData->_overlapped)) // 重叠体的地址必须是自定义结构体的初始位置，必须是自定义结构体的第一个成员可以是自定义的结构体
    {
        auto netLastError = ::WSAGetLastError();
        const auto &errStr = SockErrorMsgUtil::GetString(netLastError);
        if(netLastError == ERROR_IO_PENDING)
        {
            g_Log->NetDebug(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "accept pending io will complete in future sock:%d."), ioData->_sock);
            return Status::SockError_Pending;
        }

        g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "PostAccept listenSocket[%llu] ioSocket[%llu] to completionport failed windows error<%d> status[%d], system error info:%s")
                            , ioData->_listenSock, ioData->_sock, netLastError, Status::SockError_FatalError, errStr.c_str());
        
        return Status::SockError_FatalError;
    }

    return Status::Success;
}

Int32 SocketUtil::PostRecv(IoData *ioData)
{
    UInt64 bytesOfRecv = 0;
    UInt64 flags = 0;
    ioData->_handledBytes = 0;
    ioData->_ioType = IoEventType::IO_RECV;
    Int32 ret = ::WSARecv(ioData->_sock, &ioData->_wsaBuff, 1, reinterpret_cast<DWORD *>(&bytesOfRecv), reinterpret_cast<DWORD *>(&flags), &ioData->_overlapped, NULL);
    if (ret == SOCKET_ERROR)
    {
        int netLastError = ::WSAGetLastError();
        const auto &errStr = SockErrorMsgUtil::GetString(netLastError);
        if (netLastError == WSA_IO_PENDING) // 异步等待完成通知
        {// pending 不是失败,而是io需要异步处理,未完成，此时内存将被锁定，如果锁定过多将造成资源枯竭,系统将无法继续投递io请求，并返回WSAENOBUFS
            // g_Log->NetDebug(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "windows io pending when post recv sock:%d, sessionId:%llu"), ioData->_sock, ioData->_sessionId);
            return Status::SockError_Pending;
        }

        // 大量io请求,请重试
        if(netLastError == WSAEWOULDBLOCK)
        {
            g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "large overlapped io request please retry later sock:%d, sessionId:%llu system error info:%s.")
                        , ioData->_sock, ioData->_sessionId, errStr.c_str());
            return Status::SockError_EWOULDBLOCK;
        }

        // 系统没有资源,需要等待释放 请重试
        if(netLastError == WSAENOBUFS)
        {
            g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "large application memory had locked and not complete, and net would not use!!! system error info:%s"), errStr.c_str());
            return Status::IOCP_NoBuffs;
        }

        // 对端关闭
        // if(netLastError == WSAECONNRESET)
        // {
        //     g_Log->NetInfo(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "targetsock:%d, sessionId:%llu an existing connection was forcibly closed by the remote host. win error<%d> status<%d>, system error info:%s")
        //             , ioData->_sock, ioData->_sessionId, netLastError, Status::IOCP_RemoteForciblyClosed, errStr.c_str());
        //     return Status::IOCP_RemoteForciblyClosed;
        // }

        g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "PostRecv ioSocket[%llu], sessionId:%llu to completionport failed windows error<%d> status[%d], system error info:%s")
                            , ioData->_sock, ioData->_sessionId, netLastError, Status::SockError_FatalError, errStr.c_str());

        return Status::SockError_FatalError;
    }

    ioData->_handledBytes = bytesOfRecv;
    return Status::Success;
}

Int32 SocketUtil::PostSend(IoData *ioData)
{
    UInt64 flags = 0;
    UInt64 bytesSent = 0;
    ioData->_handledBytes = 0;
    ioData->_ioType = IoEventType::IO_SEND;
    Int32 ret = ::WSASend(ioData->_sock, &ioData->_wsaBuff, 1
                , reinterpret_cast<DWORD *>(&bytesSent), static_cast<DWORD>(flags)
                , &ioData->_overlapped, NULL);

    if (ret == SOCKET_ERROR)
    {
        int netLastError = ::WSAGetLastError();
        const auto &errStr = SockErrorMsgUtil::GetString(netLastError);
        if (netLastError == WSA_IO_PENDING) // 异步等待完成通知
        {// pending 不是失败,而是io需要异步处理,未完成，此时内存将被锁定，如果锁定过多将造成资源枯竭,系统将无法继续投递io请求，并返回WSAENOBUFS
            g_Log->NetDebug(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "windows io pending when post send sock:%d, sessionId:%llu"), ioData->_sock, ioData->_sessionId);
            return Status::SockError_Pending;
        }

        // 大量io请求,请重试
        if(netLastError == WSAEWOULDBLOCK)
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "large overlapped io request please retry later sock:%d, sessionId:%llu system error info:%s.")
                        , ioData->_sock, ioData->_sessionId, errStr.c_str());
            return Status::SockError_EWOULDBLOCK;
        }

        // 系统没有资源,需要等待释放 请重试
        if(netLastError == WSAENOBUFS)
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "large application memory had locked and not complete, and net would not use!!! system error info:%s"), errStr.c_str());
            return Status::IOCP_NoBuffs;
        }

        // 对端关闭
        // if (netLastError == WSAECONNRESET)
        // {
        //     g_Log->NetInfo(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "targetsock:%d, sessionId:%llu an existing connection was forcibly closed by the remote host. win error<%d> status<%d>, %s")
        //             , ioData->_sock, ioData->_sessionId, netLastError, Status::IOCP_RemoteForciblyClosed, errStr.c_str());
        // }

        g_Log->NetInfo(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "other netLastError:%d from net and should close socket:%d, sessionId:%llu, system error info:%s")
                        , netLastError, ioData->_sock, ioData->_sessionId, errStr.c_str());

        return Status::SockError_FatalError;
    }

    ioData->_handledBytes = bytesSent;

    return Status::Success;
}

Int32 SocketUtil::PostConnect(IoData *ioData, UInt16 family, const LibString &ip, UInt16 port)
{
    // 1.指定绑定地址
    Int32 errCode = Status::Success;

    ioData->_ioType = IoEventType::IO_CONNECT;
    const bool isIpv4 = family == AF_INET;
    const UInt64 addrInLen = isIpv4 ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
    
    KernelSockAddrInRaw addrInRaw;
    ::memset(&addrInRaw, 0, sizeof(addrInRaw));
    if(isIpv4)
        SocketUtil::FillTcpAddrInfo(ip.c_str(), port, family, addrInRaw._sinV4);
    else
        SocketUtil::FillTcpAddrInfo(ip.c_str(), port, family, addrInRaw._sinV6);

    BOOL ret = (*(_fnConnectEx))(ioData->_sock,
                                (struct sockaddr *)&addrInRaw,
                                static_cast<Int32>(addrInLen),
                                NULL,   // 连接并发送数据,不发
                                0,
                                NULL,
                                &ioData->_overlapped);
    if (ret == FALSE)
    {
        auto netLastError = ::WSAGetLastError();
        const auto &errStr = SockErrorMsgUtil::GetString(netLastError);

        if (netLastError == WSA_IO_PENDING)
        {// 异步完成
            g_Log->NetDebug(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "connect pending io will complete in future sock:%d, sessionId:%llu, family:%hu, targetIp:%s, targetPort:%hu.")
                            , ioData->_sock, ioData->_sessionId, family, ip.c_str(), port);
            return Status::SockError_Pending;
        }

        g_Log->NetError(LOGFMT_NON_OBJ_TAG(KERNEL_NS::SocketUtil, "PostConnect fatal error socket[%d], sessionId:%llu netLastError:%d, status[%d],"
                    " system error info:%s, family:%hu, targetIp:%s, targetPort:%hu")
                            , ioData->_sock, ioData->_sessionId, netLastError, Status::SockError_FatalError, errStr.c_str()
                            , family, ip.c_str(), port);

        return Status::SockError_FatalError;
    }

    return Status::Success;
}
#endif

Int32 SocketUtil::ShutdownRead(SOCKET sock)
{
    if(UNLIKELY(!IsValidSock(sock)))
        return Status::Socket_InvalidSocket;

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (::shutdown(sock, SHUT_RD) != 0)
        return Status::SockError_ShutdownFail;

    return Status::Success;
#else // CRYSTAL_TARGET_PLATFORM_WINDOWS
    if (::shutdown(sock, SD_RECEIVE) == SOCKET_ERROR)
        return Status::SockError_ShutdownFail;

    return Status::Success;
#endif // CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
}

Int32 SocketUtil::ShutdownWrite(SOCKET sock)
{
    if(UNLIKELY(!IsValidSock(sock)))
        return Status::Socket_InvalidSocket;

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (::shutdown(sock, SHUT_WR) != 0)
        return Status::SockError_ShutdownFail;

    return Status::Success;
#else // CRYSTAL_TARGET_PLATFORM_WINDOWS
    if (::shutdown(sock, SD_SEND) == SOCKET_ERROR)
        return Status::SockError_ShutdownFail;

    return Status::Success;
#endif // CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
}

Int32 SocketUtil::ShutdownReadWrite(SOCKET sock)
{
    if(UNLIKELY(!IsValidSock(sock)))
        return Status::Socket_InvalidSocket;

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (::shutdown(sock, SHUT_RDWR) != 0)
        return Status::SockError_ShutdownFail;
    
    return Status::Success;
#else // CRYSTAL_TARGET_PLATFORM_WINDOWS
    if (::shutdown(sock, SD_BOTH) == SOCKET_ERROR)
        return Status::SockError_ShutdownFail;

    return Status::Success;
#endif // CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
}

KERNEL_END