/**
 * @file    ServerCommProtocol.h
 * @author  Longwei Lai<lailongwei@126.com>
 * @date    2018/03/02
 * @brief   服务器内部互联使用的协议栈定义
 */

#pragma once

#include "Common/ThirdParties.h"
#include "Common/Protocols/ProtocolDefs.h"

class ServerCommProtocol_Pack : public LLBC_PacketProtocol
{
    typedef LLBC_PacketProtocol Base;

public:
    ServerCommProtocol_Pack();
    virtual ~ServerCommProtocol_Pack();

public:
    virtual int Send(void *in, void *&out, bool &removeSession);
    virtual int Recv(void *in, void *&out, bool &removeSession);

private:
    void _DelRecvBlock(void *block);
    void _DelPacketList(void *packetList);

private:
    LLBC_PacketHeaderAssembler _headerAssembler;

    LLBC_Packet *_packet;
    size_t _payloadNeedRecv;
    size_t _payloadRecved;

    bool _isGateway;
    LLBC_String _serverName;

    typedef std::map<int, LLBC_ICoderFactory *> Coders;
    const Coders *_coders;
};

class ServerCommProtocol_Codec : public LLBC_CodecProtocol
{
public:
    ServerCommProtocol_Codec();
    virtual ~ServerCommProtocol_Codec();

public:
    virtual int Send(void* in, void*& out, bool& removeSession);
    virtual int Recv(void *in, void *&out, bool &removeSession);
};

/**
* GameServer Codec协议栈
*/
class GSServerCommProtocol_Codec : public LLBC_CodecProtocol
{
public:
    virtual int Recv(void *in, void *&out, bool &removeSession);
};

/**
* Gateway内部节点通信协议栈工厂类
*/
class ServerCommProtocolFactory : public LLBC_IProtocolFactory
{
public:
    virtual LLBC_IProtocol *Create(int layer) const;
};
