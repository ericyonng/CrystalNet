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
 * Date: 2022-09-19 02:24:42
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/TestService/Common/ServiceCommon.h>

#include <service/TestService/Comps/StubHandle/Impl/StubHandleMgr.h>
#include <service/TestService/Comps/StubHandle/Impl/StubHandleMgrFactory.h>

SERVICE_BEGIN


StubHandleMgr::StubHandleMgr()
:SERVICE_NS::IStubHandleMgr(KERNEL_NS::RttiUtil::GetTypeId<StubHandleMgr>())
,_maxStub(0)
,_addListenResEvStub(INVALID_LISTENER_STUB)
,_asynConnectResEvStub(INVALID_LISTENER_STUB)
{

}

StubHandleMgr::~StubHandleMgr()
{
    _Clear();
}

void StubHandleMgr::Release()
{
    StubHandleMgr::DeleteByAdapter_StubHandleMgr(StubHandleMgrFactory::_buildType.V, this);
}

KERNEL_NS::LibString StubHandleMgr::ToString() const
{
    return IStubHandleMgr::ToString();
}

UInt64 StubHandleMgr::NewStub()
{
    return ++_maxStub;
}

bool StubHandleMgr::HasStub(UInt64 stub) const 
{
    auto iter = _stubRefCallback.find(stub);
    return iter != _stubRefCallback.end();
}

Int32 StubHandleMgr::NewHandle(UInt64 stub, KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *, bool &> *delg)
{
    if(_stubRefCallback.find(stub) != _stubRefCallback.end())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("repeat stub:%llu"), stub);
        return Status::Repeat;
    }

    _stubRefCallback.insert(std::make_pair(stub, delg));

    // g_Log->Info(LOGFMT_OBJ_TAG("new stub with callback stub:%llu"), stub);
    return Status::Success;
}

Int32 StubHandleMgr::NewHandle(KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *, bool &> *delg, UInt64 &stub)
{
    auto newStub = NewStub();
    auto st = NewHandle(newStub, delg);
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("repeate stub:%llu"), newStub);
        return st;
    }

    stub = newStub;
    return Status::Success;
}

void StubHandleMgr::InvokeHandle(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params)
{
    auto iter = _stubRefCallback.find(stub);
    if(iter == _stubRefCallback.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("stub:%llu, callback not found errCode:%d"), stub, errCode);
        return;
    }

    bool doRemove = true;
    if(LIKELY(iter->second))
    {
        iter->second->Invoke(stub, errCode, params, doRemove);

        if(LIKELY(doRemove))
           iter->second->Release();
    }

    if(LIKELY(doRemove))
        _stubRefCallback.erase(iter);
}

Int32 StubHandleMgr::_OnGlobalSysInit() 
{
    _RegisterEvents();
    return Status::Success;
}

void StubHandleMgr::_OnGlobalSysClose()
{
    _Clear();
}

void StubHandleMgr::_Clear()
{
    _UnRegisterEvents();

    KERNEL_NS::ContainerUtil::DelContainer(_stubRefCallback, [](KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *, bool &> *ptr){
        if(LIKELY(ptr))
        {
            ptr->Release();
        }
    });
}

void StubHandleMgr::_RegisterEvents()
{
    if(_addListenResEvStub)
        return;

    // TODO:监听有stub的相关事件
    auto eventMgr = GetEventMgr();
    _addListenResEvStub = eventMgr->AddListener(EventEnums::ADD_LISTEN_RES, this, &StubHandleMgr::_OnAddListenResEvent);
    _asynConnectResEvStub = eventMgr->AddListener(EventEnums::ASYN_CONNECT_RES, this, &StubHandleMgr::_OnAsynConnectResEvent);
}

void StubHandleMgr::_UnRegisterEvents()
{
    if(_addListenResEvStub == INVALID_LISTENER_STUB)
        return;

    auto eventMgr = GetEventMgr();
    eventMgr->RemoveListenerX(_addListenResEvStub);
    eventMgr->RemoveListenerX(_asynConnectResEvStub);
}

void StubHandleMgr::_OnAddListenResEvent(KERNEL_NS::LibEvent *ev)
{
    auto errCode = ev->GetParam(Params::ERROR_CODE).AsInt32();
    auto localAddr = ev->GetParam(Params::LOCAL_ADDR).AsPtr<KERNEL_NS::BriefSockAddr>();
    auto family = ev->GetParam(Params::FAMILY).AsUInt16();
    auto serviceId = ev->GetParam(Params::SERVICE_ID).AsUInt64();
    auto stub = ev->GetParam(Params::STUB).AsUInt64();
    auto protocolType = ev->GetParam(Params::PROTOCOL_TYPE).AsInt32();
    auto sessionId = ev->GetParam(Params::SESSION_ID).AsUInt64();

    auto var = KERNEL_NS::Variant::NewThreadLocal_Variant();
    var->BecomeDict();
    (*var)[Params::ERROR_CODE] = errCode;
    (*var)[Params::LOCAL_ADDR] = localAddr;
    (*var)[Params::FAMILY] = family;
    (*var)[Params::SERVICE_ID] = serviceId;
    (*var)[Params::STUB] = stub;
    (*var)[Params::PROTOCOL_TYPE] = protocolType;
    (*var)[Params::SESSION_ID] = sessionId;
    
    InvokeHandle(stub, errCode, var);
    KERNEL_NS::Variant::DeleteThreadLocal_Variant(var);
}

void StubHandleMgr::_OnAsynConnectResEvent(KERNEL_NS::LibEvent *ev)
{
    auto var = KERNEL_NS::Variant::NewThreadLocal_Variant();
    var->BecomeDict();
    (*var)[Params::ERROR_CODE] = ev->GetParam(Params::ERROR_CODE).AsInt32();
    (*var)[Params::LOCAL_ADDR] = ev->GetParam(Params::LOCAL_ADDR).AsPtr<KERNEL_NS::BriefAddrInfo>();
    (*var)[Params::REMOTE_ADDR] = ev->GetParam(Params::REMOTE_ADDR).AsPtr<KERNEL_NS::BriefAddrInfo>();
    (*var)[Params::FAMILY] = ev->GetParam(Params::FAMILY).AsUInt16();
    (*var)[Params::PROTOCOL_TYPE] = ev->GetParam(Params::PROTOCOL_TYPE).AsInt32();
    (*var)[Params::SESSION_POLLER_ID] = ev->GetParam(Params::SESSION_POLLER_ID).AsUInt64();
    (*var)[Params::SERVICE_ID] = ev->GetParam(Params::SERVICE_ID).AsUInt64();
    (*var)[Params::STUB] = ev->GetParam(Params::STUB).AsUInt64();
    (*var)[Params::SESSION_ID] = ev->GetParam(Params::SESSION_ID).AsUInt64();
    (*var)[Params::TARGET_ADDR_IP_CONFIG] = ev->GetParam(Params::TARGET_ADDR_IP_CONFIG).AsPtr<KERNEL_NS::AddrIpConfig>();
    (*var)[Params::TARGET_ADDR_FAILURE_IP_SET] = ev->GetParam(Params::TARGET_ADDR_FAILURE_IP_SET).AsPtr<std::set<KERNEL_NS::LibString>>();
    
    InvokeHandle(ev->GetParam(Params::STUB).AsUInt64()
                , ev->GetParam(Params::ERROR_CODE).AsInt32()
                , var);
                
    KERNEL_NS::Variant::DeleteThreadLocal_Variant(var);
}



SERVICE_END