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
 * Date: 2022-04-03 19:15:51
 * Author: Eric Yonng
 * Description: 接收或者发送数据,要限制每帧处理的数据上限1MB(配置)避免影响其他session，导致饥饿
*/

#include <pch.h>
#include <kernel/comp/Log/log.h>

#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/NetEngine/Poller/Defs/TcpPollerConfig.h>
#include <kernel/comp/NetEngine/Defs/LibConnectInfo.h>
#include <kernel/comp/NetEngine/Defs/BuildSessionInfo.h>
#include <kernel/comp/NetEngine/LibPacket.h>
#include <kernel/comp/NetEngine/Poller/interface/IPollerMgr.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerFeature.h>
#include <kernel/comp/NetEngine/Defs/LibListenInfo.h>
#include <kernel/comp/Utils/SocketUtil.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgrFactory.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/IocpTcpPoller.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/EpollTcpPoller.h>

#include <kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgr.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(TcpPollerMgr);

TcpPollerMgr::TcpPollerMgr()
:_config(NULL)
,_maxPollerId(0)
,_serviceProxy(NULL)
{

}

TcpPollerMgr::~TcpPollerMgr()
{
    _Clear();
}

void TcpPollerMgr::Release()
{
    // delete this;
    TcpPollerMgr::DeleteByAdapter_TcpPollerMgr(TcpPollerMgrFactory::_buildType.V, this);
}

Int32 TcpPollerMgr::_OnCreated()
{
    auto ret = CompObject::_OnCreated();
    if(ret != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("comp object on created fail ret:%d"), ret);
        return ret;
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("tcp poller mgr created suc."));
    return Status::Success;
}

Int32 TcpPollerMgr::_OnInit()
{
    Int32 err = CompObject::_OnInit();
    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("comp init fail err:%d"), err);
        return err;
    }

    for(auto iter : _config->_pollerFeatureRefConfig)
    {
        TcpPollerFeatureConfig *featureConfig = iter.second;
        auto &pollerInstConfigs = featureConfig->_pollerInstConfigs;
        for(auto pollerInstConfig : pollerInstConfigs)
        {
            if(LIKELY(pollerInstConfig))
                _CreatePoller(pollerInstConfig);
        }
    }

    for(auto iter : _pollerFeatureRefPollerCluster)
    {
        auto pollerCluster = iter.second;
        for(auto poller : *pollerCluster)
        {
            auto errCode = poller->Init();
            if(errCode != Status::Success)
            {
                g_Log->NetError(LOGFMT_OBJ_TAG("poller init fail poller info:%s errCode:%d"), poller->ToString().c_str(), errCode);
                return errCode;
            }
        }
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("tcp poller mgr init suc."));
    return Status::Success;
}

Int32 TcpPollerMgr::_OnStart()
{
    Int32 err = CompObject::_OnStart();
    if(err != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("comp start fail err:%d"), err);
        return err;
    }

    for(auto iter : _pollerFeatureRefPollerCluster)
    {
        auto pollerCluster = iter.second;
        for(auto poller : *pollerCluster)
        {
            auto errCode = poller->Start();
            if(errCode != Status::Success)
            {
                g_Log->NetError(LOGFMT_OBJ_TAG("poller start fail poller info:%s errCode:%d"), poller->ToString().c_str(), errCode);
                return errCode;
            }
        }
    }

    // 等待所有polle rready
    for(;!IsAllReady();)
    {
        SystemUtil::ThreadSleep(1000);

        if(GetErrCode() != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("error happen errCode:%d"), GetErrCode());
            return GetErrCode();
        }
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("tcp poller mgr start suc."));
    return Status::Success;
}

void TcpPollerMgr::_OnWillClose()
{
    for(auto iter : _pollerFeatureRefPollerCluster)
    {
        auto pollerCluster = iter.second;
        for(auto poller : *pollerCluster)
            poller->WillClose();
    }

    CompObject::_OnWillClose();
}

void TcpPollerMgr::_OnClose()
{
    for(auto iter : _pollerFeatureRefPollerCluster)
    {
        auto pollerCluster = iter.second;
        for(auto poller : *pollerCluster)
            poller->Close();
    }

    // 等待所有结束
    for(;!IsAllDown();)
        SystemUtil::ThreadSleep(1000);

    _Clear();
    CompObject::_OnClose();
}

void TcpPollerMgr::Clear()
{
    _Clear();
    CompObject::Clear();
}

LibString TcpPollerMgr::ToString() const 
{
    LibString info;
    info.AppendFormat("tcp poller mgr comp obj info: %s\n", CompObject::ToString().c_str())
        .AppendFormat("_maxPollerId:%llu", _maxPollerId);
    
    // TODO:
    for(auto iter : _pollerFeatureRefPollerCluster)
    {
        auto pollerCluster = iter.second;
        for(auto poller : *pollerCluster)
            info.AppendFormat("poller:%s\n", poller->ToString().c_str());
    }

    return info;
}

TcpPollerMgr::TcpPoller *TcpPollerMgr::_CreatePoller(const TcpPollerInstConfig *cfg)
{
    // 创建poller
    auto pollerId = ++_maxPollerId;
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        auto newPoller = EpollTcpPoller::New_EpollTcpPoller(this, pollerId, cfg);
    #endif

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        auto newPoller = IocpTcpPoller::New_IocpTcpPoller(this, pollerId, cfg);
    #endif

    // 配置
    auto featureCfg = cfg->_owner;
    
    // pollerfeature映射
    auto iterFeature = _pollerFeatureRefPollerCluster.find(featureCfg->_pollerFeature);
    if(iterFeature == _pollerFeatureRefPollerCluster.end())
        iterFeature = _pollerFeatureRefPollerCluster.insert(std::make_pair(featureCfg->_pollerFeature, CRYSTAL_NEW(PollerClusterType))).first;

    // 集群
    auto pollerCluster = iterFeature->second;
    pollerCluster->push_back(newPoller);

    _idRefPoller.insert(std::make_pair(pollerId, newPoller));

    // 统计poller
    auto pollerMgr = GetOwner()->CastTo<IPollerMgr>();
    if(featureCfg->_pollerFeature == PollerFeature::LINKER)
    {
        pollerMgr->AddLinkerPollerCount(1);
    }
    else if(featureCfg->_pollerFeature == PollerFeature::DATA_TRANSFER)
    {
        pollerMgr->AddDataTransferPollerCount(1);
    }

    return newPoller;
}

TcpPollerMgr::PollerClusterType *TcpPollerMgr::_GetPollerCluster(Int32 pollerFeature)
{
    auto iter = _pollerFeatureRefPollerCluster.find(pollerFeature);
    return iter == _pollerFeatureRefPollerCluster.end() ? NULL : iter->second;
}

TcpPollerMgr::TcpPoller *TcpPollerMgr::_SelectLowerLoaderPoller(TcpPollerMgr::PollerClusterType *cluster)
{
    TcpPollerMgr::TcpPoller *finalPoller = (*cluster)[0];
    const Int32 clusterCount = static_cast<Int32>(cluster->size());
    UInt64 loaderScore = finalPoller->CalcLoadScore();
    for(Int32 idx = 0; idx < clusterCount; ++idx)
    {
        auto poller = (*cluster)[idx];
        UInt64 curLoaderScore = 0;
        if(poller && (curLoaderScore = poller->CalcLoadScore()) < loaderScore)
        {
            finalPoller = poller;
            loaderScore = curLoaderScore;
        }
    }

    return finalPoller;
}

bool TcpPollerMgr::IsAllReady() const
{
    for(auto iter : _idRefPoller)
    {
        if(!iter.second->IsReady())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("poller not ready:%s"), iter.second->ToString().c_str());
            return false;
        }
    }

    return true;
}

bool TcpPollerMgr::IsAllDown() const
{
    for(auto iter : _idRefPoller)
    {
        if(iter.second->IsReady())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("poller not down:%s"), iter.second->ToString().c_str());
            return false;
        }
    }

    return true;
}

void TcpPollerMgr::_Clear()
{
    ContainerUtil::DelContainer(_pollerFeatureRefPollerCluster, [](PollerClusterType *pollerCluster)
    {
        ContainerUtil::DelContainer<TcpPoller *, AutoDelMethods::Release>(*pollerCluster);
        CRYSTAL_DELETE_SAFE(pollerCluster);
    });

    _idRefPoller.clear();
}

void TcpPollerMgr::PostConnect(LibConnectInfo *connectInfo)
{
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        auto pollerCluster = _GetPollerCluster(PollerFeature::DATA_TRANSFER);
    #else
        auto pollerCluster = _GetPollerCluster(PollerFeature::LINKER);
    #endif
    if(UNLIKELY(!pollerCluster))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("have no linker poller cluster connect info:%s"), connectInfo->ToString().c_str());
        LibConnectInfo::Delete_LibConnectInfo(connectInfo);
        return;
    }

    auto poller = _SelectLowerLoaderPoller(pollerCluster);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("have no linker poller cluster connect info:%s"), connectInfo->ToString().c_str());
        LibConnectInfo::Delete_LibConnectInfo(connectInfo);
        return;
    }

    poller->PostConnect(connectInfo->_priorityLevel, connectInfo);
}

void TcpPollerMgr::PostAddlisten(Int32 level, LibListenInfo *listenInfo)
{
    auto pollerCluster = _GetPollerCluster(PollerFeature::LINKER);
    if(UNLIKELY(!pollerCluster))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("have no linker poller cluster listen info:%s"), listenInfo->ToString().c_str());
        LibListenInfo::Delete_LibListenInfo(listenInfo);
        return;
    }

    auto poller = _SelectLowerLoaderPoller(pollerCluster);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("have no linker poller cluster listen info:%s"), listenInfo->ToString().c_str());
        LibListenInfo::Delete_LibListenInfo(listenInfo);
        return;
    }

    poller->PostAddlisten(level, listenInfo);
}

void TcpPollerMgr::PostAddlistenList(Int32 level, std::vector<LibListenInfo *> &listenInfoList)
{
    auto pollerCluster = _GetPollerCluster(PollerFeature::LINKER);
    if(UNLIKELY(!pollerCluster))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("have no linker poller cluster listen info size:%llu"), static_cast<UInt64>(listenInfoList.size()));
        ContainerUtil::DelContainer(listenInfoList, [](LibListenInfo *&listenInfo){
            LibListenInfo::Delete_LibListenInfo(listenInfo);
            listenInfo = NULL;
        });
        return;
    }

    auto poller = _SelectLowerLoaderPoller(pollerCluster);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("have no linker poller cluster listen size:%llu"), static_cast<UInt64>(listenInfoList.size()));
        ContainerUtil::DelContainer(listenInfoList, [](LibListenInfo *&listenInfo){
            LibListenInfo::Delete_LibListenInfo(listenInfo);
            listenInfo = NULL;
        });
        return;
    }

    poller->PostAddlistenList(level, listenInfoList);
}

void TcpPollerMgr::PostSend(UInt64 pollerId, Int32 level, UInt64 sessionId, LibPacket *packet)
{
    auto poller = GetPoller(pollerId);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("poller not exists pollerId:%llu, sessionId:%llu, packet:%s"), pollerId, sessionId, packet ? packet->ToString().c_str():"no packet");
        LibPacket::Delete_LibPacket(packet);
        return;
    }

    poller->PostSend(level, sessionId, packet);
}

void TcpPollerMgr::PostSend(UInt64 pollerId, Int32 level, UInt64 sessionId, LibList<LibPacket *> *packets)
{
    auto poller = GetPoller(pollerId);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("poller not exists pollerId:%llu, sessionId:%llu, packets:\n%s")
                        , pollerId, sessionId, packets ? packets->ToString(
                            [](const LibPacket *packet){ return packet->ToString();}
                            ).c_str() : "no packet");

        ContainerUtil::DelContainer(*packets, [](LibPacket *&packet){
            LibPacket::Delete_LibPacket(packet);
            packet = NULL;
        });
        LibList<LibPacket *>::Delete_LibList(packets);
        return;
    }

    poller->PostSend(level, sessionId, packets);
}

void TcpPollerMgr::PostCloseSession(UInt64 pollerId, UInt64 fromeService, Int32 level, UInt64 sessionId, Int64 closeMillisecondTime, bool forbidRead, bool forbidWrite)
{
    auto poller = GetPoller(pollerId);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("poller not exists pollerId:%llu, sessionId:%llu")
                        , pollerId, sessionId);
        return;
    }

    poller->PostCloseSession(fromeService, level, sessionId, closeMillisecondTime, forbidRead, forbidWrite);
}

void TcpPollerMgr::PostIpControl(Int32 level, const std::list<IpControlInfo *> &controlList)
{
    for(auto iter : _idRefPoller)
        iter.second->PostIpControl(level, controlList);
}

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
void TcpPollerMgr::OnConnectRemoteSuc(BuildSessionInfo *newSessionInfo)
{

    auto pollerCluster = _GetPollerCluster(PollerFeature::DATA_TRANSFER);
    if(UNLIKELY(!pollerCluster))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("have no data transfer poller cluster newSessionInfo:%s"), newSessionInfo->ToString().c_str());
        GetOwner()->CastTo<IPollerMgr>()->ReduceSessionPending(1);
        SocketUtil::DestroySocket(newSessionInfo->_sock);
        BuildSessionInfo::Delete_BuildSessionInfo(newSessionInfo);
        return;
    }

    auto poller = _SelectLowerLoaderPoller(pollerCluster);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("_SelectLowerLoaderPoller fail have no data transfer poller cluster newSessionInfo:%s"), newSessionInfo->ToString().c_str());
        
        GetOwner()->CastTo<IPollerMgr>()->ReduceSessionPending(1);
        SocketUtil::DestroySocket(newSessionInfo->_sock);
        BuildSessionInfo::Delete_BuildSessionInfo(newSessionInfo);
        return;
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("connect remote suc new session info:%s"), newSessionInfo->ToString().c_str());
    
    poller->PostNewSession(newSessionInfo->_priorityLevel, newSessionInfo);
}
#endif

void TcpPollerMgr::OnAcceptedSuc(BuildSessionInfo *newSessionInfo)
{
    auto pollerCluster = _GetPollerCluster(PollerFeature::DATA_TRANSFER);
    if(UNLIKELY(!pollerCluster))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("have no data transfer poller cluster newSessionInfo:%s"), newSessionInfo->ToString().c_str());
        GetOwner()->CastTo<IPollerMgr>()->ReduceSessionPending(1);
        SocketUtil::DestroySocket(newSessionInfo->_sock);
        BuildSessionInfo::Delete_BuildSessionInfo(newSessionInfo);
        return;
    }

    auto poller = _SelectLowerLoaderPoller(pollerCluster);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("_SelectLowerLoaderPoller fail have no data transfer poller cluster newSessionInfo:%s"), newSessionInfo->ToString().c_str());
        GetOwner()->CastTo<IPollerMgr>()->ReduceSessionPending(1);
        SocketUtil::DestroySocket(newSessionInfo->_sock);
        BuildSessionInfo::Delete_BuildSessionInfo(newSessionInfo);
        return;
    }

    g_Log->NetInfo(LOGFMT_OBJ_TAG("accepted new link in suc new session info:%s"), newSessionInfo->ToString().c_str());
    poller->PostNewSession(newSessionInfo->_priorityLevel, newSessionInfo);
}

void TcpPollerMgr::QuitAllSessions(UInt64 serviceId)
{
    for(auto iter : _idRefPoller)
        iter.second->PostQuitServiceSessionsEvent(serviceId);
}


KERNEL_END