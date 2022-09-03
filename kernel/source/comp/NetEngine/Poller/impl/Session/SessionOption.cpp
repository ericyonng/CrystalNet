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
 * Date: 2022-05-28 02:07:44
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Poller/impl/Session/SessionOption.h>

KERNEL_BEGIN
SessionOption::SessionOption()
:_noDelay(true)
,_sockSendBufferSize(0)
,_sockRecvBufferSize(0)
,_sessionBufferCapicity(0)
,_maxPacketSize(0)
,_forbidRecv(false)
,_sessionType(0)
{

}

LibString SessionOption::ToString() const
{
    LibString info;
    info.AppendFormat("_noDelay:%s, ", _noDelay ? "true":"false")
        .AppendFormat("_sockSendBufferSize:%llu, ", _sockSendBufferSize)
        .AppendFormat("_sockRecvBufferSize:%llu, ", _sockRecvBufferSize)
        .AppendFormat("_sessionBufferCapicity:%llu, ", _sessionBufferCapicity)
        .AppendFormat("_maxPacketSize:%llu, ", _maxPacketSize)
        .AppendFormat("_forbidRecv:%d, ", static_cast<Int32>(_forbidRecv))
        .AppendFormat("_sessionType:%d, ", _sessionType)
        ;
    return info;
}


KERNEL_END