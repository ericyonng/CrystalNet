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
#include <kernel/comp/NetEngine/Poller/impl/PollerMgrFactory.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgrFactory.h>
#include <kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgrFactory.h>
#include <kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgr.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgr.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/EpollTcpPoller.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/IocpTcpPoller.h>
#include <kernel/comp/Utils/SocketUtil.h>
#include <kernel/comp/Log/log.h>

#include <kernel/comp/NetEngine/Poller/impl/PollerMgr.h>

KERNEL_BEGIN

PollerMgr::PollerMgr()
:_config(NULL)
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
    RegisterComp<IpRuleMgrFactory>();
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
    return ++_maxSessionId;
}

void PollerMgr::AddSessionPending(UInt64 num)
{
    _sessionQuantityPending += num;
}

bool PollerMgr::CheckAddSessionPending(UInt64 num, UInt64 &totalSessionNum) 
{
    _sessionQuantityPending += num;
    if(UNLIKELY(_config->_maxSessionQuantity == 0))
    {
        totalSessionNum = _sessionQuantityPending + _sessionQuantity;
        return true;
    }

    if(UNLIKELY(_sessionQuantityPending + _sessionQuantity > _config->_maxSessionQuantity))
    {
        _sessionQuantityPending -= num;
        totalSessionNum = _sessionQuantityPending + _sessionQuantity;
        return false;
    }

    totalSessionNum = _sessionQuantityPending + _sessionQuantity;
    return true;
}

void PollerMgr::ReduceSessionPending(UInt64 num)
{
    _sessionQuantityPending -= num;
}

void PollerMgr::AddSessionQuantity(UInt64 num)
{
    _sessionQuantity += num;
}

void PollerMgr::ReduceSessionQuantity(UInt64 num)
{
    _sessionQuantity -= num;
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

void PollerMgr::OnMonitor(LibString &info)
{
    info.AppendFormat("[- POLLER MGR BEGIN -]\n");

    UInt64 recvPacketPerFrame = _recvPacketCount.load();
    UInt64 recvBytesPerFrame = _recvBytes.load();
    UInt64 sendPacketPerFrame = _sendPacketCount.load();
    UInt64 sendBytesPerFrame = _sendBytes.load();
    UInt64 sessionCountPerFrame = _sessionCount.load();
    UInt64 acceptedSessionCountPerFrame = _acceptedSessionCount.load();
    UInt64 connectedSessionCountPerFrame = _connectedSessionCount.load();

    // TODO:监控信息输出
    info.AppendFormat("session-[online amount:%llu, speed:%llu, history amount:%llu]\n"
                    , _onlineSessionCount.load(), sessionCountPerFrame, _historySessionCount.load());
    info.AppendFormat("accepted session-[online amount:%llu, speed:%llu, history amount:%llu]\n"
                    , _onlineAcceptedSessionCount.load(), acceptedSessionCountPerFrame, _historyAcceptedSessionCount.load());
    info.AppendFormat("connect to remote session-[online amount:%llu, speed:%llu, history amount:%llu]\n"
                    , _onlineConnectedSessionCount.load(), connectedSessionCountPerFrame, _historyConnectedSessionCount.load());
    info.AppendFormat("listener session-[online amount:%llu]\n"
                    , _onlineListenerSessionCount.load());


    LibString pollerInfo;
    {
        auto tcpPollerMgr = GetComp<TcpPollerMgr>();
        auto &allPollers = tcpPollerMgr->GetAllPollers();
        for(auto &iter : allPollers)
        {
            // TODO:loaded
            auto poller = iter.second;
            pollerInfo.AppendFormat("pollerId:%llu, sessions:%llu, loaded:\n"
                    , poller->GetPollerId(), poller->GetSessionAmount());
        }
    }

    info.AppendFormat("poller-[total:%llu, linker:%llu, data transfer:%llu]\n[\n%s\n]"
    , _pollerCounts.load(),  _linkerCount.load(), _dataTransferCount.load(), pollerInfo.c_str());

    info.AppendFormat("recv-[packet qps:%llu, speed:%s, history packet:%llu, history bytes:%llu]\n"
                    , recvPacketPerFrame, SocketUtil::ToFmtSpeedPerSec(static_cast<Int64>(recvBytesPerFrame)).c_str()
                    , _historyRecvPacketCount.load(), _historyRecvBytes.load());
    info.AppendFormat("send-[packet qps:%llu, speed:%s, history packet:%llu, history bytes:%llu]\n"
                    , sendPacketPerFrame, SocketUtil::ToFmtSpeedPerSec(static_cast<Int64>(sendBytesPerFrame)).c_str()
                    , _historySendPacketCount.load(), _historySendBytes.load());

    info.AppendFormat("[- POLLER MGR END -]\n");

    // 帧清零
    _recvPacketCount = 0;
    _recvBytes = 0;
    _sendPacketCount = 0;
    _sendBytes = 0;
    _sessionCount = 0;
    _acceptedSessionCount = 0;
    _connectedSessionCount = 0;
    
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
    _recvPacketCount += num;
    _historyRecvPacketCount += num;
}

void PollerMgr::AddRecvBytes(UInt64 num)
{
    _recvBytes += num;
    _historyRecvBytes += num;
}

void PollerMgr::AddSendPacketCount(UInt64 num)
{
    _sendPacketCount += num;
    _historySendPacketCount += num;
}

void PollerMgr::AddSendBytes(UInt64 num)
{
    _sendBytes += num;
    _historySendBytes += num;
}

void PollerMgr::AddAcceptedSessionCount(UInt64 num)
{
    _sessionCount += num;
    _onlineSessionCount += num;
    _historySessionCount += num;

    _acceptedSessionCount += num;
    _onlineAcceptedSessionCount += num;
    _historyAcceptedSessionCount += num;
}

void PollerMgr::AddConnectedSessionCount(UInt64 num)
{
    _sessionCount += num;
    _onlineSessionCount += num;
    _historySessionCount += num;

    _connectedSessionCount += num;
    _onlineConnectedSessionCount += num;
    _historyConnectedSessionCount += num;
}

void PollerMgr::AddListenerSessionCount(UInt64 num)
{
    _sessionCount += num;
    _onlineSessionCount += num;
    _historySessionCount += num;

    _onlineListenerSessionCount += num;
}

void PollerMgr::ReduceAcceptedSessionCount(UInt64 num)
{
    _sessionCount -= num;
    _onlineSessionCount -= num;

    _acceptedSessionCount -= num;
    _onlineAcceptedSessionCount -= num;
}

void PollerMgr::ReduceConnectedSessionCount(UInt64 num)
{
    _sessionCount -= num;
    _onlineSessionCount -= num;

    _connectedSessionCount -= num;
    _onlineConnectedSessionCount -= num;
}

void PollerMgr::ReduceListenerSessionCount(UInt64 num)
{
    _sessionCount -= num;
    _onlineSessionCount -= num;

    _onlineListenerSessionCount -= num;
}

void PollerMgr::AddLinkerPollerCount(UInt64 num)
{
    _pollerCounts += num;

    _linkerCount += num;
}

void PollerMgr::AddDataTransferPollerCount(UInt64 num)
{
    _pollerCounts += num;
    
    _dataTransferCount += num;
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
    auto ipRuleMgr = GetComp<IpRuleMgr>();
    if(!ipRuleMgr->SetBlackWhiteListFlag(_config->_blackWhiteListFlag))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("SetBlackWhiteListFlag fail black white list flag:%u"), _config->_blackWhiteListFlag);
        if(owner)
            owner->SetErrCode(this, Status::Failed);
        return Status::Failed;
    }

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
