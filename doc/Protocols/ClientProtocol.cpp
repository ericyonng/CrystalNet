/**
 * @file    ClientProtocol.cpp
 * @author  Longwei Lai<lailongwei@126.com>
 * @date    2018/03/02
 * @brief   �ͻ���ͨ��Э��ջʵ��
 */

#include "Common/pch.h"

#include "Common/Macro.h"
#include "Common/Log/LogIndexes.h"

#include "Common/Crypto/Aes.h"
#include "Common/Crypto/Xor.h"
#include "Common/Crypto/lz4.h"

#include "Common/Event/EventIds.h"
#include "Common/Event/Params.h"
#include "Common/Event/EventMgr.h"

#include "Common/Packets/Packets.h"

#include "Common/Facades/CommonFacadeObjs.h"
#include "Common/Facades/GameLogController/GameLogController.h"

#include "Common/NameEnums/ServerNames.h"
#include "Common/Application/GameApplication.h"

#include "Common/Protocols/ClientProtocol.h"

sint32 ClientProtocol_Pack::_gsSessionId = 0;
sint32 ClientProtocol_Pack::_msSessionId = 0;
LLBC_ListenerStub ClientProtocol_Pack::_eventStub;

// ClientProtocol_Packʵ��
ClientProtocol_Pack::ClientProtocol_Pack()
{
    ::memset(_header, 0, sizeof(_header));
    _headerRecved = 0;

    _packet = NULL;
    _payloadNeedRecv = 0;
    _payloadRecved = 0;

    _clientCoders = NULL;

    _status = This::NotEnterScene;

    const GameApplication &app = *LLBC_IApplication::ThisApp<GameApplication>();
    _serverName = app.GetName();
    _isLoginServer = _serverName == ServerNames::LoginServer;
    _shiftedGlobalNodeId = static_cast<uint64>(app.GetGlobalNodeId()) << 32;

    _aesCrypto = app.GetAesKey();
    _xorUpCrypto = app.GetXorUpKey();
    _xorDownCrypto = app.GetXorDownKey();

    _crypt = new AESCrypto(_aesCrypto);

    _pendingSendPacket = 0;
    _pendingSendBlock = NULL;
}

ClientProtocol_Pack::~ClientProtocol_Pack()
{
    LLBC_STLHelper::RecycleContainer(_pendingSendPackets, false, false);

    LLBC_XRecycle(_packet);
    LLBC_XDelete(_crypt);
}

int ClientProtocol_Pack::GetLayer() const
{
    return LLBC_ProtocolLayer::PackLayer;
}

int ClientProtocol_Pack::Connect(LLBC_SockAddr_IN &local, LLBC_SockAddr_IN &peer)
{
    // ����connect�����⴦��
    return LLBC_OK;
}

int ClientProtocol_Pack::Send(void *in, void *&out, bool &removeSession)
{
    LLBC_Packet *packet = reinterpret_cast<LLBC_Packet *>(in);
    const int opcode = packet->GetOpcode();

    // Log
    LogPacketInfo(true, __FUNCTION, packet);

    // �������PendingSend״̬, ��������
    const bool isPendingSendPkt = PacketAnnotation::IsPacketPendingSend(opcode);
    if (_pendingSendPacket &&
        (!isPendingSendPkt || (isPendingSendPkt && _pendingSendPacket != 1)))
    {
        _pendingSendPackets.push_back(packet);
        out = NULL;

        return LLBC_OK;
    }

    // ���ܱ�ʶ����
    // TODO: �߼�����, ���ܱ�ʶ����ʵ�ʼ����߼�����
    if (opcode == Opcodes::AuthRes || opcode == Opcodes::EnterSceneRes)
        packet->SetFlags(packet->GetFlags() | ClientProtocolFlags::AesCrypt); // ����AES���ܱ�ʶ
    //else
        //packet->SetFlags(packet->GetFlags() | ClientProtocolFlags::XorCrypt); // ����������
    

    // ���ȼ��
    size_t length = ClientProtocolHeaderParts::Header_Size + packet->GetPayloadLength();
    if (length > ClientProtocolHeaderParts::MaxPacketLen)
    {
        GetStack()->Report(this,
            LLBC_ProtoReportLevel::Error,
            LLBC_String().format(
            "Packet[%d] data too long after encoded, length: %lu(>0x00ffffff), send failed!", packet->GetOpcode(), length));

        LLBC_Recycle(packet);

        removeSession = true;
        LLBC_SetLastError(LLBC_ERROR_PACK);

        return LLBC_FAILED;
    }
    

    // Opcode/Status/Flags���
    // Opcode���
    if (!_CheckSendHeaderField(packet, opcode, ClientProtocolHeaderParts::MaxOpcode, "opcode"))
    {
        removeSession = true;
        return LLBC_FAILED;
    }
    // Status���
    const int status = packet->GetStatus();
    if (!_CheckSendHeaderField(packet, status, ClientProtocolHeaderParts::MaxStatus, "status"))
    {
        removeSession = true;
        return LLBC_FAILED;
    }
    // Flags���
    int flags = packet->GetFlags();
    if (!_CheckSendHeaderField(packet, flags, ClientProtocolHeaderParts::MaxFlags, "flags"))
    {
        removeSession = true;
        return LLBC_FAILED;
    }
    

    // �Ự״̬���
    if (opcode == Opcodes::ServerEnterSceneFinishNty)
    {
        if (status == 0)
            _status = This::EnteredScene;
        else
            _status = This::EnteringScene;
    }
    else if (opcode == Opcodes::LeaveSceneRes)
    {
        if (status == 0)
            _status = This::LeavedScene;
        else
            _status = This::NotEnterScene;
    }
    

    // ����MessageBlock, д���ͷ
    LLBC_MessageBlock *block = new LLBC_MessageBlock(length);
    const size_t begWrigePos = block->GetWritePos();
    block->Write(&length, ClientProtocolHeaderParts::Length_Size);
    block->Write(&flags, ClientProtocolHeaderParts::Flags_Size);
    block->Write(&opcode, ClientProtocolHeaderParts::Opcode_Size);
    block->Write(&status, ClientProtocolHeaderParts::Status_Size);
    

    // д�����
    const void *payloadPtr = packet->GetPayload();
    if (payloadPtr)
    {
        // ����&ѹ��
        // TODO: ��Ҫ�Ż�, ������ʱLLBC_String���󴴽�
        LLBC_String enStr;
        if (opcode == Opcodes::AuthRes || 
            opcode == Opcodes::EnterSceneRes) // TODO: �ж���Ҫ��ȡPacketAnnotation
        {// ִ��AES����
            uint8 extendLen;
            _crypt->Encrypt(payloadPtr, packet->GetPayloadLength(), extendLen, enStr);
            block->Write(&extendLen, sizeof(uint8));
        }
        else
        {//������
            //EnXor(payloadPtr, packet->GetPayloadLength(), _xorDownCrypto, enStr);
            enStr.append(static_cast<const char *>(packet->GetPayload()), packet->GetPayloadLength());
        }

        // TODO: �߼�����, ѹ����ʶ����ѹ������
        const bool bcompress = _Compress(enStr);
        

        // д����嵽Block
        // д�����
        block->Write(enStr.c_str(), enStr.size());

        // �������ð�������Ϊ���ܺ��п��ܻ�߳�
        const size_t curWritePos = block->GetWritePos();
        block->SetWritePos(begWrigePos);
        size_t pktSize = curWritePos - begWrigePos;
        block->Write(&pktSize, ClientProtocolHeaderParts::Length_Size);
        if (bcompress) // TODO: �߼�����, ѹ����ʶ����ѹ������
        {
            flags = flags | ClientProtocolFlags::Compressed; //����ѹ����ʶ
            block->Write(&flags, ClientProtocolHeaderParts::Flags_Size);
        }

        block->SetWritePos(curWritePos);
        
    }
    

    // ɾ��packet
    LLBC_Recycle(packet);

    // ����˰�ΪpendingSendPkt, ����PendingSend
    if (isPendingSendPkt)
    {
        _pendingSendBlock = block;
        const bool ctrlRet = _Ctrl_EndPendingSendPacket(LLBC_Variant::nil, removeSession);
        _pendingSendBlock = NULL;

        if (!ctrlRet)
            return LLBC_FAILED;
    }

    out = block;

    return LLBC_OK;
}

int ClientProtocol_Pack::Recv(void *in, void *&out, bool &removeSession)
{
    out = NULL;
    LLBC_MessageBlock *block = reinterpret_cast<LLBC_MessageBlock *>(in);

    // Log
    GAME_LOG_CTRL_BEGIN(GameLogControlType::DetailPktLog)
    Log.d4<ClientProtocol_Pack>("net", "Client session[%d] received data(len:%lu):\n%s",
                                _sessionId,
                                block->GetReadableSize(),
                                LLBC_Byte2Hex(block->GetDataStartWithReadPos(), block->GetReadableSize(), 16).c_str());
    GAME_LOG_CTRL_END()

    // ����blockɾ���ػ���ȷ��block������pack.
    LLBC_InvokeGuard guard(this, &ClientProtocol_Pack::_DelRecvBlock, block);

    size_t readableSize;
    while ((readableSize = block->GetReadableSize()) > 0)
    {
        const char *buf = reinterpret_cast<const char *>(block->GetDataStartWithReadPos());

        // ���հ�ͷ
        if (!_packet)
        {
            size_t headerNeed = sizeof(_header) - _headerRecved;
            if (readableSize < headerNeed)
            {
                ::memcpy(_header +_headerRecved, buf, readableSize);
                _headerRecved += readableSize;

                return LLBC_OK;
            }
            else
            {
                ::memcpy(_header + _headerRecved, buf, headerNeed);
                _headerRecved += headerNeed;
            }

            // ��д��ͷ��Ϣ��Packet
            _packet = _pktPoolInst->GetObject();
            _SetHeaderToPacket(_packet);

            // ����_headerRecved
            _headerRecved = 0;

            // Shift buf������readableSize
            const size_t packetLen = _packet->GetLength();
            if (packetLen < ClientProtocolHeaderParts:: Header_Size || 
                    packetLen > ClientProtocolHeaderParts::MaxPacketLen) // packet length�Ϸ����ж�
            {
                GetStack()->Report(this,
                    LLBC_ProtoReportLevel::Error,
                    LLBC_String().format("Invalid packet while recving packet, length: %lu", _packet->GetLength()));

                _ResetRecvInfos();
                _DelPacketList(out);

                removeSession = true;
                LLBC_SetLastError(LLBC_ERROR_PACK);

                return LLBC_FAILED;
            }

            buf += headerNeed;
            readableSize -= headerNeed;
            block->ShiftReadPos(headerNeed);

            _payloadRecved = 0;
            _payloadNeedRecv = packetLen - ClientProtocolHeaderParts::Header_Size;
        }

        // ���հ���
        size_t contentNeed = _payloadNeedRecv - _payloadRecved;
        if (readableSize < contentNeed)
        {
            _packet->Write(buf, readableSize);
            _payloadRecved += readableSize;
            return LLBC_OK;
        }

        _packet->Write(buf, contentNeed);
        _payloadRecved = 0;
        _payloadNeedRecv = 0;

        block->ShiftReadPos(contentNeed);

        sint32 opcode = _packet->GetOpcode();

        // Net log
        LogPacketInfo(false, __FUNCTION__, _packet);

        // ����
        _Decrypt();
        if (opcode == Opcodes::EnterSceneReq ||
            opcode == Opcodes::ReconnectReq)
            _status = This::EnteringScene;
        else if (opcode == Opcodes::LeaveSceneReq)
            _status = This::LeavingScene;

        //״̬����ֱ�ӹر�����
        sint32 requireStatus = PacketAnnotation::GetRequireStatus(opcode);
        if (_status != requireStatus)
        {
            GetStack()->Report(this,
                LLBC_ProtoReportLevel::Error,
                LLBC_String().format("Invalid opcode %d status %d required status %d while recving packet",
                opcode, _status, requireStatus));

            _ResetRecvInfos();
            _DelPacketList(out);

            removeSession = true;
            LLBC_SetLastError(LLBC_ERROR_PACK);

            return LLBC_FAILED;
        }

        if (!_clientCoders)
        {
            const LLBC_IProtocol *coderProtocol = _stack->GetCoderProtocol();
            _clientCoders = coderProtocol->GetCoders();
        }
        Coders::const_iterator it = _clientCoders->find(opcode);
        //��ʱLoginServer������ת��
        if (_isLoginServer || it != _clientCoders->end())
        {//���ڵ���ע��
            if (!out)
                out = new LLBC_MessageBlock(sizeof(LLBC_Packet *));
            (reinterpret_cast<LLBC_MessageBlock *>(out))->Write(&_packet, sizeof(LLBC_Packet *));
            _packet = NULL;
            continue;
        }

        //ֻ��Gateway�Ż�ִ��ת���ж�
        bool needRecycle = true;
        sint32 sessionId = _packet->GetSessionId();
        uint64 exdata = _shiftedGlobalNodeId | sessionId;
        _packet->SetExtData1(exdata);
        if (_gsSessionId && BindToSvrCheck::OpcodeCheck(ServerNames::GameServer, opcode))
        {//ת����GameServer
            _packet->SetSessionId(_gsSessionId);
            needRecycle = false;
        }
        else if (_msSessionId && BindToSvrCheck::OpcodeCheck(ServerNames::MiscServer, opcode))
        {//ת����MiscServer
            _packet->SetSessionId(_msSessionId);
            needRecycle = false;
        }

        if (!needRecycle)
        {
            _svc->Send(_packet);
            _packet = NULL;
        }
        else
        {
            if (!out)
                out = new LLBC_MessageBlock(sizeof(LLBC_Packet *));
            (reinterpret_cast<LLBC_MessageBlock *>(out))->Write(&_packet, sizeof(LLBC_Packet *));
            _packet = NULL;
        }
    }

    return LLBC_OK;
}

bool ClientProtocol_Pack::Ctrl(int ctrlType, const LLBC_Variant &ctrlData, bool &removeSession)
{
    switch (ctrlType)
    {
    case ProtoStackCtrlType::BeginPendingSendPacket:
        return _Ctrl_BeginPendingSendPacket(ctrlData, removeSession);

    case ProtoStackCtrlType::EndPendingSendPacket:
        return _Ctrl_EndPendingSendPacket(ctrlData, removeSession);

    default:
        Log.je2(__FUNCTION__).Finish("Unknown ctrl type:%d", ctrlType);
        return true;
    }
}

void ClientProtocol_Pack::_Decrypt()
{
    // TODO: ��Ҫ�Ż�, �����copy��LLBC_String�ֲ����󴴽�/����
    const void *payloadPtr = _packet->GetPayload();
    bool bcrypt = false;
    LLBC_String deStr;

    size_t payloadLen = _packet->GetPayloadLength();
    if (_packet->HasFlags(ClientProtocolFlags::AesCrypt) && payloadPtr)
    {//ִ��AES����
        uint8 extendLen = 0;
        _packet->Read(extendLen);
        _crypt->Decrypt(reinterpret_cast<const char *>(payloadPtr) + 1, payloadLen - 1, extendLen, deStr);
        bcrypt = true;
    }
    else if (_packet->HasFlags(ClientProtocolFlags::XorCrypt) && payloadPtr)
    {//ִ��������
        DeXor(payloadPtr, payloadLen, _xorUpCrypto, deStr);
        bcrypt = true;
    }

    if (bcrypt)
    {
        _packet->ResetPayload();
        // ����д�뵽payload
        _packet->Write(deStr.c_str(), deStr.size());
    }
}

LLBC_THREAD_LOCAL char ClientProtocol_Pack::_compressBuff[LZ4_COMPRESS_BUFF_SIZE] = { 0 };

bool ClientProtocol_Pack::_Compress(LLBC_String &compressStr)
{
    size_t compressLen = compressStr.size();
    // ����̫С�Ͳ�ѹ����
    if (compressLen < LZ4_MIN_SIZE_FOR_COMPRESS)
        return false;

    int dstSize = LZ4_compressBound(static_cast<int>(compressLen));
    if (dstSize >= LZ4_COMPRESS_BUFF_SIZE)
    {
        Log.w2<ClientProtocol_Pack>("Data too long for compress, length: %d > 65536 compressLen %u", dstSize, compressLen);
        return false;
    }

    int csz = LZ4_compress_default(compressStr.c_str(), _compressBuff, compressLen, LZ4_COMPRESS_BUFF_SIZE);
    if (csz == 0)
    {
        Log.e2<ClientProtocol_Pack>("Data compress fail, length: %u", compressLen);
        return false;
    }

    compressStr.clear();
    compressStr.append(_compressBuff, csz);

    return true;
}

bool ClientProtocol_Pack::_Ctrl_BeginPendingSendPacket(const LLBC_Variant &ctrlData, bool &removeSession)
{
    ++_pendingSendPacket;
    return true;
}

bool ClientProtocol_Pack::_Ctrl_EndPendingSendPacket(const LLBC_Variant &ctrlData, bool &removeSession)
{
    // --Pending send packet
    _pendingSendPacket = MAX(0, _pendingSendPacket - 1);

    // �绹����Pending״̬ �� �޻�ѹPackets, ����
    if (_pendingSendPacket != 0 || _pendingSendPackets.empty())
        return true;

    // ���������Protocol Send
    LLBC_MessageBuffer blocks;
    auto session = GetStack()->GetSession();
    for (size_t i = 0; i < _pendingSendPackets.size(); ++i)
    {
        void *block = NULL;
        removeSession = false;
        int opcode = _pendingSendPackets[i]->GetOpcode();
        if (Send(_pendingSendPackets[i], block, removeSession) != LLBC_OK)
        {
            // ��ʽ����������
            LLBC_String errDesc;
            errDesc.format("End pending send packet failed, call ClientProtocol::Send() failed to send packet(%d) failed, error:%s",
                           opcode, LLBC_FormatLastError());

            // Log
            Log.je2(__FUNCTION__).Add(LI_SessionId, session->GetId()).Finish("%s", errDesc.c_str());

            // ɾ��ʣ��packets
            ++i;
            for (; i < _pendingSendPackets.size(); ++i)
                LLBC_Recycle(_pendingSendPackets[i]);
            _pendingSendPackets.clear();

            // ����_pendingSendBlock �� Send������δҪ���Ƴ�, ����Ƴ��Ự
            if (!_pendingSendBlock && !removeSession)
                removeSession = true;

            // ���ػỰ
            return false;
        }

        // �ϲ�׷�ӵ�blocks
        blocks.Append(reinterpret_cast<LLBC_MessageBlock *>(block));
    }

    // ���pendingSendPackets
    _pendingSendPackets.clear();

    // �ϲ��γ���Block
    LLBC_MessageBlock *block = blocks.MergeBlocksAndDetach();
    // �����_pendingSendBlock, ׷�ӵ�_pendingSendBlock,ֱ�ӷ���
    if (_pendingSendBlock)
    {
        _pendingSendBlock->Write(block->GetDataStartWithReadPos(), block->GetReadableSize());
        return true;
    }

    // ��_pendingSendBlock, ����Session.Send��ɷ���
    int sendRet = session->Send(block);
    if (UNLIKELY(sendRet != LLBC_OK))
    {
        // ����Ƴ��Ự
        removeSession = true;

        // ��ʽ����������, Log
        LLBC_String errDesc;
        errDesc.format("End pending send packet failed, call Session::Send() failed, error:%s", LLBC_FormatLastError());
        Log.je2(__FUNCTION__).Add(LI_SessionId, session->GetId()).Finish("%s", errDesc.c_str());

        // ����False
        return false;
    }

    return true;
}

void ClientProtocol_Pack::_SetHeaderToPacket(LLBC_Packet *packet)
{
    int length = 0;
    ::memcpy(&length, _header, ClientProtocolHeaderParts::Length_Size);
    packet->SetLength(length);

    int flags = 0;
    ::memcpy(&flags, _header + ClientProtocolHeaderParts::Flags_Begin, ClientProtocolHeaderParts::Flags_Size);
    packet->SetFlags(flags);

    int opcode = 0;
    ::memcpy(&opcode, _header + ClientProtocolHeaderParts::Opcode_Begin, ClientProtocolHeaderParts::Opcode_Size);
    packet->SetOpcode(opcode);

    int status = 0;
    ::memcpy(&status, _header + ClientProtocolHeaderParts::Status_Begin, ClientProtocolHeaderParts::Status_Size);
    packet->SetStatus(status);

    packet->SetSessionId(_sessionId);
    packet->SetAcceptSessionId(_acceptSessionId);
}

LLBC_FORCE_INLINE bool ClientProtocol_Pack::_CheckSendHeaderField(LLBC_Packet *packet, int field, uint32 maxLimit, const char *fieldDesc)
{
    if (UNLIKELY(static_cast<uint32>(field) > maxLimit))
    {
        GetStack()->Report(this,
            LLBC_ProtoReportLevel::Error,
            LLBC_String().format("Packet[%d] %s[%u] invalid(> %u), send failed!", packet->GetOpcode(), fieldDesc, field, maxLimit));

        LLBC_Recycle(packet);

        LLBC_SetLastError(LLBC_ERROR_PACK);
        return false;
    }

    return true;
}

void ClientProtocol_Pack::_ResetRecvInfos()
{
    _headerRecved = 0;

    LLBC_XRecycle(_packet);
    _payloadNeedRecv = 0;
    _payloadRecved = 0;
}

void ClientProtocol_Pack::_DelRecvBlock(void *block)
{
    LLBC_Recycle(reinterpret_cast<LLBC_MessageBlock *>(block));
}

void ClientProtocol_Pack::_DelPacketList(void *packetList)
{
    if (!packetList)
        return;

    LLBC_MessageBlock *block = reinterpret_cast<LLBC_MessageBlock *>(packetList);

    LLBC_Packet *packet;
    while (block->Read(&packet, sizeof(LLBC_Packet *)) == LLBC_OK)
        LLBC_Recycle(packet);

    LLBC_Recycle(block);
}

void ClientProtocol_Pack::RegisterNodeConnectEv()
{
    g_EventMgr->AddListener(EventIds::ServerComm_ServerNodeConnected, &ClientProtocol_Pack::_OnServerNodeConnected);
    g_EventMgr->AddListener(EventIds::ServerComm_ServerNodeDisconnected, &ClientProtocol_Pack::_OnServerNodeDisconnected);
}

void ClientProtocol_Pack::_OnServerNodeConnected(LLBC_Event *ev)
{
    LLBC_String serverName = EventPtrGetParam(ev, ServerName).AsStr();
    sint32 sessionId = EventPtrGetParam(ev, SessionId).AsInt32();

    if (serverName == ServerNames::GameServer)
        _gsSessionId = sessionId;
    else if (serverName == ServerNames::MiscServer)
        _msSessionId = sessionId;

    Log.d2<ClientProtocol_Pack>("_OnServerNodeConnected serverName %s sessionId %d _gsSessionId %d", serverName.c_str(), sessionId, _gsSessionId);
}

void ClientProtocol_Pack::_OnServerNodeDisconnected(LLBC_Event *ev)
{
    LLBC_String serverName = EventPtrGetParam(ev, ServerName).AsStr();
    if (serverName == ServerNames::GameServer)
        _gsSessionId = 0;
    else if (serverName == ServerNames::MiscServer)
        _msSessionId = 0;

    Log.d2<ClientProtocol_Pack>("_OnServerNodeDisconnected serverName %s _gsSessionId %d", serverName.c_str(), _gsSessionId);
}


// ClientProtocol_Codecʵ��


// ClientЭ���Factoryʵ��
LLBC_IProtocol *ClientProtocolFactory::Create(int layer) const
{
    switch (layer)
    {
    case LLBC_ProtocolLayer::CodecLayer: // CodecЭ���ʹ��llbc������Դ���Э���ʵ��
        return new ClientProtocol_Codec();

    case LLBC_ProtocolLayer::CompressLayer: // CompressЭ���ʹ��llbc������Դ���Э���ʵ��
        return new LLBC_CompressProtocol();

    case LLBC_ProtocolLayer::PackLayer: // ��Ϸ����Pack��ʵ��
        return new ClientProtocol_Pack();

    default:
        return NULL;
    }
}
#pragma 
