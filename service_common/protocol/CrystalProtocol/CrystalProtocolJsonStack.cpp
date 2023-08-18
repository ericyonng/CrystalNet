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
 * Date: 2023-07-15 17:00:46
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service_common/protocol/CrystalProtocol/CrystalProtocolJsonStack.h>
#include "service_common/protocol/CrystalProtocol/CrystalMsgHeader.h"


SERVICE_COMMON_BEGIN

CrystalProtocolJsonStack::~CrystalProtocolJsonStack()
{
}

Int32 CrystalProtocolJsonStack::ParsingPacket(KERNEL_NS::LibSession *session
, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream
, UInt64 &handledBytes
, UInt64 &packetCount
, KERNEL_NS::LibList<KERNEL_NS::LibList<KERNEL_NS::LibPacket *> *, KERNEL_NS::_Build::TL> *&recvPacketsBatch)
{
    #ifdef _DEBUG
    auto &&outputLogFunc = [](UInt64 costMs){
        g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(CrystalProtocolJsonStack, "costMs:%llu ms"), costMs);
    };
        
    PERFORMANCE_RECORD_DEF(pr, outputLogFunc, 10);
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
            if(streamCache.GetReadableSize() < MsgHeaderStructure::LEN_SIZE)
            {
                // g_Log->NetDebug(LOGFMT_OBJ_TAG("data not reach parsing header size session:%s, stream readable size:%llu")
                //                 , session->ToString().c_str(), streamCache.GetReadableSize());
                errCode = Status::Success;
                break;
            }

            // 格式:2字节长度 + json字符串数据
            streamCache.Read(&header._len, MsgHeaderStructure::LEN_SIZE);

            if(header._len > MsgHeaderStructure::MSG_BODY_MAX_SIZE_LIMIT)
            {
                g_Log->NetWarn(LOGFMT_OBJ_TAG("bad msg:len over limit limit:%d, header:%s, session:%s")
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
        }

        // 2.剩余包数据是否够解析包体
        if(streamCache.GetReadableSize() < (header._len - MsgHeaderStructure::LEN_SIZE))
        {
            // g_Log->NetDebug(LOGFMT_OBJ_TAG("data not reach msg body data len session:%s, stream readable size:%lld, msg len:%u")
            //     , session->ToString().c_str(), streamCache.GetReadableSize(), (header._len - static_cast<UInt32>(MsgHeaderStructure::MSG_HEADER_SIZE)));
            errCode = Status::Success;
            break;
        }

        // 4.创建编码器并解码
        KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> safeDecode;
        const Int64 msgBodySize = static_cast<Int64>(header._len - MsgHeaderStructure::LEN_SIZE);
        safeDecode.Attach(const_cast<Byte8 *>(streamCache.GetReadBegin()), msgBodySize, 0, msgBodySize);
        KERNEL_NS::LibString dataStr;
        dataStr.AppendData(safeDecode.GetReadBegin(), static_cast<Int64>(msgBodySize));

        const auto &jsonObj = nlohmann::json::parse(dataStr.c_str(), NULL, false);
        const auto isJson = jsonObj.is_object();

        g_Log->NetInfo(LOGFMT_OBJ_TAG("session id:%llu, addr:%s recv data len:%u, data:\n%s, is json:%d")
        , session->GetId(), session->GetSock()->GetAddr()->ToString().c_str(), msgBodySize, dataStr.c_str(), isJson);


        // const auto &jsonString = nlohmann::json::parse(dataStr.c_str(), NULL, false);
        // if(!jsonString.is_object())
        // {
        //     g_Log->NetWarn(LOGFMT_OBJ_TAG("json parse fail header len:%u session id:%llu, session addr:%s")
        //             , header._len, session->GetId(), session->GetSock()->GetAddr()->ToString().c_str());
        // }

        streamCache.ShiftReadPos(msgBodySize);

        // 5.解码成功一个包
        handledBytes += header._len;
        ++packetCount;
        stream.ShiftReadPos(header._len);
    }

    // g_Log->NetDebug(LOGFMT_OBJ_TAG("parsing packet finish errCode:%d, handledBytes:%llu, parsedPacket count:%llu session:%s")
    //                 , errCode, handledBytes, parsedPacket ? parsedPacket->GetAmount() : 0, session->ToString().c_str());

    return errCode;
}

Int32 CrystalProtocolJsonStack::PacketsToBin(KERNEL_NS::LibSession *session
, KERNEL_NS::LibPacket *packet
, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *stream
, UInt64 &handledBytes)
{
    // g_Log->NetInfo(LOGFMT_OBJ_TAG("packet to bin end errCode:%d, handledBytes:%llu session:%s"), errCode, handledBytes, session->ToString().c_str());
    return Status::Failed;
}

KERNEL_NS::ICoderFactory *CrystalProtocolJsonStack::GetCoderFactory(Int32 opcode)
{
    return NULL;
}

void CrystalProtocolJsonStack::RegisterCoderFactory(Int32 opcode, KERNEL_NS::ICoderFactory *factory)
{
    factory->Release();
}

SERVICE_COMMON_END
