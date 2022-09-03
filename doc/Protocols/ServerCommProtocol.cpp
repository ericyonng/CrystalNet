/**
 * @file    ServerCommProtocol.cpp
 * @author  Longwei Lai<lailongwei@126.com>
 * @date    2018/03/02
 * @brief   �������ڲ�����ʹ�õ�Э��ջʵ��
 */

#include "Common/pch.h"

#include "Common/Macro.h"
#include "Common/Status.h"
#include "Common/Packets/Packets.h"
#include "Common/NameEnums/ServerNames.h"
#include "Common/Application/GameApplication.h"

#include "Common/Facades/CommonFacadeObjs.h"
#include "Common/Facades/GameLogController/GameLogController.h"

#include "Common/Protocols/ServerCommProtocol.h"

// ServerCommProtocol_Packʵ��
ServerCommProtocol_Pack::ServerCommProtocol_Pack()
: _headerAssembler(ServerCommProtocolHeaderParts::Header_Size)
{
    _packet = NULL;
    _payloadNeedRecv = 0;
    _payloadRecved = 0;
    
    const GameApplication &app = *LLBC_IApplication::ThisApp<GameApplication>();
    _serverName = app.GetName();
    _isGateway = _serverName == ServerNames::Gateway;

    _coders = NULL;
}

ServerCommProtocol_Pack::~ServerCommProtocol_Pack()
{
    LLBC_XRecycle(_packet);
}

int ServerCommProtocol_Pack::Send(void *in, void *&out, bool &removeSession)
{
    LLBC_Packet *packet = reinterpret_cast<LLBC_Packet *>(in);
    LogPacketInfo(true, ___FUNCTION__, packet);

    return Base::Send(in, out, removeSession);
}

int ServerCommProtocol_Pack::Recv(void *in, void *&out, bool &removeSession)
{
    out = NULL;
    LLBC_MessageBlock *block = reinterpret_cast<LLBC_MessageBlock *>(in);

    LLBC_InvokeGuard guard(this, &ServerCommProtocol_Pack::_DelRecvBlock, block);

    size_t readableSize;
    while ((readableSize = block->GetReadableSize()) > 0)
    {
        // ���
        const char *readableBuf = reinterpret_cast<const char *>(block->GetDataStartWithReadPos());
        if (!_packet)
        {
            size_t headerUsed;
            if (!_headerAssembler.Assemble(readableBuf, readableSize, headerUsed)) // If header recv not done, return.
                return LLBC_OK;

            _packet = _pktPoolInst->GetObject();
            _headerAssembler.SetToPacket(*_packet);
            _packet->SetSessionId(_sessionId);
            
            const size_t packetLen = _packet->GetLength();
            if (packetLen < ServerCommProtocolHeaderParts::Header_Size)
            {
                _stack->Report(this,
                    LLBC_ProtoReportLevel::Error,
                    LLBC_String().format("invalid packet len: %lu", _packet->GetLength()));

                _headerAssembler.Reset();

                LLBC_XRecycle(_packet);
                _payloadNeedRecv = 0;

                _DelPacketList(out);

                removeSession = true;
                LLBC_SetLastError(LLBC_ERROR_PACK);
                return LLBC_FAILED;
            }

            _payloadNeedRecv = packetLen - ServerCommProtocolHeaderParts::Header_Size;

            _headerAssembler.Reset();

            readableBuf += headerUsed;
            readableSize -= headerUsed;

            block->ShiftReadPos(headerUsed);
        }

        size_t contentNeedRecv = _payloadNeedRecv - _payloadRecved;
        if (readableSize < contentNeedRecv)
        {
            _packet->Write(readableBuf, readableSize);
            _payloadRecved += readableSize;
            return LLBC_OK;
        }

        _payloadRecved = 0;
        _payloadNeedRecv = 0;
        _packet->Write(readableBuf, contentNeedRecv);
        block->ShiftReadPos(contentNeedRecv);
        

        // DebugLog
        LogPacketInfo(false, ___FUNCTION__, _packet);
        

        // ��Gateway�߼�: ��������, ����עBindToSvrCheck
        if (!_isGateway)
        {
            if (!out)
                out = LLBC_New1(LLBC_MessageBlock, sizeof(LLBC_Packet *));
            (reinterpret_cast<LLBC_MessageBlock *>(out))->Write(&_packet, sizeof(LLBC_Packet *));

            _packet = NULL;

            continue;
        }
        

        // Gateway�߼�: ת���ͻ��˻�Gateway
        // �粻�󶨵�Gateway�Ұ���extData1, ���͸��ͻ���
        if (!_coders)
        {
            const LLBC_IProtocol *coderProtocol = _stack->GetCoderProtocol();
            _coders = coderProtocol->GetCoders();
        }

        const int opcode = _packet->GetOpcode();
        const int extData1 = static_cast<int>(_packet->GetExtData1());
        const bool boundToGateway = BindToSvrCheck::OpcodeCheck(_serverName, opcode);
        if (!boundToGateway && extData1 != 0)
        {
            _packet->SetSessionId(static_cast<int>(_packet->GetExtData1()));
            GetService()->Send(_packet);

            _packet = NULL;

            continue;
        }
        

        // �󶨵�Gateway, ���͸�Gateway
        // ��󶨵�Gateway��PendingSend, ���ƶ�Ӧ��client protocol stack, pendingSend
        const bool pendingSend = PacketAnnotation::IsPacketPendingSend(opcode);
        if (pendingSend)
        {
            const int &clientSessionId = extData1;
            GetService()->CtrlProtocolStack(clientSessionId, ProtoStackCtrlType::BeginPendingSendPacket, LLBC_Variant::nil);
        }

        // �������͸�Gateway
        if (!out)
            out = LLBC_New1(LLBC_MessageBlock, sizeof(LLBC_Packet *));
        (reinterpret_cast<LLBC_MessageBlock *>(out))->Write(&_packet, sizeof(LLBC_Packet *));

        _packet = NULL;
        

        
    }

    return LLBC_OK;
}

void ServerCommProtocol_Pack::_DelRecvBlock(void *block)
{
    LLBC_Recycle(reinterpret_cast<LLBC_MessageBlock *>(block));
}

void ServerCommProtocol_Pack::_DelPacketList(void *packetList)
{
    if (!packetList)
        return;

    LLBC_MessageBlock *block = reinterpret_cast<LLBC_MessageBlock *>(packetList);

    LLBC_Packet *packet;
    while (block->Read(&packet, sizeof(LLBC_Packet *)) == LLBC_OK)
        LLBC_Recycle(packet);

    LLBC_Recycle(block);
}


// ServerCommProtocol_Codecʵ��
ServerCommProtocol_Codec::ServerCommProtocol_Codec()
{
}

ServerCommProtocol_Codec::~ServerCommProtocol_Codec()
{
}

int ServerCommProtocol_Codec::Send(void* in, void*& out, bool& removeSession)
{
    LLBC_Packet *packet = reinterpret_cast<LLBC_Packet *>(in);
    LogPacketInfo(true, __FUNCTION__, packet);

    return LLBC_CodecProtocol::Send(in, out, removeSession);
}

int ServerCommProtocol_Codec::Recv(void *in, void *&out, bool &removeSession)
{
    LLBC_Packet *packet = reinterpret_cast<LLBC_Packet *>(in);

    LogPacketInfo(false, __FUNCTION__, packet);

    if (packet->GetStatus() != Status::Success && packet->GetPayloadLength() == 0)
    {//payloadΪ�վ�û��Ҫִ��Codec
        out = packet;
        return LLBC_OK;
    }

    return LLBC_CodecProtocol::Recv(in, out, removeSession);
}


// Gateway�ڲ��ڵ�ͨ��Э��ջ������
LLBC_IProtocol *ServerCommProtocolFactory::Create(int layer) const
{
    switch (layer)
    {
    case LLBC_ProtocolLayer::CodecLayer: // CodecЭ���ʹ��llbc������Դ���Э���ʵ��
    {
        const auto &serverName = LLBC_IApplication::ThisApp<GameApplication>()->GetName();
        if (serverName == ServerNames::GameServer)
            return new GSServerCommProtocol_Codec();
        else
            return new ServerCommProtocol_Codec();
    }
    case LLBC_ProtocolLayer::CompressLayer: // CompressЭ���ʹ��llbc������Դ���Э���ʵ��
        return new LLBC_CompressProtocol();

    case LLBC_ProtocolLayer::PackLayer: // ��Ϸ����Pack��ʵ��
        return new ServerCommProtocol_Pack();

    default:
        return NULL;
    }
}


// GameServer CodecЭ��ջ
int GSServerCommProtocol_Codec::Recv(void *in, void *&out, bool &removeSession)
{
    LLBC_Packet *packet = reinterpret_cast<LLBC_Packet *>(in);

    LogPacketInfo(false, __FUNCTION__, packet);

    if (packet->GetStatus() != Status::Success && packet->GetPayloadLength() == 0)
    {//payloadΪ�վ�û��Ҫִ��Codec
        out = packet;
        return LLBC_OK;
    }

    int gwSessionId = packet->GetSessionId();
    int clientsessionId = static_cast<int>(packet->GetExtData1());
    int ret = LLBC_CodecProtocol::Recv(in, out, removeSession);
    if (ret != LLBC_OK)
    {
        // ֪ͨGatewayЭ�����ʧ��
        int errCode = LLBC_GetLastError();
        if (errCode == LLBC_ERROR_DECODE)
        {
            removeSession = false;

            auto leaveRes = new LeaveSceneRes();
            leaveRes->set_needsendkickoutnty(true);
            leaveRes->set_kickouttype(KickoutType::ProtobufDecodeError);

            auto leaveResPkt = _pktPoolInst->GetObject();
            leaveResPkt->SetSessionId(gwSessionId);
            leaveResPkt->SetOpcode(Opcodes::LeaveSceneRes);
            leaveResPkt->SetEncoder(leaveRes);
            leaveResPkt->SetExtData1(clientsessionId);

            GetService()->Send(leaveResPkt);

            LLBC_SetLastError(LLBC_ERROR_DECODE);
        }
    }

    return ret;
}

