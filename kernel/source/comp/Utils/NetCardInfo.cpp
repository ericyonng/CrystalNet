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
 * Date: 2021-10-16 22:04:43
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/Defs/NetCardInfo.h>
#include <kernel/comp/Utils/BitUtil.h>

KERNEL_BEGIN


Int32 NetCardType::AdapterTypeToNetCardType(Int32 type)
{
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    switch(type)
    {
        case IF_TYPE_IEEE80211: return NetCardType::WIRELESS;   // 无线网卡
        case MIB_IF_TYPE_ETHERNET: return NetCardType::WIRED_LAN;   // 有线网卡
        default:
            CRYSTAL_TRACE("unknown net card type:%d", type);
    };
    #else
        // TODO: linux 下需要独立识别
    #endif

    return NetCardType::NONE;
}

const Byte8 *NetCardType::CardTypeStr(Int32 type)
{
#
    switch(type)
    {
        case NetCardType::WIRELESS: return "WIRELESS";   // 无线网卡
        case NetCardType::WIRED_LAN: return "WIRED_LAN";   // 有线网卡
        default:
            CRYSTAL_TRACE("unknown net card type:%d", type);
    };

    return "UNKNOWN NET CARD";
}

bool NetCardType::CheckMask(UInt64 cardTypeMask)
{
    if(BitUtil::IsSet(cardTypeMask, NetCardType::WIRED_LAN))
        return true;
    if(BitUtil::IsSet(cardTypeMask, NetCardType::WIRELESS))
        return true;

    return false;
}

const UInt64 NetCardInfoMask::_maskDefault = (1 << NetCardInfoMask::MAC) | 
                                        (1 << NetCardInfoMask::IP) | 
                                        (1 << NetCardInfoMask::GATWAY) |
                                        (1 << NetCardInfoMask::DHCP) |
                                        (1 << NetCardInfoMask::DNS) |
                                        (1 << NetCardInfoMask::ADAPTER_NAME) |
                                        (1 << NetCardInfoMask::MTU);

const Byte8 *IpTypeMask::MaskToStr(UInt64 mask)
{
    bool isSetIpv4 = BitUtil::IsSet(mask, IpTypeMask::IPV4);
    bool isSetIpv6 = BitUtil::IsSet(mask, IpTypeMask::IPV6);
    if(isSetIpv4 && isSetIpv6)
        return "IPV4|IPV6";
    
    if(isSetIpv4)
        return "IPV4";

    if(isSetIpv6)
        return "IPV6";

    return "UNKNOWN MASK";
}

bool IpTypeMask::CheckMask(UInt64 ipTypeMask)
{
    if(BitUtil::IsSet(ipTypeMask, IpTypeMask::IPV4))
        return true;
    if(BitUtil::IsSet(ipTypeMask, IpTypeMask::IPV6))
        return true;

    return false;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(NetCardInfo);

NetCardInfo::NetCardInfo()
:_mtu(0)
,_cardType(NetCardType::NONE)
,_infoMask(NetCardInfoMask::_maskDefault)
,_ipTypeMask( (1 << IpTypeMask::IPV4) | (1 << IpTypeMask::IPV6))
{

}

LibString NetCardInfo::ToString() const
{
    LibString info;
    info.AppendFormat("adapterName:%s\n", _adapterName.c_str())
        .AppendFormat("mac address:%s\n", _mac.c_str())
        .AppendFormat("mtu:%llu\n", _mtu)
        .AppendFormat("cardType:%d, [%s]\n", _cardType, NetCardType::CardTypeStr(_cardType))
        .AppendFormat("infoMask:%llx\n", _infoMask)
        .AppendFormat("ipTypeMask:%llx, [%s]\n", _ipTypeMask, IpTypeMask::MaskToStr(_ipTypeMask));

    // // ipv4 info
        // ipv4
        Int32 group = 0;
        for(auto &ipv4:_ipv4)
        {
            ++group;
            info.AppendFormat("ipv4 %d:%s\n", group, ipv4.c_str());
        }
        // ipv4 gateway
        group = 0;
        for(auto &gatewayIpv4:_gatewayIpv4)
        {
            ++group;
            info.AppendFormat("gateway ipv4 %d:%s\n", group, gatewayIpv4.c_str());
        }
        // ipv4 dhcp
        info.AppendFormat("dhcp ipv4:%s\n", _dhcpIpv4.c_str());
        // ipv4 dns
        group = 0;
        for(auto &dnsIpv4:_dnsIpv4)
        {
            ++group;
            info.AppendFormat("dns ipv4 %d:%s\n", group, dnsIpv4.c_str());
        }
    
    
    // // ipv6 info
        // ipv6
        group = 0;
        for(auto &ipv6:_ipv6)
        {
            ++group;
            info.AppendFormat("ipv6 %d:%s\n", group, ipv6.c_str());
        }
        // ipv6 gateway
        group = 0;
        for(auto &gatewayIpv6:_gatewayIpv6)
        {
            ++group;
            info.AppendFormat("gateway ipv6 %d:%s\n", group, gatewayIpv6.c_str());
        }
        // ipv6 dhcp
        info.AppendFormat("dhcp ipv6:%s\n", _dhcpIpv6.c_str());
        // ipv6 dns
        group = 0;
        for(auto &dnsIpv6:_dnsIpv6)
        {
            ++group;
            info.AppendFormat("dns ipv6 %d:%s\n", group, dnsIpv6.c_str());
        }
    

    // 接收速率/发送速率
    info.AppendFormat("recv speed:%llu Bytes per second\n", _recvSpead)
        .AppendFormat("send speed:%llu Bytes per second\n", _sendSpead);

    return info;
}

KERNEL_END
