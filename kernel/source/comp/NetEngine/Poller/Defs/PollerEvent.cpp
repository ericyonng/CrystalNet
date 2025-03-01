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
 * Date: 2021-06-27 16:31:18
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/LibStream.h>
#include <kernel/comp/Delegate/IDelegate.h>

#include <kernel/comp/NetEngine/Defs/LibConnectInfo.h>
#include <kernel/comp/NetEngine/Defs/BuildSessionInfo.h>
#include <kernel/comp/NetEngine/Defs/LibListenInfo.h>

#include <kernel/comp/NetEngine/LibPacket.h>
#include <kernel/comp/NetEngine/Defs/IoEvent.h>
#include <kernel/comp/NetEngine/Poller/Defs/CloseSessionInfo.h>
#include <kernel/comp/Utils/ContainerUtil.h>

#include <kernel/comp/NetEngine/Poller/Defs/PollerInnerEvent.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerEvent.h>
#include <kernel/comp/Utils/StringUtil.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(MonitorPollerEvent);

MonitorPollerEvent::MonitorPollerEvent()
    :PollerEvent(PollerEventType::Monitor)

    #if CRYSTAL_TARGET_PLATFORM_LINUX
    ,_epEvents{NULL}
    ,_count(0)
    #endif
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    ,_errCode(Status::Success)
    #endif
{
    //_epEvents._bytes = NULL;
}

MonitorPollerEvent::~MonitorPollerEvent()
{
    #if CRYSTAL_TARGET_PLATFORM_LINUX
    if(_epEvents._bytes)
    {
        g_MemoryPool->Free(_epEvents._bytes);
        _epEvents._bytes = NULL;
        _count = 0;
    }

    #else

    #endif
}

void MonitorPollerEvent::Release()
{
    MonitorPollerEvent::Delete_MonitorPollerEvent(this);
}

LibString MonitorPollerEvent::ToString() const
{
    LibString info;
    info.AppendFormat("event type:%d, MonitorPollerEvent:", _type);

    #if CRYSTAL_TARGET_PLATFORM_LINUX
    info.AppendFormat("union _epEvents:%p, _count:%d", _epEvents._bytes, _count);
    #endif

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    info.AppendFormat("_io:{_sessionId:%llu, _ioData:%p, _bytesTrans:%lld}, _errCode:%d"
        , _io._sessionId, _io._ioData, _io._bytesTrans, _errCode);
    #endif

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(AsynSendEvent);

AsynSendEvent::AsynSendEvent()
    :PollerEvent(PollerEventType::Write)
    ,_sessionId(0)
    ,_packets(NULL)
{

}

AsynSendEvent::~AsynSendEvent()
{
    if(_packets)
    {
        ContainerUtil::DelContainer(*_packets, [](LibPacket *packet){
            if(LIKELY(packet))
                packet->ReleaseUsingPool();
        });

        LibList<LibPacket *>::Delete_LibList(_packets);
        _packets = NULL;
    }
}

void AsynSendEvent::Release()
{
    AsynSendEvent::Delete_AsynSendEvent(this);
}

LibString AsynSendEvent::ToString() const
{
    const UInt64 packetsSize = _packets ? _packets->GetAmount() : 0;

    LibString info;
    info.AppendFormat("event type:%d, AsynSendEvent: packets list size:%llu, [", _type, packetsSize);

    if(LIKELY(_packets))
    {
        for(auto node = _packets->Begin(); node; node = node->_next)
            info.AppendFormat("%s\n", node->_data->ToString().c_str());
    }

    info.AppendFormat("]");

    return info;  
}

POOL_CREATE_OBJ_DEFAULT_IMPL(AsynConnectEvent);

AsynConnectEvent::AsynConnectEvent()
    :PollerEvent(PollerEventType::AsynConnect)
    ,_connectInfo(NULL)
{
}

void AsynConnectEvent::Release()
{
    AsynConnectEvent::Delete_AsynConnectEvent(this);
}

LibString AsynConnectEvent::ToString() const
{
    LibString info;
    info.AppendFormat("event type:%d, AsynConnectEvent:", _type)
        .AppendFormat("_connectInfo:%s, ", _connectInfo ? _connectInfo->ToString().c_str() : "NULL")
        ;
    return info; 
}

POOL_CREATE_OBJ_DEFAULT_IMPL(NewSessionEvent);

NewSessionEvent::NewSessionEvent()
:PollerEvent(PollerEventType::NewSession)
,_buildInfo(NULL)
{

}

void NewSessionEvent::Release()
{
    NewSessionEvent::Delete_NewSessionEvent(this);
}

LibString NewSessionEvent::ToString() const
{
    LibString info;
    info.AppendFormat("event type:%d, NewSessionEvent:", _type)
        .AppendFormat("_buildInfo:%s, ", _buildInfo ? _buildInfo->ToString().c_str() : "NULL")
        ;
    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(CloseSessionEvent);

CloseSessionEvent::CloseSessionEvent()
:PollerEvent(PollerEventType::CloseSession)
,_sessionId(0)
,_fromServiceId(0)
,_priorityLevel(0)
,_closeMillisecondTime(0)
,_stub(0)
,_forbidRead(false)
,_forbidWrite(false)
{

}

void CloseSessionEvent::Release()
{
    CloseSessionEvent::Delete_CloseSessionEvent(this);
}

LibString CloseSessionEvent::ToString() const
{
    LibString info;
    info.AppendFormat("event type:%d, CloseSessionEvent:", _type)
        .AppendFormat("_sessionId:%llu, ", _sessionId)
        .AppendFormat("_fromServiceId:%llu, ", _fromServiceId)
        .AppendFormat("_priorityLevel:%u, ", _priorityLevel)
        .AppendFormat("_closeMillisecondTime:%lld, ", _closeMillisecondTime)
        .AppendFormat("_stub:%llu, ", _stub)
        .AppendFormat("_forbidRead:%s, ", _forbidRead ? "true" : "false")
        .AppendFormat("_forbidWrite:%s, ", _forbidWrite ? "true" : "false")
        ;
    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(QuitServiceSessionsEvent);

QuitServiceSessionsEvent::QuitServiceSessionsEvent()
:PollerEvent(PollerEventType::QuitServiceSessionsEvent)
,_fromServiceId(0)
,_priorityLevel(0)
{

}

void QuitServiceSessionsEvent::Release()
{
    QuitServiceSessionsEvent::Delete_QuitServiceSessionsEvent(this);
}

LibString QuitServiceSessionsEvent::ToString() const 
{
    LibString info;
    info.AppendFormat("event type:%d, QuitServiceSessionsEvent: _fromServiceId:%llu, _priorityLevel:%u"
                    , _type, _fromServiceId, _priorityLevel);

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(QuitSessionInfo);
POOL_CREATE_OBJ_DEFAULT_IMPL(RealDoQuitServiceSessionEvent);

RealDoQuitServiceSessionEvent::RealDoQuitServiceSessionEvent()
:PollerEvent(PollerEventType::RealDoQuitServiceSessionEvent)
,_fromServiceId(0)
,_quitSessionInfo(LibList<QuitSessionInfo *>::New_LibList())
{

}

RealDoQuitServiceSessionEvent::~RealDoQuitServiceSessionEvent()
{
    if(LIKELY(_quitSessionInfo))
        LibList<QuitSessionInfo *>::Delete_LibList(_quitSessionInfo);
    _quitSessionInfo = NULL;
}

void RealDoQuitServiceSessionEvent::Release()
{
    RealDoQuitServiceSessionEvent::Delete_RealDoQuitServiceSessionEvent(this);
}

LibString RealDoQuitServiceSessionEvent::ToString() const 
{
    LibString info;
    info.AppendFormat("event type:%d, RealDoQuitServiceSessionEvent: _fromServiceId:%llu, list count:%llu"
                    , _type, _fromServiceId, _quitSessionInfo->GetAmount());

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(AddListenEvent);

AddListenEvent::AddListenEvent()
:PollerEvent(PollerEventType::AddListen)
{

}

void AddListenEvent::Release()
{
    AddListenEvent::Delete_AddListenEvent(this);
}

LibString AddListenEvent::ToString() const
{
    const Int32 listSize = static_cast<Int32>(_addListenInfoList.size());
    LibString info;
    info.AppendFormat("event type:%d, AddListenEvent: listen info array amount:%d, array:[", _type, listSize);

    for(Int32 idx = 0; idx < listSize; ++idx)
        info.AppendFormat("[%d]=%s\n", idx, _addListenInfoList[idx]->ToString().c_str());

    info.AppendFormat("]");

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(SessionCreatedEvent);

SessionCreatedEvent::SessionCreatedEvent()
:PollerEvent(PollerEventType::SessionCreated)
,_sessionId(0)
,_localAddr(true)
,_targetAddr(true)
,_family(0)
,_protocolType(0)
,_priorityLevel(0)
,_sessionType(0)
,_sessionPollerId(0)
,_belongServiceId(0)
,_stub(0)
,_isFromConnect(false)
,_isLinker(false)
,_protocolStackType(0)
{

}

void SessionCreatedEvent::Release()
{
    SessionCreatedEvent::Delete_SessionCreatedEvent(this);
}

LibString SessionCreatedEvent::ToString() const
{
    LibString info;
    info.AppendFormat("event type:%d, SessionCreatedEvent:", _type)
        .AppendFormat("_sessionId:%llu, \n", _sessionId)
        .AppendFormat("_localAddr:%s, \n", _localAddr.ToString().c_str())
        .AppendFormat("_targetAddr:%s, \n", _targetAddr.ToString().c_str())
        .AppendFormat("_family:%hu, \n", _family)
        .AppendFormat("_protocolType:%d, \n", _protocolType)
        .AppendFormat("_priorityLevel:%u, \n", _priorityLevel)
        .AppendFormat("_sessionType:%d, \n", _sessionType)
        .AppendFormat("_sessionPollerId:%llu, \n", _sessionPollerId)
        .AppendFormat("_belongServiceId:%llu, \n", _belongServiceId)
        .AppendFormat("_stub:%llu, \n", _stub)
        .AppendFormat("_isFromConnect:%s, \n", _isFromConnect ? "true" : "false")
        .AppendFormat("_isLinker:%s, \n", _isLinker ? "true" : "false")
        .AppendFormat("_protocolStackType:%d, \n", _protocolStackType)
        .AppendFormat("_targetConfig:%s, \n", _targetConfig.ToString().c_str())
        .AppendFormat("_failureIps:%s, \n", KERNEL_NS::StringUtil::ToString(_failureIps, ',').c_str())
        ;

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(AsynConnectResEvent);

AsynConnectResEvent::AsynConnectResEvent()
:PollerEvent(PollerEventType::AsynConnectRes)
,_errCode(Status::Success)
,_localAddr(true)
,_targetAddr(true)
,_family(0)
,_protocolType(0)
,_priorityLevel(0)
,_sessionPollerId(0)
,_fromServiceId(0)
,_stub(0)
,_sessionId(0)
{

}

void AsynConnectResEvent::Release()
{
    AsynConnectResEvent::Delete_AsynConnectResEvent(this);
}

LibString AsynConnectResEvent::ToString() const
{
    LibString info;
    info.AppendFormat("event type:%d, AsynConnectResEvent:", _type)
        .AppendFormat("_errCode:%d, \n", _errCode)
        .AppendFormat("_localAddr:%s, \n", _localAddr.ToString().c_str())
        .AppendFormat("_targetAddr:%s, \n", _targetAddr.ToString().c_str())
        .AppendFormat("_family:%hu, \n", _family)
        .AppendFormat("_protocolType:%d, \n", _protocolType)
        .AppendFormat("_priorityLevel:%u, \n", _priorityLevel)
        .AppendFormat("_sessionPollerId:%llu, \n", _sessionPollerId)
        .AppendFormat("_fromServiceId:%llu, \n", _fromServiceId)
        .AppendFormat("_stub:%llu, \n", _stub)
        .AppendFormat("_sessionId:%llu, \n", _sessionId)
        .AppendFormat("target addr config:%s, \n", _targetAddr.ToString().c_str())
        .AppendFormat("failure ips:%s, \n", KERNEL_NS::StringUtil::ToString(_failureIps, ',').c_str())
        ;

    return info; 
}

POOL_CREATE_OBJ_DEFAULT_IMPL(AddListenResEvent);

AddListenResEvent::AddListenResEvent()
:PollerEvent(PollerEventType::AddListenRes)
,_errCode(Status::Success)
,_localAddr(true)
,_family(0)
,_serviceId(0)
,_stub(0)
,_priorityLevel(0)
,_protocolType(0)
,_sessionId(0)
{

}

void AddListenResEvent::Release()
{
    AddListenResEvent::Delete_AddListenResEvent(this);
}

LibString AddListenResEvent::ToString() const
{
    LibString info;
    info.AppendFormat("event type:%d, AddListenResEvent:", _type)
        .AppendFormat("_errCode:%d, \n", _errCode)
        .AppendFormat("_localAddr:%s, \n", _localAddr.ToString().c_str())
        .AppendFormat("_family:%hu, \n", _family)
        .AppendFormat("_serviceId:%llu, \n", _serviceId)
        .AppendFormat("_stub:%llu, \n", _stub)
        .AppendFormat("_priorityLevel:%u, \n", _priorityLevel)
        .AppendFormat("_protocolType:%d, \n", _protocolType)
        .AppendFormat("_sessionId:%llu, \n", _sessionId)
        ;

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(SessionDestroyEvent);

SessionDestroyEvent::SessionDestroyEvent()
:PollerEvent(PollerEventType::SessionDestroy)
,_closeReason(0)
,_sessionId(0)
,_serviceId(0)
,_priorityLevel(0)
,_stub(0)
{

}

void SessionDestroyEvent::Release()
{
    SessionDestroyEvent::Delete_SessionDestroyEvent(this);
}

LibString SessionDestroyEvent::ToString() const
{
    LibString info;
    info.AppendFormat("event type:%d, SessionDestroyEvent:", _type)
        .AppendFormat("_closeReason:%d, %s \n", _closeReason, CloseSessionInfo::GetCloseReason(_closeReason))
        .AppendFormat("_sessionId:%llu, \n", _sessionId)
        .AppendFormat("_serviceId:%llu, \n", _serviceId)
        .AppendFormat("_priorityLevel:%u, \n", _priorityLevel)
        .AppendFormat("_stub:%llu, \n", _stub)
        ;

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(RecvMsgEvent);

RecvMsgEvent::RecvMsgEvent()
:PollerEvent(PollerEventType::RecvMsg)
,_sessionId(0)
,_serviceId(0)
,_priorityLevel(0)
,_packets(NULL)
{

}

void RecvMsgEvent::Release()
{
    RecvMsgEvent::Delete_RecvMsgEvent(this);
}

RecvMsgEvent::~RecvMsgEvent()
{
    if(_packets)
    {
        ContainerUtil::DelContainer(*_packets, [](LibPacket *packet){
            if(LIKELY(packet))
                packet->ReleaseUsingPool();
        });

        LibList<LibPacket *>::Delete_LibList(_packets);
        _packets = NULL;
    }
}

LibString RecvMsgEvent::ToString() const
{
    LibString info;
    info.AppendFormat("event type:%d, RecvMsgEvent:", _type)
        .AppendFormat("_sessionId:%llu, \n", _sessionId)
        .AppendFormat("_serviceId:%llu, \n", _serviceId)
        .AppendFormat("_priorityLevel:%u, \n", _priorityLevel)
        .AppendFormat("packets amount:%llu, \n", _packets ? _packets->GetAmount() : 0)
        ;

    info.AppendFormat("packets :[");
    if(LIKELY(_packets))
    {
        for(auto node = _packets->Begin(); node; node = node->_next)
        {
            if(LIKELY(node->_data))
            {
                info.AppendFormat("%s\n", node->_data->ToString().c_str());
            }
            else
            {
                info.AppendFormat("null\n");
            }
        }
    }
    info.AppendFormat("]");

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(IpControlInfo);

IpControlInfo *IpControlInfo::Create()
{
    return IpControlInfo::New_IpControlInfo();
}

void IpControlInfo::Release()
{
    IpControlInfo::Delete_IpControlInfo(this);
}

LibString IpControlInfo::ToString() const
{
    LibString info;

    info.AppendFormat("control flag[");

    for(auto flag : _controlFlow)
    {
        if(flag == ADD_WHITE)
            info.AppendFormat("ADD_WHITE, ");
        else if(flag == ADD_BLACK)
            info.AppendFormat("ADD_BLACK, ");
        else if(flag == ERASE_WHITE)
            info.AppendFormat("ERASE_WHITE, ");
        else if(flag == ERASE_BLACK)
            info.AppendFormat("ERASE_BLACK, ");
    }

    info.AppendFormat("]\n");
    info.AppendFormat("ip:%s\n", _ip.c_str());

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(IpRuleControlEvent);

IpRuleControlEvent::IpRuleControlEvent()
:PollerEvent(PollerEventType::IpRuleControl)
{

}

IpRuleControlEvent::~IpRuleControlEvent()
{
    ContainerUtil::DelContainer2(_ipControlList);
}

IpRuleControlEvent *IpRuleControlEvent::Create()
{
    return IpRuleControlEvent::New_IpRuleControlEvent();
}

void IpRuleControlEvent::Release()
{
    IpRuleControlEvent::Delete_IpRuleControlEvent(this);
}

LibString IpRuleControlEvent::ToString() const
{
    LibString info;
    info.AppendFormat("%s\n", PollerEvent::ToString().c_str());

    for(auto ctrlInfo : _ipControlList)
        info.AppendFormat("%s", ctrlInfo->ToString().c_str());

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(QuitServiceEvent);

QuitServiceEvent::QuitServiceEvent()
:PollerEvent(PollerEventType::QuitServiceEvent)
{

}

QuitServiceEvent::~QuitServiceEvent()
{

}

void QuitServiceEvent::Release()
{
    QuitServiceEvent::Delete_QuitServiceEvent(this);
}

LibString QuitServiceEvent::ToString() const 
{
    LibString info;
    info.AppendFormat("%s\n", PollerEvent::ToString().c_str());

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(QuitApplicationEvent);

QuitApplicationEvent::QuitApplicationEvent()
:PollerEvent(PollerEventType::QuitApplicationEvent)
{

}

QuitApplicationEvent::~QuitApplicationEvent()
{

}

void QuitApplicationEvent::Release()
{
    QuitApplicationEvent::Delete_QuitApplicationEvent(this);
}

LibString QuitApplicationEvent::ToString() const 
{
    LibString info;
    info.AppendFormat("%s\n", PollerEvent::ToString().c_str());

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(HotfixShareLibraryEvent);

HotfixShareLibraryEvent::HotfixShareLibraryEvent()
:PollerEvent(PollerEventType::HotfixShareLibrary)
{
    
}

HotfixShareLibraryEvent::~HotfixShareLibraryEvent()
{
    
}

void HotfixShareLibraryEvent::Release()
{
    HotfixShareLibraryEvent::Delete_HotfixShareLibraryEvent(this);
}

LibString HotfixShareLibraryEvent::ToString() const
{                               
    LibString info;
    info.AppendFormat("%s\nshare lib:%s, hotfix key:%s", PollerEvent::ToString().c_str(), _shareLib ? _shareLib->ToString().c_str() : "", _hotfixKey.c_str());

    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(HotfixShareLibraryCompleteEvent);

HotfixShareLibraryCompleteEvent::HotfixShareLibraryCompleteEvent()
:PollerEvent(PollerEventType::HotfixShareLibraryComplete)
{
    
}

HotfixShareLibraryCompleteEvent::~HotfixShareLibraryCompleteEvent()
{
    
}

void HotfixShareLibraryCompleteEvent::Release()
{
    HotfixShareLibraryCompleteEvent::Delete_HotfixShareLibraryCompleteEvent(this);
}

LibString HotfixShareLibraryCompleteEvent::ToString() const
{
    LibString info;
    info.AppendFormat("%s\nshotfix complete hotkeys:%s", PollerEvent::ToString().c_str(), StringUtil::ToString(_hotfixKeys, ",").c_str());

    return info;
}


KERNEL_END
