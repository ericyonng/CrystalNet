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
 * Date: 2022-08-14 00:59:58
 * Author: Eric Yonng
 * Description: 
*/

#include "pch.h"
#include "service_common/protocol/CrystalProtocol/CrystalProtocolStack.h"
#include "service_common/protocol/CrystalProtocol/CrystalMsgHeader.h"

#ifndef DISABLE_OPCODES
 #include <protocols/protocols.h>
#endif

SERVICE_COMMON_BEGIN

CrystalProtocolStack::~CrystalProtocolStack()
{
    if(_opcodeNameParser)
        _opcodeNameParser->Release();
    _opcodeNameParser = NULL;
}

Int32 CrystalProtocolStack::ParsingPacket(KERNEL_NS::LibSession *session
, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream
, UInt64 &handledBytes
, UInt64 &packetCount
, KERNEL_NS::LibList<KERNEL_NS::LibList<KERNEL_NS::LibPacket *> *, KERNEL_NS::_Build::TL> *&recvPacketsBatch)
{
    #ifdef _DEBUG
    const auto getContent = [](){
        return KERNEL_NS::LibString().AppendFormat(PR_FMT(""));
     };
     PERFORMANCE_RECORD_DEF(pr, getContent, 5);
    #endif

    const auto &option = session->GetOption();
    KERNEL_NS::LibList<KERNEL_NS::LibPacket *> *parsedPacket = recvPacketsBatch->End() ? recvPacketsBatch->End()->_data : NULL;

    Int32 errCode = Status::Success;
    for(;;)
    {
        // 保护原始stream
        KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> streamCache;
        streamCache.Attach(stream);

        // 1.解码包头
        CrystalMsgHeader header;
        {
            if(streamCache.GetReadableSize() < MsgHeaderStructure::MSG_HEADER_SIZE)
            {
                // g_Log->NetDebug(LOGFMT_OBJ_TAG("data not reach parsing header size session:%s, stream readable size:%llu")
                //                 , session->ToString().c_str(), streamCache.GetReadableSize());
                errCode = Status::Success;
                break;
            }

            streamCache.Read(&header._len, MsgHeaderStructure::LEN_SIZE);
            streamCache.Read(&header._protocolVersion, MsgHeaderStructure::PROTOCOL_VERSION_SIZE);
            streamCache.Read(&header._flags, MsgHeaderStructure::FLAGS_SIZE);
            streamCache.Read(&header._opcodeId, MsgHeaderStructure::OPCODE_SIZE);
            streamCache.Read(&header._packetId, MsgHeaderStructure::PACKET_ID_SIZE);

            if(header._len > MsgHeaderStructure::MSG_BODY_MAX_SIZE_LIMIT)
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("bad msg:len over limit limit:%d, header:%s, session:%s")
                                ,  MsgHeaderStructure::MSG_BODY_MAX_SIZE_LIMIT, header.ToString().c_str(), session->ToString().c_str());
                errCode = Status::ParsingPacketFail;
                break;
            }

            if(_maxContenBytes && (header._len > _maxContenBytes))
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("bad msg:len over content limit limit:%d, header:%s, session:%s")
                , MsgHeaderStructure::MSG_BODY_MAX_SIZE_LIMIT, header.ToString().c_str(), session->ToString().c_str());
                errCode = Status::ParsingPacketFail;
                break;
            }

            if(header._packetId < 0)
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("bad msg:bad packet id:%lld, session:%s")
                            , header._packetId, session->ToString().c_str());
                errCode = Status::ParsingPacketFail;
                break;
            }
        }

        // 2.剩余包数据是否够解析包体
        if(streamCache.GetReadableSize() < (header._len - MsgHeaderStructure::MSG_HEADER_SIZE))
        {
            // g_Log->NetDebug(LOGFMT_OBJ_TAG("data not reach msg body data len session:%s, stream readable size:%lld, msg len:%u")
            //     , session->ToString().c_str(), streamCache.GetReadableSize(), (header._len - static_cast<UInt32>(MsgHeaderStructure::MSG_HEADER_SIZE)));
            errCode = Status::Success;
            break;
        }
        
        // 3.校验opcode
        auto coderFactory = GetCoderFactory(header._opcodeId);
        if(!coderFactory)
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("have no opcode coder opcodeId:%d, session:%s"), header._opcodeId, session->ToString().c_str());
            errCode = Status::ParsingPacketFail;
            break;
        }

        #ifdef _DEBUG
        const auto opcode = header._opcodeId;
        const auto headerLen = header._len;
        const auto getContent = [opcode, headerLen, session](){
             return   KERNEL_NS::LibString().AppendFormat(PR_FMT("Decode over limit sessionId:%llu opcode:%u, len:%u")
            , session->GetId(), opcode, headerLen);
          };
         PERFORMANCE_RECORD_DEF(middlePr, getContent, 5);
        #endif

        // 4.创建编码器并解码
        auto coder = coderFactory->Create();
        KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> safeDecode;
        const auto msgBodySize = header._len - MsgHeaderStructure::MSG_HEADER_SIZE;
        safeDecode.Attach(const_cast<Byte8 *>(streamCache.GetReadBegin()), msgBodySize, 0, msgBodySize);
        if(!coder->Decode(safeDecode))
        {
            g_Log->NetWarn(LOGFMT_OBJ_TAG("coder decode fail opcode:%d session:%s"), header._opcodeId, session->ToString().c_str());
            errCode = Status::ParsingPacketFail;
            coder->Release();
            break;
        }
        streamCache.ShiftReadPos(msgBodySize);

        // 5.创建消息包 限速保护
        if(LIKELY(session->CheckUpdateRecvSpeed()))
        {
            if (UNLIKELY(!parsedPacket))
            {
                parsedPacket = KERNEL_NS::LibList<KERNEL_NS::LibPacket *>::New_LibList();
                recvPacketsBatch->PushBack(parsedPacket);
            }

            KERNEL_NS::LibPacket *packet = KERNEL_NS::LibPacket::New_LibPacket();
            packet->SetSessionId(session->GetId());
            auto sock = session->GetSock();
            auto addr = sock->GetAddr();
            packet->SetLocalAddr(addr->GetLocalBriefAddr());
            packet->SetRemoteAddr(addr->GetRemoteBriefAddr());
            packet->SetPacketId(header._packetId);
            packet->SetOpcode(header._opcodeId);
            packet->SetCoder(coder);

            if(UNLIKELY(_enableProtocolLog))
            {
                g_Log->NetInfo(LOGFMT_OBJ_TAG("parse packet suc:%s, opcode:%s, msg len:%u")
                            , packet->ToString().c_str()
                            , _opcodeNameParser->Invoke(header._opcodeId).c_str()
                            , header._len);
            }

            if(LIKELY(option._sessionRecvPacketStackLimit != 0))
            {
                if(parsedPacket->GetAmount() >= option._sessionRecvPacketStackLimit)
                {
                    parsedPacket = KERNEL_NS::LibList<KERNEL_NS::LibPacket *>::New_LibList();
                    recvPacketsBatch->PushBack(parsedPacket);
                }
            }

            parsedPacket->PushBack(packet);
            ++packetCount;
        }
        else
        {
            auto addr = session->GetSock()->GetAddr();
            g_Log->NetWarn(LOGFMT_OBJ_TAG("session speed over limit[%llu] per %llu ms, sessionId:%llu, %s ")
                , option._sessionRecvPacketSpeedLimit, option._sessionRecvPacketSpeedTimeUnitMs
                , session->GetId(), addr->ToString().c_str());
        }

        // 6.解码成功一个包
        handledBytes += header._len;
        stream.ShiftReadPos(header._len);
    }

    // g_Log->NetDebug(LOGFMT_OBJ_TAG("parsing packet finish errCode:%d, handledBytes:%llu, parsedPacket count:%llu session:%s")
    //                 , errCode, handledBytes, parsedPacket ? parsedPacket->GetAmount() : 0, session->ToString().c_str());

    return errCode;
}

Int32 CrystalProtocolStack::PacketsToBin(KERNEL_NS::LibSession *session
, KERNEL_NS::LibPacket *packet
, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *stream
, UInt64 &handledBytes)
{
    #if _DEBUG
    const auto getContent = [](){
        return KERNEL_NS::LibString().AppendFormat(PR_FMT(""));
     };
     PERFORMANCE_RECORD_DEF(pr, getContent, 5);
    #endif
    
    Int32 errCode = Status::Success;
    handledBytes = 0;
    do
    {
        // 1.校验coder是否存在
        auto coder = packet->GetCoder();
        if(UNLIKELY(!coder))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("packet have no coder session:%s"), session->ToString().c_str());
            errCode = Status::Error;
            break;
        }

    #if _DEBUG 
        const auto opcode = packet->GetOpcode();
        const auto sessionId = packet->GetSessionId();
        const auto getContent = [opcode, sessionId](){
            return KERNEL_NS::LibString().AppendFormat(PR_FMT("sessionId:%llu, opcode:%d"), sessionId, opcode);
        };
        PERFORMANCE_RECORD_DEF(middlePr, getContent, 5);
    #endif

        // 2.预留header空间
        stream->ShiftWritePos(MsgHeaderStructure::MSG_HEADER_SIZE);

        // 3.编码数据
        const auto contentStart = stream->GetWriteBytes();
        if(!coder->Encode(*stream))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("coder encode fail have no coder session:%s"), session->ToString().c_str());
            errCode = Status::CoderFail;
            break;
        }
        const auto contentEnd = stream->GetWriteBytes();
        const auto contentSize = contentEnd - contentStart;

        // 4.长度限制(不能超过最大长度)
        if((MsgHeaderStructure::MSG_HEADER_SIZE + contentSize) > MsgHeaderStructure::MSG_BODY_MAX_SIZE_LIMIT)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("coder encode fail content size over limit content size:%llu, limit:%d, session:%s")
                            ,contentSize, MsgHeaderStructure::MSG_BODY_MAX_SIZE_LIMIT, session->ToString().c_str());
            stream->ShiftWritePos(-(contentSize));
            errCode = Status::CoderFail;
            break;
        }

        // 5.编码包头
        CrystalMsgHeader header;
        // header._len = contentSize;
        header._len = static_cast<UInt32>(MsgHeaderStructure::MSG_HEADER_SIZE + contentSize);
        header._protocolVersion = GetProtoVersionNumber();

        // 6.TODO:包特性暂时不设置 
        header._flags = 0;
        header._opcodeId = packet->GetOpcode();
        header._packetId = packet->GetPacketId();

        // 7.往回拨len字节写入header
        stream->ShiftWritePos(-static_cast<Int64>(header._len));
        stream->Write(&header._len, MsgHeaderStructure::LEN_SIZE);
        stream->Write(&header._protocolVersion, MsgHeaderStructure::PROTOCOL_VERSION_SIZE);
        stream->Write(&header._flags, MsgHeaderStructure::FLAGS_SIZE);
        stream->Write(&header._opcodeId, MsgHeaderStructure::OPCODE_SIZE);
        stream->Write(&header._packetId, MsgHeaderStructure::PACKET_ID_SIZE);

        // 8.写入完成跳过包数据
        stream->ShiftWritePos(contentSize);

        if(UNLIKELY(_enableProtocolLog))
        {
            g_Log->NetInfo(LOGFMT_OBJ_TAG("packet to bin suc:%s, opcode:%s, msg len:%u")
                        , packet->ToString().c_str()
                        , _opcodeNameParser->Invoke(header._opcodeId).c_str()
                        , header._len);
        }

        handledBytes += static_cast<UInt64>(header._len);
    } while (0);

    // g_Log->NetInfo(LOGFMT_OBJ_TAG("packet to bin end errCode:%d, handledBytes:%llu session:%s"), errCode, handledBytes, session->ToString().c_str());
    return errCode;
}

#ifndef DISABLE_OPCODES
KERNEL_NS::ICoderFactory *CrystalProtocolStack::GetCoderFactory(Int32 opcode)
{
    return Opcodes::GetCoderFactory(opcode);
}

void CrystalProtocolStack::RegisterCoderFactory(Int32 opcode, KERNEL_NS::ICoderFactory *factory)
{
    Opcodes::RegisterCoderFactory(opcode, factory);
}

#else

KERNEL_NS::ICoderFactory *CrystalProtocolStack::GetCoderFactory(Int32 opcode)
{
    return NULL;
}

void CrystalProtocolStack::RegisterCoderFactory(Int32 opcode, KERNEL_NS::ICoderFactory *factory)
{
    
}

#endif

void CrystalProtocolStack::SetOpcodeNameParser(KERNEL_NS::IDelegate<const KERNEL_NS::LibString &, Int32> *parser) 
{ 
    if(_opcodeNameParser) _opcodeNameParser->Release(); 
    _opcodeNameParser = parser;
}


SERVICE_COMMON_END
