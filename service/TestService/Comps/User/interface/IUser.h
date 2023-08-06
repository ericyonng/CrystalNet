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
 * Date: 2023-07-31 23:46:25
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <ServiceCompHeader.h>

SERVICE_BEGIN

class IUserMgr;
class IUserSys;
class IUser;

class UserStatus
{
public:
    enum ENUMS
    {
        USER_PENDING = 0,
        USER_ONLOADING,
        USER_ONLOADED,

        USER_CREATED,
        USER_INITING,
        USER_INITED,
        USER_STARTING,
        USER_STARTED,
    };
};

// user处于未创建状态
class PendingUser
{
    POOL_CREATE_OBJ_DEFAULT(PendingUser);

public:
    PendingUser();
    ~PendingUser();

    KERNEL_NS::LibString ToString() const;

    Int32 _status;
    KERNEL_NS::SmartPtr<LoginInfo, KERNEL_NS::AutoDelMethods::Release> _loginInfo;
    KERNEL_NS::LibString _byAccountName;
    UInt64 _byUserId;
    UInt64 _stub;
    KERNEL_NS::IDelegate<void, Int32, PendingUser *,  IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &> *_cb;
    KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> _var;
    Int32 _dbOperatorId;
};

class IUser : public ILogicSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(ILogicSys, IUser);

public:
    virtual IUserMgr *GetUserMgr() = 0;
    virtual const IUserMgr *GetUserMgr() const = 0;

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
    virtual int RemoveListener(int id);
    /**
     * Remove event listener using listener stub.
     * @param[in] stub - event listener stub.
     * @return int - success if return success, otherwise return Error,
     *               specially, if return Error, and fetch the last error is pending,
     *               it means operation will success on later, but pending at now.
     */
    virtual int RemoveListener(const KERNEL_NS::ListenerStub &stub);
    /**
     * Remove event listener using listener stub and clear the listener stub.
     * @param[in] stub - event listener stub.
     * @return int - success if return success, otherwise return Error,
     *               specially, if return Error, and fetch the last error is pending,
     *               it means operation will success on later, but pending at now.
     */
    virtual int RemoveListenerX(KERNEL_NS::ListenerStub &stub);
    /**
     * Fire the event. 处于isfiring中进行监听的事件，
     * FireEvent若在IsFiring状况下addlisten并在统一帧下Fire的话事件将在之后的某个合适时机触发
     * 若立即FireEvent会丢失事件,因为IsFiring状态下的AddListen在异步队列中
     * @param[in] event - event object.
     * @return(Int32) FireEvResult, 为了降低复杂度,请保证统一征内事件不同时Add与Fire(IsFiring状态下)
     */
    virtual Int32 FireEvent(KERNEL_NS::LibEvent *event);

    virtual Int64 Send(KERNEL_NS::LibPacket *packet) const = 0;
    virtual void Send(const std::list<KERNEL_NS::LibPacket *> &packets) const = 0;
    virtual Int64 Send(Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId = -1) const = 0;

    virtual void OnLogin() = 0;
    virtual void OnLoginFinish() = 0;
    virtual void OnLogout() = 0;
    virtual void OnUserCreated() = 0;

    // user状态
    virtual Int32 GetUserStatus() const = 0;
    virtual void SetUserStatus(Int32 status)  = 0;

    // dirty相关
    virtual void MaskDirty(IUserSys *userSys) = 0;
    virtual void MaskAddDirty() = 0;

    // 用户基本信息
    virtual UserBaseInfo *GetUserBaseInfo() = 0;
    virtual const UserBaseInfo *GetUserBaseInfo() const = 0;

    // 用户id
    virtual UInt64 GetUserId() const = 0;
};

ALWAYS_INLINE KERNEL_NS::ListenerStub IUser::AddListener(int id,
                                        KERNEL_NS::IDelegate<void, KERNEL_NS::LibEvent *> *listener,
                                        const KERNEL_NS::ListenerStub &bindedStub)
{
    return _eventMgr->AddListener(id, listener, bindedStub);
}

template <typename ObjectType>
ALWAYS_INLINE KERNEL_NS::ListenerStub IUser::AddListener(int id,
                                ObjectType *obj,
                                void (ObjectType::*listener)(KERNEL_NS::LibEvent *),
                                const KERNEL_NS::ListenerStub &bindedStub)
{
    auto delg = KERNEL_NS::DelegateFactory::Create(obj, listener);
    return AddListener(id, delg, bindedStub);
}

ALWAYS_INLINE int IUser::RemoveListener(int id)
{
    return _eventMgr->RemoveListener(id);
}

ALWAYS_INLINE int IUser::RemoveListener(const KERNEL_NS::ListenerStub &stub)
{
    return _eventMgr->RemoveListener(stub);
}

ALWAYS_INLINE int IUser::RemoveListenerX(KERNEL_NS::ListenerStub &stub)
{
    return _eventMgr->RemoveListenerX(stub);
}

ALWAYS_INLINE Int32 IUser::FireEvent(KERNEL_NS::LibEvent *event)
{
    return _eventMgr->FireEvent(event);
}

SERVICE_END

