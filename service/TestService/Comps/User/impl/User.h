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
 * Date: 2023-07-31 23:46:50
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <Comps/User/interface/IUser.h>
#include <protocols/protocols.h>

SERVICE_BEGIN

class IUserMgr;
class IUserSys;

// 要有自己的事件对象
class User : public IUser
{
    POOL_CREATE_OBJ_DEFAULT_P1(IUser, User);

public:
    User(IUserMgr *userMgr);
    ~User();
    void Release();

   virtual void OnRegisterComps() override;

   virtual Int32 OnLoaded(UInt64 key, const std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb);
   virtual Int32 OnSave(UInt64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const;

    virtual IUserMgr *GetUserMgr() override;
    virtual const IUserMgr *GetUserMgr() const override;

    virtual Int64 Send(KERNEL_NS::LibPacket *packet) const override;
    virtual void Send(const std::list<KERNEL_NS::LibPacket *> &packets) const override;
    virtual Int64 Send(Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId = -1) const override;

    virtual void OnLogin() override;
    virtual void OnLoginFinish() override;
    virtual void OnLogout() override;
    virtual void OnUserCreated() override;


    virtual Int32 GetUserStatus() const override;
    virtual void SetUserStatus(Int32 status)  override;

   virtual void MaskDirty() override;
    virtual void MaskDirty(IUserSys *userSys) override;
    virtual void OnMaskAddDirty() override;
    virtual void MaskDirtyAll() override;

    virtual void Purge() override;
    virtual void PurgeAndWaitComplete() override;
   virtual void PurgeEndWith(KERNEL_NS::IDelegate<void, Int32> *handler) override;

    virtual UserBaseInfo *GetUserBaseInfo() override;
    virtual const UserBaseInfo *GetUserBaseInfo() const override;

    virtual UInt64 GetUserId() const override;

    // 绑定会话
    virtual void BindSession(UInt64 sessionId) override;
    // 获取会话id
    virtual UInt64 GetSessionId() const override;

    virtual void Logout(Int32 logoutReason) override;

    // 是否登出
    virtual bool IsLogined() const override;
    // 是否在线
    virtual bool IsLogout() const override;

    virtual const KERNEL_NS::BriefSockAddr *GetUserAddr() const override;

    // 标脏是key + logic地址或者字段名
    virtual KERNEL_NS::LibString ToString() const override;

    // 获取lru时间
    virtual Int64 GetLruTime() const override;
    // 刷新lrutime
    virtual void UpdateLrtTime() override;
    // 获取心跳更新时间
    virtual Int64 GetHeartbeatExpireTime() const override;
    // 刷新心跳更新时间
    virtual void UpdateHeartbeatExpireTime(Int64 spanTimeInMs) override;

private:
    virtual Int32 _OnSysInit() override;
    virtual Int32 _OnSysCompsCreated() override;
    virtual void _OnSysClose() override;

    IUserSys *_GetSysBy(const KERNEL_NS::LibString &fieldName);
    const IUserSys *_GetSysBy(const KERNEL_NS::LibString &fieldName) const;
    IStorageInfo *_GetStorageInfoBy(const IUserSys *userSys);
    const IStorageInfo *_GetStorageInfoBy(const IUserSys *userSys) const;

    void _Clear();

    void _RegisterEvents();

private:
    IUserMgr *_userMgr;
    Int32 _status;

    /* 需要存储相关userSys */
    std::vector<IUserSys *> _needStorageSys;
    std::unordered_map<KERNEL_NS::LibString, IUserSys *> _fieldNameRefUserSys;
    std::unordered_map<const IUserSys *, IStorageInfo *> _userSysRefStorageInfo;
    mutable std::set<IUserSys *> _dirtySys;

    // 用户基本信息
    UserBaseInfo *_userBaseInfo;

    UInt64 _activedSessionId;
    Int64 _lruTime;
    Int64 _heatbeatTime;
};

ALWAYS_INLINE IUserSys *User::_GetSysBy(const KERNEL_NS::LibString &fieldName)
{
    auto iter = _fieldNameRefUserSys.find(fieldName);
    return iter == _fieldNameRefUserSys.end() ? NULL : iter->second;
}

ALWAYS_INLINE const IUserSys *User::_GetSysBy(const KERNEL_NS::LibString &fieldName) const
{
    auto iter = _fieldNameRefUserSys.find(fieldName);
    return iter == _fieldNameRefUserSys.end() ? NULL : iter->second;
}

ALWAYS_INLINE IStorageInfo *User::_GetStorageInfoBy(const IUserSys *userSys)
{
    auto iter = _userSysRefStorageInfo.find(userSys);
    return iter == _userSysRefStorageInfo.end() ? NULL : iter->second;
}

ALWAYS_INLINE const IStorageInfo *User::_GetStorageInfoBy(const IUserSys *userSys) const
{
    auto iter = _userSysRefStorageInfo.find(userSys);
    return iter == _userSysRefStorageInfo.end() ? NULL : iter->second;
}

SERVICE_END

