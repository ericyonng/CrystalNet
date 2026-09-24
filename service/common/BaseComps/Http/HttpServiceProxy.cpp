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
 * Date: 2026-09-24 10:00:00
 * Author: CrystalNet
 * Description: http服务代理实现
*/

#include <pch.h>
#include <kernel/comp/Http/HttpServiceProxy.h>
#include <kernel/comp/Poller/Poller.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

HttpServiceProxy::HttpServiceProxy()
    :IServiceProxy(KERNEL_NS::RttiUtil::GetTypeId<HttpServiceProxy>())
    , _eventPoller(NULL)
    , _stack(NULL)
{
}

HttpServiceProxy::~HttpServiceProxy()
{
}

void HttpServiceProxy::Release()
{
    HttpServiceProxy::Delete_HttpServiceProxy(this);
}

void HttpServiceProxy::SetEventPoller(Poller *poller)
{
    _eventPoller.store(poller, std::memory_order_release);
}

void HttpServiceProxy::SetProtocolStack(IProtocolStack *stack)
{
    _stack = stack;
}

void HttpServiceProxy::PostMsg(UInt64 serviceId, PollerEvent *msg, Int64 packetsCount)
{
    auto poller = _eventPoller.load(std::memory_order_acquire);
    if (UNLIKELY(!poller))
    {
        if (g_Log)
        {
            CLOG_ERROR("event poller is null, will drop msg service id:%llu, msg:%s", serviceId, msg->ToString().c_str());
        }

        msg->Release();
        return;
    }

    poller->Push(msg);
}

void HttpServiceProxy::PostQuitService()
{
}

IProtocolStack *HttpServiceProxy::GetProtocolStack(LibSession *session)
{
    return _stack;
}

bool HttpServiceProxy::IsServiceReady(const KERNEL_NS::LibString &serviceName) const
{
    return true;
}

KERNEL_END
