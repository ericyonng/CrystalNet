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
 * Date: 2024-08-24 22:00:54
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Defs/AddrIpConfig.h>
#include <kernel/comp/Utils/SocketUtil.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(AddrIpConfig);

KERNEL_NS::LibString AddrIpConfig::ToString() const
{
    return KERNEL_NS::LibString().AppendFormat("%s, toipv4:%s,most switch ip count:%d", _ip.c_str(), _toIpv4?"true":"false", _mostSwitchIpCount);
}

Int32 AddrIpConfig::GetAf() const
{
    if(_isHostName)
        return _toIpv4 ? AF_INET : AF_INET6;

    return KERNEL_NS::SocketUtil::IsIpv4(_ip) ? AF_INET : AF_INET6;
}

KERNEL_END
