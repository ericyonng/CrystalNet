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
 * Date: 2023-08-12 17:57:38
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <ServiceCompHeader.h>
#include <protocols/protocols.h>

SERVICE_BEGIN

class IClientUserMgr;

class IClientUser : public ILogicSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(ILogicSys, IClientUser);
public:
    IClientUser(UInt64 objTypeId) : ILogicSys(objTypeId) {}
    virtual Int64 GetNowServerTime() const = 0;
    virtual void SetServerTime(Int64 serverTime) = 0;

    virtual IClientUserMgr *GetUserMgr() = 0;
    virtual const IClientUserMgr *GetUserMgr() const = 0;

    virtual ClientUserInfo *GetClientInfo() = 0;
    virtual const ClientUserInfo *GetClientInfo() const = 0;

    virtual Int64 Send(KERNEL_NS::LibPacket *packet) const = 0;
    virtual void Send(const std::list<KERNEL_NS::LibPacket *> &packets) const = 0;
    virtual Int64 Send(Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId = -1) const = 0;

    virtual void OnLogin() = 0;
    virtual void OnLoginFinish()  = 0;
    virtual void OnLogout()  = 0;
    virtual void OnUserCreated()  = 0;

    virtual Int32 GetUserStatus() const  = 0;
    virtual void SetUserStatus(Int32 status)  = 0;

    virtual UInt64 GetUserId() const  = 0;

    // 绑定会话
    virtual void BindSession(UInt64 sessionId)  = 0;
    // 获取会话id
    virtual UInt64 GetSessionId() const  = 0;

    virtual void Logout()  = 0;
    virtual Int32 Login(Int32 stackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL) = 0;
    // 设置登录信息
    virtual void SetLoginInfo(const LoginInfo &loginInfo) = 0;
    virtual const LoginInfo &GetLoginInfo() const = 0;
    virtual LoginInfo &GetLoginInfo() = 0;


    // 是否登出
    virtual bool IsLogined() const  = 0;
    virtual bool IsLogining() const = 0;
    // 是否在线
    virtual bool IsLogout() const  = 0;


    virtual const KERNEL_NS::BriefSockAddr *GetUserAddr() const  = 0;

    virtual Int64 GetHeartbeatExpireTime() const = 0;
    // 刷新心跳更新时间
    virtual void UpdateHeartbeatExpireTime() = 0;

    // 包id
    virtual Int64 NewPacketId() const = 0;
};

SERVICE_END
