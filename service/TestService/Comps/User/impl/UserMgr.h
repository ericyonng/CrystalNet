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
 * Date: 2023-07-31 23:46:59
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <Comps/User/interface/IUserMgr.h>
#include <Comps/User/impl/UserLruCompare.h>
#include <protocols/protocols.h>

KERNEL_BEGIN

class MysqlResponse;

KERNEL_END

SERVICE_BEGIN

class IUser;
class PendingUser;
class User;

class UserMgr : public IUserMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IUserMgr, UserMgr);

public:
    UserMgr();
    ~UserMgr();
    void Release() override;

    virtual void OnRegisterComps() override; 
    virtual void OnWillStartup() override;
    virtual void OnStartup() override;

   virtual Int32 OnLoaded(UInt64 key, const std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) override;
   virtual Int32 OnSave(UInt64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const override;

    virtual IUser *GetUser(UInt64 userId);
    virtual const IUser *GetUser(UInt64 userId) const;
    virtual IUser *GetUser(const KERNEL_NS::LibString &accountName) override;
    virtual const IUser *GetUser(const KERNEL_NS::LibString &accountName) const override;
    virtual const IUser *GetUserBySessionId(UInt64 sessionId) const override;
    virtual IUser *GetUserBySessionId(UInt64 sessionId) override;

    virtual void MaskNumberKeyAddDirty(UInt64 key) override;

    /* 登录
    * @param(loginInfo):登录信息
    * @param(cb):回调, Rtn:void, Int32:错误码, IUser登录成功后的user对象, Variant:传入的透传参数
    */
    virtual Int32 Login(UInt64 sessionId, KERNEL_NS::SmartPtr<LoginInfo, KERNEL_NS::AutoDelMethods::Release> &loginInfo
    , KERNEL_NS::IDelegate<void, Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &> *cb
    , KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var = NULL) override;

    virtual Int32 LoadUser(const KERNEL_NS::LibString &accountName
    , KERNEL_NS::IDelegate<void, Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &> *cb
    , KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var = NULL) override;

    virtual Int32 LoadUser(UInt64 userId
    , KERNEL_NS::IDelegate<void, Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &> *cb
    , KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var = NULL) override;

    Int32 LoadUser(const KERNEL_NS::LibString &accountName, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> &pendingUser);
    
    virtual void Purge() override;
    virtual void PurgeAndWait() override;
   virtual void PurgeEndWith(KERNEL_NS::IDelegate<void, Int32> *handler) override;

    // 解除session的映射
   virtual void RemoveUserBySessionId(UInt64 sessionId) override;
   // 添加session映射
   virtual void AddUserBySessionId(UInt64 sessionId, IUser *user) override;

   virtual KERNEL_NS::LibRsa &GetRsa() const override;

   virtual void OnPassDay(const KERNEL_NS::LibTime &nowTime) override;
   virtual void OnPassWeek(const KERNEL_NS::LibTime &nowTime) override;
   virtual void OnPassMonth(const KERNEL_NS::LibTime &nowTime) override;
   virtual void OnPassYear(const KERNEL_NS::LibTime &nowTime) override;
   virtual void OnPassTimeEnd(const KERNEL_NS::LibTime &nowTime) override;

private:
    virtual Int32 _OnGlobalSysInit() override;
    virtual Int32 _OnGlobalSysCompsCreated() override;
    virtual void _OnHostWillClose() override;
    virtual void _OnGlobalSysClose() override;
    virtual void _OnQuitServiceEventDefault(KERNEL_NS::LibEvent *ev) override;

    PendingUser *_GetPendingByAccount(const KERNEL_NS::LibString &accountName);
    const PendingUser *_GetPendingByAccount(const KERNEL_NS::LibString &accountName) const;
    PendingUser *_GetPendingByStub(UInt64 stub);
    const PendingUser *_GetPendingByStub(UInt64 stub) const;
    PendingUser *_GetPendingByUserId(UInt64 userId);
    const PendingUser *_GetPendingByUserId(UInt64 userId) const;
    void _AddUserPendingInfo(UInt64 userId, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> &pendingInfo);
    void _AddUserPendingInfo(const KERNEL_NS::LibString &accountName, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> &pendingInfo);
    void _RemovePendingInfo(const KERNEL_NS::LibString &accountName);
    void _RemovePendingInfo(UInt64 userId);

    void _AddUser(User *user);
    void _RemoveUser(User *user);
    void _RemoveFromLru(IUser *user);
    void _AddToLru(IUser *user);
    void _LruPopUser();

    void _OnDbUserLoaded(KERNEL_NS::MysqlResponse *res);

    Int32 _CheckRegisterInfo(const LoginInfo &loginInfo) const;
    void _BuildPwd(UserBaseInfo *baseInfo, const std::string &pwd);
    void _OnClientLoginReq(KERNEL_NS::LibPacket *&packet);
    void _OnClientLogoutReq(KERNEL_NS::LibPacket *&packet);

    void _Clear();

    // TODO:需要考虑通过LoadPlayer userid产生的pending, 在LoadPlayer userId后需要移除所有该User的Pending, 并且执行该Pending后续的delegate
    std::map<UInt64, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete>> _stubRefPendingUser;
    std::map<KERNEL_NS::LibString, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete>> _accountNameRefPendingUser;
    std::map<UInt64, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete>> _userIdRefPendingUser;
    std::set<UInt64> _loginPendingSessions;

    std::map<UInt64, IUser *> _userIdRefUser;
    std::map<KERNEL_NS::LibString, IUser *> _accountNameRefUser;
    std::map<UInt64, IUser *> _sessionIdRefUser;
    std::set<IUser *, UserLruCompare> _lru;
    Int32 _lruCapacityLimit;    // 容量限制

    std::vector<UInt64> _pendingLoginEventOnStartup;
    KERNEL_NS::LibString _rsaPrivateKey;
    KERNEL_NS::LibString _rsaPrivateKeyRaw;
    KERNEL_NS::LibString _rsaPubKey;
    KERNEL_NS::LibString _rsaPubKeyRaw;
    mutable KERNEL_NS::LibRsa _rsa;
};

ALWAYS_INLINE PendingUser *UserMgr::_GetPendingByAccount(const KERNEL_NS::LibString &accountName)
{
    auto iter = _accountNameRefPendingUser.find(accountName);
    return iter == _accountNameRefPendingUser.end() ? NULL : iter->second.AsSelf();
}

ALWAYS_INLINE const PendingUser *UserMgr::_GetPendingByAccount(const KERNEL_NS::LibString &accountName) const
{
    auto iter = _accountNameRefPendingUser.find(accountName);
    return iter == _accountNameRefPendingUser.end() ? NULL : iter->second.AsSelf();
}

ALWAYS_INLINE PendingUser *UserMgr::_GetPendingByStub(UInt64 stub)
{
    auto iter = _stubRefPendingUser.find(stub);
    return iter == _stubRefPendingUser.end() ? NULL : iter->second.AsSelf();
}

ALWAYS_INLINE const PendingUser *UserMgr::_GetPendingByStub(UInt64 stub) const
{
    auto iter = _stubRefPendingUser.find(stub);
    return iter == _stubRefPendingUser.end() ? NULL : iter->second.AsSelf();
}

ALWAYS_INLINE PendingUser *UserMgr::_GetPendingByUserId(UInt64 userId)
{
    auto iter = _userIdRefPendingUser.find(userId);
    return iter == _userIdRefPendingUser.end() ? NULL : iter->second.AsSelf();
}

ALWAYS_INLINE const PendingUser *UserMgr::_GetPendingByUserId(UInt64 userId) const
{
    auto iter = _userIdRefPendingUser.find(userId);
    return iter == _userIdRefPendingUser.end() ? NULL : iter->second.AsSelf();
}

SERVICE_END
