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
 * Date: 2021-09-13 00:25:52
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/LibAddr.h>
#include <kernel/comp/NetEngine/LibSocket.h>
#include <kernel/comp/Utils/SocketUtil.h>
#include <kernel/comp/NetEngine/Defs/NetDefs.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibAddr);

LibAddr::LibAddr(LibSocket *sock)
    :_sock(sock)
    ,_prepareLocalPort(0)
    ,_localAddr(true)
    ,_remoteAddr(true)
{

}

LibAddr::~LibAddr()
{

}

Int32 LibAddr::UpdateRemoteAddr()
{
    SOCKET sock = _sock->GetSock();
    if(UNLIKELY(!SocketUtil::IsValidSock(sock)))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("invalid sock"));
        return Status::Socket_InvalidSocket;
    }

    Int32 family = _sock->GetFamily();
    BriefSockAddr cache(family == AF_INET);
    Int32 errCode = SocketUtil::UpdateRemoteAddr(family, sock, cache);
    if(errCode != Status::Success)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("UpdateRemoteAddr fail errCode:%d, _family:%d, sock:%d"), errCode, family, sock);
        return errCode;
    }

    _remoteAddr = cache;
    g_Log->NetTrace(LOGFMT_OBJ_TAG("sock[%d] _family[%hu] remote addr info [%s]")
    ,_sock->GetSock(), family, _remoteAddr.ToString().c_str());

    return Status::Success;
}

Int32 LibAddr::UpdateLocalAddr()
{
    SOCKET sock = _sock->GetSock();
    if(UNLIKELY(!SocketUtil::IsValidSock(sock)))
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("invalid sock"));
        return Status::Socket_InvalidSocket;
    }

    Int32 family = _sock->GetFamily();
    BriefSockAddr cache(family == AF_INET);
    Int32 errCode = SocketUtil::UpdateLocalAddr(sock, cache);
    if(errCode != Status::Success)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("UpdateLocalAddr fail errCode:%d, _family:%d, _sock:%d"), errCode, family, sock);
        return errCode;
    }

    _localAddr = cache;
    _prepareLocalIp = _localAddr._ip;
    _prepareLocalPort = _localAddr._port;
    _prepareLocalIpAndPort = _localAddr._ipAndPort;

    g_Log->NetTrace(LOGFMT_OBJ_TAG("sock[%d] _family[%hu] local addr info[%s]")
    ,_sock->GetSock(), family, _localAddr.ToString().c_str());
    return Status::Success;
}

LibString LibAddr::ToString() const
{
    LibString info;
    const UInt16 family = static_cast<UInt16>(_sock->GetFamily());
    info.AppendFormat("[ADDR INFO]: _sock[%d], family[%hu, %s], _prepareLocalIp[%s], _prepareLocalPort[%hu] remote addr info[%s], local addr info[%s]"
                    , _sock->GetSock()
                    , family
                    , FamilyType::ToString(family)
                    , _prepareLocalIp.c_str()
                    , _prepareLocalPort
                    , _remoteAddr.ToString().c_str()
                    , _localAddr.ToString().c_str());

    return info;
}

UInt16 LibAddr::GetFamily() const
{
    return _sock->GetFamily();
}

KERNEL_END
