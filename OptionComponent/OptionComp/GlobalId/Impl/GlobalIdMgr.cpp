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

#include "bsoncxx/builder/basic/document.hpp"
#include <OptionComp/GlobalParam/Interface/IGlobalParamMgr.h>
#include <kernel/comp/Timer/LibTimer.h>
#include <kernel/comp/Poller/Poller.h>
#include <bsoncxx/builder/basic/array.hpp>

KERNEL_BEGIN

const LibString GlobalIdMgr::ParamCollectionFieldName = "CurMachineId";
const LibString GlobalIdMgr::ParamCollectionKeyValue = "GlobalIdMachineId";

const LibString GlobalIdMgr::GlobalIdKeyName = "MachineId";
const LibString GlobalIdMgr::TimePartName = "TimePart";
const LibString GlobalIdMgr::HeartbeatTimeName = "HeartbeatTime";
const LibString GlobalIdMgr::CurOwnerName = "CurOwner";
const LibString GlobalIdMgr::DbName = "GlobalIdDb";
const LibString GlobalIdMgr::CollectionName = "GlobalId";
const LibString GlobalIdMgr::UniqueIndexName = "MachineIdIndexName";

GlobalIdMgr::GlobalIdMgr()
 :IGlobalIdMgr(KERNEL_NS::RttiUtil::GetTypeId<GlobalIdMgr>())
,_mongodbMgr(NULL)
,_lastId{0}
,_globalParamMgr(NULL)
// TODO:要不要加上当前机器的公网ip
,_ownerId(KERNEL_NS::GuidUtil::GenStr())
,_invalidTime(TimeSlice::FromHours(12))
,_baseTime(LibTime::FromTimeMoment(2026, 1, 1, 0, 0, 0))
,_saveDataTime{NULL}
,_saveDataPoller{NULL}
,_saveInterval(TimeSlice::FromSeconds(5))
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
}

Int64 GlobalIdMgr::NewId()
{
    auto lastId = _lastId.load(std::memory_order_acquire);
    Int64 newId = 0;
    for(;;)
    {
        auto seq = lastId & SEQ_MASK;
        auto timePart = (lastId & TIME_PART_MASK);
        const auto machineId = (lastId & MACHINE_ID_MASK);
        ++seq;

        // seq用尽调整timePart
        if(UNLIKELY((seq ^ SEQ_MOD) == 0))
        {
            seq = 0;
            timePart >>= TIME_PART_POS;
            ++timePart;
            timePart <<= TIME_PART_POS;
        }

        newId = timePart | machineId | seq;
        if(!_lastId.compare_exchange_weak(lastId, newId, std::memory_order_acq_rel))
            continue;

        break;
    }
    
    return newId;
}

void GlobalIdMgr::SetMongodbMgr(IMongoDbMgr *mongodbMgr)
{
    _mongodbMgr = mongodbMgr;
}

void GlobalIdMgr::SetGlobalParamMgr(IGlobalParamMgr *globalParamMgr)
{
    _globalParamMgr = globalParamMgr;
}

// 机器id
Int64 GlobalIdMgr::GetMachineId() const
{
    return (_lastId.load(std::memory_order_acquire) & MACHINE_ID_MASK) >> MACHINE_ID_POS;
}

// 时间位
Int64 GlobalIdMgr::GetTimePart() const
{
    return (_lastId.load(std::memory_order_acquire) & TIME_PART_MASK) >> TIME_PART_POS;
}

const KERNEL_NS::LibString &GlobalIdMgr::GetOwnerId() const
{
    return _ownerId;
}


Int32 GlobalIdMgr::_OnAfterCompsInit()
{
    if (!_mongodbMgr)
    {
        CLOG_ERROR("have no mongodb mgr");
        return Status::Failed;
    }

    if(!_globalParamMgr)
    {
        CLOG_ERROR("have no global param mgr");
        return Status::Failed;
    }

    // 初始化索引
    std::vector<std::pair<KERNEL_NS::LibString, Int32>> fields;
    fields.emplace_back(GlobalIdKeyName, 1);
    if (!_mongodbMgr->CreateIndex(DbName, CollectionName, UniqueIndexName, fields, true))
    {
        CLOG_ERROR("create index failed db:%s, collection name:%s, index name:%s", DbName.c_str()
            , CollectionName.c_str(), UniqueIndexName.c_str());
        return Status::Failed;
    }

    std::atomic<Int32> lifeCount{0};
    KERNEL_NS::ObjLife<std::atomic<Int32>> isFinished(lifeCount);

    // 注册机器id
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

    // 时间位溢出提示需要改造扩展系统(时间扩展到64位, 使用128bit全球唯一id)
    auto lastId = _lastId.load(std::memory_order_acquire);
    auto timePart = (lastId & TIME_PART_MASK) >> TIME_PART_POS;
    if(lastId == 0 || ((timePart + 1) >= TIME_PART_MAX_ID))
    {
        CLOG_ERROR("last id:%lld, timePart:%lld, last id is zero, or timePart overlimit", lastId, timePart);
        return Status::Failed;
    }

    // TODO:注册完成启动时间同步(同步失败应该要报错处理)
    g_EventLoopEasyTaskThreadPool->Send([this]()
    {
        _saveDataPoller.store(KERNEL_NS::TlsUtil::GetPoller(), std::memory_order_release);
        auto timer = LibTimer::NewThreadLocal_LibTimer();
        timer->SetTimeOutHandler([this](LibTimer *timer)
        {
            _OnTimerSave(timer);
        });
        timer->Schedule(_saveInterval);

        _saveDataTime.store(timer, std::memory_order_release);
    });
    
    // TODO:时间同步过程中如果,发生机器id被强占(一般发生在mongodb不可用(12小时以上), 导致无法同步和更新心跳,此时需要重新注册机器id)
    // TODO:监控Id是否超前, 超前多少秒, 分片速率统计等指标监控

    return Status::Success;
}

Int32 GlobalIdMgr::_OnHostWillStart()
{
    return Status::Success;
}

void GlobalIdMgr::_OnHostClose()
{
    // 即时保存
    auto saveDataPoller = _saveDataPoller.exchange(NULL, std::memory_order_acq_rel);
    if(saveDataPoller)
    {
        std::atomic<Int32> lifeCount{0};
        KERNEL_NS::ObjLife<std::atomic<Int32>> isFinished(lifeCount);

        // 关服需要保存数据(时间部分, 心跳, 解除占用机器id)
        saveDataPoller->Push([this, isFinished]()
        {
            auto timer = _saveDataTime.exchange(NULL, std::memory_order_acq_rel);
            if(timer)
            {
                KERNEL_NS::PostCaller([this, isFinished]()->CoTask<>
                {
                    co_await _SaveData(true);            
                });
                
                LibTimer::DeleteThreadLocal_LibTimer(timer);
            }
        });

        // 等待保存完成
        while (lifeCount.load(std::memory_order_acquire) > 1)
        {
            KERNEL_NS::SystemUtil::ThreadSleep(1000);
            CLOG_INFO("waiting save global id completed ...");
        }
    }
}

// 关闭时候必定不会触发 _OnTimerSave 因为HostClose时候销毁定时器
void GlobalIdMgr::_OnTimerSave(LibTimer *t)
{
    // 定时执行保存
    KERNEL_NS::RunRightNow([this]()->KERNEL_NS::CoTask<>
    {
        auto timer = _saveDataTime.load(std::memory_order_acquire);
        // 先暂停定时器
        if(timer)
            timer->Cancel();
        
        co_await _SaveData(false);

        // HostClose时 _saveDataTime为空
        timer = _saveDataTime.load(std::memory_order_acquire);
        if(timer)
            timer->Schedule(_saveInterval);
    });
}

CoTask<> GlobalIdMgr::_SaveData(bool isClose)
{
    auto lastId = _lastId.load(std::memory_order_acquire);
    const auto timePart = (lastId & TIME_PART_MASK) >> TIME_PART_POS;
    const auto machineId = (lastId & MACHINE_ID_MASK) >> MACHINE_ID_POS;

    // 关服不用顾忌, 机器id被占, 只需要打印出报错日志
    const auto &nowTime = LibTime::Now();
    if(isClose)
    {
        // 更新时间位, 心跳, 以及Owner
        std::map<LibString, Variant> kv;
        kv.emplace(GlobalIdKeyName, KERNEL_NS::Variant(machineId));
        auto updateFields = new std::map<KERNEL_NS::LibString, KERNEL_NS::Variant>();
        updateFields->emplace(TimePartName, KERNEL_NS::Variant(timePart));
        updateFields->emplace(HeartbeatTimeName, KERNEL_NS::Variant(nowTime.GetMilliTimestamp()));
        updateFields->emplace(CurOwnerName, KERNEL_NS::Variant(""));
        updateFields->emplace(GlobalIdKeyName, KERNEL_NS::Variant(machineId));
        // 得是自己占用的才需要更新
        bsoncxx::builder::basic::document filterDoc;
        filterDoc.append(bsoncxx::builder::basic::kvp(GlobalIdKeyName.GetRaw(), static_cast<std::int64_t>(machineId)));
        filterDoc.append(bsoncxx::builder::basic::kvp(CurOwnerName.GetRaw(), _ownerId.GetRaw()));

        // 如果数据库TimePart比当前timePart大, 不能覆盖掉
        filterDoc.append(bsoncxx::builder::basic::kvp(TimePartName.GetRaw(), bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$lte", static_cast<std::int64_t>(timePart))
            )));
        
        KERNEL_NS::SmartPtr<LibString> jsonBack = new LibString();
        auto ret = co_await _mongodbMgr->UpdateDataIf(DbName, CollectionName, kv, updateFields, &filterDoc, jsonBack.AsSelf());
        if(!ret)
        {
            CLOG_WARN("_SaveData UpdateDataIf fail when close, db:%s, collection:%s, kv:%s, TimePart:%lld, CurOwner:%s, HeartbeatTime:%lld,jsonBack:%s"
                , DbName.c_str(), CollectionName.c_str(), KERNEL_NS::StringUtil::ToString(kv, ',').c_str(), timePart, _ownerId.c_str(), nowTime.GetMilliTimestamp()
                , jsonBack ? jsonBack->c_str() : "");
        }
        else
        {
            CLOG_INFO("_SaveData UpdateDataIf success when close, db:%s, collection:%s, kv:%s, TimePart:%lld, CurOwner:%s, HeartbeatTime:%lld"
                , DbName.c_str(), CollectionName.c_str(), KERNEL_NS::StringUtil::ToString(kv, ',').c_str(), timePart, _ownerId.c_str(), nowTime.GetMilliTimestamp());
        }
        
        co_return;
    }

    // 非关服, 如果被其他进程占用, 则需要重新注册机器id
    {
        // 更新时间位, 心跳, 以及Owner
        std::map<LibString, Variant> kv;
        kv.emplace(GlobalIdKeyName, KERNEL_NS::Variant(machineId));
        
        auto updateFields = new std::map<KERNEL_NS::LibString, KERNEL_NS::Variant>();
        updateFields->emplace(TimePartName, KERNEL_NS::Variant(timePart));
        updateFields->emplace(HeartbeatTimeName, KERNEL_NS::Variant(nowTime.GetMilliTimestamp()));
        updateFields->emplace(CurOwnerName, KERNEL_NS::Variant(_ownerId));
        updateFields->emplace(GlobalIdKeyName, KERNEL_NS::Variant(machineId));

        // 得是自己占用的才需要更新
        bsoncxx::builder::basic::document filterDoc;
        filterDoc.append(bsoncxx::builder::basic::kvp(GlobalIdKeyName.GetRaw(), static_cast<std::int64_t>(machineId)));
        filterDoc.append(bsoncxx::builder::basic::kvp(CurOwnerName.GetRaw(), _ownerId.GetRaw()));

        // 如果数据库TimePart比当前timePart大, 不能覆盖掉
        filterDoc.append(bsoncxx::builder::basic::kvp(TimePartName.GetRaw(), bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$lte", static_cast<std::int64_t>(timePart))
            )));
        
        KERNEL_NS::SmartPtr<LibString> jsonBack = new LibString();
        auto ret = co_await _mongodbMgr->UpdateDataIf(DbName, CollectionName, kv, updateFields, &filterDoc, jsonBack.AsSelf());
        if(!ret)
        {
            CLOG_WARN("_SaveData UpdateDataIf fail, owner:%s, db:%s, collection:%s, kv:%s, TimePart:%lld, CurOwner:%s, HeartbeatTime:%lld,jsonBack:%s"
                , _ownerId.c_str(), DbName.c_str(), CollectionName.c_str(), KERNEL_NS::StringUtil::ToString(kv, ',').c_str(), timePart, _ownerId.c_str(), nowTime.GetMilliTimestamp()
                , jsonBack ? jsonBack->c_str() : "");

            // 失败了如果有jsonBack, 那么判断数据库的Owner还是不是自己, 如果是自己说明操作异常, 如果不是自己则发起注册机器id
            if(!jsonBack->empty())
            {
                auto &&json = nlohmann::json::parse(jsonBack->c_str(), NULL, false);
                if(json.is_object() && json.contains(CurOwnerName.GetRaw()))
                {
                    auto &&curOwner = json[CurOwnerName.GetRaw()];
                    if(curOwner.is_string())
                    {
                        auto &&curOwnerId = curOwner.get<std::string>();
                        if(curOwnerId != _ownerId)
                        {
                            // owner 已不再是自己, 重新注册
                            co_await RegisterMachine();
                        }
                        else
                        {
                            CLOG_WARN("_SaveData json back owner sill self, owner:%s", _ownerId.c_str());
                        }
                    }
                    else
                    {
                        CLOG_WARN("_SaveData json back param err, owner:%s", _ownerId.c_str());
                    }
                }

                // json解析失败, 无药可救了
                else
                {
                    CLOG_WARN("_SaveData json back parse fail, owner:%s", _ownerId.c_str());
                }
            }
        }
        else
        {
            CLOG_DEBUG("_SaveData UpdateDataIf success, db:%s, collection:%s, kv:%s, TimePart:%lld, CurOwner:%s, HeartbeatTime:%lld"
                , DbName.c_str(), CollectionName.c_str(), KERNEL_NS::StringUtil::ToString(kv, ',').c_str(), timePart, _ownerId.c_str(), nowTime.GetMilliTimestamp());
        }
    }

    co_return;
}


KERNEL_NS::CoTask<bool> GlobalIdMgr::RegisterMachine()
{
    KERNEL_NS::SmartPtr<KERNEL_NS::LibString, KERNEL_NS::AutoDelMethods::Release> jsonBack = new KERNEL_NS::LibString();
    Int64 curOriginMachineId = 0;
    Int64 finalMachineId = 0;
    Int64 finalTimePart = 0;
    KERNEL_NS::SmartPtr<std::map<KERNEL_NS::LibString, KERNEL_NS::Variant>> resultDict = new std::map<KERNEL_NS::LibString, KERNEL_NS::Variant>();
    std::set<Int64> skipMachineIds;
    do
    {
        // 1. 查询当前参数 id分别到哪里, rr原子更新machineid, 并尝试注册, 成功就占用不成功继续rr
        while (true)
        {
            // 构造条件
            bsoncxx::builder::basic::document doc;
            doc.append(bsoncxx::builder::basic::kvp(ParamCollectionFieldName.GetRaw(), static_cast<std::int64_t>(curOriginMachineId)));
            doc.append(bsoncxx::builder::basic::kvp(_globalParamMgr->GetUniqueKeyFieldName().GetRaw(), ParamCollectionKeyValue.GetRaw()));

            // 要更新的值
            const auto newMachineId = (curOriginMachineId + 1) % MACHINE_ID_MOD;
            if(skipMachineIds.contains(newMachineId))
            {
                CLOG_WARN("newMachineId :%lld, is invalid will skip", newMachineId);

                // 所有机器id都无效, 则lastId没有变化, 启动时候判0则告警并启动失败, 如果是运行时则无需更新_lastId, 因为怎么改都是错的,此时需要人工干预, 可以在线上搭建告警系统,
                // 当机器id时间部分值超过80%的值时告警, 并调整GlobalIdMgr的时间位位宽,采用128bit id，但是预计使用68年一个app能不能撑那么久都是个问题, 尤其是游戏行业, 十年二十年的游戏都很稀少, 所以大部分不用考虑时间位溢出问题
                // 可能所有机器id的心跳时间还没过期, 导致无可用的机器id
                if(static_cast<Int64>(skipMachineIds.size()) >= MACHINE_ID_MOD)
                {
                    CLOG_ERROR("all machine id is invalid, please check if time part overflow or all machine id is in heat beat time cd, owner:%s", _ownerId.c_str());
                    co_return false;
                }

                curOriginMachineId = newMachineId;
                continue;
            }
            auto keyRefValue = new std::map<LibString, Variant>();
            keyRefValue->emplace("CurMachineId", KERNEL_NS::Variant(newMachineId));
            jsonBack->clear();
            
            if(co_await _globalParamMgr->AtomicUpdateParam(ParamCollectionKeyValue, keyRefValue, &doc, jsonBack.AsSelf()))
            {
                CLOG_DEBUG("CurAllocMachine:%lld", newMachineId);
                finalMachineId = newMachineId;
                curOriginMachineId = newMachineId;
                break;
            }

            if(jsonBack->empty())
            {
                CLOG_WARN("AtomicUpdateParam fail but have no json back newMachineId:%lld, curOriginMachineId:%lld", newMachineId, curOriginMachineId);
                continue;
            }

            auto &&jsonDoc = nlohmann::json::parse(jsonBack->c_str(), NULL, false);
            if(!jsonDoc.is_object())
            {
                CLOG_WARN("AtomicUpdateParam fail but json back not object json back:%s, newMachineId:%lld, curOriginMachineId:%lld", jsonBack->c_str(), newMachineId, curOriginMachineId);
                continue;
            }

            if(!jsonDoc.contains(ParamCollectionFieldName.GetRaw()))
            {
                CLOG_WARN("AtomicUpdateParam fail but json back not object json back:%s, newMachineId:%lld, curOriginMachineId:%lld", jsonBack->c_str(), newMachineId, curOriginMachineId);
                continue;
            }
            
            curOriginMachineId = jsonDoc[ParamCollectionFieldName.GetRaw()].get<Int64>();
        }

        std::map<LibString, Variant> kv;
        kv.emplace(GlobalIdKeyName, KERNEL_NS::Variant(finalMachineId));
        
        // 先获取数据
        resultDict->clear();
        auto ret = co_await _mongodbMgr->Query(DbName, CollectionName, kv, resultDict.AsSelf(), true);
        if(ret && (!resultDict->empty()))
        {
            CLOG_DEBUG("query global id info, machine id:%lld, global id info:%s", finalMachineId, KERNEL_NS::StringUtil::ToString(*resultDict, ',').c_str());
            // 尝试注册机器id
            const auto &nowTime = LibTime::Now();
            const auto &nowTimeByBase = KERNEL_NS::LibTime::FromMilliSeconds((nowTime - _baseTime).GetTotalMilliSeconds());
                
            auto updateFields = new std::map<KERNEL_NS::LibString, KERNEL_NS::Variant>();

            auto iterTimePart = resultDict->find(TimePartName.GetRaw());

            // 时间位溢出, 告警提示, 并continue, 下一个机器id
            if(iterTimePart != resultDict->end())
            {
                const auto timePartValue = iterTimePart->second.AsInt64();
                if((timePartValue + 1) >= TIME_PART_MAX_ID)
                {
                    CLOG_WARN("machine id:%lld, time part:%lld, will overflow owner:%s", finalMachineId, timePartValue, _ownerId.c_str());
                    skipMachineIds.insert(finalMachineId);
                    continue;
                }
            }

            // 时间部分: 如果当前时间比数据库的时间部分大那么取当前时间, 如果小那么取数据库时间
            auto newTimePart = nowTimeByBase.GetSecTimestamp();
            Int64 curDbTimePart = 0;
            if(iterTimePart != resultDict->end())
            {
                curDbTimePart = iterTimePart->second.AsInt64();
                if(curDbTimePart > newTimePart)
                    newTimePart = curDbTimePart;
            }

            // 数据库的心跳时间
            Int64 heartbeatTimeFromDb = 0;
            auto iterHeartbeat = resultDict->find(HeartbeatTimeName);
            if(iterHeartbeat !=resultDict->end())
            {
                heartbeatTimeFromDb = iterHeartbeat->second.AsInt64();
            }

            // 用当前时间换算过期的心跳时间, 如果数据库的心跳时间比换算的过期的心跳时间小那么说明心跳时间过期了
            const auto invalidHeartbeatTimeByNow = nowTime - _invalidTime;

            // 已经用到了 newTimePart 那么可用的是 newTimePart + 1, 需要提前占用, 所以落地需落地newTimePart + 1
            updateFields->emplace(TimePartName, newTimePart + 1);
            updateFields->emplace(HeartbeatTimeName, nowTime.GetMilliTimestamp());
            updateFields->emplace(CurOwnerName, _ownerId);
            updateFields->emplace(GlobalIdKeyName, KERNEL_NS::Variant(finalMachineId));
            
            bsoncxx::builder::basic::document filterDoc;
            filterDoc.append(bsoncxx::builder::basic::kvp(GlobalIdKeyName.GetRaw(), static_cast<std::int64_t>(finalMachineId)));
            filterDoc.append(bsoncxx::builder::basic::kvp(HeartbeatTimeName.GetRaw(), static_cast<std::int64_t>(heartbeatTimeFromDb)));

            filterDoc.append(bsoncxx::builder::basic::kvp("$or", bsoncxx::builder::basic::make_array(
            bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp(HeartbeatTimeName.GetRaw(), bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp("$lte", static_cast<std::int64_t>(invalidHeartbeatTimeByNow.GetMilliTimestamp()))
                ))),
                bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp(CurOwnerName.GetRaw(), bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp("$exists", false)
                ))
                )
            )));
            
            jsonBack->clear();
            ret = co_await _mongodbMgr->UpdateDataIf(DbName, CollectionName, kv, updateFields, &filterDoc, jsonBack.AsSelf());
            if(ret)
            {
                CLOG_INFO("occupiedMachineId:%lld, RegisterMachine success, TimePart:%lld", finalMachineId, newTimePart);
                finalTimePart = newTimePart;
                break;
            }

            // 跳过机器id
            skipMachineIds.insert(finalMachineId);
            CLOG_DEBUG("UpdateDataIf fail other machine ocuppied, finalMachineId:%lld, jsonBack:%s", finalMachineId, jsonBack->c_str());
        }
        else
        {
            CLOG_DEBUG("query global id info fail have no data, machine id:%lld, global id info:%s", finalMachineId);

            // 尝试注册机器id
            const auto &nowTime = LibTime::Now();
            const auto &nowTimeByBase = KERNEL_NS::LibTime::FromMilliSeconds((nowTime - _baseTime).GetTotalMilliSeconds());
                
            auto updateFields = new std::map<KERNEL_NS::LibString, KERNEL_NS::Variant>();
            updateFields->emplace(TimePartName, nowTimeByBase.GetSecTimestamp() + 1);
            updateFields->emplace(HeartbeatTimeName, nowTime.GetMilliTimestamp());
            updateFields->emplace(CurOwnerName, _ownerId);
            updateFields->emplace(GlobalIdKeyName, KERNEL_NS::Variant(finalMachineId));

            bsoncxx::builder::basic::document filterDoc;
            filterDoc.append(bsoncxx::builder::basic::kvp(GlobalIdKeyName.GetRaw(), static_cast<std::int64_t>(finalMachineId)));
            filterDoc.append(bsoncxx::builder::basic::kvp(HeartbeatTimeName.GetRaw(), bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp("$exists", false)
                )));

            jsonBack->clear();
            ret = co_await _mongodbMgr->UpdateDataIf(DbName, CollectionName, kv, updateFields, &filterDoc, jsonBack.AsSelf());
            if(ret)
            {
                CLOG_INFO("occupiedMachineId:%lld, RegisterMachine success, TimePart:%lld", finalMachineId, nowTimeByBase.GetSecTimestamp());
                finalTimePart = nowTimeByBase.GetSecTimestamp();
                break;
            }

            // 跳过机器id
            skipMachineIds.insert(finalMachineId);
            CLOG_DEBUG("UpdateDataIf fail other machine ocuppied, finalMachineId:%lld, jsonBack:%s", finalMachineId, jsonBack->c_str());
        }

    }
    while (true);

    // 最后的时间部分是 finalTimePart 那么可用的就是 finalTimePart + 1
    {
        auto lastId = _lastId.load(std::memory_order_acquire);

        // 说明在已经使用的情况下重新注册机器id
        if(lastId != 0)
        {
            auto timePart = (lastId & TIME_PART_MASK) >> TIME_PART_POS;
            const auto machineId = (lastId & MACHINE_ID_MASK) >> MACHINE_ID_POS;

            // 重新注册到当前已经注册的机器id, 则看时间部分, 如果时间部分 finalTimePart + 1要大则使用finalTimePart + 1
            if(machineId == finalMachineId)
            {
                const auto oldTimePart = timePart;
                Int64 newTimePart = timePart;
                if(timePart < (finalTimePart + 1))
                    newTimePart = finalTimePart + 1;

                // 直接跳过 timePart避免 seq冲突的麻烦
                else
                {
                    newTimePart = timePart + 1;
                }

                // 相同的机器id需要cas
                auto newLastId = (newTimePart << TIME_PART_POS) | finalMachineId << MACHINE_ID_POS;
                while (!_lastId.compare_exchange_weak(lastId, newLastId, std::memory_order_acq_rel))
                {
                    timePart = (lastId & TIME_PART_MASK) >> TIME_PART_POS;
                    if(timePart < (finalTimePart + 1))
                        newTimePart = finalTimePart + 1;
                    // 直接跳过 timePart避免 seq冲突的麻烦
                    else
                    {
                        newTimePart = timePart + 1;
                    }

                    newLastId = (newTimePart << TIME_PART_POS) | finalMachineId << MACHINE_ID_POS;
                }
                
                CLOG_INFO("RegisterMachine success old machine id:%lld, new machine id:%lld, oldTimePart:%lld => useful time part:%lld, newLastId:%lld"
                    , machineId, finalMachineId, oldTimePart, newTimePart, newLastId);

                co_return true;
            }

            // 注册到新的机器id则使用 finalTimePart + 1
            else
            {
                timePart = finalTimePart + 1;
            }
            const auto newLastId = (timePart << TIME_PART_POS) | finalMachineId << MACHINE_ID_POS;
            _lastId.exchange(newLastId, std::memory_order_acq_rel);
            CLOG_INFO("RegisterMachine success old machine id:%lld, new machine id:%lld, useful time part:%lld, last id:%lld"
                , machineId, finalMachineId, (finalTimePart + 1), lastId);
        }

        // 启动注册时候直接使用 finalTimePart + 1
        else
        {
            lastId = ((finalTimePart + 1) << TIME_PART_POS) | finalMachineId << MACHINE_ID_POS;
            _lastId.exchange(lastId, std::memory_order_acq_rel);
            CLOG_INFO("RegisterMachine success machine id:%lld, useful time part:%lld, last id:%lld", finalMachineId, (finalTimePart + 1), lastId);
        }
    }

    co_return true;
}

KERNEL_END
