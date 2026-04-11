// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-04-11 22:04:09
// Author: Eric Yonng
// Description:

#pragma once

#include <service/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>

SERVICE_BEGIN

// 地址配置
struct AddrConfig
{
    POOL_CREATE_OBJ_DEFAULT(AddrConfig);

    AddrConfig()
    {

    }

    ~AddrConfig()
    {

    }

    static AddrConfig *Create()
    {
        return AddrConfig::New_AddrConfig();
    }

    void Release()
    {
        AddrConfig::Delete_AddrConfig(this);
    }

    KERNEL_NS::LibString ToString() const
    { 
        KERNEL_NS::LibString info;
        info.AppendFormat("local addr:%s:%hu, remote addr:%s:%hu, af:%d, PacketRecvBytesLimitSwitch:%d, PacketSendBytesLimitSwitch:%d, PacketSpeedLimitSwitch:%d"
                            ,_localIp._ip.c_str(), _localPort
                            ,  _remoteIp._ip.c_str(), _remotePort
                            , _af, PacketRecvBytesLimitSwitch, PacketSendBytesLimitSwitch, PacketSpeedLimitSwitch);

        return info;
    }

    KERNEL_NS::PacketOptions ToPacketOptions() const
    {
        KERNEL_NS::PacketOptions packetOptions;
        packetOptions.PacketRecvBytesLimitSwitch = PacketRecvBytesLimitSwitch;
        packetOptions.PacketSendBytesLimitSwitch = PacketSendBytesLimitSwitch;
        packetOptions.PacketSpeedLimitSwitch = PacketSpeedLimitSwitch;
        return packetOptions;
    }

    static SERVICE_COMMON_NS::CrystalProtocolStackType::ENUMS TurnStackType(const KERNEL_NS::LibString &protocolType)
    {
        return static_cast<SERVICE_COMMON_NS::CrystalProtocolStackType::ENUMS>(SERVICE_COMMON_NS::CrystalProtocolStackType::TurnFromString(protocolType));
    }
    
private:
    bool ParseIpInfo(const KERNEL_NS::LibString &addrInfo, KERNEL_NS::AddrIpConfig &ipConfig, UInt16 &port);

public:
    KERNEL_NS::AddrIpConfig _localIp;
    UInt16 _localPort = 0;

    KERNEL_NS::AddrIpConfig _remoteIp;
    UInt16 _remotePort = 0;

    // KERNEL_NS::LibString _localIp;
    // KERNEL_NS::LibString _remoteIp;

    Int32 _af = AF_INET;            // ipv4/ipv6
    Int32 _protocolStackType = 0;   // 通信使用的协议栈类型
    Int32 _listenSessionCount = 1;

    // 每个包是否设置接收包上限（使用App的配置控制）
    bool PacketRecvBytesLimitSwitch = false;
    // 每个包是否设置发送包上限(使用app的配置控制)
    bool PacketSendBytesLimitSwitch = true;
    // 包速率限制开关(使用app的配置控制)
    bool PacketSpeedLimitSwitch = false;
};


SERVICE_END

namespace YAML
{
    template<>
    struct convert<SERVICE_NS::AddrConfig>
    {
        static Node encode(const SERVICE_NS::AddrConfig& rhs)
        {
            Node node;
            node["BindTo"] = rhs._localIp._ip;
            node["BindPort"] = rhs._localPort;
            node["TargetHost"] = rhs._remoteIp._ip;
            node["TargetPort"] = rhs._remotePort;
            node["AsIpv4IfHost"] = rhs._remoteIp._toIpv4;
            node["FailureTryCount"] = rhs._remoteIp._mostSwitchIpCount;
            node["Protocol"] = SERVICE_COMMON_NS::CrystalProtocolStackType::ToString(rhs._protocolStackType);
            node["PacketRecvBytesLimitSwitch"] = rhs.PacketRecvBytesLimitSwitch;
            node["PacketSendBytesLimitSwitch"] = rhs.PacketSendBytesLimitSwitch;
            node["PacketSpeedLimitSwitch"] = rhs.PacketSpeedLimitSwitch;
            
            return node;
        }

        static bool decode(const Node& node, SERVICE_NS::AddrConfig& rhs)
        {
            if (!node.IsMap())
            {
                return false;
            }

            {
                auto &&value = node["BindTo"];
                if(value.IsDefined())
                {
                    rhs._localIp._ip = value.as<KERNEL_NS::LibString>();
                }
            }
            {
                auto &&value = node["BindPort"];
                if(value.IsDefined())
                {
                    rhs._localPort = value.as<UInt16>();
                }
            }
            {
                auto &&value = node["TargetHost"];
                if(value.IsDefined())
                {
                    rhs._remoteIp._ip = value.as<KERNEL_NS::LibString>();
                    rhs._remoteIp._isHostName = !KERNEL_NS::SocketUtil::IsIp(rhs._remoteIp._ip);
                }
            }
            {
                auto &&value = node["TargetPort"];
                if(value.IsDefined())
                {
                    rhs._remotePort = value.as<UInt16>();
                }
            }
            {
                auto &&value = node["AsIpv4IfHost"];
                if(value.IsDefined())
                {
                    rhs._remoteIp._toIpv4 = value.as<bool>();
                }
            }
            {
                auto &&value = node["FailureTryCount"];
                if(value.IsDefined())
                {
                    rhs._remoteIp._mostSwitchIpCount = value.as<Int32>();
                }
            }
            
            {
                // 是连接的由远程ip决定, ip是ipv4还是, ipv6，亦或者如果是域名, 它如果要转成ipv4的则af是ipv4, 如果是ipv6的则af是ipv6
                if(!rhs._remoteIp._ip.empty())
                {
                    if(!rhs._remoteIp._isHostName)
                    {
                        rhs._af = KERNEL_NS::SocketUtil::IsIpv4(rhs._remoteIp._ip) ? AF_INET : AF_INET6;
                    }
                    else
                    {
                        rhs._af = rhs._remoteIp._toIpv4 ? AF_INET : AF_INET6;
                    }
                }
                else
                {
                    rhs._af = KERNEL_NS::SocketUtil::IsIpv4(rhs._localIp._ip) ? AF_INET : AF_INET6;
                }
            }

            {
                auto &&value = node["Protocol"];
                if(value.IsDefined())
                {
                    rhs._protocolStackType = SERVICE_NS::AddrConfig::TurnStackType(value.as<KERNEL_NS::LibString>());
                }
            }

            {
                auto &&value = node["PacketRecvBytesLimitSwitch"];
                if(value.IsDefined())
                {
                    rhs.PacketRecvBytesLimitSwitch = value.as<bool>();
                }
            }
            {
                auto &&value = node["PacketSendBytesLimitSwitch"];
                if(value.IsDefined())
                {
                    rhs.PacketSendBytesLimitSwitch = value.as<bool>();
                }
            }
            {
                auto &&value = node["PacketSpeedLimitSwitch"];
                if(value.IsDefined())
                {
                    rhs.PacketSpeedLimitSwitch = value.as<bool>();
                }
            }

            return true;
        }
    };
    
}