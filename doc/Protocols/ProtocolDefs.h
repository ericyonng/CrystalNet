/**
 * @file    ProtocolDefs.h
 * @author  Longwei Lai<lailongwei@126.com>
 * @date    2019/06/22
 * @brief   Э����ض���
 */

#pragma once

// Э��ջ��������: ProtoStackCtrlType
/**
 * Э��ջ��������
 */
class ProtoStackCtrlType
{
public:
    enum
    {
        Begin = 1,

        BeginPendingSendPacket = Begin, // 1: ��ʼ���������
        EndPendingSendPacket,           // 2: ���������͹���
        SendSceneMsg,                   // 3: ���ͳ�����Ϣ

        End,
    };
};


// ClientProtocol���
// �ͻ���Э��ͷ���ֶ���: ClientProtocolHeaderParts
/**
 * �ͻ���Э��ͷ���ֶ���
 */
class ClientProtocolHeaderParts
{
public:
    static const size_t Header_Size = 8; // Э��ͷ����

    static const int Length_Begin = 0; // Э��ͷ���Ȳ��ֿ�ʼ
    static const int Length_Size = 3; // Э��ͷ���Ȳ��ִ�С

    static const int Flags_Begin = 3; // Э��ͷFlags���ֿ�ʼ
    static const int Flags_Size = 1; // Э��ͷFlags���ִ�С

    static const int Opcode_Begin = 4; // Э��ͷOpcode���ֿ�ʼ
    static const int Opcode_Size = 2; // Э��ͷOpcode���ִ�С

    static const int Status_Begin = 6; // Э��ͷStatus���ֿ�ʼ
    static const int Status_Size = 2; // Э��ͷStatus���ִ�С

    static const size_t MaxPacketLen = 0x00ffffff; // ��������
    static const int MaxOpcode = 0x0000ffff; // ���Opcodeֵ
    static const int MaxStatus = 0x0000ffff; // ���Statusֵ
    static const int MaxFlags = 0x000000ff; // ���Flagsֵ
};


// �ͻ���Э��Flags����: ClientProtocolFlags
/**
 * �ͻ���Э��Flags����
 */
class ClientProtocolFlags
{
public:
    static const int HasCoder = 0x01; // �Ƿ����coder
    static const int Compressed = 0x02; // �Ƿ���ѹ��
    static const int AesCrypt = 0x04; // �Ƿ��н���Aes����
    static const int XorCrypt = 0x08; // �Ƿ��н���������
};



// ServerCommProtocol���
// �������ڲ�����Э��ͷ���ֶ���: ServerCommProtocolHeaderParts
/**
 * �������ڲ�����Э��ͷ���ֶ���
 */
class ServerCommProtocolHeaderParts
{
public:
    static const size_t Header_Size = 28;
};


