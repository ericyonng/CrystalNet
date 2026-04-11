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
// Date: 2026-04-11 22:04:56
// Author: Eric Yonng
// Description:

#pragma once

#include <service/common/macro.h>
#include <kernel/comp/NetEngine/Defs/AddrIpConfig.h>

SERVICE_BEGIN

// 监听信息
struct TcpListenInfo
{
    KERNEL_NS::AddrIpConfig ToAddrIp() const
    {
        KERNEL_NS::AddrIpConfig addrIp;
        addrIp._ip = BindTo;
        return addrIp;
    }

    KERNEL_NS::PacketOptions ToPacketOptions() const
    {
        KERNEL_NS::PacketOptions packetOptions;
        packetOptions.PacketRecvBytesLimitSwitch = PacketRecvBytesLimitSwitch;
        packetOptions.PacketSendBytesLimitSwitch = PacketSendBytesLimitSwitch;
        packetOptions.PacketSpeedLimitSwitch = PacketSpeedLimitSwitch;
        return packetOptions;
    }

    SERVICE_COMMON_NS::CrystalProtocolStackType::ENUMS TurnStackType() const
    {
        return static_cast<SERVICE_COMMON_NS::CrystalProtocolStackType::ENUMS>(SERVICE_COMMON_NS::CrystalProtocolStackType::TurnFromString(ProtocolType));
    }

    KERNEL_NS::LibString ToString() const
    {
        return KERNEL_NS::LibString().AppendFormat("%s:%d - [%s, %d, %d, %d, %d]", BindTo.c_str(), Port, ProtocolType.c_str()
            , PacketRecvBytesLimitSwitch, PacketSendBytesLimitSwitch, PacketSpeedLimitSwitch,  ListenSessionCount);
    }
    
    // 绑定地址
    KERNEL_NS::LibString BindTo;
    // 绑定端口
    Int32 Port;
    // 协议栈类型
    KERNEL_NS::LibString ProtocolType;
    // 每个包是否设置接收上限
    bool PacketRecvBytesLimitSwitch = true;
    // 每个包是否设置发送上限
    bool PacketSendBytesLimitSwitch = true;
    // 包速率限制开关
    bool PacketSpeedLimitSwitch = true;
    // 监听该端口的session数量, 同一个端口多个session监听可以多线程负载均衡, 在linux下才可以, 因为Linux下有ReusePort选项，windows下最多只能是1
    Int32 ListenSessionCount = 1;
};

SERVICE_END


namespace YAML
{
    template<>
    struct convert<SERVICE_NS::TcpListenInfo>
    {
        static Node encode(const SERVICE_NS::TcpListenInfo& rhs)
        {
            Node node;
            node["BindTo"] = rhs.BindTo;
            node["Port"] = rhs.Port;
            node["ProtocolType"] = rhs.ProtocolType;
            node["PacketRecvBytesLimitSwitch"] = rhs.PacketRecvBytesLimitSwitch;
            node["PacketSendBytesLimitSwitch"] = rhs.PacketSendBytesLimitSwitch;
            node["PacketSpeedLimitSwitch"] = rhs.PacketSpeedLimitSwitch;
            node["ListenSessionCount"] = rhs.ListenSessionCount;
            return node;
        }
        
        static bool decode(const Node& node, SERVICE_NS::TcpListenInfo& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }
            
            {
                auto &&value = node["BindTo"];
                if(value.IsDefined())
                    rhs.BindTo = value.as<KERNEL_NS::LibString>();
            }

            {
                auto &&value = node["Port"];
                if(value.IsDefined())
                    rhs.Port = value.as<Int32>();
            }
            
            {
                auto &&value = node["ProtocolType"];
                if(value.IsDefined())
                    rhs.ProtocolType = value.as<KERNEL_NS::LibString>();
            }
            {
                auto &&value = node["PacketRecvBytesLimitSwitch"];
                if(value.IsDefined())
                    rhs.PacketRecvBytesLimitSwitch = value.as<bool>();
            }
                
            {
                auto &&value = node["PacketSendBytesLimitSwitch"];
                if(value.IsDefined())
                    rhs.PacketSendBytesLimitSwitch = value.as<bool>();
            }

            {
                auto &&value = node["PacketSpeedLimitSwitch"];
                if(value.IsDefined())
                    rhs.PacketSpeedLimitSwitch = value.as<bool>();
            }
            
            {
                auto &&value = node["ListenSessionCount"];
                if(value.IsDefined())
                    rhs.ListenSessionCount = value.as<Int32>();
            }       
            
            return true;
        }

    };
}
