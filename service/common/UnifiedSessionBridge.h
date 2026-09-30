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
 * Date: 2026-09-29 12:03:00
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <kernel/comp/NetEngine/Poller/impl/Session/ISessionBridge.h>
#include <service/common/macro.h>

#include "kernel/comp/CompObject/CompFactory.h"
#include "kernel/comp/Event/EventManager.h"

KERNEL_BEGIN
class EventManager;
KERNEL_END

SERVICE_BEGIN
    
class UnifiedService;
   
class UnifiedSessionBridge : public KERNEL_NS::ISessionBridge
{
    POOL_CREATE_OBJ_DEFAULT_P1(ISessionBridge, UnifiedSessionBridge);
 
public:
    UnifiedSessionBridge();
    ~UnifiedSessionBridge() override;
    void Release() override;
    KERNEL_NS::IProtocolStack* GetProtocolStack(KERNEL_NS::LibSession* session) override;

protected:
    void _OnSessionCreated(KERNEL_NS::PollerEvent*& msg) override;
    void _OnAsynConnectRes(KERNEL_NS::PollerEvent*& msg) override;
    void _OnAddListenRes(KERNEL_NS::PollerEvent*& msg) override;
    void _OnSessionDestroy(KERNEL_NS::PollerEvent*& msg) override;
    void _OnRecvMsg(KERNEL_NS::PollerEvent*& msg) override;
    // 退出服务
    void _OnQuitingService(KERNEL_NS::PollerEvent*& msg);

    Int32 _OnInit() override;
    
private:
    UnifiedService *_service;
    KERNEL_NS::EventManager *_eventMgr;
};

class UnifiedSessionBridgeFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::TL _buildType{};

    static CompFactory *FactoryCreate();

    virtual void Release() override;

    KERNEL_NS::CompObject* Create() const override;
};

SERVICE_END
