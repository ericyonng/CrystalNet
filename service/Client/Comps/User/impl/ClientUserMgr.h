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
 * Date: 2023-08-12 18:00:38
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <Comps/User/interface/IClientUserMgr.h>

SERVICE_BEGIN
struct AddrConfig;

class ClientUserHeartbeatComp
{
public:
    bool operator()(const IClientUser *l, const IClientUser *r) const;
};

class ClientUserMgr : public IClientUserMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IClientUserMgr, ClientUserMgr);

public:
    ClientUserMgr();
    ~ClientUserMgr();
    void Release() override;

    virtual void OnRegisterComps() override;
    virtual void OnWillStartup() override;
    virtual void OnStartup() override;

    virtual Int32 Login(const LoginInfo &loginInfo, Int32 stackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL);
    IClientUser *GetUser(const KERNEL_NS::LibString &accountName);
    const IClientUser *GetUser(const KERNEL_NS::LibString &accountName) const;
    IClientUser *GetUserBySessinId(UInt64 sessionId);
    const IClientUser *GetUserBySessinId(UInt64 sessionId) const;

    virtual void AddUserBySessionId(UInt64 sessionId, IClientUser *user) override;
    virtual void RemoveUserBySessionId(UInt64 sessionId) override;


protected:
    virtual Int32 _OnGlobalSysInit() override;
    virtual Int32 _OnGlobalSysCompsCreated() override;
    virtual Int32 _OnHostStart() override;
    virtual void _OnGlobalSysClose() override;

    void _OnLoginRes(KERNEL_NS::LibPacket *&packet);
    void _OnClientHeartbeatRes(KERNEL_NS::LibPacket *&packet);
    void _OnClientLogoutNty(KERNEL_NS::LibPacket *&packet);

    void _RestartHeartbeatTimer();
    void _OnHeartbeatTimeOut(KERNEL_NS::LibTimer *t);

    void _Clear();

private:
    std::map<KERNEL_NS::LibString, IClientUser *> _accountNameRefUser;
    std::map<UInt64, IClientUser *> _sessionIdRefUser;
    std::set<IClientUser *, ClientUserHeartbeatComp>  _heartbeatQueue;
    KERNEL_NS::LibTimer *_heartbeatRemoveUserTimer;

    KERNEL_NS::LibString _rsaPublicKey;
    KERNEL_NS::LibString _rsaPublicKeyRaw;
    KERNEL_NS::LibRsa _rsa;
    AddrConfig *_targetAddrConfig;  // 测试的目标地址
    KERNEL_NS::LibString _testLoginAccountName;
};

SERVICE_END
