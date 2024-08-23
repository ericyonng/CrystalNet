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
 * Date: 2023-08-08 13:14:36
 * Author: Eric Yonng
 * Description: 
 * 1.监听连入, 如果是指定的端口那么被判为User连入
 * 2.User连入则创建一个PendingLoginInfo, 定时15秒过期，初始状态是未鉴权UnAuth
 * 3.15秒内客户端需要请求鉴权:提供appid, 以及公钥加密的一段16字节数据
 * 4.鉴权失败则断开连接
 * 5.鉴权成功后, PendingLoginInfo，切换状态为UnLogin,重新定时15秒
 * 6.此时客户端必须进行登录(注册或者账号登录)
 * 7.登录失败时断开连接
 * 8.登录成功时移除PendingLoginInfo,并将Session添加到UserSession字典中
 * 9.并创建心跳定时,监听来自该会话的协议以及心跳协议消息，来消息就更新下过期时间
*/

#pragma once

#include <ServiceCompHeader.h>
#include <Comps/User/impl/UserHeartbeatComp.h>
#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/Event/Defs.h>

#include <set>
#include <map>

KERNEL_BEGIN

class LibEvent;
class LibTimer;
class LibPacket;
class EventManager;

KERNEL_END

SERVICE_BEGIN

struct LoginPendingInfo;
class UserMgr;
class IUser;

class UserSessionMgr : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, UserSessionMgr);

public:
    UserSessionMgr();
    ~UserSessionMgr();
    void Release() override;

protected:
    virtual Int32 _OnInit() override;
    virtual Int32 _OnStart() override;
    virtual void _OnWillClose() override;
    virtual void _OnClose() override;

    void _RegisterEvents();
    void _UnRegisterEvents();

    void _OnSessionCreated(KERNEL_NS::LibEvent *ev);
    void _OnMsgRecv(KERNEL_NS::LibEvent *ev);
    void _OnUserWillRemove(KERNEL_NS::LibEvent *ev);
    void _OnSessionWillDestroy(KERNEL_NS::LibEvent *ev);

    void _OnHeartbeatReq(KERNEL_NS::LibPacket *&packet);

    void _OnHeartbeatTimeOut(KERNEL_NS::LibTimer *t);

private:
    void _Clear();
    void _AddToHeartbeatQueue(IUser *user);
    void _RemoveFromHeartbeatQueue(IUser *user);
    void _RestartHeartbeatTimer();

    KERNEL_NS::EventManager *_eventMgr;
    UserMgr *_userMgr;

    std::map<UInt64, LoginPendingInfo *> _sessionIdRefLoginPendingInfo;

    std::set<IUser *, UserHeartbeatComp> _userHeartbeatQueue;
    KERNEL_NS::LibTimer *_heartbeatTimer;
    Int64 _heartbeatExpireTime;

    KERNEL_NS::ListenerStub _sessionCreatedStub;
    KERNEL_NS::ListenerStub _sessionMsgRecvStub;
    KERNEL_NS::ListenerStub _userWillRemoveStub;
    KERNEL_NS::ListenerStub _sessionWillRemoveStub;
};

SERVICE_END

