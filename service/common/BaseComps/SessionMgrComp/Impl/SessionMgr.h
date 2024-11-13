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
 * Date: 2022-09-18 21:59:50
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/BaseComps/SessionMgrComp/Interface/ISessionMgr.h>
#include <kernel/comp/Event/Defs.h>

#include <map>
#include <atomic>

SERVICE_BEGIN

class ServiceSession;
struct ServiceSessionInfo;

class SessionMgr : public ISessionMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(ISessionMgr, SessionMgr);

public:
    SessionMgr();
    ~SessionMgr();

    void Release() override;

public:
    virtual KERNEL_NS::LibString ToString() const override;
    virtual ServiceSession *GetSession(UInt64 sessionId) override;
    virtual const ServiceSession *GetSession(UInt64 sessionId) const override;
    virtual Int64 NewPacketId(UInt64 sessionId) override;
    virtual UInt64 GetSessionAmount() const override;
    virtual const std::map<UInt64, ServiceSession *> &GetSessions() override;

protected:
    Int32 _OnGlobalSysInit() override;
    void _OnGlobalSysClose() override;

private:
    void _OnSessionWillCreated(KERNEL_NS::LibEvent *ev);
    void _OnSessionDestroy(KERNEL_NS::LibEvent *ev);

    void _Clear();

    void _RegisterEvents();
    void _UnRegisterEvents();

    /*
    * 会话
    */
   ServiceSession *_CreateSession(const ServiceSessionInfo &sessionInfo);
   void _DestroySession(ServiceSession *session);
   void _MakeSessionDict(ServiceSession *session);

private:

    /* 会话 */
    std::map<UInt64, ServiceSession *> _sessionIdRefSession;
    std::atomic<UInt64> _sessionAmount;

    /* 事件 */
    KERNEL_NS::ListenerStub _sessionWillCreatedStub;
    KERNEL_NS::ListenerStub _sessionDestroyStub;

    // 包id
    std::map<UInt64, Int64> _sessionIdRefMaxPacketId;
};

SERVICE_END