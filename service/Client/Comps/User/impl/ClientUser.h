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
 * Date: 2023-08-12 21:10:38
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <Comps/User/interface/IClientUser.h>

SERVICE_BEGIN

class IClientUserMgr;
class IClientSys;

class ClientUser : public IClientUser
{
    POOL_CREATE_OBJ_DEFAULT_P1(IClientUser, ClientUser);

public:
    ClientUser(IClientUserMgr *userMgr);
    ~ClientUser();
    void Release() override;

   virtual void OnRegisterComps() override;

    virtual Int64 GetNowServerTime() const override;
    virtual void SetServerTime(Int64 serverTime) override;

    virtual IClientUserMgr *GetUserMgr() override;
    virtual const IClientUserMgr *GetUserMgr() const override;
    virtual ClientUserInfo *GetClientInfo() override;
    virtual const ClientUserInfo *GetClientInfo() const override;

    virtual Int64 Send(KERNEL_NS::LibPacket *packet) const override;
    virtual void Send(const std::list<KERNEL_NS::LibPacket *> &packets) const override;
    virtual Int64 Send(Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId = -1) const override;

    virtual void OnLogin() override;
    virtual void OnLoginFinish() override;
    virtual void OnLogout() override;
    virtual void OnUserCreated() override;

    virtual Int32 GetUserStatus() const override;
    virtual void SetUserStatus(Int32 status)  override;

    virtual UInt64 GetUserId() const override;

    // 绑定会话
    virtual void BindSession(UInt64 sessionId) override;
    // 获取会话id
    virtual UInt64 GetSessionId() const override;

    virtual void Logout() override;
    virtual Int32 Login(Int32 stackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL) override;
    virtual void SetLoginInfo(const LoginInfo &loginInfo) override;
    virtual const LoginInfo &GetLoginInfo() const override;
    virtual LoginInfo &GetLoginInfo() override;

    // 是否登出
    virtual bool IsLogined() const override;
    virtual bool IsLogining() const override;
    // 是否在线
    virtual bool IsLogout() const override;

    virtual const KERNEL_NS::BriefSockAddr *GetUserAddr() const override;

    virtual KERNEL_NS::LibString ToString() const override;

    virtual Int64 GetHeartbeatExpireTime() const override;
    // 刷新心跳更新时间
    virtual void UpdateHeartbeatExpireTime() override;

    virtual Int64 NewPacketId() const override;

    OBJ_GET_OBJ_TYPEID_DECLARE();

private:
    virtual Int32 _OnSysInit() override;
    virtual Int32 _OnSysCompsCreated() override;
    virtual void _OnSysClose() override;

    IClientSys *_GetSysBy(const KERNEL_NS::LibString &sysName);
    const IClientSys *_GetSysBy(const KERNEL_NS::LibString &sysName) const;

    void _Clear();

    void _RegisterEvents();

    void _OnLoginConnectRes(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params);

private:
    IClientUserMgr *_userMgr;

    ClientUserInfo *_clientInfo;

    LoginInfo _loginInfo;

    std::unordered_map<KERNEL_NS::LibString, IClientSys *> _sysNameRefUserSys;
    
    UInt64 _activedSessionId;
    Int64 _serverTime;
    Int64 _lastCpuBegin;

    Int64 _heartbeatExpireTime;
    mutable Int64 _maxPacketId;
};
SERVICE_END
