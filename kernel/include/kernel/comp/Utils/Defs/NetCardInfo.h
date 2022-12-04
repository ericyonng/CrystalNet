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
 * Date: 2021-10-16 22:01:56
 * Author: Eric Yonng
 * Description: 目前只支持单播ip, 多播暂时没有相关需求,若有需求可以后续接入
*/


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DEFS_NET_CARD_INFO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DEFS_NET_CARD_INFO_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/Utils/BitUtil.h>

KERNEL_BEGIN

struct KERNEL_EXPORT NetCardType
{
    enum 
    {
        NONE = 0,                   // 无效
        WIRELESS = 1,               // 无线网卡
        WIRED_LAN = 2,              // 有线网卡
    };

    static Int32 AdapterTypeToNetCardType(Int32 type);
    static const Byte8 *CardTypeStr(Int32 type);
    static bool CheckMask(UInt64 cardTypeMask);
};

// 用于控制获取信息的掩码
class KERNEL_EXPORT NetCardInfoMask
{
public:
    enum
    {
        NONE = 0,                       // 不获取信息
        MAC = 1,                        // 获取mac地址
        IP = 2,                         // 获取ip
        GATWAY = 3,                     // 获取gateway
        DHCP = 4,                       // 获取dhcp地址
        DNS = 5,                        // 获取dns服务器地址
        RECV_SPEED = 6,                 // 接收速率
        SEND_SPEED = 7,                 // 发送速率
        ADAPTER_NAME = 8,               // 适配器名
        MTU = 9,                        // 网络mtu
    };

    // 缺省 RECV/SEND
    static const UInt64 _maskDefault;
};

class KERNEL_EXPORT IpTypeMask
{
public:
    enum
    {
        IPV4 = 0,       // 获取ipv4
        IPV6 = 1,       // 获取ipv6
    };

    static const Byte8 *MaskToStr(UInt64 mask);
    static bool CheckMask(UInt64 ipTypeMask);
};

struct KERNEL_EXPORT NetCardInfo
{
    POOL_CREATE_OBJ_DEFAULT(NetCardInfo);

    NetCardInfo();

    bool HasIpv4() const;
    bool HasIpv6() const;
    bool HasInfo(Int32 infoMaskPos) const;
    LibString ToString() const;

    // 网卡信息
    LibString _adapterName;         // 适配器名
    LibString _mac;                 // mac地址
    UInt64 _mtu;                    // 大于mtu的网络包将被分包

    // ipv4
    std::vector<LibString> _ipv4;           // ipv4地址(一个网卡可以配置多个ip) 单播地址
    std::vector<LibString> _gatewayIpv4;    // 网关
    LibString _dhcpIpv4;                    // dhcp服务器地址
    std::vector<LibString> _dnsIpv4;        // dns地址

    // ipv6
    std::vector<LibString> _ipv6;           // ipv6地址(一个网卡可以配置多个ip) 单播地址
    std::vector<LibString> _gatewayIpv6;    // 网关
    LibString _dhcpIpv6;                    // dhcp服务器地址
    std::vector<LibString> _dnsIpv6;        // dns地址

    UInt64 _recvSpead;              // 接受速率 Byte/second
    UInt64 _sendSpead;              // 发送速率 Byte/second

    // 控制信息
    Int32 _cardType;                // 网卡类型
    UInt64 _infoMask;               // 用于获取地址信息的掩码
    UInt64 _ipTypeMask;             // ipv4/ipv6获取控制 默认两者都获取
};

inline bool NetCardInfo::HasIpv4() const
{
    return BitUtil::IsSet(_ipTypeMask, IpTypeMask::IPV4);
}

inline bool NetCardInfo::HasIpv6() const
{
    return BitUtil::IsSet(_ipTypeMask, IpTypeMask::IPV6);
}

inline bool NetCardInfo::HasInfo(Int32 infoMaskPos) const
{
    return BitUtil::IsSet(_infoMask, infoMaskPos);
}

KERNEL_END

#endif