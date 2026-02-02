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
 * Date: 2022-01-22 23:48:56
 * Author: Eric Yonng
 * Description: 
*/


#include <pch.h>

#include <kernel/comp/Utils/SocketUtil.h>
#include <kernel/comp/NetEngine/LibAddr.h>
#include <kernel/comp/NetEngine/Defs/IoData.h>
#include <kernel/comp/NetEngine/Poller/impl/Session/LibSession.h>
#include <kernel/comp/Utils/SockErrorMsgUtil.h>
#include <kernel/comp/NetEngine/Defs/IoEvent.h>
#include <kernel/comp/NetEngine/Defs/ProtocolType.h>

#include <kernel/comp/NetEngine/LibSocket.h>

#if CRYSTAL_TARGET_PLATFROM_LINUX
    #include <sys/time.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <sys/eventfd.h>    // 多线程多进程事件通知机制
#endif

#if CRYSTAL_TARGET_PLATFROM_WINDOWS

#endif

KERNEL_BEGIN

LibSocket::LibSocket()
:_af(0)
,_type(0)
,_protocol(0)
,_sock(INVALID_SOCKET)
,_option(0)
,_isLinker(false)
,_addr(NULL)
,_releaseAddr(NULL)
,_session(NULL)
{

}

LibSocket::~LibSocket()
{
    _Destroy();
}

Int32 LibSocket::SetOption(Int32 level, Int32 optname, const void *optval, Int32 optlen)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    if (::setsockopt(_sock, level, optname, 
        reinterpret_cast<const char *>(optval), optlen) != 0)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("set sock opt fail sock:%d, level:%d, optname:%d, optval:%p, optlen:%d")
                    , _sock, level, optname, optval, optlen);
        return Status::Failed;
    }
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if (::setsockopt(_sock, level, optname, 
        reinterpret_cast<const char *>(optval), optlen) == SOCKET_ERROR)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("set sock opt fail sock:%d, level:%d, optname:%d, optval:%p, optlen:%d")
            , _sock, level, optname, optval, optlen);
        return Status::Failed;
    }
#endif

    // 设置标志
    _OnSetOption(level, optname, optval);
    return Status::Success;
}

Int32 LibSocket::GetOption(Int32 level, Int32 optname, void *optval, LibSockLen *optlen) const
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (::getsockopt(_sock, level, optname, 
        reinterpret_cast<char *>(optval), optlen) != 0)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("getsockopt fail sock:%d, level:%d, optname:%d"), _sock, level, optname);
        return Status::Failed;
    }

    return Status::Success;
#else 
    if (::getsockopt(_sock, level, optname, 
        reinterpret_cast<char *>(optval), optlen) == SOCKET_ERROR)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("getsockopt fail sock:%d, level:%d, optname:%d"), _sock, level, optname);
        return Status::Failed;
    }

    return Status::Success;
#endif 
}

Int32 LibSocket::SetNoDelay(bool isNoDelay)
{
    int flag = isNoDelay ? 1 : 0;
    Int32 len = sizeof(flag);
    if(UNLIKELY(_IsSetOption(LibSocketOptionFlag::NoDelay)))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("is set NoDelay before socket info:%s"), ToString().c_str());
        return Status::Success;
    }

    return SetOption(IPPROTO_TCP,
                     TCP_NODELAY,
                     reinterpret_cast<void *>(&flag),
                     len);
}

Int32 LibSocket::SetNonBlocking()
{
    if(UNLIKELY(_IsSetOption(LibSocketOptionFlag::NonBlock)))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("is set NonBlock before socket info:%s"), ToString().c_str());
        return Status::Success;
    }

    Int32 err = SocketUtil::SetNoBlock(_sock);
    if(err != Status::Success)
        return err;
    
    _OnSetOption(LibSocketOptionFlag::NonBlock, true);

    return Status::Success;
}

Int32 LibSocket::SetBlocking()
{
    if(UNLIKELY(!_IsSetOption(LibSocketOptionFlag::NonBlock)))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("not set NonBlock before socket info:%s"), ToString().c_str());
        return Status::Success;
    }

    Int32 err = SocketUtil::SetBlock(_sock);
    if(err != Status::Success)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("set block fail err:%d, socket info:%s"), err, ToString().c_str());
        return err;
    }

    _OnSetOption(LibSocketOptionFlag::NonBlock, false);

    return Status::Success;
}

Int32 LibSocket::EnableReuseAddr()
{
    if(UNLIKELY(_IsSetOption(LibSocketOptionFlag::ReuseAddr)))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("enable reuse addr before socket info:%s"), ToString().c_str());
        return Status::Success;
    }

    Int32 flag = 1;
    Int32 err = SetOption(SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    if(err != Status::Success)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("enable reuse addr fail err:%d, socket info:%s"), err, ToString().c_str());
        return err;
    }

    return Status::Success;
}

Int32 LibSocket::DisableReuseAddr()
{
    if(UNLIKELY(!_IsSetOption(LibSocketOptionFlag::ReuseAddr)))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("disable reuse addr before socket info:%s"), ToString().c_str());
        return Status::Success;
    }

    Int32 flag = 0;
    Int32 err = SetOption(SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    if(err != Status::Success)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("enable reuse addr fail err:%d, socket info:%s"), err, ToString().c_str());
        return err;
    }

    return Status::Success;
}

Int32 LibSocket::EnableReusePort()
{
    if(UNLIKELY(_IsSetOption(LibSocketOptionFlag::ReusePort)))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("enable reuse port before socket info:%s"), ToString().c_str());
        return Status::Success;
    }

    // windows 下与reuseaddr一样
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        return EnableReuseAddr();
    #endif

    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        // linux 下内核版本3.9.0以上才有
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
            Int32 flag = 1;
            Int32 err = SetOption(SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));
            if(err != Status::Success)
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("enable reuse port fail err:%d, socket info:%s"), err, ToString().c_str());
                return err;
            }

            return Status::Success;
        #else
            g_Log->NetWarn(LOGFMT_OBJ_TAG("enable reuse port fail SO_REUSEPORT:kernel version must upper than 3.9.0 , socket info:%s"), ToString().c_str());
            return Status::Failed; 
        #endif
    #endif
}

Int32 LibSocket::DisableReusePort()
{
    if(UNLIKELY(!_IsSetOption(LibSocketOptionFlag::ReusePort)))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("reuse port not enable before socket info:%s"), ToString().c_str());
        return Status::Success;
    }

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        return DisableReuseAddr();
    #endif

    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
       // linux 下内核版本3.9.0以上才有
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
            Int32 flag = 0;
            Int32 err = SetOption(SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));
            if(err != Status::Success)
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("disable reuse port fail err:%d, socket info:%s"), err, ToString().c_str());
                return err;
            }

            return Status::Success;
        #else
            g_Log->NetWarn(LOGFMT_OBJ_TAG("SO_REUSEPORT kernel version must upper than 3.9.0 , socket info:%s"), ToString().c_str());
            return Status::Success; 
        #endif
    #endif
}

Int32 LibSocket::EnableCloseOnExec()
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    // close-on-exec
    Int32 flags = ::fcntl(_sock, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    Int32 ret = ::fcntl(_sock, F_SETFD, flags);
    if (ret == -1)
    {
        Int32 err = errno;
        g_Log->NetWarn(LOGFMT_OBJ_TAG("fcntl F_SETFD fail flags[%d] err[%d]:[%s]."), flags, err, SystemUtil::GetErrString(err).c_str());
        return Status::Socket_FcntlError;
    }

    _OnSetOption(LibSocketOptionFlag::CloseOnExec, true);
    return Status::Success;
#else
    g_Log->NetWarn(LOGFMT_OBJ_TAG("enable close on exec is enable in linux platform"));
    return Status::Success;
#endif
}

Int32 LibSocket::DisableCloseOnExec()
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    // close-on-exec
    Int32 flags = ::fcntl(_sock, F_GETFD, 0);
    flags &= (~FD_CLOEXEC);
    Int32 ret = ::fcntl(_sock, F_SETFD, flags);
    if (ret == -1)
    {
        Int32 err = errno;
        g_Log->NetWarn(LOGFMT_OBJ_TAG("fcntl F_SETFD fail flags[%d] err[%d]:[%s]."), flags, err, SystemUtil::GetErrString(err).c_str());
        return Status::Socket_FcntlError;
    }

    _OnSetOption(LibSocketOptionFlag::CloseOnExec, false);
    return Status::Success;
#else
    g_Log->NetWarn(LOGFMT_OBJ_TAG("disable close on exec is enable in linux platform"));
    return Status::Success;
#endif
}

UInt64 LibSocket::GetSendBufSize() const
{
    Int32 sndBufSize = 0;
    LibSockLen len = sizeof(Int32);
    if (GetOption(SOL_SOCKET,
                  SO_SNDBUF,
                  reinterpret_cast<void *>(&sndBufSize),
                  &len) != Status::Success)
        return 0;

    return static_cast<UInt64>(sndBufSize);
}
    
Int32 LibSocket::SetSendBufSize(UInt64 size)
{
    if (UNLIKELY(size == 0))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("size is zero socket info:%s"), ToString().c_str());
        return Status::Failed;
    }

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (::setsockopt(_sock, SOL_SOCKET, SO_SNDBUF, 
        reinterpret_cast<const char *>(&size), sizeof(Int32)) != 0)
    {
        Int32 err = errno;
        g_Log->NetWarn(LOGFMT_OBJ_TAG("setsockopt SO_SNDBUF fail size:%llu errno:%d,%s socket info:%s")
                    ,size, err, SystemUtil::GetErrString(err).c_str(), ToString().c_str());
        return Status::Failed;
    }

    return Status::Success;
#else // CRYSTAL_TARGET_PLATFORM_WINDOWS
    if (::setsockopt(_sock, SOL_SOCKET, SO_SNDBUF, 
        reinterpret_cast<const char *>(&size), sizeof(Int32)) == SOCKET_ERROR)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("setsockopt SO_SNDBUF fail size:%llu socket info:%s")
                    ,size, ToString().c_str());
        return Status::Failed;
    }

    return Status::Success;
#endif // CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
}

UInt64 LibSocket::GetRecvBufSize() const
{
    Int32 rcvBufSize = 0;
    LibSockLen len = sizeof(Int32);
    if (GetOption(SOL_SOCKET,
                  SO_RCVBUF,
                  reinterpret_cast<void *>(&rcvBufSize),
                  &len) != Status::Success)
        return 0;

    return static_cast<size_t>(rcvBufSize);
}

Int32 LibSocket::SetRecvBufSize(UInt64 size)
{
    if (UNLIKELY(size == 0))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("SetRecvBufSize size is zero socket info:%s"), ToString().c_str());
        return Status::Failed;
    }

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (::setsockopt(_sock, SOL_SOCKET, 
        SO_RCVBUF, reinterpret_cast<char *>(&size), sizeof(Int32)) != 0)
    {
        Int32 err = errno;
        g_Log->NetWarn(LOGFMT_OBJ_TAG("setsockopt SO_RCVBUF fail size:%llu errno:%d,%s socket info:%s")
                    ,size, err, SystemUtil::GetErrString(err).c_str(), ToString().c_str());
        return Status::Failed;
    }

    return Status::Success;
#else // CRYSTAL_TARGET_PLATFORM_WINDOWS
    if (::setsockopt(_sock, SOL_SOCKET, 
        SO_RCVBUF, reinterpret_cast<char *>(&size), sizeof(Int32)) == SOCKET_ERROR)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("setsockopt SO_RCVBUF fail size:%llu socket info:%s")
                    ,size, ToString().c_str());
        return Status::Failed;
    }

    return Status::Success;
#endif //CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
}

Int32 LibSocket::UpdateLocalAddr()
{
    return _addr->UpdateLocalAddr();
}

Int32 LibSocket::UpdateRemoteAddr()
{
    return _addr->UpdateRemoteAddr();
}

Int32 LibSocket::BindTo(const Byte8 *ip, UInt16 port)
{
    Int32 err = SocketUtil::Bind(_sock, LibString(ip), port, _af);
    if(err != Status::Success)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("bind to ip:%s, port:%hu, family:%d, sock:%d fail err:%d")
                    , ip, port, _af, _sock, err);
        return err;
    }

    _addr->SetPrepareLocalIp(ip);
    _addr->SetPrepareLocalPort(port);
    _addr->SetPrepareLocalIpAndPort(ip, port);
    return Status::Success;
}

Int32 LibSocket::_TakeOver(SOCKET sock, Int32 af, Int32 type, Int32 protocol, bool isLinker, bool isWinSockNonBlock)
{
    if(UNLIKELY(sock == INVALID_SOCKET))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("invalid sock"));
        return Status::Failed;
    }

    _Destroy();

    _sock = sock;
    _af = af;
    _type = type;
    _protocol = protocol;
    _option = 0;
    _isLinker = isLinker;

    // socket 的option
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        if(isWinSockNonBlock)
            _OnSetOption(LibSocketOptionFlag::NonBlock, true);
    #else
        if(SocketUtil::IsNonBlocking(_sock))
            _OnSetOption(LibSocketOptionFlag::NonBlock, true);
        if(SocketUtil::IsCloseOnExec(_sock))
            _OnSetOption(LibSocketOptionFlag::CloseOnExec, true);
    #endif
    if(SocketUtil::IsReuseAddr(_sock))
        _OnSetOption(LibSocketOptionFlag::ReuseAddr, true);
    if(SocketUtil::IsReusePort(_sock))
        _OnSetOption(LibSocketOptionFlag::ReusePort, true);
    if(SocketUtil::IsNoDelay(_sock))
        _OnSetOption(LibSocketOptionFlag::NoDelay, true);

    return Status::Success;
}


Int32 LibSocket::_Create(Int32 af, Int32 type, Int32 protocol)
{
    if(UNLIKELY(_sock != INVALID_SOCKET))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("sock already create before sock info:%s"), ToString().c_str());
        return Status::Repeat;
    }

#if CRYSTAL_TARGET_PLATFORM_LINUX
    _sock = ::socket(af, type, protocol);
#else
    _sock = ::WSASocketA(af, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
#endif

    if(UNLIKELY(_sock == INVALID_SOCKET))
    {
        Int32 err;
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            err = ::WSAGetLastError();
        #endif

        #if CRYSTAL_TARGET_PLATFORM_LINUX
            err = errno;
            const auto &errStr = SystemUtil::GetErrString(err);
            g_Log->NetError(LOGFMT_OBJ_TAG("create sock fail af:%d, type:%d, protocol:%d, err:%d, %s")
                                , af, type, protocol, err, errStr.c_str());
        #endif

        return Status::Socket_Error;
    }

    _af = af;
    _type = type;
    _protocol = protocol;
    _option = 0;
    _isLinker = false;
    return Status::Success;
}

Int32 LibSocket::Listen(Int32 unconnectQueueLen)
{
    Int32 err = SocketUtil::Listen(_sock, unconnectQueueLen);
    if(err != Status::Success)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("listen fail sock:%d, err:%d, %s"), _sock, err, SystemUtil::GetErrString(err).c_str());
        return err;
    }

    _isLinker = true;
    return Status::Success;
}

#if CRYSTAL_TARGET_PLATFORM_LINUX
SOCKET LibSocket::Accept()
{
    KernelSockAddrIn addrIn(IsIpv4());

    LibSockLen len = addrIn._isIpv4 ? sizeof(addrIn._data._sinV4) : sizeof(addrIn._data._sinV6);
    SOCKET newSock = INVALID_SOCKET;
    if ((newSock = ::accept(
        _sock, reinterpret_cast<struct sockaddr *>(&addrIn._data), &len)) == INVALID_SOCKET)
    {
        Int32 err = errno;
        g_Log->NetWarn(LOGFMT_OBJ_TAG("accept fail _sock:%d, err:%d, %s "), _sock, err, SystemUtil::GetErrString(err).c_str());
    }
    
    return newSock;
}
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

Int32 LibSocket::PostAsyncAccept()
{
    auto newIo = IoData::NewThreadLocal_IoData();
    newIo->_ioType = IoEventType::IO_ACCEPT;
    newIo->_listenSock = _sock;
    auto err = SocketUtil::CreateSock2(newIo->_sock, _af, _type, _protocol);
    if(UNLIKELY(err != Status::Success))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("create sock fail when post accept by my listen socket:%s"), ToString().c_str());
        IoData::DeleteThreadLocal_IoData(newIo);
        return err;
    }

    err = SocketUtil::PostAccept(_af, newIo);
    if(err != Status::Success && err != Status::SockError_Pending)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("post a new accept fail, new sock:%d, my listen socket:%s"), newIo->_sock, ToString().c_str());
        SocketUtil::DestroySocket(newIo->_sock);
        IoData::DeleteThreadLocal_IoData(newIo);
        return err;
    }

    g_Log->NetDebug(LOGFMT_OBJ_TAG("post async accept suc new sock:%d, my listen sock:%s"), newIo->_sock, ToString().c_str());
    return Status::Success;
}

Int32 LibSocket::PostZeroWSARecv()
{
    auto io = IoData::NewThreadLocal_IoData();
    io->_ioType = IoEventType::IO_RECV;
    io->_sock = _sock;
    io->_sessionId = _session->GetId();
    io->_wsaBuff.buf = NULL;
    io->_wsaBuff.len = 0;
    Int32 err = SocketUtil::PostRecv(io);
    if(err != Status::Success && err != Status::SockError_Pending)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("post recv fail err:%d, sock:%s"), err, ToString().c_str());
        IoData::DeleteThreadLocal_IoData(io);
        return err;
    }

    // g_Log->NetDebug(LOGFMT_OBJ_TAG("post zero recv suc my sock:%s"), ToString().c_str());
    return Status::Success;
}

#endif

Int32 LibSocket::ShutdownRead()
{
    return SocketUtil::ShutdownRead(_sock);
}

Int32 LibSocket::ShutdownWrite()
{
    return SocketUtil::ShutdownWrite(_sock);
}

Int32 LibSocket::ShutdownReadWrite()
{
    return SocketUtil::ShutdownReadWrite(_sock);
}

Int32 LibSocket::Close()
{
    if(SocketUtil::IsValidSock(_sock))
        SocketUtil::DestroySocket(_sock);

    if(LIKELY(_addr))
        _releaseAddr->Invoke(_addr);
    CRYSTAL_RELEASE_SAFE(_releaseAddr);

    _af = 0;
    _type = 0;
    _protocol = 0;
    _option = 0;
    _isLinker = false;
    _addr = NULL;
    _session = NULL;

    return Status::Success;
}

LibString LibSocket::ToString() const
{
    LibString info;
    info.AppendFormat("socket info: sessionId:%llu, family:%d, type:%d,%s, protocol:%d, sock:%d, option:0x%llx, isLinker:%d, addr:%s"
                , _session ? _session->GetId() : 0, _af, _type, ProtocolType::ToProtocolType(_type), _protocol, _sock, _option, _isLinker, _addr ? _addr->ToString().c_str() : "no addr");

    return info;
}

KERNEL_END