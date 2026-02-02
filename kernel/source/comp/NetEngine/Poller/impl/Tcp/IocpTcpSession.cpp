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
 * Date: 2022-04-23 20:29:08
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/IocpTcpSession.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

#include <kernel/comp/LibDirtyHelper.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerDirty.h>
#include <kernel/comp/NetEngine/Poller/Defs/CloseSessionInfo.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/IocpTcpPoller.h>
#include <kernel/comp/NetEngine/LibSocket.h>
#include <kernel/comp/NetEngine/Protocol/Protocol.h>
#include <kernel/comp/NetEngine/LibPacket.h>
#include <kernel/comp/NetEngine/Defs/IoData.h>
#include <kernel/comp/Utils/SockErrorMsgUtil.h>
#include <kernel/comp/NetEngine/Poller/interface/IPollerMgr.h>
#include <kernel/comp/Utils/Utils.h>
#include <kernel/comp/Service/Service.h>
#include <kernel/comp/NetEngine/Defs/IoEvent.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerInnerEvent.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerEvent.h>

KERNEL_BEGIN

IocpTcpSession::IocpTcpSession(UInt64 sessionId, bool isLinker, bool isConnectToRemote)
:LibSession(sessionId, isLinker, isConnectToRemote)
,_dirtyHelper(NULL)
,_serviceProxy(NULL)
,_pollerMgr(NULL)
,_waitingForPostSendBack(false)
{

}

IocpTcpSession::~IocpTcpSession()
{
    _Destroy();
}

void IocpTcpSession::Close()
{
    _Destroy();
    LibSession::Close();
}

void IocpTcpSession::SendPackets(LibList<LibPacket *> *packets)
{
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

    // 合并
    _sendPacketList->MergeTail(packets);
    LibList<LibPacket *>::Delete_LibList(packets);

    if(_waitingForPostSendBack)
    {
        // g_Log->NetDebug(LOGFMT_OBJ_TAG("session wait for post send back packets amount:%llu, session:%s"), _sendPacketList->GetAmount(), ToString().c_str());
        return;
    }

    // 转成字节流
    auto stream = _PacketsToBin();
    if(UNLIKELY(!stream))
    {
        if(UNLIKELY(!_sendPacketList->IsEmpty()))
            _dirtyHelper->MaskDirty(this, PollerDirty::WRITE);

        g_Log->NetWarn(LOGFMT_OBJ_TAG("_PacketsToBin fail session:%s"), ToString().c_str());
        return;
    }

    // 发送
    _PostSendStream(stream);
}

void IocpTcpSession::OnSend(IoEvent &io)
{
    _waitingForPostSendBack = false;

    IoData *ioData = io._ioData;
    UInt64 totalSend = 0;   // TODO:统计发送量

    LibStreamTL *newStream = NULL;
    if(ioData->_tlStream)
    {// NonZero-WSASend overlapped, delete the block and overlapped
        if(io._bytesTrans <= 0)
        {
            _MaskClose(CloseSessionInfo::SEND_ERROR);
            ForbidSend();
            _dirtyHelper->Clear(this, PollerDirty::WRITE);
            g_Log->NetWarn(LOGFMT_OBJ_TAG("send completion with bytes trans below equal zero! session:%s"), ToString().c_str());
            return;
        }

        if(UNLIKELY(io._bytesTrans > ioData->_tlStream->GetReadableSize()))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("bytes trans[%lld] larger than readable size[%llu]"), io._bytesTrans, ioData->_tlStream->GetReadableSize());
        }

        _pollerMgr->AddSendBytes(io._bytesTrans);
        ioData->_tlStream->ShiftReadPos(io._bytesTrans);

        // 重用
        if(!ioData->_tlStream->IsReadFull())
            newStream = ioData->_tlStream;
        else
        {
            LibStreamTL::DeleteThreadLocal_LibStream(ioData->_tlStream);
        }

        ioData->_tlStream =  NULL;
    }
    else
    {// Zero-WSASend overlapped
        if(UNLIKELY(!CanSend()))
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("cant send session:%s"), ToString().c_str());
            return;
        }

        if(LIKELY(_lastSendLeft))
        {
            SOCKET mySock = GetSock()->GetSock();

            Int32 ret = 0;
            UInt64 totalSendBytes = 0;
            if(LIKELY(_handleSendBytesPerFrameLimit))
            {
                auto readableSize = _lastSendLeft->GetReadableSize();
                const auto enableBytes = static_cast<Int64>(GetEnableSendHandleBytesLeft());
                readableSize = readableSize > enableBytes ? enableBytes : readableSize;

                for(;;)
                {
                    if(UNLIKELY(readableSize == 0))
                        break;

                    Int64 handledBytes = 0;
                    ret = SocketUtil::SyncSend(mySock, _lastSendLeft->GetReadBegin(), readableSize, 0, handledBytes);
                    if(ret != Status::Success)
                    {
                        g_Log->NetWarn(LOGFMT_OBJ_TAG("SyncSend fail ret:%d readableSize:%llu, session :%s"), ret, readableSize, ToString().c_str());
                        break;
                    }

                    _curFrameSendHandleBytes += handledBytes;
                    _lastSendLeft->ShiftReadPos(handledBytes);
                    readableSize -= handledBytes;
                    totalSendBytes += handledBytes;
                }
            }
            else
            {
                auto readableSize = _lastSendLeft->GetReadableSize();
                for(;;)
                {
                    if(UNLIKELY(readableSize == 0))
                        break;

                    Int64 handledBytes = 0;
                    ret = SocketUtil::SyncSend(mySock, _lastSendLeft->GetReadBegin(), readableSize, 0, handledBytes);
                    if(ret != Status::Success)
                    {
                        g_Log->NetWarn(LOGFMT_OBJ_TAG("SyncSend fail ret:%d readableSize:%llu, session :%s"), ret, readableSize, ToString().c_str());
                        break;
                    }

                    _curFrameSendHandleBytes += handledBytes;
                    _lastSendLeft->ShiftReadPos(handledBytes);
                    readableSize -= handledBytes;
                    totalSendBytes += handledBytes;
                }
            }

            if(LIKELY(totalSendBytes))
                _pollerMgr->AddSendBytes(totalSendBytes);

            if(ret != Status::Success && ret != Status::SockError_EAGAIN_OR_EWOULDBLOCK)
            {
                _MaskClose(CloseSessionInfo::SEND_ERROR);
                _dirtyHelper->Clear(this, PollerDirty::WRITE);
                ForbidSend();
                g_Log->NetWarn(LOGFMT_OBJ_TAG("zero wsasend iocp completiong back and call SyncSend with a error ret:%d, session:%s"), ret, ToString().c_str());
                return;
            }

            if(_lastSendLeft->IsReadFull())
            {
                LibStreamTL::DeleteThreadLocal_LibStream(_lastSendLeft);
            }
            else
            {
                newStream = _lastSendLeft;
            }
            
            _lastSendLeft = NULL;
        }
        else
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("post a zero wsasend with no _lastSendLeft session:%s "), ToString().c_str());
        }
    }

    // 到这里 _lastSendLeft 为NULL, ioData->_tlStream 为 NULL 且不存在还未完成的iocp post

    if(UNLIKELY(!newStream))
    {
        newStream = LibStreamTL::NewThreadLocal_LibStream();
        newStream->Init(this->_bufferCapacity);
    }
    else if(newStream->GetWritableSize() < static_cast<Int64>(_bufferCapacity))
        newStream->AppendCapacity(_bufferCapacity);

    // packet转字节流
    _PacketsToBin(newStream);

    // 发送
    _PostSendStream(newStream);
}

void IocpTcpSession::OnRecv(IoEvent &io)
{
    auto ioData = io._ioData;
    IoData::DeleteThreadLocal_IoData(ioData);
    io._ioData = NULL;

    ContinueRecv();
}

void IocpTcpSession::ContinueRecv()
{
    _dirtyHelper->Clear(this, PollerDirty::READ);

    // 接收流数据准备
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
    Int32 errCode = 0;
    UInt64 recvTotalBytes = 0;
    if(LIKELY(CanRecv()))
    {
        auto writableSize = stream->GetWritableSize();
        if(_handleRecvBytesPerFrameLimit)
        {
            auto leftEnableBytes = static_cast<Int64>(GetEnableRecvHandleBytesLeft());
            writableSize = (writableSize > leftEnableBytes) ? leftEnableBytes : writableSize;
        }
        if(_option._sockRecvBufferSize)
        {
            if (writableSize > static_cast<Int64>(_option._sockRecvBufferSize))
                writableSize = static_cast<Int64>(_option._sockRecvBufferSize);
        }

        for(;;)
        {
            Byte8 *recvBuffer = stream->GetWriteBegin();
            while ((ret = ::recv(sock,
                            recvBuffer, 
                            static_cast<Int32>(writableSize), 
                            0)) < 0 && errno == EINTR); 

            if(ret > 0)
            {
                _curFrameRecvHandleBytes += ret;
                stream->ShiftWritePos(ret);
                curReadBytes += ret;
                recvTotalBytes += ret;

                writableSize = stream->GetWritableSize();
                if(writableSize == 0)
                {
                    ULong pendingBytes;
                    if (UNLIKELY(::ioctlsocket(_sock->GetSock(), FIONREAD, &pendingBytes) == SOCKET_ERROR))
                    {
                        errCode = ::WSAGetLastError();
                        g_Log->NetError(LOGFMT_OBJ_TAG("%s ioctl FIONREAD fail errCode:%d, %s.")
                                    , ToString().c_str(), errCode, SockErrorMsgUtil::GetString(errCode).c_str());
                        ret = -1;

                        ForbidRecv();
                        _MaskClose(CloseSessionInfo::SOCK_ERR);
                        break;
                    }

                    // If no any ready data to read
                    if (pendingBytes == 0)
                    {// 相当于EWBLOCK
                        // g_Log->NetDebug(LOGFMT_OBJ_TAG("%s have no any data to recv"), ToString().c_str());
                        break;
                    }

                    stream = LibStreamTL::NewThreadLocal_LibStream();
                    stream->Init(pendingBytes);
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
            else if(ret < 0)
            {
                auto err = ::WSAGetLastError();
                if(err != WSAEWOULDBLOCK)
                {
                    g_Log->NetError(LOGFMT_OBJ_TAG("iocp recv fail will close after parsing packets err:%d,%s, session info:%s")
                                , err, SockErrorMsgUtil::GetString(err).c_str(), ToString().c_str());

                    ForbidRecv();
                    _MaskClose(CloseSessionInfo::SOCK_ERR);
                    break;
                }

                break;
            }
            else if(ret == 0)
            {
                auto err = SystemUtil::GetErrNo(true);
                KERNEL_NS::LibString errString;
                if (err != 0)
                {
                    errString = SystemUtil::GetErrString(err);
                }
                
                if(g_Log->IsEnable(LogLevel::NetInfo))
                    g_Log->NetInfo(LOGFMT_OBJ_TAG("socket close by peer, err:%d,%s session info = %s, and will close after parsing packets"), err, errString.c_str(), ToString().c_str());
                ForbidRecv();
                _MaskClose(CloseSessionInfo::REMOTE_DISONNECT);

                break;
            }
        }
    } // if(CanRecv())
 
    // g_Log->NetDebug(LOGFMT_OBJ_TAG("session suc recv [%llu] Bytes, have recv dirty later:%d, session info:%s")
    //                 , curReadBytes, _dirtyHelper->IsDirty(this, PollerDirty::READ), ToString().c_str());

    if(LIKELY(recvTotalBytes))
        _pollerMgr->AddRecvBytes(recvTotalBytes);

    ResetFrameRecvHandleBytes();
    if(LIKELY(HasDataToRecv()))
    {// 收到数据
        // 包解析等打包成packet
        _OnRecved();
    }

    if(LIKELY(CanRecv()))
    {
        auto err = _sock->PostZeroWSARecv();
        if(err != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("sock post zero wsa recv fail will close err:%d, session info:%s"), err, ToString().c_str());
            ForbidRecv();
            _MaskClose(CloseSessionInfo::SOCK_ERR);
        }
    }
}

LibString IocpTcpSession::ToString() const
{
    LibString str;
    str.AppendFormat("%s", LibSession::ToString().c_str())
        .AppendFormat("_waitingForPostSendBack:%d", _waitingForPostSendBack);
        
    return str;
}

void IocpTcpSession::_OnRecved()
{
   LibList<LibList<LibPacket *> *, _Build::TL> *recvPacketsBatch = LibList<LibList<LibPacket *> *, _Build::TL>::NewThreadLocal_LibList();

    UInt64 totalHandledBytes = 0;
    UInt64 packetsCount = 0;
    bool hasBadPackets = false;

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
            // 解析,碰到不完整包停止, 不完整包怎么处置：把原始数据拷贝到Packet缓冲中,等到数据完整可解析后再解析,并打标记完成
            auto errCode = _protocolStack->ParsingPacket(this, *buffer, handledBytes, packetsCount, recvPacketsBatch);
            if(errCode != Status::Success)
            {
                // 解包失败
                g_Log->NetError(LOGFMT_OBJ_TAG("ParsingPacket fail perhaps bad packet in session socket buffer, will close session %s!!!"), ToString().c_str());
                ForbidRecv();
                _MaskClose(CloseSessionInfo::PACKET_PARSING_ERROR);
                hasBadPackets = true;
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
            msg->_packets = node->_data;
            _serviceProxy->PostMsg(msg->_serviceId, msg, static_cast<Int64>(msg->_packets->GetAmount()));

            node = recvPacketsBatch->Erase(node);
        }
    }  

    LibList<LibList<LibPacket *> *, _Build::TL>::DeleteThreadLocal_LibList(recvPacketsBatch);
}

void IocpTcpSession::_Destroy()
{

}

LibStreamTL *IocpTcpSession::_PacketsToBin()
{
    bool sockCacheFull = false;
    const auto oldAmount = _sendPacketList->GetAmount();
    if(UNLIKELY(oldAmount == 0))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("have no packets to send session:%s."), ToString().c_str());
        return NULL;
    }

    auto newSteam = LibStream<_Build::TL>::NewThreadLocal_LibStream();
    newSteam->Init(_bufferCapacity, KernelGetTlsMemoryPool());

    return _PacketsToBin(newSteam);
}

LibStreamTL *IocpTcpSession::_PacketsToBin(LibStreamTL *&stream)
{
    Int64 enableBytes = -1;
    if(LIKELY(_handleSendBytesPerFrameLimit))
        enableBytes = GetEnableSendHandleBytesLeft();
    if(_option._sockSendBufferSize)
    {
        if((enableBytes > static_cast<Int64>(_option._sockSendBufferSize)) || (enableBytes == -1))
            enableBytes = static_cast<Int64>(_option._sockSendBufferSize);
    }

    UInt64 handledBytes = 0;
    UInt64 handledPacketCount = 0;
    for(auto node = _sendPacketList->Begin(); node;)
    {
        // 单帧控制
        if((enableBytes >= 0) && (static_cast<Int64>(handledBytes) >= enableBytes))
        {
            // g_Log->NetDebug(LOGFMT_OBJ_TAG("sessionId:%llu packets to bin over limit handledBytes:%llu, enableBytes:%lld")
            // , GetId(), handledBytes, enableBytes);
            if(UNLIKELY(stream->IsReadFull()))
            {
                LibStream<_Build::TL>::DeleteThreadLocal_LibStream(stream);
                stream = NULL;
            }

            break;
        }

        // 若打包失败则把包丢弃 packet 转成 缓存数据
        Int32 errCode = _protocolStack->PacketsToBin(this, node->_data, stream, handledBytes);
        if(UNLIKELY(errCode != Status::Success))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("session info = [%s] PacketsToBin fail packt info = [%s], error code = [%d]")
                                    , ToString().c_str(), node->_data->ToString().c_str(), errCode);

        }
        else
        {
            ++handledPacketCount;
        }

        LibPacket::Delete_LibPacket(node->_data);
        node = _sendPacketList->Erase(node);
    }

    if(LIKELY(handledPacketCount > 0))
        _pollerMgr->AddSendPacketCount(handledPacketCount);

    if(LIKELY(handledBytes > 0))
        _curFrameSendHandleBytes += handledBytes;

    return stream;
}

void IocpTcpSession::_PostSendStream(LibStreamTL *&newStream)
{
    if (UNLIKELY(newStream->GetReadableSize() == 0))
    {
        LibStream<_Build::TL>::DeleteThreadLocal_LibStream(newStream);
        newStream = NULL;
        return;
    }

    auto newIo = IoData::NewThreadLocal_IoData();
    newIo->_sessionId = GetId();
    newIo->_sock = GetSock()->GetSock();
    newIo->_tlStream = newStream;
    newIo->_wsaBuff.buf = KernelCastTo<Byte8>(newStream->GetReadBegin());
    newIo->_wsaBuff.len = static_cast<ULONG>(newStream->GetReadableSize());
    auto errCode = SocketUtil::PostSend(newIo);
    if(errCode != Status::Success)
    {
        // 系统繁忙(锁定的缓存过多,资源耗尽)导致nobuffer，这时候投递0字节
        if(errCode == Status::IOCP_NoBuffs)
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("PostSend IOCP_NoBuffs and will post zero wsasend session:%s"), ToString().c_str());

            newIo->Reset();
            newIo->_sessionId = GetId();
            newIo->_sock = GetSock()->GetSock();

            // 放入缓存
            if(UNLIKELY(_lastSendLeft))
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("no buff occurs before but not wait iocp notify session:%s"), ToString().c_str());
                _lastSendLeft->Write(newStream->GetReadBegin(), newStream->GetReadableSize());
                LibStream<_Build::TL>::DeleteThreadLocal_LibStream(newStream);
                newStream = NULL;
            }
            else
            {
                _lastSendLeft = newStream;
                newStream =  NULL;
            }

            errCode = SocketUtil::PostSend(newIo);
        }
        else if(errCode != Status::SockError_Pending)
        {// 其他错误
            auto err = SystemUtil::GetErrNo(true);
            const auto &errStr = SystemUtil::GetErrString(err);
            g_Log->NetError(LOGFMT_OBJ_TAG("PostSend with a error errCode:%d, errInfo:%d,%s session:%s"), errCode, err, errStr.c_str(), ToString().c_str());
            LibStream<_Build::TL>::DeleteThreadLocal_LibStream(newStream);
            newStream = NULL;
            newIo->_tlStream = NULL;
        }
    }

    // 发送失败处理
    if((errCode != Status::Success) && (errCode != Status::SockError_Pending))
    {// close
        IoData::DeleteThreadLocal_IoData(newIo);
        _MaskClose(CloseSessionInfo::SEND_ERROR);
        ForbidSend();
        _dirtyHelper->Clear(this, PollerDirty::WRITE);
        g_Log->NetError(LOGFMT_OBJ_TAG("postsend fail errCode:%d, session:%s"), errCode, ToString().c_str());
        return;
    }

    _waitingForPostSendBack = true;

    // 跟踪日志
    // g_Log->NetTrace(LOGFMT_OBJ_TAG("%s send total bytes = [%llu], _lastSendLeft left bytes unhandled = [%llu], rest packets amount = [%llu]")
    //                 , ToString().c_str(), newStream ? newStream->GetReadableSize() : 0, _lastSendLeft ? _lastSendLeft->GetReadableSize() : 0, _sendPacketList->GetAmount());
}

void IocpTcpSession::_MaskClose(Int32 reason)
{
    auto param = _dirtyHelper->MaskDirty(this, PollerDirty::CLOSE, true);
    param->BecomeDict()[1] = reason;
    MaskClose(reason);

    // throw std::logic_error("session will close.");
}

KERNEL_END

#endif
