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

#include "OptionComp/storage/MongoDB/Interface/IMongodbProxy.h"

KERNEL_BEGIN
    GlobalIdMgr::GlobalIdMgr()
 :IGlobalIdMgr(KERNEL_NS::RttiUtil::GetTypeId<GlobalIdMgr>())
,_lastId{0}
,_mongoProxy(NULL)
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
    while ((lifeCount.load(std::memory_order_acquire) > 1) &&
        (!isCompleted.load(std::memory_order_acquire)))
    {
        std::map<KERNEL_NS::LibString, MongoSerializeInfo> *dict = NULL;
        queue->TryPop(dict);
        if(dict)
        {
            // TODO:
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

    // TODO:需要销毁消息队列元素

    return Status::Success;
}

Int32 GlobalIdMgr::_OnHostWillStart()
{
    return Status::Success;
}

void GlobalIdMgr::_OnHostClose()
{
    
}

KERNEL_END
