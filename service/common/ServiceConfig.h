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
        info.AppendFormat("local addr:%s:%hu, remote addr:%s:%hu, af:%d, session type:%d"
                            ,_localIp._ip.c_str(), _localPort
                            ,  _remoteIp._ip.c_str(), _remotePort
                            , _af, _sessionType);

        return info;
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
    Int32 _sessionType = 0;         // 配置的会话类型
    Int32 _protocolStackType = 0;   // 通信使用的协议栈类型
    Int32 _listenSessionCount = 1;
};

struct ServiceConfig
{
    POOL_CREATE_OBJ_DEFAULT(ServiceConfig);

    ServiceConfig()
    :_centerAddr(AddrConfig::Create()) 
    ,_protoStackOpenLog(false)
    ,_encryptKeyExpireTime(3000)
    {

    }

    ~ServiceConfig()
    {
        KERNEL_NS::ContainerUtil::DelContainer2(_listenAddrs);
        KERNEL_NS::ContainerUtil::DelContainer2(_connectAddrGroup);
        
        if(_centerAddr)
            _centerAddr->Release();
        _centerAddr = NULL;
    }

    bool Parse(const KERNEL_NS::LibString &seg, const KERNEL_NS::LibIniFile *ini);

    bool _ParsePoller(const KERNEL_NS::LibString &seg, const KERNEL_NS::LibIniFile *ini);

    std::vector<AddrConfig *> _listenAddrs;
    std::vector<AddrConfig *> _connectAddrGroup;

    bool _protoStackOpenLog;
    Int64 _encryptKeyExpireTime;
};

// 监听信息
struct TcpListenInfo
{
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
    // 监听该端口的session数量, 同一个端口多个session监听可以多线程负载均衡, 在linux下才可以, 因为Linux下有ReusePort选项，windows下最多只能是1
    Int32 ListenSessionCount = 1;
};

struct ServiceCfg
{
    // service层poller扫描间隔(不小于20ms)
    Int32 PollerMaxSleepMilliseconds = 20;
    // 帧更新间隔(调用组件update)
    Int32 FrameUpdateTimeMs = 50;
    // 监听信息
    std::vector<TcpListenInfo> TcpListenList;
    // 是否开启网络协议打印
    bool ProtoStackOpenLog = true;
    // 数据库相关配置
    std::vector<KERNEL_NS::LibString> DbConfigList;
    // 指定当前服务的数据库
    KERNEL_NS::LibString CurrentServiceDB;
    // 数据库版本号, 跨版本号清档数据
    Int32 DbVersion = 0;
    // 系统使用operator uid, operator uid用于多线程下的负载均衡
    Int32 SystemOperatorUid = 0;
    // 数据清洗时间间隔
    Int32 PurgeIntervalMs = 3000;
};

SERVICE_END