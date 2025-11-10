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
 * Date: 2022-03-25 09:34:19
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerConfig.h>
#include <kernel/comp/NetEngine/Poller/impl/PollerMgrFactory.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgrFactory.h>
#include <kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgrFactory.h>
#include <kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgr.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgr.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/EpollTcpPoller.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/IocpTcpPoller.h>
#include <kernel/comp/Utils/SocketUtil.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Poller/PollerInc.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerStatisticsInfo.h>

#include <kernel/comp/NetEngine/Poller/impl/PollerMgr.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IPollerMgr);

POOL_CREATE_OBJ_DEFAULT_IMPL(PollerMgr);

PollerMgr::PollerMgr()
:IPollerMgr(KERNEL_NS::RttiUtil::GetTypeId<PollerMgr>())
,_config(NULL)
,_maxSessionId{0}
,_serviceProxy(NULL)
,_sessionQuantity{0}
,_sessionQuantityPending{0}
,_recvPacketCount{0}
,_recvBytes{0}
,_historyRecvPacketCount{0}
,_historyRecvBytes{0}
,_sendPacketCount{0}
,_sendBytes{0}
,_historySendPacketCount{0}
,_historySendBytes{0}
,_sessionCount{0}
,_onlineSessionCount{0}
,_historySessionCount{0}
,_acceptedSessionCount{0}
,_onlineAcceptedSessionCount{0}
,_historyAcceptedSessionCount{0}
,_connectedSessionCount{0}
,_onlineConnectedSessionCount{0}
,_historyConnectedSessionCount{0}
,_onlineListenerSessionCount{0}
,_pollerCounts{0}
,_linkerCount{0}
,_dataTransferCount{0}
{

}

PollerMgr::~PollerMgr()
{
    _Clear();
}

void PollerMgr::Clear()
{
    _Clear();
    IPollerMgr::Clear();
}

void PollerMgr::Release()
{
    PollerMgr::DeleteByAdapter_PollerMgr(PollerMgrFactory::_buildType.V, this);
}

void PollerMgr::OnRegisterComps()
{
    // ip控制
    // RegisterComp<IpRuleMgrFactory>();
    // 支持tcp
    RegisterComp<TcpPollerMgrFactory>();
}

LibString PollerMgr::ToString() const
{
    // TODO:
    LibString info;
    info.AppendFormat("poller mgr comp object info:%s", CompHostObject::ToString().c_str());
    
    info.AppendFormat("poller configs:%s\n", _config->ToString().c_str());

    auto &allComps = GetAllComps();
    for(auto comp : allComps)
        info.AppendFormat("comp info:%s\n", comp->ToString().c_str());

    return info;
}

const PollerConfig *PollerMgr::GetConfig() const
{
    return _config;
}

void PollerMgr::SetConfig(const PollerConfig &cfg)
{
    _config = PollerConfig::New_PollerConfig();
    _config->Copy(cfg);
}

UInt64 PollerMgr::NewSessionId()
{
    return _maxSessionId.fetch_add(1, std::memory_order_release) + 1;
}

void PollerMgr::AddSessionPending(UInt64 num)
{
    _sessionQuantityPending.fetch_add(num, std::memory_order_release);
}

bool PollerMgr::CheckAddSessionPending(UInt64 num, UInt64 &totalSessionNum) 
{
    _sessionQuantityPending.fetch_add(num, std::memory_order_release);
    if(UNLIKELY(_config->_maxSessionQuantity == 0))
    {
        totalSessionNum = _sessionQuantityPending.load(std::memory_order_acquire) + _sessionQuantity.load(std::memory_order_acquire);
        return true;
    }

    if(UNLIKELY(_sessionQuantityPending.load(std::memory_order_acquire) + _sessionQuantity.load(std::memory_order_acquire) > _config->_maxSessionQuantity))
    {
        _sessionQuantityPending.fetch_sub(num, std::memory_order_release);
        totalSessionNum = _sessionQuantityPending.load(std::memory_order_acquire) + _sessionQuantity.load(std::memory_order_acquire);
        return false;
    }

    totalSessionNum = _sessionQuantityPending.load(std::memory_order_acquire) + _sessionQuantity.load(std::memory_order_acquire);
    return true;
}

void PollerMgr::ReduceSessionPending(UInt64 num)
{
    _sessionQuantityPending.fetch_sub(num, std::memory_order_release);
}

void PollerMgr::AddSessionQuantity(UInt64 num)
{
    _sessionQuantity.fetch_add(num, std::memory_order_release);
}

void PollerMgr::ReduceSessionQuantity(UInt64 num)
{
    _sessionQuantity.fetch_sub(num, std::memory_order_release);
}

UInt64 PollerMgr::GetSessionQuantityLimit() const
{
    return _config->_maxSessionQuantity;
}

void PollerMgr::SetServiceProxy(IServiceProxy *serviceProxy)
{
    _serviceProxy = serviceProxy;
}

IServiceProxy *PollerMgr::GetServiceProxy()
{
    return _serviceProxy;
}

const IServiceProxy *PollerMgr::GetServiceProxy() const
{
    return _serviceProxy;
}

void PollerMgr::OnMonitor(PollerMgrStatisticsInfo &statistics)
{
    statistics._onlineSessionTotalCount += _onlineSessionCount.load(std::memory_order_acquire);
    statistics._sessionSpeedTotalCount += _sessionCount.load(std::memory_order_acquire);
    statistics._historySessionTotalCount += _historySessionCount.load(std::memory_order_acquire);
    statistics._acceptedOnlineSessionTotalCount += _onlineAcceptedSessionCount.load(std::memory_order_acquire);
    statistics._acceptedHistoryTotalCount += _historyAcceptedSessionCount.load(std::memory_order_acquire);
    statistics._acceptedSpeedTotalCount += _acceptedSessionCount.load(std::memory_order_acquire);

    statistics._connectOnlineSessionTotalCount += _onlineConnectedSessionCount.load(std::memory_order_acquire);
    statistics._connectSpeedTotalCount += _connectedSessionCount.load(std::memory_order_acquire);
    statistics._connectHistoryTotalCount += _historyConnectedSessionCount.load(std::memory_order_acquire);
    statistics._listenerSessionTotalCount += _onlineListenerSessionCount.load(std::memory_order_acquire);

    {
        auto tcpPollerMgr = GetComp<TcpPollerMgr>();
        auto &allPollers = tcpPollerMgr->GetAllPollers();
        for(auto &iter : allPollers)
        {
            // TODO:loaded
            auto tcpPoller = iter.second;
            if(tcpPoller->IsStarted())
            {
                NetPollerCompStatistics netPoller;
                auto poller = tcpPoller->GetComp<Poller>();
                if(poller && poller->IsEnable())
                    poller->OnMonitor(netPoller._pollerStatistics);

                netPoller._pollerId = tcpPoller->GetPollerId();
                netPoller._sessionCount = tcpPoller->GetSessionAmount();
                statistics._netPollerStatistics.push_back(netPoller);
            }
        }
    }

    statistics._totalPollerTotalCount += _pollerCounts.load(std::memory_order_acquire);
    statistics._dataTransferPollerCount += _dataTransferCount.load(std::memory_order_acquire);
    statistics._linkerPollerTotalCount += _linkerCount.load(std::memory_order_acquire);

    statistics._totalHistoryRecvBytes += _historyRecvBytes.load(std::memory_order_acquire);
    statistics._totalHistoryRecvPacket += _historyRecvPacketCount.load(std::memory_order_acquire);
    statistics._totalHistorySendPacket += _historySendPacketCount.load(std::memory_order_acquire);
    statistics._totalHistorySendBytes += _historySendBytes.load(std::memory_order_acquire);
    statistics._packetRecvQps += _recvPacketCount.load(std::memory_order_acquire);
    statistics._packetRecvBytesSpeed += _recvBytes.load(std::memory_order_acquire);
    statistics._packetSendQps += _sendPacketCount.load(std::memory_order_acquire);
    statistics._packetSendBytesSpeed += _sendBytes.load(std::memory_order_acquire);

    // 帧清零
    _recvPacketCount.store(0, std::memory_order_release);
    _recvBytes.store(0, std::memory_order_release);
    _sendPacketCount.store(0, std::memory_order_release);
    _sendBytes.store(0, std::memory_order_release);
    _sessionCount.store(0, std::memory_order_release);
    _acceptedSessionCount.store(0, std::memory_order_release);
    _connectedSessionCount.store(0, std::memory_order_release);
    
    // _recvPacketCount -= recvPacketPerFrame;
    // _recvBytes -= recvBytesPerFrame;
    // _sendPacketCount -= sendPacketPerFrame;
    // _sendBytes -= sendBytesPerFrame;
    // _sessionCount -= sessionCountPerFrame;
    // _acceptedSessionCount -= acceptedSessionCountPerFrame;
    // _connectedSessionCount -= connectedSessionCountPerFrame;
}

void PollerMgr::AddRecvPacketCount(UInt64 num)
{
    _recvPacketCount.fetch_add(num, std::memory_order_release);
    _historyRecvPacketCount.fetch_add(num, std::memory_order_release);
}

void PollerMgr::AddRecvBytes(UInt64 num)
{
    _recvBytes.fetch_add(num, std::memory_order_release);
    _historyRecvBytes.fetch_add(num, std::memory_order_release);
}

void PollerMgr::AddSendPacketCount(UInt64 num)
{
    _sendPacketCount.fetch_add(num, std::memory_order_release);
    _historySendPacketCount.fetch_add(num, std::memory_order_release);
}

void PollerMgr::AddSendBytes(UInt64 num)
{
    _sendBytes.fetch_add(num, std::memory_order_release);
    _historySendBytes.fetch_add(num, std::memory_order_release);
}

void PollerMgr::AddAcceptedSessionCount(UInt64 num)
{
    _sessionCount.fetch_add(num, std::memory_order_release);
    _onlineSessionCount.fetch_add(num, std::memory_order_release);
    _historySessionCount.fetch_add(num, std::memory_order_release);

    _acceptedSessionCount.fetch_add(num, std::memory_order_release);
    _onlineAcceptedSessionCount.fetch_add(num, std::memory_order_release);
    _historyAcceptedSessionCount.fetch_add(num, std::memory_order_release);
}

void PollerMgr::AddConnectedSessionCount(UInt64 num)
{
    _sessionCount.fetch_add(num, std::memory_order_release);
    _onlineSessionCount.fetch_add(num, std::memory_order_release);
    _historySessionCount.fetch_add(num, std::memory_order_release);

    _connectedSessionCount.fetch_add(num, std::memory_order_release);
    _onlineConnectedSessionCount.fetch_add(num, std::memory_order_release);
    _historyConnectedSessionCount.fetch_add(num, std::memory_order_release);
}

void PollerMgr::AddListenerSessionCount(UInt64 num)
{
    _sessionCount.fetch_add(num, std::memory_order_release);
    _onlineSessionCount.fetch_add(num, std::memory_order_release);
    _historySessionCount.fetch_add(num, std::memory_order_release);

    _onlineListenerSessionCount.fetch_add(num, std::memory_order_release);
}

void PollerMgr::ReduceAcceptedSessionCount(UInt64 num)
{
    _sessionCount.fetch_sub(num, std::memory_order_release);
    _onlineSessionCount.fetch_sub(num, std::memory_order_release);

    _acceptedSessionCount.fetch_sub(num, std::memory_order_release);
    _onlineAcceptedSessionCount.fetch_sub(num, std::memory_order_release);
}

void PollerMgr::ReduceConnectedSessionCount(UInt64 num)
{
    _sessionCount.fetch_sub(num, std::memory_order_release);
    _onlineSessionCount.fetch_sub(num, std::memory_order_release);

    _connectedSessionCount.fetch_sub(num, std::memory_order_release);
    _onlineConnectedSessionCount.fetch_sub(num, std::memory_order_release);
}

void PollerMgr::ReduceListenerSessionCount(UInt64 num)
{
    _sessionCount.fetch_sub(num, std::memory_order_release);
    _onlineSessionCount.fetch_sub(num, std::memory_order_release);

    _onlineListenerSessionCount.fetch_sub(num, std::memory_order_release);
}

void PollerMgr::AddLinkerPollerCount(UInt64 num)
{
    _linkerCount.fetch_add(num, std::memory_order_release);
}

void PollerMgr::AddDataTransferPollerCount(UInt64 num)
{
    _dataTransferCount.fetch_add(num, std::memory_order_release);
}

void PollerMgr::AddPollerCount(UInt64 num)
{
    _pollerCounts.fetch_add(num, std::memory_order_release);
}

void PollerMgr::QuitAllSessions(UInt64 serviceId)
{
    // 通知所有poller 关闭session
    auto tcpPollerMgr = GetComp<TcpPollerMgr>();
    tcpPollerMgr->QuitAllSessions(serviceId);

    // 等待所有session退出
    while(true)
    {
        if(_onlineSessionCount.load(std::memory_order_acquire) == 0)
            break;

        SystemUtil::ThreadSleep(1);
    }
}

Int32 PollerMgr::_OnHostInit()
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("poller mgr init."));
    return Status::Success;
}

Int32 PollerMgr::_OnCompsCreated()
{
    auto owner = GetOwner();
    auto ret = CompHostObject::_OnCompsCreated();
    if(ret != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("_OnCompsCreated fail ret:%d"), ret);
        if(owner)
            owner->SetErrCode(this, ret);
        return ret;
    }

    // ip rule mgr 设置
    // auto ipRuleMgr = GetComp<IpRuleMgr>();
    // if(!ipRuleMgr->SetBlackWhiteListFlag(_config->_blackWhiteListFlag))
    // {
    //     g_Log->NetError(LOGFMT_OBJ_TAG("SetBlackWhiteListFlag fail black white list flag:%u"), _config->_blackWhiteListFlag);
    //     if(owner)
    //         owner->SetErrCode(this, Status::Failed);
    //     return Status::Failed;
    // }

    // tcp poller mgr 设置    
    auto tcpPollerMgr = GetComp<TcpPollerMgr>();
    tcpPollerMgr->SetConfig(&_config->_tcpPollerConfig);
    tcpPollerMgr->SetServiceProxy(_serviceProxy);

    g_Log->NetInfo(LOGFMT_OBJ_TAG("_OnCompsCreated suc."));
    return Status::Success;
}

Int32 PollerMgr::_OnHostWillStart()
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("poller mgr _OnWillStart."));

    return Status::Success;
}

Int32 PollerMgr::_OnHostStart()
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("poller mgr _OnStart."));

    // 等待所有组件成功
    CompObject *notReady = NULL;
    for(;!IsAllCompsReady(notReady);)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("poller mgr not ready comp:%s"), notReady->ToString().c_str());
        if(GetErrCode() != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("error happen errCode:%d"), GetErrCode());
            break;
        }
    }

    return GetErrCode();
}

void PollerMgr::_OnHostWillClose()
{
    g_Log->NetInfo(LOGFMT_OBJ_TAG("poller mgr _OnWillClose."));
}

void PollerMgr::_OnHostClose()
{
    CompObject *notDown = NULL;
    for(;!IsAllCompsDown(notDown);)
        g_Log->Warn(LOGFMT_OBJ_TAG("poller mgr not down comp:%s"), notDown->ToString().c_str());

    _Clear();
    g_Log->NetInfo(LOGFMT_OBJ_TAG("poller mgr _OnWillClose."));
}

void PollerMgr::_Clear()
{
    if(LIKELY(_config))
    {
        PollerConfig::Delete_PollerConfig(_config);
        _config = NULL;
    }
}


KERNEL_END
