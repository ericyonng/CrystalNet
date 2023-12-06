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
 * Date: 2023-09-19 21:33:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/TestService/Common/ServiceCommon.h>

#include <Comps/EventRelay/impl/EventRelayGlobal.h>
#include <Comps/EventRelay/impl/EventRelayGlobalFactory.h>
#include <Comps/User/User.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IEventRelayGlobal);
POOL_CREATE_OBJ_DEFAULT_IMPL(EventRelayGlobal);

EventRelayGlobal::EventRelayGlobal()
:_removeLibraryMemberStub(INVALID_LISTENER_STUB)
,_joinLibraryMemberStub(INVALID_LISTENER_STUB)
,_userObjCreatedStub(INVALID_LISTENER_STUB)
,_userObjWillRemoveStub(INVALID_LISTENER_STUB)
{

}

EventRelayGlobal::~EventRelayGlobal()
{
    _Clear();
}

void EventRelayGlobal::Release()
{
    EventRelayGlobal::DeleteByAdapter_EventRelayGlobal(EventRelayGlobalFactory::_buildType.V, this);
}

void EventRelayGlobal::OnRegisterComps()
{

}

Int32 EventRelayGlobal::_OnGlobalSysInit()
{
    _RegisterEvents();
    return Status::Success;
}

Int32 EventRelayGlobal::_OnGlobalSysCompsCreated()
{
    return Status::Success;
}

void EventRelayGlobal::_OnGlobalSysClose()
{
    _Clear();
}   

void EventRelayGlobal::_Clear()
{
    _UnRegisterEvents();
}

void EventRelayGlobal::_RegisterEvents()
{
    if(_removeLibraryMemberStub != INVALID_LISTENER_STUB)
        return;

    auto eventMgr = GetEventMgr();
    _removeLibraryMemberStub = eventMgr->AddListener(EventEnums::REMOVE_LIBRARY_MEMBER, this, &EventRelayGlobal::_OnRemoveLibraryMember);
    _joinLibraryMemberStub = eventMgr->AddListener(EventEnums::JOIN_LIBRARY_MEMBER, this, &EventRelayGlobal::_OnJoinLibraryMember);
    _userObjCreatedStub = eventMgr->AddListener(EventEnums::USER_LOADED, this, &EventRelayGlobal::_OnUserObjLoaded);
    _userObjWillRemoveStub = eventMgr->AddListener(EventEnums::USER_WILL_REMOVE, this, &EventRelayGlobal::_OnUserObjWillRemove);
}

void EventRelayGlobal::_UnRegisterEvents()
{
    if(_removeLibraryMemberStub == INVALID_LISTENER_STUB)
        return;

    auto eventMgr = GetEventMgr();
    eventMgr->RemoveListenerX(_removeLibraryMemberStub);
    eventMgr->RemoveListenerX(_joinLibraryMemberStub);
    eventMgr->RemoveListenerX(_userObjCreatedStub);
}

void EventRelayGlobal::_OnRemoveLibraryMember(KERNEL_NS::LibEvent *ev)
{
    auto userId = ev->GetParam(Params::USER_ID).AsUInt64();
    if(UNLIKELY(userId == 0))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("USER_ID not found when recieve event"));
        return;
    }

    auto user = GetGlobalSys<IUserMgr>()->GetUser(userId);
    if(UNLIKELY(!user))
    {
        g_Log->Debug(LOGFMT_OBJ_TAG("user not online event user id:%llu"), userId);
        return;
    }

    // 避免从global => relay => user => global 的死循环
    const auto eventId = ev->GetId();
    if(_IsEventInCircleHandling(user->GetUserId(), ev->GetId()))
    {
        g_Log->Debug(LOGFMT_OBJ_TAG("skip remove library member event in circle handle user id:%llu, eventid:%d")
        , userId, ev->GetId());
        return;
    }

    _DisableFromRelayToUser(userId, eventId);

    auto oldStatus = ev->IsDontDelAfterFire();
    ev->SetDontDelAfterFire(true);

    user->FireEvent(ev);

    ev->SetDontDelAfterFire(oldStatus);

    _OnEventFromUserHandled(userId, eventId);
}

void EventRelayGlobal::_OnJoinLibraryMember(KERNEL_NS::LibEvent *ev)
{
    auto userId = ev->GetParam(Params::USER_ID).AsUInt64();
    if(UNLIKELY(userId == 0))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("USER_ID not found when recieve event"));
        return;
    }

    auto user = GetGlobalSys<IUserMgr>()->GetUser(userId);
    if(UNLIKELY(!user))
    {
        g_Log->Debug(LOGFMT_OBJ_TAG("user not online event user id:%llu"), userId);
        return;
    }

    const auto eventId = ev->GetId();
    if(_IsEventInCircleHandling(user->GetUserId(), eventId))
    {
        g_Log->Debug(LOGFMT_OBJ_TAG("skip join library member event in circle handle user id:%llu, eventid:%d")
        , userId, ev->GetId());
        return;
    }

    _DisableFromRelayToUser(userId, eventId);

    auto oldStatus = ev->IsDontDelAfterFire();
    ev->SetDontDelAfterFire(true);

    user->FireEvent(ev);

    ev->SetDontDelAfterFire(oldStatus);

    _OnEventFromUserHandled(userId, eventId);
}

void EventRelayGlobal::_OnUserObjLoaded(KERNEL_NS::LibEvent *ev)
{
    auto user = ev->GetParam(Params::USER_OBJ).AsPtr<IUser>();

    auto iter = _fromUserToGlobalStub.find(user->GetUserId());
    if(iter == _fromUserToGlobalStub.end())
    {
        g_Log->Info(LOGFMT_OBJ_TAG("user created and will add relay from user to global user:%s"), user->ToString().c_str());

        auto stub = user->AddListener(0, this, &EventRelayGlobal::_EventFromUserToGlobal);
        _fromUserToGlobalStub.insert(std::make_pair(user->GetUserId(), stub));
    }
}

void EventRelayGlobal::_OnUserObjWillRemove(KERNEL_NS::LibEvent *ev)
{
    auto userId = ev->GetParam(Params::USER_ID).AsUInt64();
    auto iter = _fromUserToGlobalStub.find(userId);
    if(iter != _fromUserToGlobalStub.end())
    {
        auto user = GetGlobalSys<IUserMgr>()->GetUser(userId);
        if(LIKELY(user))
            user->RemoveListener(iter->second);

        _fromUserToGlobalStub.erase(iter);
    }
}

void EventRelayGlobal::_EventFromUserToGlobal(KERNEL_NS::LibEvent *ev)
{
    // 必须要带USER_OBJ参数, 没带的不处理
    auto user = ev->GetParam(Params::USER_OBJ).AsPtr<IUser>();
    if(!user)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("relay from user to global event need USER_OBJ param please check evId:%d"), ev->GetId());
        return;
    }

    const auto userId = user->GetUserId();
    const auto eventId = ev->GetId();
    if(_IsEventInCircleHandling(userId, eventId))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("event is in circle user id:%llu, event:%d"), userId, eventId);
        return;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("recieve event from user userId:%llu, evId:%d"), userId, ev->GetId());

    _DisableFromRelayToUser(userId, ev->GetId());

    auto flag = ev->IsDontDelAfterFire();
    ev->SetDontDelAfterFire(true);
    GetEventMgr()->FireEvent(ev);
    ev->SetDontDelAfterFire(flag);

    _OnEventFromUserHandled(userId, eventId);
}

SERVICE_END
