/**
 * @file    ClientProtocol.h
 * @author  Longwei Lai<lailongwei@126.com>
 * @date    2018/03/02
 * @brief   �ͻ���ͨ��Э��ջ����
 */

#pragma once

#include "Common/ThirdParties.h"
#include "Common/Protocols/ProtocolDefs.h"

/**
 * �ͻ���ͨ��PacketЭ���
 * Э��ͷ��ʽ��
 *   |       Type       | Offset |  Len |
 * --|------------------|--------|------|--
 *   |      Length      |    0   |   3  |
 *   |      Flags       |    3   |   1  |
 *   |      Opcode      |    4   |   2  |
 *   |      Status      |    6   |   2  |
 *   
 * Flags˵����
 * 0x00000001: �Ƿ���Э���(���ͻ���/�������Ƿ���new coder�����л�)
 * 0x00000002: �Ƿ��Ѿ���ѹ��
 * 0x00000004; �Ƿ�AES����
 * 0x00000008; �Ƿ�������
 */

class AESCrypto;

#define LZ4_COMPRESS_BUFF_SIZE 65536
#define LZ4_MIN_SIZE_FOR_COMPRESS 64

class ClientProtocol_Pack : public LLBC_IProtocol
{
public:
    typedef ClientProtocol_Pack This;
    typedef std::map<int, LLBC_ICoderFactory *> Coders;

    enum
    {
        Begin,

        NotEnterScene = Begin,
        EnteringScene,
        EnteredScene,
        LeavingScene,
        LeavedScene,
        
        End,
    };

public:
    ClientProtocol_Pack();
    virtual ~ClientProtocol_Pack();

public:
    virtual int GetLayer() const;

    virtual int Connect(LLBC_SockAddr_IN &local, LLBC_SockAddr_IN &peer);
    virtual int Send(void *in, void *&out, bool &removeSession);
    virtual int Recv(void *in, void *&out, bool &removeSession);

    virtual bool Ctrl(int ctrlType, const LLBC_Variant &ctrlData, bool &removeSession);

    static void RegisterNodeConnectEv();
    static void _OnServerNodeConnected(LLBC_Event *ev);
    static void _OnServerNodeDisconnected(LLBC_Event *ev);

private:
    bool _CheckSendHeaderField(LLBC_Packet *packet, int field, uint32 maxLimit, const char *fieldDesc);
    void _SetHeaderToPacket(LLBC_Packet *packet);

    void _ResetRecvInfos();

    void _DelRecvBlock(void *block);
    void _DelPacketList(void *packetList);

    void _Decrypt();
    bool _Compress(LLBC_String &compressStr);

    bool _Ctrl_BeginPendingSendPacket(const LLBC_Variant &ctrlData, bool &removeSession);
    bool _Ctrl_EndPendingSendPacket(const LLBC_Variant &ctrlData, bool &removeSession);

private:
    char _header[ClientProtocolHeaderParts::Header_Size];
    size_t _headerRecved;

    LLBC_Packet *_packet;
    size_t _payloadNeedRecv;
    size_t _payloadRecved;

    sint32 _status;

    LLBC_String _serverName;
    bool _isLoginServer;
    uint64 _shiftedGlobalNodeId;

    static sint32 _gsSessionId;
    static sint32 _msSessionId;
    static LLBC_ListenerStub _eventStub;

    const Coders *_clientCoders;

    AESCrypto *_crypt;

    LLBC_String _aesCrypto;
    LLBC_String _xorUpCrypto;
    LLBC_String _xorDownCrypto;

    static LLBC_THREAD_LOCAL char _compressBuff[LZ4_COMPRESS_BUFF_SIZE]; // TODO: ���Ż�

    int _pendingSendPacket;
    std::vector<LLBC_Packet *> _pendingSendPackets;
    LLBC_MessageBlock *_pendingSendBlock;
};

typedef LLBC_CodecProtocol ClientProtocol_Codec;

/**
 * �ͻ���Э��ջ������
 */
class ClientProtocolFactory : public LLBC_IProtocolFactory
{
public:
    virtual LLBC_IProtocol *Create(int layer) const;
};