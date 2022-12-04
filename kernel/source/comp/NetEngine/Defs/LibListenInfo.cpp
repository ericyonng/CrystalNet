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
#include <kernel/comp/NetEngine/Defs/LibListenInfo.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibListenInfo);

LibListenInfo::LibListenInfo()
:_port(0)
,_family(0)
,_serviceId(0)
,_stub(0)
,_priorityLevel(0)
{

}

LibString LibListenInfo::ToString() const
{
    LibString info;
    info.AppendFormat("_ip:%s, ", _ip.c_str())
    .AppendFormat("_port:%hu, ",  _port)
    .AppendFormat("_family:%d, ",  _family)
    .AppendFormat("_serviceId:%llu, ",  _serviceId)
    .AppendFormat("_stub:%llu, ",  _stub)
    .AppendFormat("_priorityLevel:%u, ",  _priorityLevel)
    .AppendFormat("_sessionOption:%s, ",  _sessionOption.ToString().c_str())
    ;

    return info;
}

KERNEL_END