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
 * Date: 2026-07-06 16:59:12
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <OptionComp/GlobalId/Impl/GlobalIdMgr.h>
#include <OptionComp/GlobalId/Impl/GlobalIdMgrFactory.h>
#include <OptionComp/storage/MongoDB/Interface/IMongoDbMgr.h>
#include <kernel/comp/Log/log.h>
#include <OptionComp/GlobalId/Impl/GlobalIdMgrMongoFactory.h>
#include <OptionComp/GlobalId/Impl/GlobalIdMgrMongo.h>

#include "bsoncxx/builder/basic/document.hpp"
#include "OptionComp/storage/MongoDB/Interface/IMongodbProxy.h"

KERNEL_BEGIN

GlobalIdMgr::GlobalIdMgr()
 :IGlobalIdMgr(KERNEL_NS::RttiUtil::GetTypeId<GlobalIdMgr>())
,_lastId{0}
,_mongoProxy(NULL)
,_ownerId(KERNEL_NS::GuidUtil::GenStr())
,_baseTime(LibTime::FromTimeMoment(2026, 1, 1, 0, 0, 0))
,_lockPrefix(KERNEL_NS::LibString(GlobalIdMgrMongo::DbName) + "_" + "GlobalIdMgr")
{
 
}

GlobalIdMgr::~GlobalIdMgr()
{
 
}

void GlobalIdMgr::Release()
{
    GlobalIdMgr::DeleteByAdapter_GlobalIdMgr(GlobalIdMgrFactory::_buildType.V, this);
}

void GlobalIdMgr::OnRegisterComps()
{
    RegisterComp<GlobalIdMgrMongoFactory>();
}

Int64 GlobalIdMgr::NewId()
{
    return 0;
}

void GlobalIdMgr::SetMongoProxy(IMongodbProxy *mongoProxy)
{
    _mongoProxy = mongoProxy;
}


Int32 GlobalIdMgr::_OnAfterCompsInit()
{
    if (!_mongoProxy)
    {
        CLOG_ERROR("have no mongodb proxy");
        return Status::Failed;
    }

    std::atomic<Int32> lifeCount{0};
    KERNEL_NS::ObjLife<std::atomic<Int32>> isFinished(lifeCount);
    
    g_EventLoopEasyTaskThreadPool->Send([this, isFinished]()
    {
        KERNEL_NS::RunRightNow([this, isFinished]()->KERNEL_NS::CoTask<>
        {
            co_await RegisterMachine();
        });
    });

    // 等待注册完成
    while (lifeCount.load(std::memory_order_acquire) > 1)
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        CLOG_INFO("waiting register machine ...");
    }

    return Status::Success;
}

Int32 GlobalIdMgr::_OnHostWillStart()
{
    return Status::Success;
}

void GlobalIdMgr::_OnHostClose()
{
    
}

KERNEL_NS::CoTask<> GlobalIdMgr::RegisterMachine()
{
    auto storageInfo = GetComp<KERNEL_NS::IMongodbStorageInfo>();

    // 注册机器id
    KERNEL_NS::SmartPtr<MPMCQueue<std::map<KERNEL_NS::LibString, MongoSerializeInfo> *, 1024>, KERNEL_NS::AutoDelMethods::Release> queue = MPMCQueue<std::map<KERNEL_NS::LibString, MongoSerializeInfo> *, 1024>::NewThreadLocal_MPMCQueue();

    std::atomic<Int32> lifeCount{0};
    KERNEL_NS::ObjLife<std::atomic<Int32>> isFinished(lifeCount);

    // 控制查询速度, 避免密集查询导致mongodb负荷重
    KERNEL_NS::SmartPtr<KERNEL_NS::CoLocker, KERNEL_NS::AutoDelMethods::Release> lck = KERNEL_NS::CoLocker::NewThreadLocal_CoLocker();
    std::atomic_bool isContinue = {false};
    std::atomic_bool isCompleted = {false};
    std::atomic<Int64> curTotal = {0};
    QueryRoundContinue roundContinue;
    roundContinue._roundWaiter = lck.AsSelf();
    roundContinue._isContinue = &isContinue;
    roundContinue._curTotal = &curTotal;
    roundContinue._isCompleted = &isCompleted;

    KERNEL_NS::RunRightNow([this, queue, storageInfo, isFinished, roundContinue]() mutable ->KERNEL_NS::CoTask<>
    {
        auto ret = co_await _mongoProxy->GetMongodbMgr()->Query(storageInfo->GetDbName(), storageInfo->GetSystemName(), {}, queue.AsSelf(), roundContinue);

        if(!ret)
        {
            CLOG_WARN("query fail, db:%s, system name:%s", storageInfo->GetObjName().c_str(), storageInfo->GetSystemName().c_str());
        }
    });

    // 边查询边处理
    std::set<Int64> machineIds;
    while ((lifeCount.load(std::memory_order_acquire) > 1) &&
        (!isCompleted.load(std::memory_order_acquire)))
    {
        std::map<KERNEL_NS::LibString, MongoSerializeInfo> *dict = NULL;
        queue->TryPop(dict);
        SmartMongoSerializeInfoWrapper wrapper(dict);
        if(dict)
        {
            auto iter = dict->find(GlobalIdMgrMongo::KeyName);
            if (iter != dict->end())
            {
                if (iter->second._stream)
                {
                    auto machineId = iter->second._stream->ReadInt64();
                    machineIds.insert(machineId);
                }
            }
        }

        if(queue->Empty())
        {
            isContinue.store(true, std::memory_order_release);
            lck->Sinal();
        }
        CLOG_INFO("wait Query done...");
        KERNEL_NS::SystemUtil::ThreadSleep(500);
    }

    isContinue.store(false, std::memory_order_release);
    lck->Broadcast();

    // 遍历
    Int64 finalMachineId = 0;
    Int64 curMaxMachineId = 0;
    if(machineIds.empty())
    {
        for(Int64 idx = 1; idx <= MAX_MACHINE_ID; ++idx)
        {
            const auto &nowTime = LibTime::Now();
            const auto &nowTimeByBase = KERNEL_NS::LibTime::FromMilliSeconds((nowTime - _baseTime).GetTotalMilliSeconds());
            
            auto updateFields = new std::map<KERNEL_NS::LibString, KERNEL_NS::Variant>();
            updateFields->emplace(GlobalIdMgrMongo::KeyName, idx);
            updateFields->emplace(GlobalIdMgrMongo::TimePartName, nowTimeByBase.GetMilliTimestamp());
            updateFields->emplace(GlobalIdMgrMongo::HeartbeatTimeName, nowTimeByBase.GetMilliTimestamp());
            updateFields->emplace(GlobalIdMgrMongo::CurOwnerName, _ownerId);
            
            bsoncxx::builder::basic::document filterDoc;
            filterDoc.append(bsoncxx::builder::basic::kvp(GlobalIdMgrMongo::KeyName, static_cast<std::int64_t>(idx)));

            auto ret = co_await _mongoProxy->GetMongodbMgr()->UpdateDataIf(storageInfo->GetDbName(), storageInfo->GetSystemName(), updateFields, &filterDoc, true);
            
        }

        co_return;
    }
    else
    {
        
    }

    // TODO:需要销毁消息队列元素
}

CoTask<Int64> GlobalIdMgr::TryOccupiedMachine(std::map<KERNEL_NS::LibString, MongoSerializeInfo> *dict, const LibTime &nowByBase)
{
    // TODO:
    auto iter = dict->find(GlobalIdMgrMongo::KeyName);
    Int64 machineId = 0;
    if (iter != dict->end())
    {
        auto v = iter->second._stream;
        if (v)
        {
            machineId = v->ReadInt64();
        }
    }
    if (UNLIKELY(machineId == 0))
    {
        CLOG_WARN("TryOccupiedMachine fail machine is zero");
        co_return 0;
    }

    auto storageInfo = GetComp<KERNEL_NS::IMongodbStorageInfo>();

    auto updateFields = new std::map<KERNEL_NS::LibString, KERNEL_NS::Variant>();

    // 
    bsoncxx::builder::basic::document filterDoc;
    auto ret = co_await _mongoProxy->GetMongodbMgr()->UpdateDataIf(storageInfo->GetDbName(), storageInfo->GetSystemName(), updateFields, )

    iter = dict->find(GlobalIdMgrMongo::TimePartName);
    Int64 timePartId = 0;
    if (iter != dict->end())
    {
        auto v = iter->second._stream;
        if (v)
        {
            timePartId = v->ReadInt64();
        }
    }

    iter = dict->find(GlobalIdMgrMongo::HeartbeatTimeName);
    KERNEL_NS::LibTime heartbeatTime;
    if (iter != dict->end())
    {
        auto v = iter->second._stream;
        if (v)
        {
            heartbeatTime = KERNEL_NS::LibTime::FromMilliSeconds(v->ReadInt64());
        }
    }

    iter = dict->find(GlobalIdMgrMongo::CurOwnerName);
    KERNEL_NS::LibString curOwnerName;
    if (iter != dict->end())
    {
        auto v = iter->second._stream;
        if (v)
        {
            curOwnerName.AppendData(v->GetReadBegin(), v->GetReadableSize());
        }
    }

    // 过期了可以占用
    if (!heartbeatTime || (heartbeatTime + _invalidTime) < nowByBase)
    {
        // TODO:
        // _mongoProxy->GetMongodbMgr()->TryAcquireLock()
        return machineId;
    }
}


KERNEL_END
