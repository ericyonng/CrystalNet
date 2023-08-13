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
 * Date: 2023-08-12 22:15:38
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <ServiceCompHeader.h>

SERVICE_BEGIN

class IClientUser;
class IClientUserMgr;

class IClientSys : public ILogicSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(ILogicSys, IClientSys);

public:
    IClientSys();
    ~IClientSys();

    IClientUser *GetUser();
    const IClientUser *GetUser() const;

    IClientUserMgr *GetUserMgr();
    const IClientUserMgr *GetUserMgr() const;

    KERNEL_NS::ListenerStub AddListener(int id,
                                        KERNEL_NS::IDelegate<void, KERNEL_NS::LibEvent *> *listener,
                                        const KERNEL_NS::ListenerStub &bindedStub = INVALID_LISTENER_STUB);
    template <typename ObjectType>
    KERNEL_NS::ListenerStub AddListener(int id,
                                  ObjectType *obj,
                                  void (ObjectType::*listener)(KERNEL_NS::LibEvent *),
                                  const KERNEL_NS::ListenerStub &bindedStub = INVALID_LISTENER_STUB);
    int RemoveListener(int id);
    int RemoveListener(const KERNEL_NS::ListenerStub &stub);
    int RemoveListenerX(KERNEL_NS::ListenerStub &stub);
    Int32 FireEvent(KERNEL_NS::LibEvent *event);

    Int64 Send(KERNEL_NS::LibPacket *packet) const;
    void Send(const std::list<KERNEL_NS::LibPacket *> &packets) const;
    Int64 Send(Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId = -1) const;

    virtual void OnLogin();
    virtual void OnLoginFinish();
    virtual void OnLogout();
    virtual void OnUserCreated();

protected:
    virtual Int32 _OnSysInit() final;
    virtual Int32 _OnUserSysInit() { return Status::Success; }

protected:
    IClientUser *_userOwner;
    IClientUserMgr *_userMgr;
};

ALWAYS_INLINE KERNEL_NS::ListenerStub IClientSys::AddListener(int id,
                                        KERNEL_NS::IDelegate<void, KERNEL_NS::LibEvent *> *listener,
                                        const KERNEL_NS::ListenerStub &bindedStub)
{
    return _eventMgr->AddListener(id, listener, bindedStub);
}

template <typename ObjectType>
ALWAYS_INLINE KERNEL_NS::ListenerStub IClientSys::AddListener(int id,
                                ObjectType *obj,
                                void (ObjectType::*listener)(KERNEL_NS::LibEvent *),
                                const KERNEL_NS::ListenerStub &bindedStub)
{
    auto delg = KERNEL_NS::DelegateFactory::Create(obj, listener);
    return AddListener(id, delg, bindedStub);
}

ALWAYS_INLINE int IClientSys::RemoveListener(int id)
{
    return _eventMgr->RemoveListener(id);
}

ALWAYS_INLINE int IClientSys::RemoveListener(const KERNEL_NS::ListenerStub &stub)
{
    return _eventMgr->RemoveListener(stub);
}

ALWAYS_INLINE int IClientSys::RemoveListenerX(KERNEL_NS::ListenerStub &stub)
{
    return _eventMgr->RemoveListenerX(stub);
}

ALWAYS_INLINE Int32 IClientSys::FireEvent(KERNEL_NS::LibEvent *event)
{
    return _eventMgr->FireEvent(event);
}

SERVICE_END
