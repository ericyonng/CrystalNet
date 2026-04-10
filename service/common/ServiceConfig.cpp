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

SERVICE_END
