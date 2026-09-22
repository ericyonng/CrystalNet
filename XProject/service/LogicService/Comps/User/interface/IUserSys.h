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
 * Date: 2023-07-31 23:46:34
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <ServiceCompHeader.h>
#include <Comps/User/interface/IUser.h>
#include <Comps/User/interface/IUserMgr.h>
#include <service/common/status.h>

SERVICE_BEGIN

// OnLoaded:user还没有添加到UserMgr字典中
class IUserSys : public ILogicSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(ILogicSys, IUserSys);

public:
    IUserSys(UInt64 objTypeId);
    ~IUserSys();

    IUser *GetUser();
    const IUser *GetUser() const;

    IUserMgr *GetUserMgr();
    const IUserMgr *GetUserMgr() const;

    KERNEL_NS::ListenerStub AddListener(int id,
                                        KERNEL_NS::IDelegate<void, KERNEL_NS::LibEvent *> *listener,
                                        const KERNEL_NS::ListenerStub &bindedStub = INVALID_LISTENER_STUB);
    template <typename ObjectType>
    KERNEL_NS::ListenerStub AddListener(int id,
                                  ObjectType *obj,
                                  void (ObjectType::*listener)(KERNEL_NS::LibEvent *),
                                  const KERNEL_NS::ListenerStub &bindedStub = INVALID_LISTENER_STUB);
    /**
     * Remove event listener.
     * @param[in] id - event Id.
     * @return int - success if return StatusDefs::success, otherwise return Error.
     *               specially, if return Error,  and fetch the last error is pending,
     *               it means operation will success on later, but pending at now.
     */
    int RemoveListener(int id);
    /**
     * Remove event listener using listener stub.
     * @param[in] stub - event listener stub.
     * @return int - success if return success, otherwise return Error,
     *               specially, if return Error, and fetch the last error is pending,
     *               it means operation will success on later, but pending at now.
     */
    int RemoveListener(const KERNEL_NS::ListenerStub &stub);
    /**
     * Remove event listener using listener stub and clear the listener stub.
     * @param[in] stub - event listener stub.
     * @return int - success if return success, otherwise return Error,
     *               specially, if return Error, and fetch the last error is pending,
     *               it means operation will success on later, but pending at now.
     */
    int RemoveListenerX(KERNEL_NS::ListenerStub &stub);
    /**
     * Fire the event. 处于isfiring中进行监听的事件，
     * FireEvent若在IsFiring状况下addlisten并在统一帧下Fire的话事件将在之后的某个合适时机触发
     * 若立即FireEvent会丢失事件,因为IsFiring状态下的AddListen在异步队列中
     * @param[in] event - event object.
     * @return(Int32) FireEvResult, 为了降低复杂度,请保证统一征内事件不同时Add与Fire(IsFiring状态下)
     */
    Int32 FireEvent(KERNEL_NS::LibEvent *event);

    // @return(Int64):返回packetId
    void Send(KERNEL_NS::LibPacket *packet) const;
    void Send(const std::list<KERNEL_NS::LibPacket *> &packets) const;
    // @return(Int64):返回packetId
    void Send(Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId = -1) const;
  
   virtual void MaskDirty() override;
   virtual void MaskNumberKeyAddDirty(UInt64 key) final {}
   virtual void MaskNumberKeyModifyDirty(UInt64 key) final {}
   virtual void MaskNumberKeyDeleteDirty(UInt64 key) final {}
   virtual void MaskStringKeyAddDirty(const KERNEL_NS::LibString &key) final {}
   virtual void MaskStringKeyModifyDirty(const KERNEL_NS::LibString &key) final {}
   virtual void MaskStringKeyDeleteDirty(const KERNEL_NS::LibString &key) final {}

    virtual void OnLogin();
    virtual void OnLoginFinish();
    virtual void OnClientLoginFinish();
    // 经登录之后状态都会是登录状态,除非玩家点击登出或者登录令牌过期,或者ip变更了
    virtual void OnLogout();
    virtual void OnUserCreated();

protected:
    virtual Int32 _OnSysInit() final;
    virtual Int32 _OnUserSysInit() { return Status::Success; }


protected:
    IUser *_userOwner;
    IUserMgr *_userMgr;
};

ALWAYS_INLINE IUser *IUserSys::GetUser()
{
    return _userOwner;
}

ALWAYS_INLINE const IUser *IUserSys::GetUser() const
{
    return _userOwner;
}

ALWAYS_INLINE IUserMgr *IUserSys::GetUserMgr()
{
    return _userMgr;
}

ALWAYS_INLINE const IUserMgr *IUserSys::GetUserMgr() const
{
    return _userMgr;
}

ALWAYS_INLINE KERNEL_NS::ListenerStub IUserSys::AddListener(int id,
                                    KERNEL_NS::IDelegate<void, KERNEL_NS::LibEvent *> *listener,
                                    const KERNEL_NS::ListenerStub &bindedStub)
{
    return GetEventMgr()->AddListener(id, listener, bindedStub);
}

template <typename ObjectType>
ALWAYS_INLINE KERNEL_NS::ListenerStub IUserSys::AddListener(int id,
                                ObjectType *obj,
                                void (ObjectType::*listener)(KERNEL_NS::LibEvent *),
                                const KERNEL_NS::ListenerStub &bindedStub)
{
    return GetEventMgr()->AddListener(id, obj, listener, bindedStub);
}

ALWAYS_INLINE int IUserSys::RemoveListener(int id)
{
    return GetEventMgr()->RemoveListener(id);
}

ALWAYS_INLINE int IUserSys::RemoveListener(const KERNEL_NS::ListenerStub &stub)
{
    return GetEventMgr()->RemoveListener(stub);
}

ALWAYS_INLINE int IUserSys::RemoveListenerX(KERNEL_NS::ListenerStub &stub)
{
    return GetEventMgr()->RemoveListenerX(stub);
}

ALWAYS_INLINE Int32 IUserSys::FireEvent(KERNEL_NS::LibEvent *event)
{
    return GetEventMgr()->FireEvent(event);
}

SERVICE_END
