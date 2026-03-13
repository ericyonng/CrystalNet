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
#include <kernel/comp/LibList.h>

#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/NetEngine/Defs/LibConnectInfo.h>
#include <kernel/comp/NetEngine/Defs/BuildSessionInfo.h>
#include <kernel/comp/NetEngine/LibPacket.h>
#include <kernel/comp/NetEngine/Poller/interface/IPollerMgr.h>
#include <kernel/comp/NetEngine/Defs/LibListenInfo.h>
#include <kernel/comp/Utils/SocketUtil.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgrFactory.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/IocpTcpPoller.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/EpollTcpPoller.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerConfig.h>
#include <kernel/comp/NetEngine/Defs/NetCfgDefs.h>

#include <kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgr.h>

#include "kernel/comp/Config/KernelConfig.h"

KERNEL_BEGIN
    TcpPollerMgr::TcpPollerMgr()
:CompObject(KERNEL_NS::RttiUtil::GetTypeId<TcpPollerMgr>())
,_config(NULL)
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

    // 判断linker和Transferid是否一样, 一样表示共享poller,创建poller数量取最大值那个, 不一样表示不共享, 那么poller各自创建
    auto pollerMgr = GetOwner()->CastTo<IPollerMgr>();
    if (_config->Linker.Id == _config->DataTransfer.Id)
    {
        auto count = _config->Linker.Count > _config->DataTransfer.Count ? _config->Linker.Count : _config->DataTransfer.Count;
        std::vector<TcpPoller *> pollers;
        _CreatePoller(count, pollers);
        for (auto poller : pollers)
        {
            // 连接器
            if (static_cast<Int32>(_linker.size()) < _config->Linker.Count)
            {
                _linker.push_back(poller);

                // // 统计poller
                pollerMgr->AddLinkerPollerCount(1);
            }
            
            // 数据传输器
            if (static_cast<Int32>(_dataTransfer.size()) < _config->DataTransfer.Count)
            {
                pollerMgr->AddDataTransferPollerCount(1);
            }
        }
    }
    else
    {
        // 创建连接器
        _CreatePoller(_config->Linker.Count, _linker);
        pollerMgr->AddLinkerPollerCount(static_cast<UInt64>(_config->Linker.Count));

        // 创建数据传输器
        _CreatePoller(_config->DataTransfer.Count, _dataTransfer);
        pollerMgr->AddDataTransferPollerCount(static_cast<UInt64>(_config->DataTransfer.Count));
    }

    for (auto iter : _idRefPoller)
    {
        auto errCode = iter.second->Init();
        if(errCode != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("poller init fail poller info:%s errCode:%d"), iter.second->ToString().c_str(), errCode);
            return errCode;
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

    for (auto iter : _idRefPoller)
    {
        auto poller = iter.second;
        auto errCode = poller->Start();
        if(errCode != Status::Success)
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("poller start fail poller info:%s errCode:%d"), poller->ToString().c_str(), errCode);
            return errCode;
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

    MaskReady(true);

    g_Log->NetInfo(LOGFMT_OBJ_TAG("tcp poller mgr start suc."));
    return Status::Success;
}

void TcpPollerMgr::_OnWillClose()
{
    for (auto iter : _idRefPoller)
    {
        auto poller = iter.second;
        poller->WillClose();
    }

    CompObject::_OnWillClose();
}

void TcpPollerMgr::_OnClose()
{
    for (auto iter : _idRefPoller)
    {
        auto poller = iter.second;
        poller->Close();
    }

    // 等待所有结束
    for(;!IsAllDown();)
    {
        SystemUtil::ThreadSleep(1000);
        CLOG_NET_WARN("waiting tcp thread down...");
    }

    MaskReady(false);

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
    for(auto iter : _idRefPoller)
    {
        auto poller = iter.second;
        info.AppendFormat("poller:%s\n", poller->ToString().c_str());
    }

    return info;
}

void TcpPollerMgr::_CreatePoller(Int32 count, std::vector<TcpPoller *> &pollers)
{
    pollers.resize(count);
    for (Int32 i = 0; i < count; i++)
    {
        // 创建poller
        auto pollerId = ++_maxPollerId;
#if CRYSTAL_TARGET_PLATFORM_LINUX
        auto newPoller = EpollTcpPoller::New_EpollTcpPoller(this, pollerId, _config);
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
        auto newPoller = IocpTcpPoller::New_IocpTcpPoller(this, pollerId, _config);
#endif

        pollers[i] = newPoller;
        _idRefPoller.insert(std::make_pair(pollerId, newPoller));

        // // 统计poller
        // auto pollerConfig = pollerMgr->GetConfig();
        // auto iterFeatureString = pollerConfig->_pollerFeatureIdRefString.find(featureCfg->_pollerFeature);
        // auto &featureStrings = iterFeatureString->second;
        // for(auto &featureStr : featureStrings)
        // {
        //     if(featureStr == g_LinkerPollerName)
        //     {
        //         pollerMgr->AddLinkerPollerCount(1);
        //         _linkerFeatureId = featureCfg->_pollerFeature;
        //     }
        //     else if(featureStr == g_TransferPollerName)
        //     {
        //         pollerMgr->AddDataTransferPollerCount(1);
        //         _trasferFeatureId = featureCfg->_pollerFeature;
        //     }
        // }

        auto pollerMgr = GetOwner()->CastTo<IPollerMgr>();
        pollerMgr->AddPollerCount(1);
    }
}

TcpPollerMgr::TcpPoller *TcpPollerMgr::_SelectLowerLoaderPoller(TcpPollerMgr::PollerClusterType *cluster, const std::set<TcpPollerMgr::TcpPoller *> &excludes)
{
    TcpPollerMgr::TcpPoller *finalPoller = (*cluster)[0];
    const Int32 clusterCount = static_cast<Int32>(cluster->size());
    UInt64 loaderScore = finalPoller->CalcLoadScore();
    for(Int32 idx = 0; idx < clusterCount; ++idx)
    {
        auto poller = (*cluster)[idx];
        if(!poller)
            continue;

        if(excludes.find(poller) != excludes.end())
            continue;

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
    ContainerUtil::DelContainer2(_idRefPoller);
}

void TcpPollerMgr::PostConnect(LibConnectInfo *connectInfo)
{
    std::set<TcpPollerMgr::TcpPoller *> excludes;
    auto poller = _SelectLowerLoaderPoller(&_linker, excludes);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("have no linker poller cluster connect info:%s"), connectInfo->ToString().c_str());
        LibConnectInfo::Delete_LibConnectInfo(connectInfo);
        return;
    }

    poller->PostConnect(connectInfo);
}

void TcpPollerMgr::PostAddlisten(LibListenInfo *listenInfo)
{
    // windows 下不能绑定同一个端口
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if(listenInfo->_port != 0)
    {
        listenInfo->_sessionCount = 1;
        CLOG_NET_WARN("windows cant listen port multi session, listenInfo:%s", listenInfo->ToString().c_str());
    }
    #endif

    std::set<TcpPollerMgr::TcpPoller *> excludes;
    for(Int32 idx = 0; idx < listenInfo->_sessionCount; ++idx)
    {
        auto poller = _SelectLowerLoaderPoller(&_linker, excludes);
        if(UNLIKELY(!poller))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("have no linker poller cluster listen info:%s"), listenInfo->ToString().c_str());
            break;
        }

        // 多个session负载均衡需要做拷贝
        auto newListenInfo = LibListenInfo::New_LibListenInfo();
        *newListenInfo = *listenInfo;
        excludes.insert(poller);
        poller->PostAddlisten(newListenInfo);
    }

    LibListenInfo::Delete_LibListenInfo(listenInfo);
}

void TcpPollerMgr::PostAddlistenList(std::vector<LibListenInfo *> &listenInfoList)
{
    std::set<TcpPollerMgr::TcpPoller *> excludes;
    for(auto listenInfo : listenInfoList)
    {
        for(Int32 idx = 0; idx < listenInfo->_sessionCount; ++idx)
        {
            auto poller = _SelectLowerLoaderPoller(&_linker, excludes);
            if(UNLIKELY(!poller))
            {
                g_Log->NetError(LOGFMT_OBJ_TAG("have no linker poller cluster listen size:%llu"), static_cast<UInt64>(listenInfoList.size()));
                ContainerUtil::DelContainer(listenInfoList, [](LibListenInfo *&listenInfo){
                    LibListenInfo::Delete_LibListenInfo(listenInfo);
                    listenInfo = NULL;
                });
                break;
            }

            excludes.insert(poller);
            poller->PostAddlisten(listenInfo);
        }
    }
}

void TcpPollerMgr::PostSend(UInt64 pollerId, UInt64 sessionId, LibPacket *packet)
{
    auto poller = GetPoller(pollerId);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("poller not exists pollerId:%llu, sessionId:%llu, packet:%s"), pollerId, sessionId, packet ? packet->ToString().c_str():"no packet");
        LibPacket::Delete_LibPacket(packet);
        return;
    }

    poller->PostSend(sessionId, packet);
}

void TcpPollerMgr::PostSend(UInt64 pollerId, UInt64 sessionId, LibList<LibPacket *> *packets)
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

    poller->PostSend(sessionId, packets);
}

void TcpPollerMgr::PostCloseSession(UInt64 pollerId, UInt64 fromeService, UInt64 sessionId, Int64 closeMillisecondTimeDelay, bool forbidRead, bool forbidWrite)
{
    auto poller = GetPoller(pollerId);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("poller not exists pollerId:%llu, sessionId:%llu")
                        , pollerId, sessionId);
        return;
    }

    poller->PostCloseSession(fromeService, sessionId, closeMillisecondTimeDelay, forbidRead, forbidWrite);
}

void TcpPollerMgr::PostIpControl(const std::list<IpControlInfo *> &controlList)
{
    for(auto iter : _idRefPoller)
        iter.second->PostIpControl(controlList);
}

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
void TcpPollerMgr::OnConnectRemoteSuc(BuildSessionInfo *newSessionInfo)
{
    std::set<TcpPollerMgr::TcpPoller *> excludes;
    auto poller = _SelectLowerLoaderPoller(_dataTransfer, excludes);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("_SelectLowerLoaderPoller fail have no data transfer poller cluster newSessionInfo:%s"), newSessionInfo->ToString().c_str());
        
        GetOwner()->CastTo<IPollerMgr>()->ReduceSessionPending(1);
        SocketUtil::DestroySocket(newSessionInfo->_sock);
        BuildSessionInfo::Delete_BuildSessionInfo(newSessionInfo);
        return;
    }

    if(g_Log->IsEnable(LogLevel::NetInfo))
        g_Log->NetInfo(LOGFMT_OBJ_TAG("connect remote suc new session info:%s"), newSessionInfo->ToString().c_str());
    
    poller->PostNewSession(newSessionInfo);
}
#endif

void TcpPollerMgr::OnAcceptedSuc(BuildSessionInfo *newSessionInfo)
{
    std::set<TcpPollerMgr::TcpPoller *> excludes;
    auto poller = _SelectLowerLoaderPoller(&_dataTransfer, excludes);
    if(UNLIKELY(!poller))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("_SelectLowerLoaderPoller fail have no data transfer poller cluster newSessionInfo:%s"), newSessionInfo->ToString().c_str());
        GetOwner()->CastTo<IPollerMgr>()->ReduceSessionPending(1);
        SocketUtil::DestroySocket(newSessionInfo->_sock);
        BuildSessionInfo::Delete_BuildSessionInfo(newSessionInfo);
        return;
    }

    if(g_Log->IsEnable(LogLevel::NetInfo))
        g_Log->NetInfo(LOGFMT_OBJ_TAG("accepted new link in suc new session info:%s"), newSessionInfo->ToString().c_str());
    poller->PostNewSession(newSessionInfo);
}

void TcpPollerMgr::QuitAllSessions(UInt64 serviceId)
{
    for(auto iter : _idRefPoller)
        iter.second->PostQuitServiceSessionsEvent(serviceId);
}


KERNEL_END