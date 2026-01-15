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
#include <service/common/SessionType.h>
#include <service_common/protocol/protocol.h>
#include <service_common/common/Configs.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(AddrConfig);


bool AddrConfig::Parse(const KERNEL_NS::LibString &configContent)
{
   // Local,127.0.0.1,3901-Remote,127.0.0.1,3901/INNER
    // ip:端口/ipv4 [=> ip:端口/ipv4]|会啊哈类型,消息优先级,会话数量,协议栈类型
    const KERNEL_NS::LibString sessionSep = "!Attr:";
    const KERNEL_NS::LibString linkSep = "=>";

    auto &&linkSessionParts = configContent.Split(sessionSep);

    // 没有任何信息
    if(linkSessionParts.empty())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("addr config error:%s"), configContent.c_str());
        return false;
    }

    // 默认认为只有连接地址, 没有会话信息
    KERNEL_NS::LibString linkContent;
    KERNEL_NS::LibString sessionConfigContent;
    if(linkSessionParts.size() == 1LLU)
    {
        linkContent = linkSessionParts[0];
    }
    // 指定了连接地址和会话信息
    else if(linkSessionParts.size() >= 2LLU)
    {
        linkContent = linkSessionParts[0];
        sessionConfigContent = linkSessionParts[1];
    }

    linkContent.strip();
    sessionConfigContent.strip();

    // 解析会话信息
    {
        _sessionType = SessionType::INNER;
        _protocolStackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL;
        _listenSessionCount = 1;

        do
        {
            if(sessionConfigContent.empty())
                break;

            auto &&sessionParts = sessionConfigContent.Split(',');
            // 协议栈类型
            if(sessionParts.size() >= 1LLU)
            {
                const auto &stackTypeStr = sessionParts[0].strip();
                if(!stackTypeStr.empty())
                {
                    // 大小写无感全部转化成大写
                    auto stackType = SERVICE_COMMON_NS::CrystalProtocolStackType::TurnFromString(stackTypeStr.toupper());
                    if(stackType == SERVICE_COMMON_NS::CrystalProtocolStackType::UNKNOWN)
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("unknown protocol stack configContent:%s, stack type:%s,sessionConfigContent:%s")
                        , configContent.c_str(), stackTypeStr.c_str(), sessionConfigContent.c_str());
                        return false;
                    }

                    _protocolStackType = stackType;
                }
            }

            // 会话类型
            if(sessionParts.size() >= 2LLU)
            {
                const auto &part = sessionParts[1].strip();
                if(!part.empty())
                {
                    // 大小写无感全部转化成大写
                    const auto sessionType = SessionType::SessionStringToSessionType(part.toupper());
                    if(sessionType == SessionType::UNKNOWN)
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("bad session type:%s, sessionConfigContent:%s, configContent:%s")
                        , part.c_str(), sessionConfigContent.c_str(), configContent.c_str());
                        return false;
                    }

                    _sessionType = sessionType;
                }
            }

            // 消息优先级
            if(sessionParts.size() >= 3LLU)
            {
                const auto &part = sessionParts[2].strip();
                if(!part.empty())
                {
                    if(!part.isdigit())
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("bad priority level:%s, sessionConfigContent:%s, configContent:%s")
                            , part.c_str(), sessionConfigContent.c_str(), configContent.c_str());
                        return false;
                    }
                }
            }

            // 监听会话数量
            if(sessionParts.size() >= 4LLU)
            {
                const auto &part = sessionParts[3].strip();

                // 监听同一个端口会话数量
                if(!part.empty())
                {
                    if(!part.isdigit())
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("listen session count not digit, configContent:%s, listen session count:%s, sessionConfigContent:%s")
                        , configContent.c_str(), part.c_str(), sessionConfigContent.c_str());
                        return false;
                    }

                    _listenSessionCount = KERNEL_NS::StringUtil::StringToInt32(part.c_str());
                }   
            }
            
        } while (false);
    }

    // 解析连接信息
    {
        auto &&linkParts = linkContent.Split("=>");
        if(linkParts.empty())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("bad link config, configContent:%s, linkContent:%s")
            , configContent.c_str(), linkContent.c_str());

            return false;
        }

        // 至少有监听地址信息
        if(linkParts.size() >= 1LLU)
        {
            auto &&localAddrInfo = linkParts[0].strip();
            if(!localAddrInfo.empty())
            {
                if(!ParseIpInfo(localAddrInfo, _localIp, _localPort))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("bad local addr, configContent:%s, linkContent:%s, localAddrInfo:%s")
                    , configContent.c_str(), linkContent.c_str(), localAddrInfo.c_str());
                    return false;
                }

                // windows下同一个端口只能一个session
                #if CRYSTAL_TARGET_PLATFORM_WINDOWS
                if(_localPort != 0)
                    _listenSessionCount = 1;
                #endif

                // 被绑定的地址只能是ip
                if(!KERNEL_NS::SocketUtil::IsIp(_localIp._ip))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("local addr:%s, not a ip addr, configContent:%s, linkContent:%s, localAddrInfo:%s")
                    , _localIp._ip.c_str(), configContent.c_str(), linkContent.c_str(), localAddrInfo.c_str());
                    return false;
                }
            }
        }

        // 至少有远程地址信息
        if(linkParts.size() >= 2LLU)
        {
            auto &&remoteAddrInfo = linkParts[1].strip();
            if(!remoteAddrInfo.empty())
            {
                if(!ParseIpInfo(remoteAddrInfo, _remoteIp, _remotePort))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("bad remote addr, configContent:%s, linkContent:%s, remoteAddrInfo:%s")
                    , configContent.c_str(), linkContent.c_str(), remoteAddrInfo.c_str());
                    return false;
                }
            }
        }

        // 校验同时没有监听地址信息, 也没有远程地址信息, 报错
        if(_localIp._ip.empty() && _remoteIp._ip.empty())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("have no local or remote addr info configContent:%s, linkContent:%s"), configContent.c_str(), linkContent.c_str());
            return false;
        }

        // 是连接的由远程ip决定, ip是ipv4还是, ipv6，亦或者如果是域名, 它如果要转成ipv4的则af是ipv4, 如果是ipv6的则af是ipv6
        if(!_remoteIp._ip.empty())
        {
            if(!_remoteIp._isHostName)
            {
                _af = KERNEL_NS::SocketUtil::IsIpv4(_remoteIp._ip) ? AF_INET : AF_INET6;
            }
            else
            {
                _af = _remoteIp._toIpv4 ? AF_INET : AF_INET6;
            }
        }
        else
        {
            _af = KERNEL_NS::SocketUtil::IsIpv4(_localIp._ip) ? AF_INET : AF_INET6;
        }

        // local / remote ip必须是同一个性质, 要么同是ipv4, 要么同是ipv6, 如果远程的是域名, 那么得判断_toIpv4
        if(!_remoteIp._ip.empty())
        {
            // 如果是ip需要判断两种同是ipv4或者ipv6
            if(!_remoteIp._isHostName)
            {
                if(KERNEL_NS::SocketUtil::IsIpv4(_localIp._ip) && KERNEL_NS::SocketUtil::IsIpv6(_remoteIp._ip))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("local ip:%s is ipv4, remote ip:%s is ipv6, invalid pair configContent:%s, linkContent:%s")
                    , _localIp._ip.c_str(), _remoteIp._ip.c_str(), configContent.c_str(), linkContent.c_str());
                    return false;
                }

                if(KERNEL_NS::SocketUtil::IsIpv6(_localIp._ip) && KERNEL_NS::SocketUtil::IsIpv4(_remoteIp._ip))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("local ip:%s is ipv6, remote ip:%s is ipv4, invalid pair configContent:%s, linkContent:%s")
                    , _localIp._ip.c_str(), _remoteIp._ip.c_str(), configContent.c_str(), linkContent.c_str());
                    return false;
                }
            }
            else
            {
                if(KERNEL_NS::SocketUtil::IsIpv4(_localIp._ip) && !_remoteIp._toIpv4)
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("local ip:%s is ipv4, remote ip:%s will turn ipv6, invalid pair configContent:%s, linkContent:%s")
                    , _localIp._ip.c_str(), _remoteIp._ip.c_str(), configContent.c_str(), linkContent.c_str());
                    return false;
                }

                if(KERNEL_NS::SocketUtil::IsIpv6(_localIp._ip) && _remoteIp._toIpv4)
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("local ip:%s is ipv6, remote ip:%s will turn ipv4, invalid pair configContent:%s, linkContent:%s")
                    , _localIp._ip.c_str(), _remoteIp._ip.c_str(), configContent.c_str(), linkContent.c_str());
                    return false;
                }
            }
        }
    }

    return true;
}

bool AddrConfig::ParseIpInfo(const KERNEL_NS::LibString &addrInfo, KERNEL_NS::AddrIpConfig &ipConfig, UInt16 &port)
{
    if(addrInfo.empty())
        return false;

    // 域名的转化类型
    auto &&addrTurnTypeParts = addrInfo.Split("/");
    if(addrTurnTypeParts.empty())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("bad addrInfo:%s"), addrInfo.c_str());
        return false;
    }

    // 只有是域名的时候才用的上
    ipConfig._toIpv4 = true;

    // 至少有地址信息
    if(addrTurnTypeParts.size() >= 1LLU)
    {
        auto &&addrPartStr = addrTurnTypeParts[0].strip();
        
        // 拆分成端口
        if(!addrPartStr.empty())
        {
            auto &&ip2PortParts = addrPartStr.Split('$');
            if(ip2PortParts.size() < 2LLU)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("bad addrInfo:%s, addrPartStr:%s"), addrInfo.c_str(), addrPartStr.c_str());
                return false;
            }

            ipConfig._ip = ip2PortParts[0].strip();
            ipConfig._isHostName = !KERNEL_NS::SocketUtil::IsIp(ipConfig._ip);

            port = 0;
            auto &portPart = ip2PortParts[1];
            if(!portPart.empty())
            {
                portPart.strip();
                if(!portPart.isdigit())
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("bad addr port:%s, addrInfo:%s, addrPartStr:%s"), portPart.c_str(), addrInfo.c_str(), addrPartStr.c_str());
                    return false;
                }

                port = KERNEL_NS::StringUtil::StringToUInt16(portPart.c_str());
            }
        }
    }

    // 还有域名转化ip地址信息
    if(addrTurnTypeParts.size() >= 2LLU)
    {
        // 大小写不敏感全部转成小写
        auto &&remoteAttr = addrTurnTypeParts[1].strip();
        auto &&remoteAttrParts = remoteAttr.Split(',');

        if(remoteAttrParts.empty())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("remote addr attr empty addrInfo:%s, remoteAttr:%s"), addrInfo.c_str(), remoteAttr.c_str());
            return false;
        }

        if(remoteAttrParts.size() >= 1LLU)
        {
            auto &&turnIpType = remoteAttrParts[0].strip().tolower();
            if(turnIpType == "ipv6")
            {
                ipConfig._toIpv4 = false;
            }
            else
            {
                ipConfig._toIpv4 = true;
            }
        }

        if(remoteAttrParts.size() >= 2LLU)
        {
            auto &&maxSwitchIpCountStr = remoteAttrParts[1].strip();
            if(!maxSwitchIpCountStr.empty())
            {
                if(!maxSwitchIpCountStr.isdigit())
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("maxSwitchIpCountStr:%s not digit, addrInfo:%s, remoteAttr:%s"), maxSwitchIpCountStr.c_str(), addrInfo.c_str(), remoteAttr.c_str());
                    return false;
                }

                ipConfig._mostSwitchIpCount = KERNEL_NS::StringUtil::StringToInt32(maxSwitchIpCountStr.c_str());
            }
        }
    }

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
                const Int32 count = static_cast<Int32>(addrGroup.size());
                for(Int32 idx = 0; idx < count; ++idx)
                {
                    auto &&addrInfo = addrGroup[idx].strip();
                    if(addrInfo.empty())
                        continue;

                    auto addrConfig = AddrConfig::New_AddrConfig();
                    if(!addrConfig->Parse(addrInfo))
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("addr parse fail addrInfo:%s, idx:%d listenAddrs:%s"), addrInfo.c_str(), idx, listenAddrs.c_str());
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
            centerAddr.strip();
            if(!centerAddr.empty())
            {
                _centerAddr = AddrConfig::New_AddrConfig();
                if(!_centerAddr->Parse(centerAddr))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("addr parse fail centerAddr:%s"), centerAddr.c_str());
                    _centerAddr->Release();
                    _centerAddr = NULL;
                    return false;
                }
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
                const Int32 count = static_cast<Int32>(addrGroup.size());
                for(Int32 idx = 0; idx < count; ++idx)
                {
                    auto &&addrInfo = addrGroup[idx].strip();
                    if(addrInfo.empty())
                        continue;

                    auto addrConfig = AddrConfig::New_AddrConfig();
                    if(!addrConfig->Parse(addrInfo))
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

            if(static_cast<UInt64>(newPollerFeatureConfig->_pollerInstConfigs.size()) < linkInOutPollerAmount)
                newPollerFeatureConfig->_pollerInstConfigs.resize(static_cast<size_t>(linkInOutPollerAmount));

            // 创建配置
            auto &pollerInstConfigs = newPollerFeatureConfig->_pollerInstConfigs;
            for(Int32 idx = 0; idx < static_cast<Int32>(linkInOutPollerAmount); ++idx)
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

            if(static_cast<UInt64>(newPollerFeatureConfig->_pollerInstConfigs.size()) < dataTransferPollerAmount)
                newPollerFeatureConfig->_pollerInstConfigs.resize(static_cast<size_t>(dataTransferPollerAmount));

            // 创建配置
            auto &pollerInstConfigs = newPollerFeatureConfig->_pollerInstConfigs;
            for(Int32 idx = 0; idx < static_cast<Int32>(dataTransferPollerAmount); ++idx)
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
