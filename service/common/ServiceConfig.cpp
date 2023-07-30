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
#include <service/common/ServiceConfig.h>
#include <service/common/PriorityLevelDefine.h>
#include <service/common/SessionType.h>
#include <service_common/protocol/protocol.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(AddrConfig);

bool AddrConfig::Parse(const KERNEL_NS::LibString &configContent, const std::unordered_map<UInt16, Int32> &portRefSessinType, bool isStreamSock)
{
    // Local,127.0.0.1,3901-Remote,127.0.0.1,3901/INNER
    const KERNEL_NS::LibString inclineSep = "@";
    const KERNEL_NS::LibString horizontalSep = "-";
    const KERNEL_NS::LibString elemSep = ",";
    const KERNEL_NS::LibString localSign = "local";
    const KERNEL_NS::LibString remoteSign = "remote";
    const KERNEL_NS::LibString hostNameSep = "'";
    const KERNEL_NS::LibString ipv6Flag = "ipv6";
    
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
    Int32 localStack = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL;
    KERNEL_NS::LibString remoteIp;
    Int32 listenSessionCount = 1;
    UInt16 remotePort = 0;
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
                if(!endianPartGroup[2].isdigit())
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("port not digit, configContent:%s, port:%s"), configContent.c_str(), endianPartGroup[2].c_str());
                    return false;
                }

                localPort = KERNEL_NS::StringUtil::StringToUInt16(endianPartGroup[2].c_str());
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
                if(!endianPartGroup[2].isdigit())
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("port not digit, configContent:%s, port:%s"), configContent.c_str(), endianPartGroup[2].c_str());
                    return false;
                }

                remotePort = KERNEL_NS::StringUtil::StringToUInt16(endianPartGroup[2].c_str());
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
        _sessionType = SessionType::INNER;

        auto iter = portRefSessinType.find(_remotePort);
        if(iter != portRefSessinType.end())
            _sessionType = iter->second;

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
        _sessionType = SessionType::INNER;
        auto iter = portRefSessinType.find(_localPort);
        if(iter != portRefSessinType.end())
            _sessionType = iter->second;

        _priorityLevel = priorityLevel;
        return true;
    }

    // 3.本地没有,远程有
    _remoteIp = remoteIp;
    _remotePort = remotePort;
    _remoteProtocolStackType = remoteStack;

    _af = KERNEL_NS::SocketUtil::IsIpv4(remoteIp) ? AF_INET : AF_INET6;
    _sessionType = SessionType::INNER;
    auto iter = portRefSessinType.find(_remotePort);
    if(iter != portRefSessinType.end())
        _sessionType = iter->second;

    _priorityLevel = priorityLevel;

    return true;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(ServiceConfig);

bool ServiceConfig::Parse(const KERNEL_NS::LibString &seg, const KERNEL_NS::LibIniFile *ini)
{
    {// 端口的会话类型配置
        KERNEL_NS::LibString portSessionTypes;
        const KERNEL_NS::LibString seps = "|";
        const KERNEL_NS::LibString sepPair = ",";
        if(ini->ReadStr(seg.c_str(), "PORT_SESSION_TYPE", portSessionTypes))
        {
            const auto &group = portSessionTypes.Split(seps);
            for(auto &portInfo : group)
            {
                const auto &portInfoPair = portInfo.Split(sepPair);
                if(portInfoPair.size() != 2)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("bad port session config portInfo:%s, portSessionTypes:%s")
                                    , portInfo.c_str(), portSessionTypes.c_str());
                    continue;
                }

                UInt16 port = KERNEL_NS::StringUtil::StringToUInt16(portInfoPair[0].c_str());
                Int32 sessionType = SessionType::SessionStringToSessionType(portInfoPair[1]);
                if(sessionType == SessionType::UNKNOWN)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("bad port session type config:unknown session type decription, portInfo:%s, portSessionTypes:%s ")
                    , portInfo.c_str(), portSessionTypes.c_str());
                    continue;
                }

                _portRefSessionType.insert(std::make_pair(port, sessionType));
            }
        }
    }

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
                    if(!addrConfig->Parse(addrInfo, _portRefSessionType))
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
            if(!_centerAddr->Parse(centerAddr, _portRefSessionType, true))
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
                    if(!addrConfig->Parse(addrInfo, _portRefSessionType, true))
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

    return true;
}
SERVICE_END
