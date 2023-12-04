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
 * Date: 2022-04-22 13:31:48
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/LibStream.h>
#include <kernel/comp/LibList.h>

#include <kernel/comp/Log/log.h>
#include <kernel/comp/NetEngine/LibSocket.h>
#include <kernel/comp/NetEngine/LibPacket.h>
#include <kernel/comp/NetEngine/Defs/ProtocolType.h>
#include <kernel/comp/Utils/ContainerUtil.h>

#include <kernel/comp/NetEngine/Poller/impl/Session/LibSession.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibSession);

LibSession::LibSession(UInt64 sessionId, bool isLinker, bool isConnectToRemote)
:_inited{false}
,_id(sessionId)
,_priorityLevel(0)
,_protocolType(0)
,_bufferCapacity(0)
,_serviceId(0)
,_pollerId(0)
,_handleRecvBytesPerFrameLimit(0)
,_handleSendBytesPerFrameLimit(0)
,_handleAcceptCountPerFrameLimit(0)
,_maxPacketSize(0)
,_protocolStack(NULL)
,_isLinker(isLinker)
,_isConnectToRemote(isConnectToRemote)
,_sock(NULL)
,_curFrameRecvHandleBytes(0)
,_curFrameSendHandleBytes(0)
,_curFrameAcceptCount(0)
,_sendPacketList(NULL)
,_lastSendLeft(NULL)
,_recvBuffers(NULL)
,_streamCtrlMask(0)
,_currentTokenNumber(0)
{

}

LibSession::~LibSession()
{
    _Destroy();
}

Int32 LibSession::Init()
{
    if(_inited.exchange(true))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("inited before session info:%s."), ToString().c_str());
        return Status::Repeat;
    }

    _sendPacketList = LibList<LibPacket *>::New_LibList();
    _lastSendLeft = NULL;
    _recvBuffers = LibList<LibStream<_Build::TL> *, _Build::TL>::NewThreadLocal_LibList();

    _recvLastCpuTime.Update();
    _currentTokenNumber = _option._sessionRecvPacketSpeedLimit;

    return Status::Success;
}

void LibSession::SetSocket(LibSocket *sock)
{
    sock->SetSession(this);
    _sock = sock;
}

LibString LibSession::ToString() const
{
    LibString info;
    info.AppendFormat("session info: sessionId:%llu, sock info:%s, service id:%llu, poller id:%llu, _protocolType:%d, %s"
            , _id, _sock ? _sock->ToString().c_str() : "no socket", _serviceId, _pollerId, _protocolType, ProtocolType::ToString(_protocolType))
        .AppendFormat(" session option:%s, \n", _option.ToString().c_str())
        .AppendFormat(" is linker:%s, \n", _isLinker ? "true" : "false")
        .AppendFormat(" _priorityLevel:%d, _bufferCapacity:%llu, \n", _priorityLevel, _bufferCapacity)
        .AppendFormat(" recvHandleBytes:%llu, recvBytesPerFrameLimit:%llu, \n", _curFrameRecvHandleBytes, _handleRecvBytesPerFrameLimit)
        .AppendFormat(" sendHandleBytes:%llu, sendBytesPerFrameLimit:%llu, \n", _curFrameSendHandleBytes, _handleSendBytesPerFrameLimit)
        .AppendFormat(" acceptCount:%llu, accpetPerFrameLimit:%llu, \n", _curFrameAcceptCount, _handleAcceptCountPerFrameLimit)
        .AppendFormat(" max packet size:%llu,\n", _maxPacketSize)
        .AppendFormat(" send packet list packcets count:%llu, last unhandled send stream bytes:%llu,", _sendPacketList ? _sendPacketList->GetAmount() : 0, _lastSendLeft ? _lastSendLeft->GetReadableSize() : 0)
        .AppendFormat(" unhandled recv buffer stream count:%llu", _recvBuffers ? _recvBuffers->GetAmount():0)
        .AppendFormat(" %s\n", SessionStreamCtrl::ToString(_streamCtrlMask).c_str())
        .AppendFormat(" close reason: %s", _closeReason.ToString().c_str())
        ;

    return info;
}

void LibSession::Close()
{
    _Destroy();
}

LibSession &LibSession::BeginTransaction()
{
    // TODO:开启事务
    // g_Log->NetInfo(LOGFMT_OBJ_TAG("[Session BeginTransaction] %s"), ToString().c_str());

    return *this;
}

LibSession &LibSession::CommitTransaction()
{
    // 清理,还有数据则标脏
    // g_Log->NetInfo(LOGFMT_OBJ_TAG("[Session CommitTransaction] %s"), ToString().c_str());
    return *this;
}

void LibSession::_Destroy()
{
    if(!_inited.exchange(false))
        return;

    g_Log->NetInfo(LOGFMT_OBJ_TAG("session destroy:%s"), ToString().c_str());

    if(LIKELY(_sock))
    {
        _sock->Close();
        LibSocket::DeleteThreadLocal_LibSocket(_sock);
        _sock = NULL;
    }

    _curFrameRecvHandleBytes = 0;
    _curFrameSendHandleBytes = 0;
    _curFrameAcceptCount = 0;

    if(LIKELY(_sendPacketList))
    {
        ContainerUtil::DelContainer(*_sendPacketList, [](LibPacket *&packet)
        {
            LibPacket::Delete_LibPacket(packet);
            packet = NULL;
        });

        LibList<LibPacket *>::Delete_LibList(_sendPacketList);
        _sendPacketList = NULL;
    }

    if(LIKELY(_lastSendLeft))
    {
        LibStream<_Build::TL>::DeleteThreadLocal_LibStream(_lastSendLeft);
        _lastSendLeft = NULL;
    }
    
    if(LIKELY(_recvBuffers))
    {
        ContainerUtil::DelContainer(*_recvBuffers, [this](LibStream<_Build::TL> *stream)
        {
            g_Log->NetInfo(LOGFMT_OBJ_TAG("unhandled recv buffer data len:%llu"), stream->GetReadableSize());
            LibStream<_Build::TL>::DeleteThreadLocal_LibStream(stream);
        });

        LibList<LibStream<_Build::TL> *, _Build::TL>::DeleteThreadLocal_LibList(_recvBuffers);
        _recvBuffers = NULL;
    }

    _streamCtrlMask = 0;
}

KERNEL_END
