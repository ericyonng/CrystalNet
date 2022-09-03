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
 * Date: 2022-06-22 01:50:54
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service_common/service/IService.h>
#include <service_common/service_proxy/ServiceProxyCompType.h>

SERVICE_COMMON_BEGIN

IService::PollerEventHandler IService::_pollerEventHandler[KERNEL_NS::PollerEventType::EvMax] = {NULL};

IService::IService()
:_serviceId(0)
,_poller(NULL)
,_serviceProxy(NULL)
,_maxPieceTimeInMicroseconds(0)
,_maxPriorityLevel(0)
,_maxSleepMilliseconds(0)
{
    _SetType(ServiceProxyCompType::COMP_SERVICE);
    
    InitPollerEventHandler();
}

IService::~IService()
{

}

void IService::Clear()
{
    _OnServiceClear();
    _Clear();
    CompHostObject::Clear();
}

void IService::OnRegisterComps()
{
    // 基础组件
    RegisterComp<KERNEL_NS::PollerFactory>();

    _OnServiceRegisterComps();
}  

KERNEL_NS::LibString IService::ToString() const
{
    return KERNEL_NS::LibString().AppendFormat("%s, id:%llu", _serviceName.c_str(), _serviceId);
}

KERNEL_NS::LibString IService::IntroduceInfo() const
{
    return KERNEL_NS::LibString().AppendFormat("service id:%llu, name:%s"
                , _serviceId, _serviceName.c_str());
}

void IService::InitPollerEventHandler()
{
    _pollerEventHandler[KERNEL_NS::PollerEventType::SessionCreated] = &IService::_OnSessionCreated;
    _pollerEventHandler[KERNEL_NS::PollerEventType::AsynConnectRes] = &IService::_OnAsynConnectRes;
    _pollerEventHandler[KERNEL_NS::PollerEventType::AddListenRes] = &IService::_OnAddListenRes;
    _pollerEventHandler[KERNEL_NS::PollerEventType::SessionDestroy] = &IService::_OnSessionDestroy;
    _pollerEventHandler[KERNEL_NS::PollerEventType::RecvMsg] = &IService::_OnRecvMsg;
}

Int32 IService::_OnHostInit()
{
    auto errCode = _OnServiceInit();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("service %s init fail suc errCode:%d."), IntroduceInfo().c_str(), errCode);
        return errCode;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("service %s init suc."), IntroduceInfo().c_str());
    return Status::Success;
}

Int32 IService::_OnCompsCreated()
{
    _poller = GetComp<KERNEL_NS::Poller>();

    // poller 设置
    KERNEL_NS::TimeSlice span(0, 0, _maxPieceTimeInMicroseconds);
    _poller->SetMaxPriorityLevel(_maxPriorityLevel);
    _poller->SetMaxPieceTime(span);
    _poller->SetMaxSleepMilliseconds(_maxSleepMilliseconds);
    _poller->SetPepareEventWorkerHandler(this, &IService::_OnPollerPrepare);
    _poller->SetEventWorkerCloseHandler(this, &IService::_OnPollerWillDestroy);
    _poller->SetEventHandler(this, &IService::_OnMsg);

    Int32 errCode = _OnServiceCompsCreated();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("service %s handle _OnServiceCompsCreated fail errCode:%d.")
                    , IntroduceInfo().c_str(), errCode);
        return errCode;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("service %s comps created suc."), IntroduceInfo().c_str());
    return Status::Success;
}

Int32 IService::_OnHostWillStart()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s will start suc."), IntroduceInfo().c_str());
    return Status::Success;
}

Int32 IService::_OnHostStart()
{
    Int32 errCode = _OnServiceStartup();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("service:%s, _OnServiceStartup fail errCode:%d"), IntroduceInfo().c_str(), errCode);
        return errCode;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("service %s start suc."), IntroduceInfo().c_str());
    return GetErrCode();
}

void IService::_OnHostBeforeCompsWillClose() 
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s before comps will close suc."), IntroduceInfo().c_str());
}

void IService::_OnHostWillClose()
{
    _OnServiceWillClose();
    g_Log->Info(LOGFMT_OBJ_TAG("service %s will close suc."), IntroduceInfo().c_str());
}

void IService::_OnHostClose() 
{
    _OnServiceClosed();
    g_Log->Info(LOGFMT_OBJ_TAG("service %s close suc."), IntroduceInfo().c_str());
}

void IService::_OnServiceClear()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s on service clear default."), IntroduceInfo().c_str());
}

Int32 IService::_OnServiceCompsCreated()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s on service comps created default."), IntroduceInfo().c_str());
    return Status::Success;
}

Int32 IService::_OnServiceStartup()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s on service start up default."), IntroduceInfo().c_str());
    return Status::Success;
}

void IService::_OnServiceWillClose()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s on service will close default."), IntroduceInfo().c_str());
}

void IService::_OnServiceClosed()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service %s on service closed default."), IntroduceInfo().c_str());
}

void IService::_OnMsg(KERNEL_NS::PollerEvent *msg)
{
    auto handler = _pollerEventHandler[msg->_type];
    if(UNLIKELY(!handler))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("unregister event handler event type:%d, event:%s"), msg->_type, msg->ToString().c_str());
        return;
    }

    (this->*handler)(msg);

    g_Log->Debug(LOGFMT_OBJ_TAG("finish handle msg:%s"), msg->ToString().c_str());
}

void IService::_OnSessionCreated(KERNEL_NS::PollerEvent *msg)
{
    g_Log->Info(LOGFMT_OBJ_TAG("session created:%s"), msg->ToString().c_str());
}

void IService::_OnSessionDestroy(KERNEL_NS::PollerEvent *msg) 
{
    g_Log->Info(LOGFMT_OBJ_TAG("session destroyed:%s"), msg->ToString().c_str());
}

void IService::_OnAsynConnectRes(KERNEL_NS::PollerEvent *msg)
{
    g_Log->Info(LOGFMT_OBJ_TAG("asyn connect response:%s"), msg->ToString().c_str());
}

void IService::_OnAddListenRes(KERNEL_NS::PollerEvent *msg) 
{
    g_Log->Info(LOGFMT_OBJ_TAG("add listen response:%s"), msg->ToString().c_str());
}

void IService::_OnRecvMsg(KERNEL_NS::PollerEvent *msg) 
{
    g_Log->Info(LOGFMT_OBJ_TAG("recieve message from net:%s"), msg->ToString().c_str());
}

bool IService::_OnPollerPrepare(KERNEL_NS::Poller *poller)
{
    g_Log->Info(LOGFMT_OBJ_TAG("service poller prepare"));
    return true;
}

void IService::_OnPollerWillDestroy(KERNEL_NS::Poller *poller)
{
    g_Log->Info(LOGFMT_OBJ_TAG("service will destroy."));
}

void IService::_Clear()
{
    g_Log->Info(LOGFMT_OBJ_TAG("service will clear."));
}


SERVICE_COMMON_END
