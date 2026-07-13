// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-07-12 18:07:42
// Author: Eric Yonng
// Description:

#include <pch.h>
#include "TestGlobalOptionComps.h"
#include <OptionComp/storage/MongoDB/MongoDBComp.h>
#include <OptionComp/GlobalId/GlobalId.h>
#include <OptionComp/GlobalParam/GlobalParam.h>
#include <OptionComp/Command/Command.h>
#include "bsoncxx/builder/basic/document.hpp"
#include "bsoncxx/builder/basic/array.hpp"

void TestGlobalOptionComps::Run()
{
    auto poller = KERNEL_NS::TlsUtil::GetPoller();

    // 命令行初始化
    KERNEL_NS::SmartPtr<KERNEL_NS::ICommandMgr, KERNEL_NS::AutoDelMethods::Release> commnadMgr = KERNEL_NS::CommandMgrFactory().Create()->CastTo<KERNEL_NS::ICommandMgr>();
    commnadMgr->AddCommand("quit", [poller]()
    {
        CLOG_INFO_GLOBAL(TestGlobalOptionComps, "quit...");
        poller->QuitLoop();
    });
    
    // 初始化mongodb mgr
    KERNEL_NS::SourceWrap sourceWrap;
    sourceWrap.Path = KERNEL_NS::SystemUtil::GetCurProgRootPath() + "/ini/service.yaml";
    KERNEL_NS::SmartPtr<KERNEL_NS::IMongoDbMgr, KERNEL_NS::AutoDelMethods::Release> mongodbMgr = KERNEL_NS::MongoDbMgrFactory().Create()->CastTo<KERNEL_NS::IMongoDbMgr>();
    mongodbMgr->SetConfigSource(sourceWrap);
    mongodbMgr->SetConfigKeyName("MongoTestSuit");

    KERNEL_NS::SmartPtr<KERNEL_NS::IGlobalParamMgr, KERNEL_NS::AutoDelMethods::Release> globalParamMgr = KERNEL_NS::GlobalParamMgrFactory().Create()->CastTo<KERNEL_NS::IGlobalParamMgr>();
    globalParamMgr->SetMongodbMgr(mongodbMgr.AsSelf());

    KERNEL_NS::SmartPtr<KERNEL_NS::IGlobalIdMgr, KERNEL_NS::AutoDelMethods::Release> globalIdMgr = KERNEL_NS::GlobalIdMgrFactory().Create()->CastTo<KERNEL_NS::IGlobalIdMgr>();
    globalIdMgr->SetMongodbMgr(mongodbMgr.AsSelf());
    globalIdMgr->SetGlobalParamMgr(globalParamMgr.AsSelf());

    auto st = commnadMgr->Init();
    if(st != Status::Success)
    {
        CLOG_ERROR_GLOBAL(TestGlobalOptionComps, "commnadMgr init fail st:%d", st);

        return;
    }
    
    st = mongodbMgr->Init();
    if(st != Status::Success)
    {
        CLOG_ERROR_GLOBAL(TestGlobalOptionComps, "mongodb init fail st:%d", st);
        return;
    }
    st = globalParamMgr->Init();
    if(st != Status::Success)
    {
        CLOG_ERROR_GLOBAL(TestGlobalOptionComps, "globalParamMgr init fail st:%d", st);
        return;
    }
    st = globalIdMgr->Init();
    if(st != Status::Success)
    {
        CLOG_ERROR_GLOBAL(TestGlobalOptionComps, "globalIdMgr init fail st:%d", st);
        return;
    }

    st = commnadMgr->Start();
    if(st != Status::Success)
    {
        CLOG_ERROR_GLOBAL(TestGlobalOptionComps, "commnadMgr start fail st:%d", st);

        return;
    }
    
    st = mongodbMgr->Start();
    if(st != Status::Success)
    {
        CLOG_ERROR_GLOBAL(TestGlobalOptionComps, "mongodb start fail st:%d", st);

        return;
    }

    st = globalParamMgr->Start();
    if(st != Status::Success)
    {
        CLOG_ERROR_GLOBAL(TestGlobalOptionComps, "globalParamMgr start fail st:%d", st);
        return;
    }

    st = globalIdMgr->Start();
    if(st != Status::Success)
    {
        CLOG_ERROR_GLOBAL(TestGlobalOptionComps, "globalIdMgr start fail st:%d", st);
        return;
    }

    std::atomic<Int64> genIdCount{0};
    std::atomic<Int64> sameCount{0};

    // 消费者线程, 监控每秒生成数量, 当前机器id
    std::atomic<Int32> lifeCount;
    KERNEL_NS::ObjLife<std::atomic<Int32>> lifeCountGuard(lifeCount);
    g_EventLoopEasyTaskThreadPool->Send([&genIdCount, poller, &globalIdMgr, &sameCount, lifeCountGuard]()
    {
        KERNEL_NS::PostCaller([&genIdCount, poller, &globalIdMgr, &sameCount, lifeCountGuard]()->KERNEL_NS::CoTask<>
        {
            auto ownerId = globalIdMgr->GetOwnerId();
            while (!poller->IsQuit())
            {
                co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
                auto genCount = genIdCount.exchange(0, std::memory_order_acq_rel);
                auto machineId = globalIdMgr->GetMachineId();
                auto timePart = globalIdMgr->GetTimePart();
                auto sames = sameCount.load(std::memory_order_acquire);
                CLOG_INFO_GLOBAL(TestGlobalOptionComps, "gen %lld qps, machine id:%lld, owner:%s, last time part:%lld, total same count:%lld"
                    , genCount, machineId, ownerId.c_str(), timePart, sames);
            }
        });
    });
    
    // 生产者线程, 生产id
    KERNEL_NS::SmartPtr<KERNEL_NS::LibEventLoopThread> thread =  new KERNEL_NS::LibEventLoopThread([&genIdCount, &globalIdMgr, poller, &sameCount, lifeCountGuard]()
    {
        KERNEL_NS::PostCaller([&genIdCount, &globalIdMgr, poller, &sameCount, lifeCountGuard]()->KERNEL_NS::CoTask<>
        {
            std::set<Int64> genIds;
            while (!poller->IsQuit())
            {
                globalIdMgr->NewId();
                // auto iter = genIds.insert(id);
                //
                // // 有没id冲突
                // if (!iter.second)
                //     sameCount.fetch_add(1, std::memory_order_release);
                
                genIdCount.fetch_add(1, std::memory_order_release);
            }

            co_return;
        });
    });

    thread->Start();

    // 另一个线程, 强行占用Owner, 

    // TODO:先测试UpdatParam
    // g_EventLoopEasyTaskThreadPool->Send([globalParamMgr, poller] () mutable -> void
    // {
    //     KERNEL_NS::PostCaller([globalParamMgr, poller]() mutable  ->KERNEL_NS::CoTask<>
    //     {
    //         const auto fieldName = globalParamMgr->GetUniqueKeyFieldName();
    //         while (!poller->IsQuit())
    //         {
    //             co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
    //
    //             auto nowTime = KERNEL_NS::LibTime::Now();
    //             auto randNum = KERNEL_NS::LibRandom<>::GetInstance<1, 1000>().Gen();
    //             auto kv = new std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> ();
    //             kv->emplace("CurMachine", KERNEL_NS::Variant(156LL));
    //             kv->emplace("CurHeartbeatTime", KERNEL_NS::Variant(nowTime.GetMilliTimestamp() + randNum));
    //
    //             bsoncxx::builder::basic::document doc;
    //             doc.append(bsoncxx::builder::basic::kvp(fieldName.GetRaw(), "TestParam"));
    //             doc.append(bsoncxx::builder::basic::kvp("$or", bsoncxx::builder::basic::make_array(
    //             bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("CurHeartbeatTime",bsoncxx::builder::basic::make_document(
    //             bsoncxx::builder::basic::kvp("$exists", false))
    //         )),
    //             bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("CurHeartbeatTime",bsoncxx::builder::basic::make_document(
    //                             bsoncxx::builder::basic::kvp("$lte", static_cast<std::int64_t>(nowTime.GetMilliTimestamp() + randNum))
    //                         )
    //             )
    //                 ))));
    //
    //             KERNEL_NS::SmartPtr<KERNEL_NS::LibString> jsonBack = new KERNEL_NS::LibString();
    //             const auto kvStr =  KERNEL_NS::StringUtil::ToString(*kv, ',');
    //
    //             CLOG_INFO_GLOBAL(TestGlobalOptionComps, "AtomicUpdateParam 1 kv:%s", kvStr.c_str());
    //             auto ret = co_await globalParamMgr->AtomicUpdateParam("TestParam", kv, &doc, jsonBack.AsSelf());
    //
    //             CLOG_INFO_GLOBAL(TestGlobalOptionComps, "AtomicUpdateParam 1 ret:%d, kv:%s, jsonback:%s", ret, kvStr.c_str(), jsonBack->c_str());
    //         }
    //     });
    // });
    //
    // KERNEL_NS::PostCaller([globalParamMgr, poller]() mutable ->KERNEL_NS::CoTask<>
    // {
    //
    //     auto kv = new std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> ();
    //     kv->emplace("CurMachine", KERNEL_NS::Variant(155LL));
    //     auto ret = co_await globalParamMgr->UpdateParam("TestParam", kv);
    //
    //     CLOG_INFO_GLOBAL(TestGlobalOptionComps, "UpdateParam ret:%d", ret);
    //
    //     const auto fieldName = globalParamMgr->GetUniqueKeyFieldName();
    //     while (!poller->IsQuit())
    //     {
    //         co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
    //
    //         auto nowTime = KERNEL_NS::LibTime::Now();
    //         auto randNum = KERNEL_NS::LibRandom<>::GetInstance<1, 1000>().Gen();
    //         kv = new std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> ();
    //         kv->emplace("CurMachine", KERNEL_NS::Variant(157LL));
    //         kv->emplace("CurHeartbeatTime", KERNEL_NS::Variant(nowTime.GetMilliTimestamp() + randNum));
    //
    //         bsoncxx::builder::basic::document doc;
    //         doc.append(bsoncxx::builder::basic::kvp(fieldName.GetRaw(), "TestParam"));
    //         doc.append(bsoncxx::builder::basic::kvp("$or", bsoncxx::builder::basic::make_array(
    //         bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("CurHeartbeatTime",bsoncxx::builder::basic::make_document(
    //         bsoncxx::builder::basic::kvp("$exists", false))
    //     )),
    //         bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("CurHeartbeatTime",bsoncxx::builder::basic::make_document(
    //                         bsoncxx::builder::basic::kvp("$lte", static_cast<std::int64_t>(nowTime.GetMilliTimestamp() + randNum))
    //                     )
    //         )
    //             ))));
    //
    //         KERNEL_NS::SmartPtr<KERNEL_NS::LibString> jsonBack = new KERNEL_NS::LibString();
    //         const auto kvStr =  KERNEL_NS::StringUtil::ToString(*kv, ',');
    //         CLOG_INFO_GLOBAL(TestGlobalOptionComps, "AtomicUpdateParam 2 kv:%s", kvStr.c_str());
    //
    //         ret = co_await globalParamMgr->AtomicUpdateParam("TestParam", kv, &doc, jsonBack.AsSelf());
    //
    //         CLOG_INFO_GLOBAL(TestGlobalOptionComps, "AtomicUpdateParam 2 ret:%d, kv:%s, jsonback:%s", ret,kvStr.c_str(), jsonBack->c_str());
    //     }
    // });

    poller->PrepareLoop();
    poller->EventLoop();
    poller->OnLoopEnd();

    thread->Close();

    while (lifeCount.load(std::memory_order_acquire) > 1)
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        CLOG_INFO_GLOBAL(TestGlobalOptionComps, "waiting life count reset...");
    }

    CLOG_INFO_GLOBAL(TestGlobalOptionComps, "life count reset completed.");

    globalIdMgr->WillClose();
    globalParamMgr->WillClose();
    mongodbMgr->WillClose();
    commnadMgr->WillClose();

    globalIdMgr->Close();
    globalParamMgr->Close();
    mongodbMgr->Close();
    commnadMgr->Close();
}

