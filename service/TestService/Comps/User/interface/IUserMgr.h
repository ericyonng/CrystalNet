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
 * Date: 2023-07-31 23:50:09
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <ServiceCompHeader.h>
#include <protocols/protocols.h>

SERVICE_BEGIN

class IUser;
class PendingUser;

class IUserMgr : public IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, IUserMgr);

public:
    virtual IUser *GetUser(UInt64 userId) = 0;
    virtual const IUser *GetUser(UInt64 userId) const = 0;
    virtual IUser *GetUser(const KERNEL_NS::LibString &accountName) = 0;
    virtual const IUser *GetUser(const KERNEL_NS::LibString &accountName) const = 0;
    virtual const IUser *GetUserBySessionId(UInt64 sessionId) const = 0;
    virtual IUser *GetUserBySessionId(UInt64 sessionId) = 0;

    /* 登录
    * @param(loginInfo):登录信息
    * @param(cb):回调, Rtn:void, Int32:错误码, IUser登录成功后的user对象, Variant:传入的透传参数
    * @return(Int32):调用是否成功 Status
    */
    virtual Int32 Login(UInt64 sessionId, KERNEL_NS::SmartPtr<LoginInfo, KERNEL_NS::AutoDelMethods::Release> &loginInfo
    , KERNEL_NS::IDelegate<void, Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &> *cb
    , KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var = NULL) = 0;

    // @return(Int32):调用是否成功 Status
    template<typename ObjType>
    Int32 LoginBy(UInt64 sessionId, KERNEL_NS::SmartPtr<LoginInfo, KERNEL_NS::AutoDelMethods::Release> &loginInfo, ObjType *obj, void (ObjType::*handler)(Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &)
    ,  KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var = NULL);

    // @return(Int32):调用是否成功 Status
    template<typename CallbackType>
    Int32 LoginBy(UInt64 sessionId, KERNEL_NS::SmartPtr<LoginInfo, KERNEL_NS::AutoDelMethods::Release> &loginInfo, CallbackType &&cb, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var = NULL);

    virtual Int32 LoadUser(const KERNEL_NS::LibString &accountName
    , KERNEL_NS::IDelegate<void, Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &> *cb
    , KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var = NULL) = 0;
    
    template<typename ObjType>
    Int32 LoadUserBy(const KERNEL_NS::LibString &accountName
    , ObjType *obj, void (ObjType::*handler)(Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &)
    ,  KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var = NULL);
    
    template<typename CallbackType>
    Int32 LoadUserBy(const KERNEL_NS::LibString &accountName
    , CallbackType &&cb
    , KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var = NULL);

    virtual Int32 LoadUser(UInt64 userId
    , KERNEL_NS::IDelegate<void, Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &> *cb
    , KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var = NULL) = 0;

    template<typename ObjType>
    Int32 LoadUserBy(UInt64 userId, ObjType *obj, void (ObjType::*handler)(Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &)
    ,  KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var = NULL);

    template<typename CallbackType>
    Int32 LoadUserBy(UInt64 userId 
    , CallbackType &&cb
    , KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var = NULL);

    virtual void Purge() = 0;
    virtual void PurgeAndWait() = 0;

   virtual void PurgeEndWith(KERNEL_NS::IDelegate<void, Int32> *handler) = 0;
   template<typename CallbackType>
   void PurgeEndWith(CallbackType &&cb);
   template<typename ObjType>
   void PurgeEndWith(ObjType *obj, void (ObjType::*handler)(Int32 errCode));

    // 解除session的映射
   virtual void RemoveUserBySessionId(UInt64 sessionId) = 0;
   // 添加session映射
   virtual void AddUserBySessionId(UInt64 sessionId, IUser *user) = 0;

   virtual KERNEL_NS::LibRsa &GetRsa() const = 0;
};

template<typename ObjType>
ALWAYS_INLINE Int32 IUserMgr::LoginBy(UInt64 sessionId, KERNEL_NS::SmartPtr<LoginInfo, KERNEL_NS::AutoDelMethods::Release> &loginInfo, ObjType *obj, void (ObjType::*handler)(Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &)
,  KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var)
{
    auto deleg = KERNEL_NS::DelegateFactory::Create(obj, handler);
    return Login(sessionId, loginInfo, deleg, var);
}

template<typename CallbackType>
ALWAYS_INLINE Int32 IUserMgr::LoginBy(UInt64 sessionId, KERNEL_NS::SmartPtr<LoginInfo, KERNEL_NS::AutoDelMethods::Release> &loginInfo, CallbackType &&cb
, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var)
{
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &);
    return Login(sessionId, loginInfo, deleg, var);
}

template<typename ObjType>
ALWAYS_INLINE Int32 IUserMgr::LoadUserBy(const KERNEL_NS::LibString &accountName
, ObjType *obj, void (ObjType::*handler)(Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &)
,  KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var)
{
    auto deleg = KERNEL_NS::DelegateFactory::Create(obj, handler);
    return LoadUser(accountName, deleg, var);
}

template<typename CallbackType>
ALWAYS_INLINE Int32 IUserMgr::LoadUserBy(const KERNEL_NS::LibString &accountName
, CallbackType &&cb
, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var)
{
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &);
    return LoadUser(accountName, deleg, var);
}

template<typename ObjType>
ALWAYS_INLINE Int32 IUserMgr::LoadUserBy(UInt64 userId
, ObjType *obj, void (ObjType::*handler)(Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &)
,  KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var)
{
    auto deleg = KERNEL_NS::DelegateFactory::Create(obj, handler);
    return LoadUser(userId, deleg, var);
}

template<typename CallbackType>
ALWAYS_INLINE Int32 IUserMgr::LoadUserBy(UInt64 userId
, CallbackType &&cb
, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var)
{
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &);
    return LoadUser(userId, deleg, var);
}

template<typename CallbackType>
ALWAYS_INLINE void IUserMgr::PurgeEndWith(CallbackType &&cb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, Int32);
    PurgeEndWith(delg);
}

template<typename ObjType>
ALWAYS_INLINE void IUserMgr::PurgeEndWith(ObjType *obj, void (ObjType::*handler)(Int32 errCode))
{
    auto delg = KERNEL_NS::DelegateFactory::Create(obj, handler);
    PurgeEndWith(delg);
}


SERVICE_END

