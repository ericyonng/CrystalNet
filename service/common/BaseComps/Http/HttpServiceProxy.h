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
 * Description: http服务代理: IServiceProxy桥接实现, 将网络引擎事件转发到业务派发线程的Poller
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_SERVICE_PROXY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_SERVICE_PROXY_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/comp/Service/IServiceProxy.h>

#include <atomic>

KERNEL_BEGIN

class Poller;

class KERNEL_EXPORT HttpServiceProxy : public IServiceProxy
{
    POOL_CREATE_OBJ_DEFAULT_P1(IServiceProxy, HttpServiceProxy);

public:
    HttpServiceProxy();
    virtual ~HttpServiceProxy();
    virtual void Release() override;

    // 设置事件派发poller(业务线程), 网络事件将被Push到该poller
    void SetEventPoller(Poller *poller);
    Poller *GetEventPoller();
    // 设置http协议栈, 不接管所有权
    void SetProtocolStack(IProtocolStack *stack);

public:
    virtual void PostMsg(UInt64 serviceId, PollerEvent *msg, Int64 packetsCount = 0) override;
    virtual void PostQuitService() override;
    virtual IProtocolStack *GetProtocolStack(LibSession *session) override;
    virtual bool IsServiceReady(const KERNEL_NS::LibString &serviceName) const override;

private:
    std::atomic<Poller *> _eventPoller;
    IProtocolStack *_stack;
};

ALWAYS_INLINE Poller *HttpServiceProxy::GetEventPoller()
{
    return _eventPoller.load(std::memory_order_acquire);
}

KERNEL_END

#endif
