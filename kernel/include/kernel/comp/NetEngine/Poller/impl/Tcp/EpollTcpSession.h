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
 * Date: 2022-04-23 20:22:53
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_TCP_EPOLL_TCP_SESSION_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_TCP_EPOLL_TCP_SESSION_H__

#if CRYSTAL_TARGET_PLATFORM_LINUX

#pragma once

#include <kernel/comp/NetEngine/Poller/impl/Session/LibSession.h>
#include <kernel/comp/LibDirtyHelper.h>

KERNEL_BEGIN

class LibPacket;
class EpollTcpPoller;
class IServiceProxy;
class IPollerMgr;

class KERNEL_EXPORT EpollTcpSession : public LibSession
{
    POOL_CREATE_OBJ_DEFAULT_P1(LibSession, EpollTcpSession);

public:
    EpollTcpSession(UInt64 sessionId, bool isLinker, bool isConnectToRemote);
    ~EpollTcpSession();

    virtual void Close() override;

    void SetDirtyHelper(LibDirtyHelper<void *, UInt32> *dirtyHelper);
    void SetServiceProxy(IServiceProxy *serviceProxy);
    void SetPollerMgr(IPollerMgr *pollerMgr);

    void SendPackets(LibList<LibPacket *> *packets);
    void ContinueSend();
    void OnRecv();

    bool HasDataToRecv() const;
    bool HasDataToSend() const;

private:
    // return:continue to send packets
    bool _SendStream(LibStream<_Build::TL> *&stream, UInt64 &handledBytes);
    Int32 _DoSend(const Byte8 *buffer, Int64 bufferSize, UInt64 &handledBytes);
    void _SendPackets(UInt64 &handledBytes);

    void _OnRecved();

    void _Destroy();

    LibDirtyHelper<void *, UInt32> *_dirtyHelper;
    IServiceProxy *_serviceProxy;
    IPollerMgr *_pollerMgr;
};

ALWAYS_INLINE void EpollTcpSession::SetDirtyHelper(LibDirtyHelper<void *, UInt32> *dirtyHelper)
{
    _dirtyHelper = dirtyHelper;
}

ALWAYS_INLINE void EpollTcpSession::SetServiceProxy(IServiceProxy *serviceProxy)
{
    _serviceProxy = serviceProxy;
}

ALWAYS_INLINE void EpollTcpSession::SetPollerMgr(IPollerMgr *pollerMgr)
{
    _pollerMgr = pollerMgr;
}

ALWAYS_INLINE bool EpollTcpSession::HasDataToRecv() const
{
    if(_recvBuffers && !_recvBuffers->IsEmpty())
        return true;

    return false;
}

ALWAYS_INLINE bool EpollTcpSession::HasDataToSend() const
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
