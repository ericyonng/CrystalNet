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
 * Date: 2022-01-22 23:42:29
 * Author: Eric Yonng
 * Description: 对socket跨平台封装简化socket跨平台下的使用
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_LIB_SOCKET_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_LIB_SOCKET_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/Utils/Defs/Socket.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/BitUtil.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/NetEngine/LibAddr.h>

KERNEL_BEGIN

class LibSession;

class KERNEL_EXPORT LibSocketOptionFlag
{
public:
    enum Type
    {
        Unknown = 0,
        NonBlock = 1,           // 设置sock非阻塞
        NoDelay = 2,            // 禁用nagle算法保证低延迟
        ReuseAddr = 3,          // 地址重用(windows下是地址与端口重用,linux下是地址重用)
        ReusePort = 4,          // 端口重用(windows下等效于ReuseAddr, linux下是端口重用,可以实现连接的负载均衡)
        CloseOnExec = 5,        // SOCK_CLOEXEC是当服务器宕掉后还可以仍然使用端口避免设备掉线
    };

    static Int32 ToOptionFlag(Int32 level, Int32 optname)
    {
        switch (level)
        {
        case SOL_SOCKET: return _ToSockOptionFlag(optname);
            break;
        case IPPROTO_TCP: return _ToProtoTcpOptionFlag(optname);
        default:
            break;
        }

        return LibSocketOptionFlag::Unknown;
    }

private:
    static Int32 _ToSockOptionFlag(Int32 optname)
    {
        switch (optname)
        {
        case SO_REUSEADDR: return LibSocketOptionFlag::ReuseAddr;
            break;
    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
     #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
        case SO_REUSEPORT: return LibSocketOptionFlag::ReusePort;
            break;
     #endif
    #endif
        default:
            break;
        }

        return LibSocketOptionFlag::Unknown;
    }

    static Int32 _ToProtoTcpOptionFlag(Int32 optname)
    {
        switch (optname)
        {
        case TCP_NODELAY: return LibSocketOptionFlag::NoDelay;
            break;
        default:
            break;
        }

        return LibSocketOptionFlag::Unknown;
    }
};

class KERNEL_EXPORT LibSocketOptionPresetParam
{
public:
    enum Param : UInt64
    {
        // 非阻塞,低延迟,端口重用,设备不掉线
        NonBlockingNoDelayReuseAddrCloseOnExec = UInt64(1) << LibSocketOptionFlag::NonBlock | 
                                                 UInt64(1) << LibSocketOptionFlag::NoDelay | 
                                                 UInt64(1) << LibSocketOptionFlag::ReuseAddr | 
                                                 UInt64(1) << LibSocketOptionFlag::CloseOnExec, 
    };
};

class KERNEL_EXPORT LibSocket
{
    POOL_CREATE_OBJ_DEFAULT(LibSocket);
    NO_COPY(LibSocket);

public:
    explicit LibSocket();
    ~LibSocket();

public:
    const LibAddr *GetAddr() const;
    LibAddr *GetAddr();

    template<typename BuildType>
    Int32 Create(Int32 af = AF_INET, Int32 type = SOCK_STREAM, Int32 protocol = IPPROTO_TCP);
    template<typename BuildType>
    Int32 TakeOver(SOCKET sock, Int32 af = AF_INET, Int32 type = SOCK_STREAM, Int32 protocol = IPPROTO_TCP, bool isLinker = false, bool isWinSockNonBlock = false);
    void SetSession(LibSession *session);
    /**
     * Set socket option.
     * @param[in] level   - socket level, eg: SOL_TCP, IPPROTO_TCP, IPPROTO_IP, ....
     * @param[in] optname - the socket option name, eg: SO_KEEPALIVE.
     * @param[in] optval  - the option value buffer.
     * @param[in] optlen  - the option value length.
     * @return int - return 0 if success, otherwise return -1.
     */
    Int32 SetOption(Int32 level, Int32 optname, const void *optval, Int32 optlen);
    /**
     * Get socket option.
     * @param[in] level      - socket level, eg: SOL_TCP, SOL_SOCKET.
     * @param[in] optname    - the socket option name, eg: SO_KEEPALIVE.
     * @param[in] optval     - the buffer, option value will store in this buffer.
     * @param[in/out] optlen - pointer to the size of the optval buffer.
     * @return int - return 0 if success, otherwise return -1.
     */
    Int32 GetOption(Int32 level, Int32 optname, void *optval, LibSockLen *optlen) const;

    // 禁用nagle算法保证低延迟
    Int32 SetNoDelay(bool isNoDelay);
    Int32 SetNonBlocking();
    Int32 SetBlocking();
    Int32 EnableReuseAddr();
    Int32 DisableReuseAddr();
    Int32 EnableReusePort();
    Int32 DisableReusePort();
    Int32 EnableCloseOnExec();
    Int32 DisableCloseOnExec();
    
    /**
     * Get send buffer size.
     * @return size_t - the send buffer size.
     */
    UInt64 GetSendBufSize() const;

    /**
     * Set send buffer size.
     * @param[in] size - buffer size, in bytes.
     * @return int - return 0 if success, otherwise return -1.
     */
    Int32 SetSendBufSize(UInt64 size);

    /**
     * Get recv buffer size.
     * @return size_t - the recv buffer size.
     */
    UInt64 GetRecvBufSize() const;

    /**
     * Set recv buffer size.
     * @param[in] size - buffer size, in bytes.
     * @return int - return 0 if succerss, otherwise return -1.
     */
    Int32 SetRecvBufSize(UInt64 size);

    bool IsReuseAddr() const;
    // windows下等效于IsReuseAddr(windows下reuse addr会重用端口)， linux下额外增加重用端口,但只针对程序所在用户开放
    bool IsReusePort() const;
    bool IsNoDelay() const;
    bool IsNonBlocking() const;
    bool IsCloseOnExec() const;
    bool IsClosed() const;
    bool IsLinker() const;
    operator bool() const;
    bool operator !() const;
    Int32 GetFamily() const;
    SOCKET GetSock() const;
    SOCKET &GetSock();
    bool IsIpv4() const;

    Int32 UpdateLocalAddr();
    Int32 UpdateRemoteAddr();

    // 绑定地址
    Int32 BindTo(const Byte8 *ip, UInt16 port);
    // 监听
    Int32 Listen(Int32 unconnectQueueLen = SOMAXCONN);
    
    #if CRYSTAL_TARGET_PLATFORM_LINUX
    // 接受连入 accept之后需要添加到其他线程
    SOCKET Accept();
    #endif

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    Int32 PostAsyncAccept();
    Int32 PostZeroWSARecv();
    #endif

    Int32 ShutdownRead();
    Int32 ShutdownWrite();
    Int32 ShutdownReadWrite();
    Int32 Close();

    LibString ToString() const;

private:
    Int32 _TakeOver(SOCKET sock, Int32 af = AF_INET, Int32 type = SOCK_STREAM, Int32 protocol = IPPROTO_TCP, bool isLinker = false, bool isWinSockNonBlock = false);
    Int32 _Create(Int32 af = AF_INET, Int32 type = SOCK_STREAM, Int32 protocol = IPPROTO_TCP);
    void _OnSetOption(Int32 level, Int32 optname, const void *optval);
    void _OnSetOption(Int32 flagPos, bool isSet);
    bool _IsSetOption(Int32 optionFlag) const;
    void _Destroy();

private:
    Int32 _af;          // family AF_INET/AF_INET6
    Int32 _type;        // SOCK_STREAM(tcp)/SOCK_DGRAM(udp)
    Int32 _protocol;    // IPPROTO_TCP/IPPROTO_UDP
    SOCKET _sock;

    UInt64 _option;     // socket特性
    bool _isLinker;     // 是否属于连接器

    LibAddr  *_addr;
    IDelegate<void, LibAddr *> *_releaseAddr;
    LibSession *_session;
};

ALWAYS_INLINE const LibAddr *LibSocket::GetAddr() const
{
    return _addr;
}

ALWAYS_INLINE LibAddr *LibSocket::GetAddr()
{
    return _addr;
}

template<typename BuildType>
ALWAYS_INLINE Int32 LibSocket::Create(Int32 af, Int32 type, Int32 protocol)
{
    Int32 err = _Create(af, type, protocol);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("create sock fail err:%d"), err);
        return err;
    }

    _addr = LibAddr::NewByAdapter_LibAddr(BuildType::V, this);

    // 创建删除器
    auto __releaseAddrFunc = [](LibAddr *addr)->void {
        LibAddr::DeleteByAdapter_LibAddr(BuildType::V, addr);
    };
    _releaseAddr = KERNEL_CREATE_CLOSURE_DELEGATE(__releaseAddrFunc, void, LibAddr *);

    return Status::Success;
}

template<typename BuildType>
ALWAYS_INLINE Int32 LibSocket::TakeOver(SOCKET sock, Int32 af, Int32 type, Int32 protocol, bool isNonBlock, bool isLinker)
{
    Int32 err = _TakeOver(sock, af, type, protocol, isLinker, isNonBlock);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("_takerover fail err:%d"), err);
        return err;
    }

    _addr = LibAddr::NewByAdapter_LibAddr(BuildType::V, this);
    // 创建删除器
    auto __releaseAddrFunc = [](LibAddr *addr)->void {
        LibAddr::DeleteByAdapter_LibAddr(BuildType::V, addr);
        addr = NULL;
    };
    _releaseAddr = KERNEL_CREATE_CLOSURE_DELEGATE(__releaseAddrFunc, void, LibAddr *);

    return Status::Success;
}

ALWAYS_INLINE void LibSocket::SetSession(LibSession *session)
{
    _session = session;
}

ALWAYS_INLINE bool LibSocket::IsReuseAddr() const
{
    return _IsSetOption(LibSocketOptionFlag::ReuseAddr);
}

ALWAYS_INLINE bool LibSocket::IsReusePort() const
{
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        return IsReuseAddr();
    #endif

    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        return _IsSetOption(LibSocketOptionFlag::ReusePort);
    #endif
}

ALWAYS_INLINE bool LibSocket::IsNoDelay() const
{
    return _IsSetOption(LibSocketOptionFlag::NoDelay);
}

ALWAYS_INLINE bool LibSocket::IsNonBlocking() const
{
    return _IsSetOption(LibSocketOptionFlag::NonBlock);
}

ALWAYS_INLINE bool LibSocket::IsCloseOnExec() const
{
    return _IsSetOption(LibSocketOptionFlag::CloseOnExec);
}

ALWAYS_INLINE bool LibSocket::IsClosed() const
{
    return _sock == INVALID_SOCKET;
}

ALWAYS_INLINE bool LibSocket::IsLinker() const
{
    return _isLinker;
}

ALWAYS_INLINE LibSocket::operator bool() const
{
    return !IsClosed();
}

ALWAYS_INLINE bool LibSocket::operator !() const
{
    return IsClosed();
}

ALWAYS_INLINE Int32 LibSocket::GetFamily() const
{
    return _af;
}

ALWAYS_INLINE SOCKET LibSocket::GetSock() const
{
    return _sock;
}

ALWAYS_INLINE SOCKET &LibSocket::GetSock()
{
    return _sock;
}

ALWAYS_INLINE bool LibSocket::IsIpv4() const
{
    return GetFamily() == AF_INET;
}

ALWAYS_INLINE void LibSocket::_OnSetOption(Int32 level, Int32 optname, const void *optval)
{
    const Int32 flag = *KernelCastTo<Int32>(optval);
    const Int32 flagPos = LibSocketOptionFlag::ToOptionFlag(level, optname);
    if(UNLIKELY(flagPos == LibSocketOptionFlag::Unknown))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("optname is unknown level:%d, optname:%d socket info::%s"), level, optname, ToString().c_str());
        return;
    }

    _OnSetOption(flagPos, flag != 0);

    // windows下reuseaddr 和reuseport一样的效果
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        if(flagPos == LibSocketOptionFlag::ReuseAddr)
            _OnSetOption(LibSocketOptionFlag::ReusePort, flag != 0);
    #endif
}

ALWAYS_INLINE void LibSocket::_OnSetOption(Int32 flagPos, bool isSet)
{
    if(isSet)
    {
        _option = BitUtil::Set(_option, flagPos);
        return;
    }
    
    _option = BitUtil::Clear(_option, flagPos);
}

ALWAYS_INLINE bool LibSocket::_IsSetOption(Int32 optionFlag) const
{
    return BitUtil::IsSet(_option, optionFlag);
}

ALWAYS_INLINE void LibSocket::_Destroy()
{
    Close();
}

KERNEL_END

#endif
