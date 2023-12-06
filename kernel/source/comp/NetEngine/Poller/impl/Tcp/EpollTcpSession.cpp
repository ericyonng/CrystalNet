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
 * Date: 2022-04-23 20:28:56
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerDirty.h>
#include <kernel/comp/NetEngine/Poller/Defs/CloseSessionInfo.h>
// #include <kernel/comp/NetEngine/Poller/impl/Tcp/EpollTcpPoller.h>
#include <kernel/comp/NetEngine/LibSocket.h>
#include <kernel/comp/NetEngine/Protocol/Protocol.h>
#include <kernel/comp/NetEngine/LibPacket.h>
#include <kernel/comp/NetEngine/Poller/interface/IPollerMgr.h>
#include <kernel/comp/Service/Service.h>
#include <kernel/comp/LibDirtyHelper.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerInnerEvent.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerEvent.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/EpollTcpSession.h>
#include <kernel/comp/Utils/BackTraceUtil.h>

#if CRYSTAL_TARGET_PLATFORM_LINUX

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(EpollTcpSession);

EpollTcpSession::EpollTcpSession(UInt64 sessionId, bool isLinker, bool isConnectToRemote)
:LibSession(sessionId, isLinker, isConnectToRemote)
,_dirtyHelper(NULL)
,_serviceProxy(NULL)
,_pollerMgr(NULL)
{

}

EpollTcpSession::~EpollTcpSession()
{
    _Destroy();
}

void EpollTcpSession::Close()
{
    _Destroy();
    LibSession::Close();
}

void EpollTcpSession::SendPackets(LibList<LibPacket *> *packets)
{
    // g_Log->Info(LOGFMT_OBJ_TAG("session:%llu, send packets :%s"), GetId(), packets->ToString([](LibPacket *packet){
    //     return packet->ToString();
    // }).c_str());

    _dirtyHelper->Clear(this, PollerDirty::WRITE);
    if(UNLIKELY(!CanSend()))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("%s cant send now!"), ToString().c_str());
        ContainerUtil::DelContainer(*packets, [](LibPacket *&packet){
            LibPacket::Delete_LibPacket(packet);
            packet = NULL;
        });

        LibList<LibPacket *>::Delete_LibList(packets);
        return;
    }

    _sendPacketList->MergeTail(packets);
    LibList<LibPacket *>::Delete_LibList(packets);

    UInt64 handledBytes = 0;

    if(UNLIKELY(IsSendEagain()))
    {
        // g_Log->NetTrace(LOGFMT_OBJ_TAG("%s send eagain, waiting for epoll turn send from eagain send packets total count:%llu."), ToString().c_str(),  _sendPacketList->GetAmount());
        return;
    }

    if(!_lastSendLeft || _SendStream(_lastSendLeft, handledBytes))
        _SendPackets(handledBytes);
    
    // 跟踪日志
    // g_Log->Info(LOGFMT_OBJ_TAG("%s send total bytes = [%llu], _lastSendLeft left bytes unhandled = [%llu], rest packets amount = [%llu], trace:%s")
    //                 , ToString().c_str(), handledBytes, _lastSendLeft ? _lastSendLeft->GetReadableSize() : 0, _sendPacketList->GetAmount(), KERNEL_NS::BackTraceUtil::CrystalCaptureStackBackTrace().c_str());
}

void EpollTcpSession::ContinueSend()
{
    _dirtyHelper->Clear(this, PollerDirty::WRITE);
    if(UNLIKELY(!CanSend()))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("%s cant send now!"), ToString().c_str());
        return;
    }

    // g_Log->Info(LOGFMT_OBJ_TAG("ContinueSend session:%llu, _sendPacketList :%s"), GetId(), _sendPacketList ? _sendPacketList->ToString([](LibPacket *packet){
    //     return packet->ToString();
    // }).c_str():"NONE");

    UInt64 handledBytes = 0;

    if(UNLIKELY(IsSendEagain()))
    {
        // g_Log->NetTrace(LOGFMT_OBJ_TAG("%s send eagain, waiting for epoll turn send from eagain send packets total count:%llu."), ToString().c_str(), _sendPacketList->GetAmount());
        return;
    }

    if(!_lastSendLeft || _SendStream(_lastSendLeft, handledBytes))
        _SendPackets(handledBytes);
    
    // 跟踪日志
    // g_Log->Info(LOGFMT_OBJ_TAG("%s send total bytes = [%llu], _lastSendLeft left bytes unhandled = [%llu], rest packets amount = [%llu], backtrace:%s")
    //                 , ToString().c_str(), handledBytes, _lastSendLeft ? _lastSendLeft->GetReadableSize() : 0, _sendPacketList->GetAmount(),  KERNEL_NS::BackTraceUtil::CrystalCaptureStackBackTrace().c_str());
}

void EpollTcpSession::OnRecv()
{
    _dirtyHelper->Clear(this, PollerDirty::READ);

    Int64 ret = 0;
    LibStreamTL *stream = NULL;
    auto recvNode = _recvBuffers->End();
    if(UNLIKELY(!recvNode))
    {
        if(LIKELY(CanRecv()))
        {
            stream = LibStreamTL::NewThreadLocal_LibStream();
            stream->Init(_bufferCapacity);
            _recvBuffers->PushBack(stream);
        }
    }
    else
    {
        stream = recvNode->_data;
        if((stream->GetWritableSize() == 0) && CanRecv())
        {
            stream = LibStreamTL::NewThreadLocal_LibStream();
            stream->Init(_bufferCapacity);
            _recvBuffers->PushBack(stream);
        }
    }

    UInt64 curReadBytes = 0;
    SOCKET sock = _sock->GetSock();

    // 可以保证清理缓冲区的速度>发送（因为网络io速度慢,且接收缓冲区满后滑动窗口为0,阻止接收数据）
    if(LIKELY(CanRecv()))
    {
        auto writableSize = stream->GetWritableSize();
        if(_handleRecvBytesPerFrameLimit)
        {
            auto leftEnableBytes = static_cast<Int64>(GetEnableRecvHandleBytesLeft());
            writableSize = (writableSize > leftEnableBytes) ? leftEnableBytes : writableSize;
        }

        for(;;)
        {            
            Byte8 *recvBuffer = stream->GetWriteBegin();
            while (((ret = ::recv(sock, recvBuffer, writableSize, 0)) < 0) && (errno == EINTR));
            if(LIKELY(ret > 0))
            {
                _curFrameRecvHandleBytes += ret;
                stream->ShiftWritePos(ret);
                curReadBytes += ret;
                _pollerMgr->AddRecvBytes(ret);

                writableSize = stream->GetWritableSize();
                if(writableSize == 0)
                {
                    Int32 pendingBytes;
                    if (UNLIKELY(::ioctl(_sock->GetSock() , FIONREAD, &pendingBytes) != 0))
                    {
                        g_Log->NetError(LOGFMT_OBJ_TAG("%s ioctl FIONREAD fail"), ToString().c_str());
                        
                        ForbidRecv();
                        auto param = _dirtyHelper->MaskDirty(this, PollerDirty::CLOSE, true);
                        param->BecomeDict()[1] = CloseSessionInfo::SOCK_ERR;
                        MaskClose(CloseSessionInfo::SOCK_ERR);
                        break;
                    }

                    // If no any ready data to read
                    if (pendingBytes == 0)
                    {// 相当于EWBLOCK
                        // g_Log->NetDebug(LOGFMT_OBJ_TAG("%s have no any data to recv"), ToString().c_str());
                        break;
                    }

                    stream = LibStreamTL::NewThreadLocal_LibStream();
                    stream->Init(static_cast<UInt64>(pendingBytes));
                    _recvBuffers->PushBack(stream);
                    writableSize = stream->GetWritableSize();
                }

                if(_handleRecvBytesPerFrameLimit)
                {
                    auto leftEnableBytes = static_cast<Int64>(GetEnableRecvHandleBytesLeft());
                    writableSize = (writableSize > leftEnableBytes) ? leftEnableBytes : writableSize;
                }

                if(UNLIKELY(writableSize == 0))
                {
                    // g_Log->NetDebug(LOGFMT_OBJ_TAG("cur frame recv bytes reach limit and will continue next frame curRecvBytes:%llu, recv bytes limit:%llu, session info:%s")
                    //             , _curFrameRecvHandleBytes, _handleRecvBytesPerFrameLimit, ToString().c_str());

                    _dirtyHelper->MaskDirty(this, PollerDirty::READ);
                    break;
                }
            }
            else if(ret == -1)
            {
                if(errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    // g_Log->NetDebug(LOGFMT_OBJ_TAG("socket buffer empty when recv data session brief info = [%s] errno=[%d, %s] , real recv bytes = [%llu]")
                    // , ToString().c_str(), errno, SystemUtil::GetErrString(errno).c_str(), curReadBytes);

                    // 读尽
                    break;
                }

                g_Log->NetWarn(LOGFMT_OBJ_TAG("sock error session brief info = [%s], errno = [%d, %s], real recv bytes = [%llu], and will close after parsing packets")
                                        , ToString().c_str(), errno, SystemUtil::GetErrString(errno).c_str(), curReadBytes);

                ForbidRecv();
                auto param = _dirtyHelper->MaskDirty(this, PollerDirty::CLOSE, true);
                param->BecomeDict()[1] = CloseSessionInfo::SOCK_ERR;
                MaskClose(CloseSessionInfo::SOCK_ERR);
                break;
            }
            else
            {// 对端关闭
                ForbidRecv();
                // g_Log->NetDebug(LOGFMT_OBJ_TAG("socket close by peer session info = %s, and will close after parsing packets"), ToString().c_str());
                auto param = _dirtyHelper->MaskDirty(this, PollerDirty::CLOSE, true);
                param->BecomeDict()[1] = CloseSessionInfo::REMOTE_DISONNECT;
                MaskClose(CloseSessionInfo::REMOTE_DISONNECT);
                break;
            }
        }
    } // if(CanRecv())
    

    // g_Log->NetDebug(LOGFMT_OBJ_TAG("session suc recv [%llu] Bytes, have recv dirty later:%d, session info:%s")
    //                 , curReadBytes, _dirtyHelper->IsDirty(this, PollerDirty::READ), ToString().c_str());

    ResetFrameRecvHandleBytes();
    if(LIKELY(HasDataToRecv()))
    {// 收到数据
        // 包解析等打包成packet
        _OnRecved();
    }
}

bool EpollTcpSession::_SendStream(LibStream<_Build::TL> *&stream, UInt64 &handledBytes)
{
    if(UNLIKELY(!CanSend()))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("%s cant send now!"), ToString().c_str());
        return false;
    }

    auto leftBytes = stream->GetReadableSize();
    if(_handleSendBytesPerFrameLimit)
    {
        const Int64 sendEnableBytes = static_cast<Int64>(GetEnableSendHandleBytesLeft());
        leftBytes = leftBytes > sendEnableBytes ? sendEnableBytes : leftBytes;
    }

    UInt64 sendBytes = 0;
    auto ret = _DoSend(stream->GetReadBegin(), static_cast<Int64>(leftBytes), sendBytes);
    if(LIKELY(sendBytes))
    {
        handledBytes += sendBytes;
        _curFrameSendHandleBytes += sendBytes;
        stream->ShiftReadPos(sendBytes);
        _pollerMgr->AddSendBytes(sendBytes);
    }

    if(UNLIKELY(ret != Status::Success))
    {
        if(ret == Status::SockError_EAGAIN_OR_EWOULDBLOCK)
        {
            MaskSendEagain();
            // g_Log->NetTrace(LOGFMT_OBJ_TAG("%s EWOULDBLOCK or EAGAIN errno=[%d] kernel sock buffer is full waiting for sending next time last send stream has bytes = [%llu] not send!")
            //                 , ToString().c_str(), errno, stream->GetReadableSize());
        }
        else
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("%s net is error errno = [%d], last send ringbuffer has bytes = [%llu] not send")
                        ,ToString().c_str(), errno, stream->GetReadableSize());

            ForbidSend();
            _dirtyHelper->Clear(this, PollerDirty::WRITE);
            auto param = _dirtyHelper->MaskDirty(this, PollerDirty::CLOSE, true);
            param->BecomeDict()[1] = CloseSessionInfo::SOCK_ERR;
            MaskClose(CloseSessionInfo::SOCK_ERR);
        }
        
        // 发送完则销毁
        if(stream->IsReadFull())
        {
            LibStream<_Build::TL>::DeleteThreadLocal_LibStream(stream);
            stream = NULL;
        }
        // else
        // {
        //     g_Log->NetTrace(LOGFMT_OBJ_TAG("%s has left stream not send left bytes = %llu ")
        //                     , ToString().c_str(), stream->GetReadableSize());
        // }

        return false;
    }

    ASSERT(leftBytes == static_cast<Int64>(sendBytes));

    // 跟踪日志
    // g_Log->NetTrace(LOGFMT_OBJ_TAG("%s send bytes = [%llu], left bytes:%lld")
    //             , ToString().c_str(), sendBytes, stream->GetReadableSize());

    if(UNLIKELY(WillSessionClose()))
        ForbidSend();

    if(stream->IsReadFull())
    {
        LibStream<_Build::TL>::DeleteThreadLocal_LibStream(stream);
        stream = NULL;
        return true;
    }
    else if(LIKELY(CanSend()))
        _dirtyHelper->MaskDirty(this, PollerDirty::WRITE);

    return false;
}

Int32 EpollTcpSession::_DoSend(const Byte8 *buffer, Int64 bufferSize, UInt64 &handledBytes)
{
    handledBytes = 0;
    Int64 realSendBytes = 0;
    while (bufferSize)
    {
        while( ( (realSendBytes = ::send(_sock->GetSock(), buffer + handledBytes, bufferSize, 0)) < 0 ) && (errno == EINTR));
        if(realSendBytes == -1)
        {
            // 缓冲区满不可写
            if(errno == EWOULDBLOCK || errno == EAGAIN)
            {
                // g_Log->NetTrace(LOGFMT_OBJ_TAG("%s do send SockError_EAGAIN_OR_EWOULDBLOCK error errno = [%d]"), ToString().c_str(), errno);
                return Status::SockError_EAGAIN_OR_EWOULDBLOCK;
            }
            else
            {
                // g_Log->NetTrace(LOGFMT_OBJ_TAG("%s do send unknown error errno = [%d]"), ToString().c_str(), errno);
                return Status::SockError_UnknownError;
            }
        }

        bufferSize -= realSendBytes;
        handledBytes += realSendBytes;
    }

    // g_Log->NetDebug(LOGFMT_OBJ_TAG("%s left bytes = [%lld] sent bytes=[%lld]"), ToString().c_str(), bufferSize, handledBytes);

    return Status::Success;
}

void EpollTcpSession::_SendPackets(UInt64 &handledBytes)
{
// 协议栈处理,一次处理1个包并返回处理后的字节长度 直到_maxSendHandleBytesPerTimes
    UInt64 totalPacketsCountHandled = 0;
    for(auto node = _sendPacketList->Begin(); node;)
    {
        // 若打包失败则把包丢弃 packet 转成 缓存数据
        auto newSteam = LibStream<_Build::TL>::NewThreadLocal_LibStream();
        newSteam->Init(_bufferCapacity, KernelGetTlsMemoryPool());
        
        UInt64 binBytes = 0;
        // g_Log->Info(LOGFMT_OBJ_TAG("epoll tcp session send msg session id:%llu, packet:%s"), GetId(), node->_data->ToString().c_str());
        Int32 errCode = _protocolStack->PacketsToBin(this, node->_data, newSteam, binBytes);
        if(UNLIKELY(errCode != Status::Success))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("session info = [%s] PacketsToBin fail packt info = [%s], error code = [%d]")
                                    , ToString().c_str(), node->_data->ToString().c_str(), errCode);
            LibStream<_Build::TL>::DeleteThreadLocal_LibStream(newSteam);
        }
        else
        {
            ++totalPacketsCountHandled;
            // 发送缓存数据并释放缓存 
            if(UNLIKELY(!_SendStream(newSteam, handledBytes)))
            {
                _lastSendLeft = newSteam;
                LibPacket::Delete_LibPacket(node->_data);
                node = _sendPacketList->Erase(node);
                break;
            }
        }


        LibPacket::Delete_LibPacket(node->_data);
        node = _sendPacketList->Erase(node);
    }

    // 发送数量统计
    _pollerMgr->AddSendPacketCount(totalPacketsCountHandled);

    // g_Log->NetTrace(LOGFMT_OBJ_TAG("old packet amount = [%llu], amount after send = [%llu], last send left bytes = [%llu]")
    //                         , oldAmount, _sendPacketList->GetAmount(), _lastSendLeft?_lastSendLeft->GetReadableSize():0);
}

void EpollTcpSession::_OnRecved()
{
    LibList<LibList<LibPacket *> *, _Build::TL> *recvPacketsBatch = LibList<LibList<LibPacket *> *, _Build::TL>::NewThreadLocal_LibList();

    UInt64 totalHandledBytes = 0;
    UInt64 packetsCount = 0;

    // 最终还有剩余的stream会被塞到_recvBuffers中
    LibStreamTL *leftStream = NULL;
    for(auto node = _recvBuffers->Begin(); node;)
    {
        auto buffer = node->_data;
        UInt64 handledBytes = 0;
        if(leftStream)
        {
            leftStream->Write(buffer->GetReadBegin(), buffer->GetReadableSize());
            LibStreamTL::DeleteThreadLocal_LibStream(buffer);
            buffer = leftStream;
            leftStream = NULL;
        }

        // 应该把buffer数据都读干净, 不存在还有剩余,剩余的应该存到不完整包中_incompletedRecvPacket
        if (buffer->GetReadableSize() > 0)
        {
            // 解析,碰到不完整包停止, 不完整包怎么处置：把不完整的放在一个stream中下一刻合并stream
            auto errCode = _protocolStack->ParsingPacket(this, *buffer, handledBytes, packetsCount, recvPacketsBatch);
            if(errCode != Status::Success)
            {
                // 解包失败
                g_Log->NetError(LOGFMT_OBJ_TAG("ParsingPacket fail perhaps bad packet in session socket buffer, errCode:%d will close session %s!!!"), errCode, ToString().c_str());
                ForbidRecv();
                auto param = _dirtyHelper->MaskDirty(this, PollerDirty::CLOSE, true);
                param->BecomeDict()[1] = CloseSessionInfo::PACKET_PARSING_ERROR;
                MaskClose(CloseSessionInfo::PACKET_PARSING_ERROR);
                node = _recvBuffers->Erase(node);

                break;
            }

            totalHandledBytes += handledBytes;
        }
        
        if(buffer->IsReadFull())
        {
            LibStreamTL::DeleteThreadLocal_LibStream(buffer);
        }
        else
        {// 保留住
            // 有已读超过capicity的一半需要压缩空间
            // if(buffer->GetReadBytes() >= (buffer->GetBufferSize() >> 1))
            //     buffer->Compress();

            leftStream = buffer;
        }

        node = _recvBuffers->Erase(node);
    }

    if(leftStream)
        _recvBuffers->PushBack(leftStream);

    // 本次接收数据信息
    // g_Log->NetDebug(LOGFMT_OBJ_TAG("session info = [%s]"
    //                         " total recv buffer amount = [%llu] "
    //                         "recv data bytes = [%llu],"
    //                         " generate packets = [%llu], "
    //                         "hasBadPackets = [%d], "
    //                         "rest recv buffer node amount = [%llu]")
    //                         , ToString().c_str()
    //                         , totalBufferAmount
    //                         , totalHandledBytes
    //                         , recvPackets ? recvPackets->GetAmount():0
    //                         , hasBadPackets
    //                         , _recvBuffers->GetAmount());

    // 给服务层发收到消息事件
    if(LIKELY(!recvPacketsBatch->IsEmpty()))
    {
        _pollerMgr->AddRecvPacketCount(packetsCount);

        for(auto node = recvPacketsBatch->Begin(); node;)
        {
            auto msg = RecvMsgEvent::New_RecvMsgEvent();
            msg->_sessionId = GetId();
            msg->_serviceId = GetServiceId();
            msg->_priorityLevel = GetPriorityLevel();
            msg->_packets = node->_data;
            _serviceProxy->PostMsg(msg->_serviceId, msg->_priorityLevel, msg, static_cast<Int64>(msg->_packets->GetAmount()));

            node = recvPacketsBatch->Erase(node);
        }
    }

    LibList<LibList<LibPacket *> *, _Build::TL>::DeleteThreadLocal_LibList(recvPacketsBatch);
}

void EpollTcpSession::_Destroy()
{

}

KERNEL_END

#endif
