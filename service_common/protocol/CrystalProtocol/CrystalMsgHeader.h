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
 * Date: 2022-06-26 22:41:34
 * Author: Eric Yonng
 * Description: 采用小端字节序
 * 
 * // 协议头定义 (区分客户端协议栈/内网协议栈（不加密,统一小端,）)
 * 1.包长度 3字节
 * 2.flags 开启加解密(aes, xor, rsa, ), 开启,压缩解压 2字节
 * 3.opcode 3字节
 * 4.PacketId 8字节（本组包代码，一组完整的包可能需要多个Packet组成, 高47位为序列号 低16位：8位包个数 + 8 位序号） 业务层可以依据48位包序号进行组包 = seriaid + packetnum + 包编号
 * 5.包体
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_PROTOCOL_CRYSTAL_PROTOCOL_CRYSTAL_MSG_HEADER_H__
#define __CRYSTAL_NET_SERVICE_COMMON_PROTOCOL_CRYSTAL_PROTOCOL_CRYSTAL_MSG_HEADER_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>
#include <service_common/common/common.h>

SERVICE_COMMON_BEGIN

// 协议生成时候: XorEncrypt:true, KeyBase64:true等
class MsgFlagsType
{
    enum FLAG_POS_ENUMS : UInt32
    {
        XOR_ENCRYPT_FLAG_POS = 0,
        AES_ENCRYPT_FLAG_POS,
        RSA_ENCRYPT_FLAG_POS,
        KEY_IN_BASE64_FLAG_POS,
    };
public:
    // 加密方法互斥, 按照xor=>aes=>rsa优先级
    enum FLAGS_TYPE :UInt32
    {
        // xor 加密
        XOR_ENCRYPT_FLAG = 1U << XOR_ENCRYPT_FLAG_POS,
        // aes 加密
        AES_ENCRYPT_FLAG = 1U << AES_ENCRYPT_FLAG_POS,
        // rsa 加密
        RSA_ENCRYPT_FLAG = 1U << RSA_ENCRYPT_FLAG_POS,
        // key使用了base64
        KEY_IN_BASE64_FLAG = 1U << KEY_IN_BASE64_FLAG_POS,
    };
};

struct CrystalMsgHeader
{
    POOL_CREATE_OBJ_DEFAULT(CrystalMsgHeader);

    CrystalMsgHeader()
    :_len(0)
    ,_protocolVersion(0)
    ,_flags(0)
    ,_opcodeId(0)
    ,_packetId(0)
    ,_keyLen(0)
    {

    }

    KERNEL_NS::LibString ToString() const;

    UInt32 _len;            // 包长度
    UInt64 _protocolVersion;    // 协议版本号
    UInt32 _flags;          // 包特性位标记
    UInt32 _opcodeId;       // 操作码
    Int64 _packetId;        // 最高位为0 
    UInt32 _keyLen;         // key的长度
};

struct MsgHeaderStructure
{
    // 消息长度起始与大小
    static constexpr Int32 LEN_START_POS = 0;            // 消息长度开始
    static constexpr Int32 LEN_SIZE = 3;                // 消息长度大小

    // 协议版本号
    static constexpr Int32 PROTOCOL_VERSION_NUMBER_START_POS = LEN_SIZE;   // 协议版本号
    static constexpr Int32 PROTOCOL_VERSION_SIZE = 8;              // 主版本号(2).子版本号(2).修订版本号(2).修饰符(2)(alpha(enum(1), ...))

    // flags起始与大小
    static constexpr Int32 FLAGS_START_POS = PROTOCOL_VERSION_NUMBER_START_POS + PROTOCOL_VERSION_SIZE;   // flags起始位置
    static constexpr Int32 FLAGS_SIZE = 2;              // flags大小

    // opcode起始与大小
    static constexpr Int32 OPCODE_START_POS = FLAGS_START_POS + FLAGS_SIZE;  // opcode起始位置
    static constexpr Int32 OPCODE_SIZE = 3;             // opcode长度

    // packetId的起始与大小
    static constexpr Int32 PACKET_ID_START_POS = OPCODE_START_POS + OPCODE_SIZE;   // packetId开始
    static constexpr Int32 PACKET_ID_SIZE = 8;           // packetid大小

    // key len
    static constexpr Int32 KEY_LEN_SIZE_START_POS = PACKET_ID_START_POS + PACKET_ID_SIZE;   // KEY LEN
    static constexpr Int32 KEY_LEN_SIZE = 4;

    // 消息头大小 = 长度大小 + 版本号大小 + flags大小 + opcode大小 + packetid大小
    static constexpr Int32 MSG_HEADER_SIZE = LEN_SIZE + PROTOCOL_VERSION_SIZE + FLAGS_SIZE + OPCODE_SIZE + PACKET_ID_SIZE + KEY_LEN_SIZE;    // 消息头大小
    
    // 消息体开始与最大大小 可以进行_len的校验
    static constexpr Int32 MSG_BODY_START_POS = MSG_HEADER_SIZE;  // 消息体起始位置
    static constexpr Int32 MSG_BODY_MAX_SIZE_LIMIT = 16777215;     // 包体最大大小 2^24 - 1 16MB

    // 外部协议通信上行包限制
    // static constexpr Int32 UPLOAD_MSG_MAX_PACKET_LIMIT = 4194304;   // 外部上行包大小限制 4MB
};

SERVICE_COMMON_END

#endif
