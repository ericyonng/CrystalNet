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

#include <pch.h>
#include <kernel/kernel.h>
#include <service/common/ServiceConfig.h>
#include <service/common/PriorityLevelDefine.h>
#include <service/common/SessionType.h>
#include <service_common/protocol/protocol.h>
#include <service_common/common/Configs.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(AddrConfig);

bool AddrConfig::Parse(const KERNEL_NS::LibString &configContent, bool isStreamSock)
{
    // Local,127.0.0.1,3901-Remote,127.0.0.1,3901/INNER
    const KERNEL_NS::LibString inclineSep = "@";
    const KERNEL_NS::LibString horizontalSep = "-";
    const KERNEL_NS::LibString elemSep = ",";
    const KERNEL_NS::LibString localSign = "local";
    const KERNEL_NS::LibString remoteSign = "remote";
    const KERNEL_NS::LibString hostNameSep = "'";
    const KERNEL_NS::LibString ipv6Flag = "ipv6";
    const KERNEL_NS::LibString portSep = ":";

    
    UInt32 priorityLevel = PriorityLevelDefine::INNER;

    const auto &addrAndPriority = configContent.Split(inclineSep);
    if(addrAndPriority.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("addr parse fail have no / sign configContent:%s"), configContent.c_str());
        return false;
    }
        
    if(addrAndPriority.size() == 2)
        priorityLevel = PriorityLevelDefine::StringToPriorityLevel(addrAndPriority[1]);

    auto &addrInfo = addrAndPriority[0];
    const auto &sepLocalAndRemote = addrInfo.Split(horizontalSep);
    if(sepLocalAndRemote.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("addr parse fail configContent:%s, addrInfo:%s"), configContent.c_str(), addrInfo.c_str());
        return false;
    }

    // 识别Local/remote地址
    KERNEL_NS::LibString localIp;
    UInt16 localPort = 0;
    Int32 localPortSessionType = SessionType::INNER;
    Int32 localStack = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL;
    KERNEL_NS::LibString remoteIp;
    Int32 listenSessionCount = 1;
    UInt16 remotePort = 0;
    Int32 remotePortSessionType = SessionType::INNER;
    Int32 remoteStack = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL;
    for(auto &endianInfo : sepLocalAndRemote)
    {
        const auto &endianPartGroup = endianInfo.Split(elemSep);
        if(endianPartGroup.size() < 3)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("addr parse error: endian part group not match endian format, configContent:%s, endianInfo:%s"), configContent.c_str(), endianInfo.c_str());
            return false;
        }

        if(endianPartGroup[0].tolower() == localSign)
        {// 本地地址
            // ip
            if(!endianPartGroup[1].empty())
            {
                if(!KERNEL_NS::SocketUtil::IsIp(endianPartGroup[1]))
                {
                    bool toIpv4 = true;
                    auto hostNameParts = endianPartGroup[1].Split(hostNameSep);
                    if((hostNameParts.size() > 1) && !hostNameParts[1].empty())
                    {
                        auto flagPart = hostNameParts[1].strip().tolower();
                        if(flagPart == ipv6Flag)
                            toIpv4 = false;
                    }

                    // 看是不是域名
                    auto err = KERNEL_NS::IPUtil::GetIpByHostName(hostNameParts[0].strip(), localIp, 0, true, isStreamSock, toIpv4);
                    if(err != Status::Success)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("invalid ip: configContent:%s, ip:%s"), configContent.c_str(), endianPartGroup[1].c_str());
                        return false;
                    }

                    localIp.strip();
                }
                else
                {
                    localIp = endianPartGroup[1].strip();
                }
            }

            // 端口
            if(!endianPartGroup[2].empty())
            {
                const auto &portParts = endianPartGroup[2].Split(portSep);
                if(portParts.empty())
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("port invalid configContent:%s, port config str:%s"), configContent.c_str(), endianPartGroup[2].c_str());
                    return false;
                }

                if(!portParts[0].isdigit())
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("port not digit, configContent:%s, port:%s"), configContent.c_str(), endianPartGroup[2].c_str());
                    return false;
                }

                localPort = KERNEL_NS::StringUtil::StringToUInt16(portParts[0].c_str());

                if(portParts.size() >= 2)
                {
                    auto sessionType = SessionType::SessionStringToSessionType(portParts[1]);
                    if(sessionType == SessionType::UNKNOWN)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("port session type invalid, configContent:%s, port str:%s"), configContent.c_str(), endianPartGroup[2].c_str());
                        return false;
                    }

                    localPortSessionType = sessionType;
                }
            }

            // 监听同一个端口会话数量
            if(!endianPartGroup[3].empty())
            {
                if(!endianPartGroup[3].isdigit())
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("listen session count not digit, configContent:%s, listen session count:%s")
                    , configContent.c_str(), endianPartGroup[3].c_str());
                    return false;
                }

                listenSessionCount = KERNEL_NS::StringUtil::StringToInt32(endianPartGroup[3].c_str());
            }

            if(endianPartGroup.size() >= 5)
            {
                localStack = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL;
                if(!endianPartGroup[4].empty())
                {
                    auto stackType = SERVICE_COMMON_NS::CrystalProtocolStackType::TurnFromString(endianPartGroup[4]);
                    if(stackType == SERVICE_COMMON_NS::CrystalProtocolStackType::UNKNOWN)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("unknown local protocol stack configContent:%s, stack type:%s")
                        , configContent.c_str(), endianPartGroup[4].c_str());
                        return false;
                    }

                    localStack = stackType;
                }
            }
            continue;
        }

        if(endianPartGroup[0].tolower() == remoteSign)
        {// 远程地址
            // ip
            if(!endianPartGroup[1].empty())
            {
                if(!KERNEL_NS::SocketUtil::IsIp(endianPartGroup[1]))
                {
                    bool toIpv4 = true;
                    auto hostNameParts = endianPartGroup[1].Split(hostNameSep);
                    if((hostNameParts.size() > 1) && !hostNameParts[1].empty())
                    {
                        auto flagPart = hostNameParts[1].strip().tolower();
                        if(flagPart == ipv6Flag)
                            toIpv4 = false;
                    }

                    auto err = KERNEL_NS::IPUtil::GetIpByHostName(hostNameParts[0].strip(), remoteIp, 0, true, isStreamSock, toIpv4);
                    if(err != Status::Success)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("invalid ip: configContent:%s, ip:%s"), configContent.c_str(), endianPartGroup[1].c_str());
                        return false;
                    }

                    remoteIp.strip();
                }
                else
                {
                    remoteIp = endianPartGroup[1].strip();
                }
            }

            // 端口
            if(!endianPartGroup[2].empty())
            {
                const auto &portParts = endianPartGroup[2].Split(portSep);
                if(portParts.empty())
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("port invalid configContent:%s, port config str:%s"), configContent.c_str(), endianPartGroup[2].c_str());
                    return false;
                }

                if(!portParts[0].isdigit())
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("port not digit, configContent:%s, port:%s"), configContent.c_str(), endianPartGroup[2].c_str());
                    return false;
                }

                remotePort = KERNEL_NS::StringUtil::StringToUInt16(portParts[0].c_str());

                if(portParts.size() >= 2)
                {
                    auto sessionType = SessionType::SessionStringToSessionType(portParts[1]);
                    if(sessionType == SessionType::UNKNOWN)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("port session type invalid, configContent:%s, port str:%s"), configContent.c_str(), endianPartGroup[2].c_str());
                        return false;
                    }

                    remotePortSessionType = sessionType;
                }
            }

            if(endianPartGroup.size() >= 4)
            {
                remoteStack = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL;
                if(!endianPartGroup[3].empty())
                {
                    auto stackType = SERVICE_COMMON_NS::CrystalProtocolStackType::TurnFromString(endianPartGroup[3]);
                    if(stackType == SERVICE_COMMON_NS::CrystalProtocolStackType::UNKNOWN)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("unknown remote protocol stack configContent:%s, stack type:%s")
                        , configContent.c_str(), endianPartGroup[3].c_str());
                        return false;
                    }

                    remoteStack = stackType;
                }
            }

            continue;
        }
        
        g_Log->Warn(LOGFMT_OBJ_TAG("addr parse fail :have no local/remote sign configContent:%s, endianInfo:%s"), configContent.c_str(), endianInfo.c_str());
    }

    // 1.本地远程都有的情况 不支持两者不同的AF类型，要么同是ipv4,要么同是ipv6
    if(!localIp.empty() && !remoteIp.empty())
    {
        if( (KERNEL_NS::SocketUtil::IsIpv4(localIp) && !KERNEL_NS::SocketUtil::IsIpv4(remoteIp)) || 
            (!KERNEL_NS::SocketUtil::IsIpv4(localIp) &&  KERNEL_NS::SocketUtil::IsIpv4(remoteIp)) )
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("cant support diffrent af between local and remote ip local ip:%s, remote ip:%s"), localIp.c_str(), remoteIp.c_str());
                return false;
            }

        _localIp = localIp;
        _localPort = localPort;
        _remoteIp = remoteIp;
        _remotePort = remotePort;
        _listenSessionCount = listenSessionCount;
        _localProtocolStackType = localStack;
        _remoteProtocolStackType = remoteStack;

        _af = KERNEL_NS::SocketUtil::IsIpv4(localIp) ? AF_INET : AF_INET6;

        // 有远端地址，优先选择远端的端口作为sessionType的判断依据
        _sessionType = remotePortSessionType;
        _priorityLevel = priorityLevel;
        return true;
    }

    // 2.本地有,远程没有
    if(!localIp.empty())
    {
        _localIp = localIp;
        _localPort = localPort;
        _listenSessionCount = listenSessionCount;
        _localProtocolStackType = localStack;
        
        _af = KERNEL_NS::SocketUtil::IsIpv4(localIp) ? AF_INET : AF_INET6;
        _sessionType = localPortSessionType;

        _priorityLevel = priorityLevel;
        return true;
    }

    // 3.本地没有,远程有
    _remoteIp = remoteIp;
    _remotePort = remotePort;
    _remoteProtocolStackType = remoteStack;

    _af = KERNEL_NS::SocketUtil::IsIpv4(remoteIp) ? AF_INET : AF_INET6;
    _sessionType = remotePortSessionType;
    _priorityLevel = priorityLevel;

    return true;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(ServiceConfig);

bool ServiceConfig::Parse(const KERNEL_NS::LibString &seg, const KERNEL_NS::LibIniFile *ini)
{
    {// 监听地址配置
        KERNEL_NS::LibString listenAddrs;
        const KERNEL_NS::LibString sepAddrs = "|";
        if(ini->ReadStr(seg.c_str(), "TcpListen", listenAddrs))
        {
            const auto &addrGroup = listenAddrs.Split(sepAddrs);
            if(!addrGroup.empty())
            {
                for(auto &addrInfo : addrGroup)
                {
                    auto addrConfig = AddrConfig::New_AddrConfig();
                    if(!addrConfig->Parse(addrInfo))
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("addr parse fail addrInfo:%s, listenAddrs:%s"), addrInfo.c_str(), listenAddrs.c_str());
                        addrConfig->Release();
                        return false;
                    }

                    _listenAddrs.push_back(addrConfig);
                }
            }
        }
    }

    {// 控制中心地址配置
        KERNEL_NS::LibString centerAddr;
        if(ini->ReadStr(seg.c_str(), "CenterTcpAddr", centerAddr))
        {
            _centerAddr = AddrConfig::New_AddrConfig();
            if(!_centerAddr->Parse(centerAddr, true))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("addr parse fail centerAddr:%s"), centerAddr.c_str());
                _centerAddr->Release();
                _centerAddr = NULL;
                return false;
            }
        }
    }

    {// 远程连接地址
        KERNEL_NS::LibString connectAddrs;

        const KERNEL_NS::LibString sepAddrs = "|";
        if(ini->ReadStr(seg.c_str(), "ConnectAddrGroup", connectAddrs))
        {
            const auto &addrGroup = connectAddrs.Split(sepAddrs);
            if(!addrGroup.empty())
            {
                for(auto &addrInfo : addrGroup)
                {
                    auto addrConfig = AddrConfig::New_AddrConfig();
                    if(!addrConfig->Parse(addrInfo, true))
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("addr parse fail addrInfo:%s, connectAddrs:%s"), addrInfo.c_str(), connectAddrs.c_str());
                        addrConfig->Release();
                        return false;
                    }

                    _connectAddrGroup.push_back(addrConfig);
                }
            }
        }
    }

    {// 是否开启网络协议日志打印
        KERNEL_NS::LibString cache;
        _protoStackOpenLog = false;
        if(ini->ReadStr(seg.c_str(), "ProtoStackOpenLog", cache))
        {
            _protoStackOpenLog = (cache.strip().tolower()) == "true";
        }
    }

    {// 是否开启网络协议日志打印
        KERNEL_NS::LibString cache;
        _encryptKeyExpireTime = 3000;
        ini->CheckReadNumber(seg.c_str(), "EncryptKeyExpireTime", _encryptKeyExpireTime);
    }

    {// poller 配置
        if(!_ParsePoller(seg, ini))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("_ParsePoller fail"));
            return false;
        }
    }

    return true;
}

bool ServiceConfig::_ParsePoller(const KERNEL_NS::LibString &seg, const KERNEL_NS::LibIniFile *ini)
{
    {// 黑白名单规则
        UInt64 cache = 0;
        if(!ini->CheckReadUInt(seg.c_str(), "BlackWhiteListMode", cache))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                        , seg.c_str(), "BlackWhiteListMode");
            return false;
        }

        _pollerConfig._blackWhiteListFlag = static_cast<UInt32>(cache);
    }

    {// 会话数量
        UInt64 cache = 0;
        if(!ini->CheckReadUInt(seg.c_str(), "MaxSessionQuantity", cache))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                        , seg.c_str(), "MaxSessionQuantity");
            return false;
        }
        _pollerConfig._maxSessionQuantity = cache;
    }

    // tcp poller 相关配置
    auto &tcpPollerConfig = _pollerConfig._tcpPollerConfig;
    std::unordered_map<KERNEL_NS::LibString, Int32> pollerFeatureStringRefId;
    std::unordered_map<Int32, std::set<KERNEL_NS::LibString>> pollerFeatureIdRefString;  // poller feature id定义
    
    {
        UInt64 linkInOutPollerAmount = 0;
        if(!ini->CheckReadUInt(seg.c_str(), "LinkInOutPollerAmount", linkInOutPollerAmount))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("read ini fail seg:%s, key:%s")
                        , seg.c_str(), "LinkInOutPollerAmount");
            return false;
        }

        UInt64 dataTransferPollerAmount = 0;
        if(!ini->CheckReadUInt(seg.c_str(), "DataTransferPollerAmount", dataTransferPollerAmount))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                        , seg.c_str(), "DataTransferPollerAmount");
            return false;
        }

        UInt64 maxRecvBytesPerFrame = 0;
        {// 单帧最大接收数据量
            if(!ini->CheckReadUInt(seg.c_str(), "MaxRecvBytesPerFrame", maxRecvBytesPerFrame))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "MaxRecvBytesPerFrame");
                return false;
            }
        }

        UInt64 maxSendBytesPerFrame = 0;
        {// 单帧最大发送数据量
            if(!ini->CheckReadUInt(seg.c_str(), "MaxSendBytesPerFrame", maxSendBytesPerFrame))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "MaxSendBytesPerFrame");
                return false;
            }
        }

        UInt64 maxAcceptCountPerFrame = 0;
        {// 单帧最大处理连接数
            if(!ini->CheckReadUInt(seg.c_str(), "MaxAcceptCountPerFrame", maxAcceptCountPerFrame))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "MaxAcceptCountPerFrame");
                return false;
            }
        }

        UInt64 maxPieceTimeInMicroSecPerFrame = 0;
        {// 最大帧时间片
            if(!ini->CheckReadUInt(seg.c_str(), "MaxPieceTimeInMicroSecPerFrame", maxPieceTimeInMicroSecPerFrame))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "MaxPieceTimeInMicroSecPerFrame");
                return false;
            }
        }

        UInt64 maxPollerScanMilliseconds = 0;
        {// 最大poller扫描时间间隔
            if(!ini->CheckReadUInt(seg.c_str(), "MaxPollerScanMilliseconds", maxPollerScanMilliseconds))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "MaxPollerScanMilliseconds");
                return false;
            }
        }

        Int64 maxPollerMsgPriorityLevel = 0;
        {// 最大消息优先级等级
            if(!ini->CheckReadInt(seg.c_str(), "MaxPollerMsgPriorityLevel", maxPollerMsgPriorityLevel))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "MaxPollerMsgPriorityLevel");
                return false;
            }
        }

        Int64 pollerMonitorEventPriorityLevel = 0;
        {// 指定poller monitor事件的消息优先级等级
            if(!ini->CheckReadInt(seg.c_str(), "PollerMonitorEventPriorityLevel", pollerMonitorEventPriorityLevel))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "PollerMonitorEventPriorityLevel");
                return false;
            }
        }

        UInt64 sessionBufferCapicity = 0;
        {// session缓冲大小设置
            if(!ini->CheckReadUInt(seg.c_str(), "SessionBufferCapicity", sessionBufferCapicity))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "SessionBufferCapicity");
                return false;
            }
        }

        UInt64 sessionRecvPacketSpeedLimit = 0;
        {// session 限速
            if(!ini->CheckReadUInt(seg.c_str(), "SessionRecvPacketSpeedLimit", sessionRecvPacketSpeedLimit))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "SessionRecvPacketSpeedLimit");
                return false;
            }
        }

        UInt64 sessionRecvPacketSpeedTimeUnitMs = 0;
        {// session 限速时间单位
            if(!ini->CheckReadUInt(seg.c_str(), "SessionRecvPacketSpeedTimeUnitMs", sessionRecvPacketSpeedTimeUnitMs))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "SessionRecvPacketSpeedTimeUnitMs");
                return false;
            }
        }

        UInt64 sessionRecvPacketStackLimit = 0;
        {// session 收包堆叠上限
            if(!ini->CheckReadUInt(seg.c_str(), "SessionRecvPacketStackLimit", sessionRecvPacketStackLimit))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "SessionRecvPacketStackLimit");
                return false;
            }
        }

        UInt64 sessionRecvPacketContentLimit = 0;
        {// session 收包单包限制
            if(!ini->CheckReadUInt(seg.c_str(), "SessionRecvPacketContentLimit", sessionRecvPacketContentLimit))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "SessionRecvPacketContentLimit");
                return false;
            }
        }

        UInt64 sessionSendPacketContentLimit = 0;
        {// session 发包单包限制
            if(!ini->CheckReadUInt(seg.c_str(), "SessionSendPacketContentLimit", sessionSendPacketContentLimit))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "SessionSendPacketContentLimit");
                return false;
            }
        }


        {// poller feature定义
            KERNEL_NS::LibString cache;
            if(!ini->ReadStr(seg.c_str(), "PollerFeatureType", cache))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write str ini fail seg:%s, key:%s")
                            , seg.c_str(), "PollerFeatureType");
                return false;
            }

            const auto &featurePairParts = cache.Split(',');
            if(featurePairParts.size() < 2)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("have no poller feature please check seg:%s, key:%s, value:%s")
                            , seg.c_str(), "PollerFeatureType", cache.c_str());
                return false;
            }

            for(auto &part : featurePairParts)
            {
                const auto &items = part.Split(':');
                if(items.size() < 2)
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("feature format error please check seg:%s, key:%s, value:%s, part:%s")
                            , seg.c_str(), "PollerFeatureType", cache.c_str(), part.c_str());
                    return false;
                }

                const auto &featureString = items[0];
                const auto &featureIdString = items[1];
                if(!featureIdString.isdigit())
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("feature id format error please check seg:%s, key:%s, value:%s, part:%s, featureIdString:%s")
                            , seg.c_str(), "PollerFeatureType", cache.c_str(), part.c_str(), featureIdString.c_str());
                    return false;
                }

                if(pollerFeatureStringRefId.find(featureString) != pollerFeatureStringRefId.end())
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("repeate feature please check seg:%s, key:%s, value:%s, part:%s, featureString:%s")
                            , seg.c_str(), "PollerFeatureType", cache.c_str(), part.c_str(), featureString.c_str());
                    return false;
                }
                
                auto featureId = KERNEL_NS::StringUtil::StringToInt32(featureIdString.c_str());
                pollerFeatureStringRefId.insert(std::make_pair(featureString, featureId));
                
                auto iterFeatureString = pollerFeatureIdRefString.find(featureId);
                if(iterFeatureString == pollerFeatureIdRefString.end())
                    iterFeatureString = pollerFeatureIdRefString.insert(std::make_pair(featureId, std::set<KERNEL_NS::LibString>())).first;
                iterFeatureString->second.insert(featureString);
            }

            if(pollerFeatureStringRefId.empty())
            {
                g_Log->Error(LOGFMT_OBJ_TAG("lack of poller feature config please check seg:%s, key:%s")
                        , seg.c_str(), "PollerFeatureType");
                return false;
            }
        }
        
        {// linker相关配置
            auto iterLinker = pollerFeatureStringRefId.find("Linker");
            const auto linkerPollerFeatureId = iterLinker->second;
            auto iterPollerFeatureConfig = tcpPollerConfig._pollerFeatureRefConfig.find(linkerPollerFeatureId);
            KERNEL_NS::TcpPollerFeatureConfig *newPollerFeatureConfig = NULL;
            if(iterPollerFeatureConfig == tcpPollerConfig._pollerFeatureRefConfig.end())
            {
                newPollerFeatureConfig = KERNEL_NS::TcpPollerFeatureConfig::New_TcpPollerFeatureConfig(&tcpPollerConfig, linkerPollerFeatureId);
                tcpPollerConfig._pollerFeatureRefConfig.insert(std::make_pair(linkerPollerFeatureId, newPollerFeatureConfig));
            }
            else
            {
                newPollerFeatureConfig = iterPollerFeatureConfig->second;
            }

            if(static_cast<Int32>(newPollerFeatureConfig->_pollerInstConfigs.size()) < linkInOutPollerAmount)
                newPollerFeatureConfig->_pollerInstConfigs.resize(static_cast<size_t>(linkInOutPollerAmount));

            // 创建配置
            auto &pollerInstConfigs = newPollerFeatureConfig->_pollerInstConfigs;
            for(Int32 idx = 0; idx < linkInOutPollerAmount; ++idx)
            {
                auto newInstConfig = pollerInstConfigs[idx];
                if(newInstConfig)
                    continue;

                newInstConfig = KERNEL_NS::TcpPollerInstConfig::New_TcpPollerInstConfig(newPollerFeatureConfig, static_cast<UInt32>(idx + 1));
                newInstConfig->_handleRecvBytesPerFrameLimit = maxRecvBytesPerFrame;
                newInstConfig->_handleSendBytesPerFrameLimit = maxSendBytesPerFrame;
                newInstConfig->_handleAcceptPerFrameLimit = maxAcceptCountPerFrame;
                newInstConfig->_maxPieceTimeInMicroseconds = maxPieceTimeInMicroSecPerFrame;
                newInstConfig->_maxSleepMilliseconds = maxPollerScanMilliseconds;
                newInstConfig->_maxPriorityLevel = static_cast<Int32>(maxPollerMsgPriorityLevel);
                newInstConfig->_pollerInstMonitorPriorityLevel = static_cast<Int32>(pollerMonitorEventPriorityLevel);
                newInstConfig->_bufferCapacity = sessionBufferCapicity;
                newInstConfig->_sessionRecvPacketSpeedLimit = sessionRecvPacketSpeedLimit;
                newInstConfig->_sessionRecvPacketSpeedTimeUnitMs = sessionRecvPacketSpeedTimeUnitMs;
                newInstConfig->_sessionRecvPacketStackLimit = sessionRecvPacketStackLimit;
                pollerInstConfigs[idx] = newInstConfig;
            }
        }
       
        {// transfer相关配置
            auto iterTransfer = pollerFeatureStringRefId.find("DataTransfer");
            const auto transferPollerFeatureId = iterTransfer->second;
            auto iterPollerFeatureConfig = tcpPollerConfig._pollerFeatureRefConfig.find(transferPollerFeatureId);
            KERNEL_NS::TcpPollerFeatureConfig *newPollerFeatureConfig = NULL;
            if(iterPollerFeatureConfig == tcpPollerConfig._pollerFeatureRefConfig.end())
            {
                newPollerFeatureConfig = KERNEL_NS::TcpPollerFeatureConfig::New_TcpPollerFeatureConfig(&tcpPollerConfig, transferPollerFeatureId);
                tcpPollerConfig._pollerFeatureRefConfig.insert(std::make_pair(transferPollerFeatureId, newPollerFeatureConfig));
            }
            else
            {
                newPollerFeatureConfig = iterPollerFeatureConfig->second;
            }

            if(static_cast<Int32>(newPollerFeatureConfig->_pollerInstConfigs.size()) < dataTransferPollerAmount)
                newPollerFeatureConfig->_pollerInstConfigs.resize(static_cast<size_t>(dataTransferPollerAmount));

            // 创建配置
            auto &pollerInstConfigs = newPollerFeatureConfig->_pollerInstConfigs;
            for(Int32 idx = 0; idx < dataTransferPollerAmount; ++idx)
            {
                auto newInstConfig = pollerInstConfigs[idx];
                if(newInstConfig)
                    continue;

                newInstConfig = KERNEL_NS::TcpPollerInstConfig::New_TcpPollerInstConfig(newPollerFeatureConfig,  static_cast<UInt32>(idx + 1));
                newInstConfig->_handleRecvBytesPerFrameLimit = maxRecvBytesPerFrame;
                newInstConfig->_handleSendBytesPerFrameLimit = maxSendBytesPerFrame;
                newInstConfig->_handleAcceptPerFrameLimit = maxAcceptCountPerFrame;
                newInstConfig->_maxPieceTimeInMicroseconds = maxPieceTimeInMicroSecPerFrame;
                newInstConfig->_maxSleepMilliseconds = maxPollerScanMilliseconds;
                newInstConfig->_maxPriorityLevel = static_cast<Int32>(maxPollerMsgPriorityLevel);
                newInstConfig->_pollerInstMonitorPriorityLevel = static_cast<Int32>(pollerMonitorEventPriorityLevel);
                newInstConfig->_bufferCapacity = sessionBufferCapicity;
                newInstConfig->_sessionRecvPacketSpeedLimit = sessionRecvPacketSpeedLimit;
                newInstConfig->_sessionRecvPacketSpeedTimeUnitMs = sessionRecvPacketSpeedTimeUnitMs;
                newInstConfig->_sessionRecvPacketStackLimit = sessionRecvPacketStackLimit;
                pollerInstConfigs[idx] = newInstConfig;
            }
        }
    }

    _pollerConfig._pollerFeatureIdRefString = pollerFeatureIdRefString;
    _pollerConfig._pollerFeatureStringRefId = pollerFeatureStringRefId;
    
    return true;
}

SERVICE_END
