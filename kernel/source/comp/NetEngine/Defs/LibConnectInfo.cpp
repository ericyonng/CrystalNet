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
 * Author: Eric Yonng
 * Date: 2021-03-22 15:23:43
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Defs/LibConnectInfo.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/Timer/Timer.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibConnectInfo);

LibConnectInfo::LibConnectInfo()
:_localPort(0)
,_targetPort(0)
,_family(0)
,_protocolType(0)
,_priorityLevel(0)
,_pollerId(0)
,_retryTimes(0)
,_periodMs(0)
,_stub(0)
,_fromServiceId(0)
,_stack(NULL)
{

}

POOL_CREATE_OBJ_DEFAULT_IMPL(LibConnectPendingInfo);

LibConnectPendingInfo::LibConnectPendingInfo()
:_connectInfo(NULL)
,_reconnectTimer(NULL)
,_leftRetryTimes(0)
,_newSock(INVALID_SOCKET)
,_sessionId(0)
,_localAddr(true)
,_remoteAddr(true)
,_finalLocalPort(0)
{

}

LibString LibConnectPendingInfo::ToString() const
{
    LibString info;
    info.AppendFormat("connect info:%s, ", _connectInfo->ToString().c_str())
    .AppendFormat("reconnect timer:%s", _reconnectTimer ? _reconnectTimer->ToString().c_str() : "no retry timer")
    .AppendFormat("left retry times:%d, new sock:%d, sessionId:%llu, local addr:%s, remote addr:%s, final local ip:[%s:%hu]"
    , _leftRetryTimes, _newSock, _sessionId, _localAddr.ToString().c_str()
    , _remoteAddr.ToString().c_str(), _finalLocalIp.c_str(), _finalLocalPort);

    return info;
}

KERNEL_END