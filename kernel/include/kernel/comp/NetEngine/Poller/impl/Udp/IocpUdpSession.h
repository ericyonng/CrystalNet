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
 * Date: 2022-04-23 20:23:09
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_UDP_IOCP_UDP_SESSION_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_UDP_IOCP_UDP_SESSION_H__

#pragma once

#include <kernel/comp/NetEngine/Poller/impl/Session/LibSession.h>
#include <kernel/comp/LibDirtyHelper.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

KERNEL_BEGIN

struct IoData;
class LibPacket;
class IServiceProxy;
class IPollerMgr;

class KERNEL_EXPORT IocpUdpSession : public LibSession
{
    POOL_CREATE_OBJ_DEFAULT_P1(LibSession, IocpUdpSession);

public:
    IocpUdpSession(UInt64 sessionId, bool isLinker, bool isConnectToRemote);
    ~IocpUdpSession();

    virtual void Close() override;
    void SetDirtyHelper(LibDirtyHelper<void *, UInt32> *dirtyHelper);
    void SetServiceProxy(IServiceProxy *serviceProxy);
    void SetPollerMgr(IPollerMgr *pollerMgr);

    void SendPackets(LibList<LibPacket *> *packets);
    void OnSend(IoEvent &io);
    void OnRecv(IoEvent &io);
    void ContinueRecv();

    bool HasDataToRecv() const;
    bool HasDataToSend() const;

    virtual LibString ToString() const;

private:
    void _OnRecved();
    void _Destroy();

    // packets => LibStream
    LibStreamTL *_PacketsToBin();
    LibStreamTL *_PacketsToBin(LibStreamTL *&stream);
    void _PostSendStream(LibStreamTL *&stream);

    void _MaskClose(Int32 reason);

    LibDirtyHelper<void *, UInt32> *_dirtyHelper;
    IServiceProxy *_serviceProxy;
    IPollerMgr *_pollerMgr;

    bool _waitingForPostSendBack;
};

ALWAYS_INLINE void IocpTcpSession::SetDirtyHelper(LibDirtyHelper<void *, UInt32> *dirtyHelper)
{
    _dirtyHelper = dirtyHelper;
}

ALWAYS_INLINE void IocpTcpSession::SetServiceProxy(IServiceProxy *serviceProxy)
{
    _serviceProxy = serviceProxy;
}

ALWAYS_INLINE void IocpTcpSession::SetPollerMgr(IPollerMgr *pollerMgr)
{
    _pollerMgr = pollerMgr;
}

ALWAYS_INLINE bool IocpTcpSession::HasDataToRecv() const
{
    if(_recvBuffers && !_recvBuffers->IsEmpty())
        return true;

    return false;
}

ALWAYS_INLINE bool IocpTcpSession::HasDataToSend() const
{
    if(_lastSendLeft && !_lastSendLeft->IsReadFull())
        return true;

    if(_sendPacketList && !_sendPacketList->IsEmpty())
        return true;

    return false;
}

KERNEL_END


#endif

#endif
