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

    bool Parse(const KERNEL_NS::LibString &configContent, const std::unordered_map<UInt16, Int32> &portRefSessinType, bool isStreamSock = true);

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
        info.AppendFormat("local addr:%s:%hu, remote addr:%s:%hu, af:%d, session type:%d, priority level:%u"
                            ,_localIp.c_str(), _localPort
                            ,  _remoteIp.c_str(), _remotePort
                            , _af, _sessionType, _priorityLevel);

        return info;
    }

    KERNEL_NS::LibString _localIp;
    UInt16 _localPort = 0;
    Int32 _listenSessionCount = 1;
    Int32 _localProtocolStackType = 0;   // 协议栈类型

    KERNEL_NS::LibString _remoteIp;
    UInt16 _remotePort = 0;
    Int32 _remoteProtocolStackType = 0;   // 协议栈类型

    Int32 _af = AF_INET;         // ipv4/ipv6
    Int32 _sessionType = 0;      // 配置的会话类型
    UInt32 _priorityLevel = 0;   // 消息优先级
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

    std::vector<AddrConfig *> _listenAddrs;
    std::vector<AddrConfig *> _connectAddrGroup;

    AddrConfig *_centerAddr;    // 控制中心
    std::unordered_map<UInt16, Int32> _portRefSessionType;  // 端口的会话类型配置 PORT_SESSION_TYPE
    bool _protoStackOpenLog;
    Int64 _encryptKeyExpireTime;
};

SERVICE_END