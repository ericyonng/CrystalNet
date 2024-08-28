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
 * 
 * Author: Eric Yonng
 * Date: 2021-03-27 15:08:44
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Defs/BuildSessionInfo.h>
#include <kernel/comp/Utils/StringUtil.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(BuildSessionInfo);

BuildSessionInfo::BuildSessionInfo(bool isIpv4)
    :_protocolType(0)
    ,_family(0)
    ,_sessionId(0)
    ,_localAddr(isIpv4)
    ,_remoteAddr(isIpv4)
    ,_sock(INVALID_SOCKET)
    ,_serviceId(0)
    ,_stub(0)
    ,_priorityLevel(0)
    ,_isFromConnect(false)
    ,_protocolStack(NULL)
    ,_isLinker(false)
    ,_isWinSockNonBlock(false)
{

}

LibString BuildSessionInfo::ToString() const
{
    LibString info;
    info
        .AppendFormat("_protocolType:%d, ", _protocolType)
        .AppendFormat("_family:%hu, ", _family)
        .AppendFormat("_sessionId:%llu, ", _sessionId)
        .AppendFormat("_localAddr:%s, ", _localAddr.ToString().c_str())
        .AppendFormat("_remoteAddr:%s, ", _remoteAddr.ToString().c_str())
        .AppendFormat("_sock:%d, ", _sock)
        .AppendFormat("_serviceId:%llu, ", _serviceId)
        .AppendFormat("_stub:%llu, ", _stub)
        .AppendFormat("_priorityLevel:%u, ", _priorityLevel)
        .AppendFormat("_isFromConnect:%d, ", _isFromConnect)
        .AppendFormat("_protocolStack:%p, ", _protocolStack)
        .AppendFormat("_isLinker:%d, ", _isLinker)
        .AppendFormat("_isWinSockNonBlock:%d, ", _isWinSockNonBlock)
        .AppendFormat("_sessionOption:%s, ", _sessionOption.ToString().c_str())
        .AppendFormat("_remoteOriginIpConfig:%s, ", _remoteOriginIpConfig.ToString().c_str())
        .AppendFormat("_failureIps:%s, ", KERNEL_NS::StringUtil::ToString(_failureIps, ',').c_str())
        ;

    return info;
}

KERNEL_END
