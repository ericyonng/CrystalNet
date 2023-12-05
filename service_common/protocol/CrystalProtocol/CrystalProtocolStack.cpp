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
#include <kernel/kernel.h>
#include "service_common/protocol/CrystalProtocol/CrystalProtocolStack.h"
#include "service_common/protocol/CrystalProtocol/CrystalMsgHeader.h"

#ifndef DISABLE_OPCODES
 #include <protocols/protocols.h>
#endif

SERVICE_COMMON_BEGIN

static ALWAYS_INLINE KERNEL_NS::LibString StackPacketToString(const KERNEL_NS::LibPacket *packet)
{
    KERNEL_NS::LibString &&packetString = packet->ToString();

    #ifndef DISABLE_OPCODES
    auto opcodeInfo = Opcodes::GetOpcodeInfo(packet->GetOpcode());
    if(LIKELY(opcodeInfo))
        packetString.AppendFormat(", opcode name:%s", opcodeInfo->_opcodeName.c_str());
    else
        packetString.AppendFormat(", unknown opcode");
    #endif

    return packetString;
}

static ALWAYS_INLINE KERNEL_NS::LibString StackOpcodeToString(Int32 opcode)
{
    KERNEL_NS::LibString opcodeName;

    #ifndef DISABLE_OPCODES
    auto opcodeInfo = Opcodes::GetOpcodeInfo(opcode);
    if(LIKELY(opcodeInfo))
        opcodeName.AppendFormat("opcode name:%s", opcodeInfo->_opcodeName.c_str());
    else
        opcodeName.AppendFormat("unknown opcode");
    #endif

    return opcodeName;
}

static ALWAYS_INLINE bool IsNeedLog(Int32 opcode)
{
    #ifndef DISABLE_OPCODES
        return Opcodes::IsNeedLog(opcode);
    #else
        return false;
    #endif
}

static ALWAYS_INLINE UInt32 GetProtoFlags(Int32 opcode)
{
    #ifndef DISABLE_OPCODES
        return Opcodes::GetFlags(opcode);
    #else
        return false;
    #endif
}

CrystalProtocolStack::~CrystalProtocolStack()
{
}

Int32 CrystalProtocolStack::ParsingPacket(KERNEL_NS::LibSession *session
, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream
, UInt64 &handledBytes
, UInt64 &packetCount
, KERNEL_NS::LibList<KERNEL_NS::LibList<KERNEL_NS::LibPacket *> *, KERNEL_NS::_Build::TL> *&recvPacketsBatch)
{
    #ifdef _DEBUG
    auto &&outputLogFunc = [](UInt64 costMs){
        g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(CrystalProtocolStack, "costMs:%llu ms"), costMs);
    };
        
    PERFORMANCE_RECORD_DEF(pr, outputLogFunc, 5);
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
            streamCache.Read(&header._keyLen, MsgHeaderStructure::KEY_LEN_SIZE);

            if((header._len < MsgHeaderStructure::MSG_HEADER_SIZE) || (header._len > MsgHeaderStructure::MSG_BODY_MAX_SIZE_LIMIT))
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("bad msg len, len limit:%d, header:%s, session:%s")
                                ,  MsgHeaderStructure::MSG_BODY_MAX_SIZE_LIMIT, header.ToString().c_str(), session->ToString().c_str());
                errCode = Status::ParsingPacketFail;
                break;
            }

            if(_maxRecvContenBytes && (header._len > _maxRecvContenBytes))
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("bad msg:len over content limit limit:%d, header:%s, session:%s")
                , MsgHeaderStructure::MSG_BODY_MAX_SIZE_LIMIT, header.ToString().c_str(), session->ToString().c_str());
                errCode = Status::ParsingPacketFail;
                break;
            }

            if(header._packetId < 0)
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("bad msg:bad packet id:%lld, header:%s, session:%s")
                            , header._packetId, header.ToString().c_str(), session->ToString().c_str());
                errCode = Status::ParsingPacketFail;
                break;
            }

        }

        // 2.剩余包数据是否够解析包体
        auto leftLen = (header._len - MsgHeaderStructure::MSG_HEADER_SIZE);
        if(streamCache.GetReadableSize() < leftLen)
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
            g_Log->NetWarn(LOGFMT_OBJ_TAG("have no opcode coder will skip, opcodeId:%d, header:%s, session:%s")
            , header._opcodeId, header.ToString().c_str(), session->ToString().c_str());

            // 跳过这个包
            handledBytes += header._len;
            stream.ShiftReadPos(header._len);
            continue;
        }

        // 解密
        if((header._flags & MsgFlagsType::XOR_ENCRYPT_FLAG) == MsgFlagsType::XOR_ENCRYPT_FLAG)
        {
            if(header._keyLen == 0 || header._keyLen > leftLen)
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("bad key len:%u, header:%s, session:%s")
                            , header._keyLen, header.ToString().c_str(), session->ToString().c_str());
                errCode = Status::ParsingPacketFail;
                break;
            }

            // 加了一层base64
            UInt32 keyLen = header._keyLen;
            if((header._flags & MsgFlagsType::KEY_IN_BASE64_FLAG) == MsgFlagsType::KEY_IN_BASE64_FLAG)
            {
                UInt64 decodeLen = KERNEL_NS::LibBase64::CalcDecodeLen(streamCache.GetReadBegin(), header._keyLen);
                if(UNLIKELY(!KERNEL_NS::LibBase64::Decode(streamCache.GetReadBegin(), header._keyLen, streamCache.GetReadBegin(), decodeLen)))
                {
                    g_Log->NetWarn(LOGFMT_OBJ_TAG("base 64 decode fail key len:%u, header:%s, session:%s")
                                , header._keyLen, header.ToString().c_str(), session->ToString().c_str());
                    errCode = Status::ParsingPacketFail;
                    break;
                }

                keyLen = static_cast<UInt32>(decodeLen);
            }

            // 解出key
            KERNEL_NS::LibString plaintextKey;
            if(_parsingRsa.IsPubEncryptPrivDecrypt())
            {
                _parsingRsa.PrivateKeyDecrypt((const U8 *)streamCache.GetReadBegin(), keyLen, plaintextKey);

            }
            else
            {
                _parsingRsa.PubKeyDecrypt((const U8 *)streamCache.GetReadBegin(), keyLen, plaintextKey);
            }

            if(plaintextKey.empty())
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("decrypt key fail key len:%u, header:%s, session:%s")
                            , header._keyLen, header.ToString().c_str(), session->ToString().c_str());
                errCode = Status::ParsingPacketFail;
                break;
            }
            streamCache.ShiftReadPos(header._keyLen);
            leftLen -= header._keyLen;

            // 解密正文
            if(LIKELY(leftLen != 0))
            {
                KERNEL_NS::XorEncrypt::Decrypt(plaintextKey.data(), static_cast<Int32>(plaintextKey.size()), streamCache.GetReadBegin(), leftLen, streamCache.GetReadBegin());
            }
        }

        #ifdef _DEBUG
        const auto opcode = header._opcodeId;
        const auto headerLen = header._len;
        auto &&outputLogFunc = [opcode, headerLen, session](UInt64 costMs){
            g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(CrystalProtocolStack, "Decode over limit sessionId:%llu opcode:%u, %s, len:%u, costMs:%llu ms")
            ,  session->GetId(), opcode, StackOpcodeToString(opcode).c_str(), headerLen, costMs);
        };
            
        PERFORMANCE_RECORD_DEF(middlePr, outputLogFunc, 5);
        #endif

        // 4.创建编码器并解码
        auto coder = coderFactory->Create();
        if(LIKELY(leftLen != 0))
        {
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> safeDecode;
            safeDecode.Attach(const_cast<Byte8 *>(streamCache.GetReadBegin()), leftLen, 0, leftLen);
            if(!coder->Decode(safeDecode))
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("coder decode fail opcode:%d, header:%s, session:%s"), header._opcodeId, header.ToString().c_str(), session->ToString().c_str());
                errCode = Status::ParsingPacketFail;
                coder->Release();
                break;
            }

            streamCache.ShiftReadPos(leftLen);
        }

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

            if(_enableProtocolLog && IsNeedLog(header._opcodeId))
            {
                auto &localAddr = addr->GetLocalBriefAddr();
                auto &remoteAddr = addr->GetRemoteBriefAddr();
                std::string jsonString;
                coder->ToJsonString(&jsonString);
               g_Log->NetInfo(LOGFMT_OBJ_TAG("[RECV: session id:%llu, local:%s:%hu <= remote:%s:%hu, header:%s]\n[coder data]:\n%s")
               , session->GetId(), localAddr._ip.c_str(), localAddr._port, remoteAddr._ip.c_str(), remoteAddr._port
               , header.ToString().c_str(), jsonString.c_str()); 
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

            coder->Release();
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
    auto &&outputLogFunc = [](UInt64 costMs){
        g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(CrystalProtocolStack, "costMs:%llu ms"), costMs);
    };
        
    PERFORMANCE_RECORD_DEF(pr, outputLogFunc, 5);
    #endif
    
    Int32 errCode = Status::Success;
    handledBytes = 0;
    do
    {
        // 1.校验coder是否存在
        auto coder = packet->GetCoder();
        if(UNLIKELY(!coder))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("packet have no coder session:%s, packet:%s"), session->ToString().c_str(), StackPacketToString(packet).c_str());
            errCode = Status::Error;
            break;
        }

    #if _DEBUG 
        const auto opcode = packet->GetOpcode();
        const auto sessionId = packet->GetSessionId();
        auto &&outputLogFunc = [opcode, sessionId](UInt64 costMs){
            g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(CrystalProtocolStack, "sessionId:%llu, opcode:%d, %s, costMs:%llu ms.")
            , sessionId, opcode, StackOpcodeToString(opcode).c_str(), costMs);
        };
            
        PERFORMANCE_RECORD_DEF(middlePr, outputLogFunc, 5);
    #endif

        // 2.预留header空间
        stream->ShiftWritePos(MsgHeaderStructure::MSG_HEADER_SIZE);

        // 包特性
        UInt32 headerFlags = 0;

        // 加密
        const auto flags = GetProtoFlags(packet->GetOpcode());
        Int64 keySize = 0; 
        if(UNLIKELY((flags & MsgFlagsType::XOR_ENCRYPT_FLAG) == MsgFlagsType::XOR_ENCRYPT_FLAG))
        {// 需要加密
            if(!UpdateKey())
            {
                g_Log->Error(LOGFMT_OBJ_TAG("UpdateKey fail session:%s, packet:%s"), session->ToString().c_str(), StackPacketToString(packet).c_str());
                errCode = Status::Error;
                stream->ShiftWritePos(-(static_cast<Int64>(MsgHeaderStructure::MSG_HEADER_SIZE)));
                break;
            }

            headerFlags |= MsgFlagsType::XOR_ENCRYPT_FLAG;

            // // 先生成key
            // KERNEL_NS::CypherGeneratorUtil::SpeedGen<KERNEL_NS::_Build::TL>(key, KERNEL_NS::CypherGeneratorUtil::CYPHER_128BIT);

            // // rsa加密
            // auto &rsa = GetPacketToBinRsa();
            // KERNEL_NS::LibString cypherKey;

            // if(rsa.IsPubEncryptPrivDecrypt())
            // {
            //     rsa.PubKeyEncrypt(key, cypherKey);
            // }
            // else
            // {
            //     rsa.PrivateKeyEncrypt(key, cypherKey);
            // }
            // if(UNLIKELY(cypherKey.empty()))
            // {
            //     g_Log->Error(LOGFMT_OBJ_TAG("rsa encrypt fail rsa mode:%d session:%s, packet:%s"), rsa.GetMode(), session->ToString().c_str(), StackPacketToString(packet).c_str());
            //     errCode = Status::Error;
            //     stream->ShiftWritePos(-(static_cast<Int64>(MsgHeaderStructure::MSG_HEADER_SIZE)));
            //     break;
            // }

            // base64编码
            if(UNLIKELY((flags & MsgFlagsType::KEY_IN_BASE64_FLAG) == MsgFlagsType::KEY_IN_BASE64_FLAG))
            {
                headerFlags |= MsgFlagsType::KEY_IN_BASE64_FLAG;

                const auto &base64Key = GetBase64Key();
                keySize = static_cast<Int64>(base64Key.size());
                stream->Write(base64Key.data(), keySize);
            }
            else
            {
                const auto &key = GetCypherKey();
                keySize = static_cast<Int64>(key.size());
                stream->Write(key.data(), keySize);
            }
        }

        // 3.编码数据
        const auto contentStart = stream->GetWriteBytes();
        if(!coder->Encode(*stream))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("coder encode fail have no coder, packet:%s, session:%s"), StackPacketToString(packet).c_str(), session->ToString().c_str());
            errCode = Status::CoderFail;
            stream->ShiftWritePos(-(static_cast<Int64>(MsgHeaderStructure::MSG_HEADER_SIZE + keySize)));
            break;
        }
        const auto contentEnd = stream->GetWriteBytes();
        const auto contentSize = contentEnd - contentStart;

        // 加密数据
        const auto &key = GetKey();
        if(UNLIKELY(!key.empty()))
        {
            if((contentSize > 0) && ((flags & MsgFlagsType::XOR_ENCRYPT_FLAG) == MsgFlagsType::XOR_ENCRYPT_FLAG))
            {
                stream->ShiftWritePos(-(contentSize));
                KERNEL_NS::XorEncrypt::Encrypt(key.data(), static_cast<Int32>(key.size()), stream->GetWriteBegin(), static_cast<Int32>(contentSize), stream->GetWriteBegin());
                stream->ShiftWritePos(contentSize);
            }
        }

        // 4.长度限制(不能超过最大长度)
        if((MsgHeaderStructure::MSG_HEADER_SIZE + keySize + contentSize) > MsgHeaderStructure::MSG_BODY_MAX_SIZE_LIMIT)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("coder encode fail content size over limit content size:%lld, limit:%d, packet:%s, session:%s")
                            ,contentSize, MsgHeaderStructure::MSG_BODY_MAX_SIZE_LIMIT, StackPacketToString(packet).c_str(), session->ToString().c_str());
            stream->ShiftWritePos(-(contentSize));
            stream->ShiftWritePos(-(static_cast<Int64>(MsgHeaderStructure::MSG_HEADER_SIZE + keySize)));

            errCode = Status::CoderFail;
            break;
        }

        // 有包内容大小限制
        const auto sessionPacketContentLimit = session->GetOption()._sessionSendPacketContentLimit;
        if(sessionPacketContentLimit && contentSize > static_cast<Int64>(sessionPacketContentLimit))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("coder encode fail packet content size over session packet content limit, content size:%lld, sessionType:%d, limit:%llu, packet:%s, session:%s")
            , contentSize, session->GetSessionType(), sessionPacketContentLimit, StackPacketToString(packet).c_str(), session->ToString().c_str());
            errCode = Status::CoderFail;
            stream->ShiftWritePos(-(contentSize));
            stream->ShiftWritePos(-(static_cast<Int64>(MsgHeaderStructure::MSG_HEADER_SIZE + keySize)));
            break;
        }

        // 5.编码包头
        CrystalMsgHeader header;
        // header._len = contentSize;
        header._len = static_cast<UInt32>(MsgHeaderStructure::MSG_HEADER_SIZE + keySize + contentSize);
        header._protocolVersion = GetProtoVersionNumber();

        // 6.TODO:包特性暂时不设置 
        header._flags = headerFlags;
        header._opcodeId = packet->GetOpcode();
        header._packetId = packet->GetPacketId();
        header._keyLen = static_cast<UInt32>(keySize);

        // 7.往回拨len字节写入header
        stream->ShiftWritePos(-static_cast<Int64>(header._len));
        stream->Write(&header._len, MsgHeaderStructure::LEN_SIZE);
        stream->Write(&header._protocolVersion, MsgHeaderStructure::PROTOCOL_VERSION_SIZE);
        stream->Write(&header._flags, MsgHeaderStructure::FLAGS_SIZE);
        stream->Write(&header._opcodeId, MsgHeaderStructure::OPCODE_SIZE);
        stream->Write(&header._packetId, MsgHeaderStructure::PACKET_ID_SIZE);
        stream->Write(&header._keyLen, MsgHeaderStructure::KEY_LEN_SIZE);

        // 8.写入完成跳过包数据
        stream->ShiftWritePos(keySize + contentSize);

        if(_enableProtocolLog && IsNeedLog(header._opcodeId))
        {
            auto sock = session->GetSock();
            auto addr = sock->GetAddr();
            auto &localAddr = addr->GetLocalBriefAddr();
            auto &remoteAddr = addr->GetRemoteBriefAddr();
            std::string jsonString;
            coder->ToJsonString(&jsonString);

            g_Log->NetInfo(LOGFMT_OBJ_TAG("[SEND: session id:%llu, local:%s:%hu => remote:%s:%hu, header:%s]\n[coder data]:\n%s")
            , session->GetId(), localAddr._ip.c_str(), localAddr._port, remoteAddr._ip.c_str(), remoteAddr._port
            , header.ToString().c_str(), jsonString.c_str()); 
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


SERVICE_COMMON_END
