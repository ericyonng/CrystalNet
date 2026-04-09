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
 * Date: 2022-09-21 15:00:00
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/macro.h>
#include <service_common/ServiceCommon.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerConfig.h>
#include <kernel/comp/NetEngine/Defs/AddrIpConfig.h>
#include <unordered_map>
#include <vector>
#include <service_common/protocol/CrystalProtocol/CrystalProtocolStackType.h>


SERVICE_BEGIN

// 包配置
struct PacketOptions
{
    // 每个包是否设置接收上限
    bool PacketRecvBytesLimitSwitch = true;
    // 每个包是否设置发送上限
    bool PacketSendBytesLimitSwitch = true;
    // 包速率限制开关
    bool PacketSpeedLimitSwitch = true;
};

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

    bool Parse(const KERNEL_NS::LibString &configContent);

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

    PacketOptions ToPacketOptions() const
    {
        PacketOptions packetOptions;
        packetOptions.PacketRecvBytesLimitSwitch = PacketRecvBytesLimitSwitch;
        packetOptions.PacketSendBytesLimitSwitch = PacketSendBytesLimitSwitch;
        packetOptions.PacketSpeedLimitSwitch = PacketSpeedLimitSwitch;
        return packetOptions;
    }

    static SERVICE_COMMON_NS::CrystalProtocolStackType::ENUMS TurnStackType(const KERNEL_NS::LibString &protocolType) const
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




// 监听信息
struct TcpListenInfo
{
    KERNEL_NS::AddrIpConfig ToAddrIp() const
    {
        KERNEL_NS::AddrIpConfig addrIp;
        addrIp._ip = BindTo;
        return addrIp;
    }

    PacketOptions ToPacketOptions() const
    {
        PacketOptions packetOptions;
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

struct ServiceConfig
{
    POOL_CREATE_OBJ_DEFAULT(ServiceConfig);
    
    // 创建自己
    static ServiceConfig *CreateNewObj(ServiceConfig &&cfg)
    {
        return ServiceConfig::New_ServiceConfig(std::move(cfg));
    }

    // 释放自己
    void Release()
    {
        ServiceConfig::Delete_ServiceConfig(this);
    }
    
    // service层poller扫描间隔(不小于20ms)
    Int32 PollerMaxSleepMilliseconds = 20;
    // 帧更新间隔(调用组件update)
    Int32 FrameUpdateTimeMs = 50;
    // 监听信息
    std::vector<TcpListenInfo> TcpListenList;
    // 是否开启网络协议打印
    bool ProtoStackOpenLog = true;
    // 默认的存储引擎(可以选择默认的存储引擎作为主要存储)
    KERNEL_NS::LibString DefaultStorage;
    // 数据库版本号, 跨版本号清档数据
    Int32 DbVersion = 0;
    // 系统使用operator uid, operator uid用于多线程下的负载均衡
    Int32 SystemOperatorUid = 0;
    // 数据清洗时间间隔
    Int32 PurgeIntervalMs = 3000;

    // 用户lru限制数量
    Int32 UserLruCapacityLimit = 1000;
    // 配置表数据目录
    KERNEL_NS::LibString ConfigDataPath = "./Cfgs";

    // 密钥 公钥使用PKC8
    KERNEL_NS::LibString RsaPrivateKey;
    KERNEL_NS::LibString RsaPublicKey;

    // 协议加密key过期时间
    Int64 EncryptKeyExpireTime;
    // 热更集 hotfixkey
    KERNEL_NS::LibString PluginHotfixKey;

    // 系统表MysqlMgr表禁用自动清库
    bool DisableSystemTableAutoDrop = true;
    // 用清库功能 用于线上, 线上不可以自动清库，线上必须设置成1, 只能修数据 TODO:
    bool DisableAutoDropDB = true;
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
    
    // 网络单元定义
    template<>
    struct convert<SERVICE_NS::ServiceConfig>
    {
        static Node encode(const SERVICE_NS::ServiceConfig& rhs)
        {
            Node node;
            node["PollerMaxSleepMilliseconds"] = rhs.PollerMaxSleepMilliseconds;
            node["FrameUpdateTimeMs"] = rhs.FrameUpdateTimeMs;
            node["TcpListenList"] = rhs.TcpListenList;
            node["ProtoStackOpenLog"] = rhs.ProtoStackOpenLog;
            node["CurrentServiceDB"] = rhs.CurrentServiceDB;
            node["DbVersion"] = rhs.DbVersion;
            node["SystemOperatorUid"] = rhs.SystemOperatorUid;
            node["PurgeIntervalMs"] = rhs.PurgeIntervalMs;
            node["UserLruCapacityLimit"] = rhs.UserLruCapacityLimit;
            node["ConfigDataPath"] = rhs.ConfigDataPath;
            node["RsaPrivateKey"] = rhs.RsaPrivateKey;
            node["RsaPublicKey"] = rhs.RsaPublicKey;
            node["EncryptKeyExpireTime"] = rhs.EncryptKeyExpireTime;
            node["PluginHotfixKey"] = rhs.PluginHotfixKey;
            node["DisableSystemTableAutoDrop"] = rhs.DisableSystemTableAutoDrop;
            node["DisableAutoDropDB"] = rhs.DisableAutoDropDB;
            node["DefaultStorage"] = rhs.DefaultStorage;
            return node;
        }

        static bool decode(const Node& node,  SERVICE_NS::ServiceConfig& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }
            
            {
                auto &&value = node["PollerMaxSleepMilliseconds"];
                if(value.IsDefined())
                    rhs.PollerMaxSleepMilliseconds = value.as<Int32>();
            }

            {
                auto &&value = node["FrameUpdateTimeMs"];
                if(value.IsDefined())
                    rhs.FrameUpdateTimeMs = value.as<Int32>();
            }

            {
                auto &&value = node["TcpListenList"];
                if(value.IsDefined())
                    rhs.TcpListenList = value.as<std::vector<SERVICE_NS::TcpListenInfo>>();
            }

            {
                auto &&value = node["ProtoStackOpenLog"];
                if(value.IsDefined())
                    rhs.ProtoStackOpenLog = value.as<bool>();
            }

            {
                auto &&value = node["DefaultStorage"];
                if(value.IsDefined())
                    rhs.DefaultStorage = value.as<KERNEL_NS::LibString>();
            }

            {
                auto &&value = node["DbVersion"];
                if(value.IsDefined())
                    rhs.DbVersion = value.as<Int32>();
            }

            {
                auto &&value = node["SystemOperatorUid"];
                if(value.IsDefined())
                    rhs.SystemOperatorUid = value.as<Int32>();
            }

            {
                auto &&value = node["PurgeIntervalMs"];
                if(value.IsDefined())
                    rhs.PurgeIntervalMs = value.as<Int32>();
            }
            {
                auto &&value = node["UserLruCapacityLimit"];
                if(value.IsDefined())
                    rhs.UserLruCapacityLimit = value.as<Int32>();
            }
            {
                auto &&value = node["ConfigDataPath"];
                if(value.IsDefined())
                    rhs.ConfigDataPath = value.as<KERNEL_NS::LibString>();
            }            
            {
                auto &&value = node["RsaPrivateKey"];
                if(value.IsDefined())
                    rhs.RsaPrivateKey = value.as<KERNEL_NS::LibString>();
            }                 
            {
                auto &&value = node["RsaPublicKey"];
                if(value.IsDefined())
                    rhs.RsaPublicKey = value.as<KERNEL_NS::LibString>();
            }  
            {
                auto &&value = node["EncryptKeyExpireTime"];
                if(value.IsDefined())
                    rhs.EncryptKeyExpireTime = value.as<Int32>();
            }             
            {
                auto &&value = node["PluginHotfixKey"];
                if(value.IsDefined())
                    rhs.PluginHotfixKey = value.as<KERNEL_NS::LibString>();
            }

            {
                auto &&value = node["DisableSystemTableAutoDrop"];
                if(value.IsDefined())
                    rhs.DisableSystemTableAutoDrop = value.as<bool>();
            }
            {
                auto &&value = node["DisableAutoDropDB"];
                if(value.IsDefined())
                    rhs.DisableAutoDropDB = value.as<bool>();
            }
            return true;
        }
    };
}