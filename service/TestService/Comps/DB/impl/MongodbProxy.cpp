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
// Date: 2026-06-24 22:06:52
// Author: Eric Yonng
// Description:

#include <pch.h>

#include <Comps/DB/impl/MongodbProxy.h>
#include <Comps/DB/impl/MongodbProxyFactory.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <OptionComp/storage/MongoDB/MongoDBComp.h>
#include <service/common/BaseComps/Event/Event.h>
#include <service/common/Params.h>
#include <service/common/BaseComps/ServiceCompType.h>

namespace 
{
    static ALWAYS_INLINE void DelStreamContainer(std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *d)
    {
        KERNEL_NS::ContainerUtil::DelContainer(*d, [](const KERNEL_NS::MongoSerializeInfo &p)
        {
            KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(p._stream);
        });
        CRYSTAL_DELETE_SAFE(d);
    }

    static ALWAYS_INLINE KERNEL_NS::LibString DictContainerToString(const std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> &kv)
    {
        return KERNEL_NS::StringUtil::ToStringBy(kv, ",", [](const std::pair<KERNEL_NS::LibString, KERNEL_NS::Variant> &iter)->KERNEL_NS::LibString
        {
            return KERNEL_NS::LibString().AppendFormat("%s:%s", iter.first.c_str(), iter.second.ToString().c_str());
        });
    }
}
SERVICE_BEGIN

MongodbProxy::MongodbProxy()
    :IMongodbProxy(KERNEL_NS::RttiUtil::GetTypeId<MongodbProxy>())
    ,_closeServiceStub(INVALID_LISTENER_STUB)
    ,_purgeTimer(KERNEL_NS::LibTimer::NewThreadLocal_LibTimer())
    ,_options(new KERNEL_NS::FileMonitor<KERNEL_NS::MongodbConfig, KERNEL_NS::YamlDeserializer>())
    ,_checkQuit(false)
{
    
}

MongodbProxy::~MongodbProxy()
{
    _Clear();
}

void MongodbProxy::Release()
{
    MongodbProxy::DeleteByAdapter_MongodbProxy(MongodbProxyFactory::_buildType.V, this);
}

void MongodbProxy::OnRegisterComps()
{
    RegisterComp<KERNEL_NS::MongoDbMgrFactory>();
}

void MongodbProxy::RegisterDependence(ILogicSys *obj)
{
    GetComp<KERNEL_NS::IMongoDbMgr>()->RegisterDependence(obj);
}

void MongodbProxy::UnRegisterDependence(const ILogicSys *obj)
{
    GetComp<KERNEL_NS::IMongoDbMgr>()->UnRegisterDependence(obj);
}

// 标脏
void MongodbProxy::MaskLogicNumberKeyAddDirty(const ILogicSys *logic, Int64 key)
{
    if(_dirtyLogicRefIsNumberKey.find(logic) == _dirtyLogicRefIsNumberKey.end())
        _dirtyLogicRefIsNumberKey.insert(std::make_pair(logic, true));

    auto iter = _logicRefNumberDirtyHelper.find(logic);
    if(iter == _logicRefNumberDirtyHelper.end())
    {
        iter = _logicRefNumberDirtyHelper.insert(std::make_pair(logic, KERNEL_NS::LibDirtyHelper<Int64, UInt64>::NewThreadLocal_LibDirtyHelper())).first;
        _InitDirtyHelper(iter->second);
    }

    auto dirtyHelper = iter->second;

    // 如果有其他脏标记则移除
    if(dirtyHelper->HasDirty(key))
        dirtyHelper->Clear(key);

    auto var = dirtyHelper->MaskDirty(key, StorageMode::REPLACE_DATA, true);
    var->BecomePtr()[Params::VAR_LOGIC] = logic;
}
void MongodbProxy::MaskLogicNumberKeyModifyDirty(const ILogicSys *logic, Int64 key)
{
    auto iter = _logicRefNumberDirtyHelper.find(logic);
    if(iter == _logicRefNumberDirtyHelper.end())
    {
        iter = _logicRefNumberDirtyHelper.insert(std::make_pair(logic, KERNEL_NS::LibDirtyHelper<Int64, UInt64>::NewThreadLocal_LibDirtyHelper())).first;
        _InitDirtyHelper(iter->second);
    }

    auto dirtyHelper = iter->second;

    // 如果已删除则不可修改
    const auto isDelDirty = dirtyHelper->IsDirty(key, StorageMode::DEL_DATA);
    if(UNLIKELY(isDelDirty))
    {
        CLOG_ERROR("data had mask del before, logic:%s, key:%lld", logic->GetObjName().c_str(), key);
        return;
    }

    // 已经标脏过不需要再标脏(增加会覆盖最新的数据)
    if(dirtyHelper->HasDirty(key))
        return;

    auto var = dirtyHelper->MaskDirty(key, StorageMode::UPDATE_DATA, true);
    var->BecomePtr()[Params::VAR_LOGIC] = logic;

    if(_dirtyLogicRefIsNumberKey.find(logic) == _dirtyLogicRefIsNumberKey.end())
        _dirtyLogicRefIsNumberKey.insert(std::make_pair(logic, true));
}

void MongodbProxy::MaskLogicNumberKeyDeleteDirty(const ILogicSys *logic, Int64 key)
{
    auto iter = _logicRefNumberDirtyHelper.find(logic);
    if(iter == _logicRefNumberDirtyHelper.end())
    {
        iter = _logicRefNumberDirtyHelper.insert(std::make_pair(logic, KERNEL_NS::LibDirtyHelper<Int64, UInt64>::NewThreadLocal_LibDirtyHelper())).first;
        _InitDirtyHelper(iter->second);
    }

    auto dirtyHelper = iter->second;

    // 标脏
    dirtyHelper->Clear(key);
    auto var = dirtyHelper->MaskDirty(key, StorageMode::DEL_DATA, true);
    var->BecomePtr()[Params::VAR_LOGIC] = logic;

    if(_dirtyLogicRefIsNumberKey.find(logic) == _dirtyLogicRefIsNumberKey.end())
        _dirtyLogicRefIsNumberKey.insert(std::make_pair(logic, true));
}
    
void MongodbProxy::MaskLogicStringKeyAddDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key)
{
    auto iter = _logicRefStringDirtyHelper.find(logic);
    if(iter == _logicRefStringDirtyHelper.end())
    {
        iter = _logicRefStringDirtyHelper.insert(std::make_pair(logic, KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64>::NewThreadLocal_LibDirtyHelper())).first;
        _InitDirtyHelper(iter->second);
    }
    auto dirtyHelper = iter->second;
    // 如果有其他脏标记则移除
    if(dirtyHelper->HasDirty(key))
        dirtyHelper->Clear(key);

    auto var = dirtyHelper->MaskDirty(key, StorageMode::REPLACE_DATA, true);
    var->BecomePtr()[Params::VAR_LOGIC] = logic;

    if(_dirtyLogicRefIsNumberKey.find(logic) == _dirtyLogicRefIsNumberKey.end())
        _dirtyLogicRefIsNumberKey.insert(std::make_pair(logic, false));
}

void MongodbProxy::MaskLogicStringKeyModifyDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key)
{
    auto iter = _logicRefStringDirtyHelper.find(logic);
    if(iter == _logicRefStringDirtyHelper.end())
    {
        iter = _logicRefStringDirtyHelper.insert(std::make_pair(logic, KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64>::NewThreadLocal_LibDirtyHelper())).first;
        _InitDirtyHelper(iter->second);
    }
    auto dirtyHelper = iter->second;

    // 如果已删除则不可修改
    const auto isDelDirty = dirtyHelper->IsDirty(key, StorageMode::DEL_DATA);
    if(UNLIKELY(isDelDirty))
    {
        CLOG_ERROR("data had mask del before, logic:%s, key:%s", logic->GetObjName().c_str(), key.c_str());
        return;
    }

    // 已经标脏过不需要再标脏(增加会覆盖最新的数据)
    if(dirtyHelper->HasDirty(key))
        return;

    auto var = dirtyHelper->MaskDirty(key, StorageMode::UPDATE_DATA, true);
    var->BecomePtr()[Params::VAR_LOGIC] = logic;
    
    if(_dirtyLogicRefIsNumberKey.find(logic) == _dirtyLogicRefIsNumberKey.end())
        _dirtyLogicRefIsNumberKey.insert(std::make_pair(logic, false));
}

void MongodbProxy::MaskLogicStringKeyDeleteDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key)
{
    auto iter = _logicRefStringDirtyHelper.find(logic);
    if(iter == _logicRefStringDirtyHelper.end())
    {
        iter = _logicRefStringDirtyHelper.insert(std::make_pair(logic, KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64>::NewThreadLocal_LibDirtyHelper())).first;
        _InitDirtyHelper(iter->second);
    }
    auto dirtyHelper = iter->second;

    // 标脏
    dirtyHelper->Clear(key);
    auto var = dirtyHelper->MaskDirty(key, StorageMode::DEL_DATA, true);
    var->BecomePtr()[Params::VAR_LOGIC] = logic;

    if(_dirtyLogicRefIsNumberKey.find(logic) == _dirtyLogicRefIsNumberKey.end())
        _dirtyLogicRefIsNumberKey.insert(std::make_pair(logic, false));
}

// 等待落库完成
KERNEL_NS::CoTask<> MongodbProxy::Purge()
{
    if(_dirtyLogicRefIsNumberKey.empty())
        co_return;

    // 拷贝避免字典破坏
    auto dirtyDict = _dirtyLogicRefIsNumberKey;
    _dirtyLogicRefIsNumberKey.clear();
    
    for(auto iter : dirtyDict)
    {
        auto logic = iter.first;
        
        // number为key
        if(iter.second)
        {
            co_await _DorPurgeNumber(logic);
            continue;
        }

        // string 为key
        co_await _DorPurgeString(logic);
    }
    co_return;
}

// 等待logic落库完成
KERNEL_NS::CoTask<> MongodbProxy::Purge(const ILogicSys *logic)
{
    auto iter = _dirtyLogicRefIsNumberKey.find(logic);
    if(iter == _dirtyLogicRefIsNumberKey.end())
        co_return;

    if(iter->second)
    {
        co_await _DorPurgeNumber(logic);
        co_return;
    }

    co_await _DorPurgeString(logic);
    co_return;
}

KERNEL_NS::CoTask<bool> MongodbProxy::Query(const ILogicSys *logic, Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult)
{
    if(UNLIKELY(!logic || !fieldNameRefDataResult))
    {
        CLOG_ERROR("query fail logic or fieldNameRefDataResult is null logic:%s, key:%lld", logic ? logic->GetObjName().c_str() : "", key);
        co_return false;
    }
    
    auto storageComp = logic->GetComp<SERVICE_NS::IMongodbStorageInfo>();
    if(UNLIKELY(!storageComp))
    {
        CLOG_ERROR("query fail logic have no mongodb storage info comp logic:%s, key:%lld", logic->GetObjName().c_str(), key);
        co_return false;
    }

    // 必须初始化的时候被建立过表信息
    auto iter = _sysRefFieldStorageType.find(logic);
    if(UNLIKELY(iter == _sysRefFieldStorageType.end() || iter->second.empty()))
    {
        CLOG_ERROR("query fail logic have no mongodb storage info comp logic:%s, db:%s key:%lld", logic->GetObjName().c_str(), storageComp->GetDbName().c_str(), key);
        co_return false;
    }
    
    auto mongodbMgr = GetComp<KERNEL_NS::IMongoDbMgr>();
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    auto &uniqueIndexFields = storageComp->GetUniqueIndexFields();
    for(auto &field : uniqueIndexFields)
    {
        kv.emplace(field.first, KERNEL_NS::Variant(key));
        break;
    }

    auto &rawFieldInfo = iter->second;
    if(fieldNameRefDataResult->empty())
    {
        for(auto &iterFieldInfo : rawFieldInfo)
        {
            fieldNameRefDataResult->emplace(iterFieldInfo.first, KERNEL_NS::MongoSerializeInfo(iterFieldInfo.second, KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream()));
        }
    }

    // 指定了字段, 则检查MongoSerializeInfo
    else
    {
        for(auto iterFieldInfo = fieldNameRefDataResult->begin(); iterFieldInfo != fieldNameRefDataResult->end();)
        {
            // 不存在就移除, 并打印日志
            auto iterRaw = rawFieldInfo.find(iterFieldInfo->first);
            if(iterRaw == rawFieldInfo.end())
            {
                CLOG_WARN("logic have no field info:%s, logic:%s", iterFieldInfo->first.c_str(), logic->GetObjName().c_str());
                iterFieldInfo = fieldNameRefDataResult->erase(iterFieldInfo);
                continue;
            }

            auto &fieldInfo = iterFieldInfo->second;
            if(UNLIKELY(!fieldInfo._stream))
            {
                fieldInfo._stream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
            }
            fieldInfo.DataType = iterRaw->second;
            
            ++iterFieldInfo;
        }
    }

    const auto systemName = storageComp->GetSystemName();
    const auto dbName = storageComp->GetDbName();
    auto ret = co_await mongodbMgr->Query(dbName, storageComp->GetSystemName(), kv, fieldNameRefDataResult);
    if(UNLIKELY(!ret))
    {
        CLOG_DEBUG("query data fail db:%s, collection:%s, kv:%s", dbName.c_str(), systemName.c_str(), DictContainerToString(kv).c_str());
    }

    co_return ret;
}

KERNEL_NS::CoTask<bool> MongodbProxy::Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString keyName, Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult)
{
     if(UNLIKELY(!fieldNameRefDataResult))
    {
        CLOG_ERROR("query fail logic or fieldNameRefDataResult is null collectionName:%s, db:%s, key:%lld", collectionName.c_str(), dbName.c_str(), key);
        co_return false;
    }
    
    auto mongodbMgr = GetComp<KERNEL_NS::IMongoDbMgr>();
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    kv.emplace(keyName, KERNEL_NS::Variant(key));
    auto ret = co_await mongodbMgr->Query(dbName, collectionName, kv, fieldNameRefDataResult);
    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("query data fail db:%s, collection:%s, kv:%s", dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str());
    }

    co_return ret;
}

KERNEL_NS::CoTask<bool> MongodbProxy::Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString keyName, KERNEL_NS::LibString key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult)
{
    if(UNLIKELY(!fieldNameRefDataResult))
    {
        CLOG_ERROR("query fail logic or fieldNameRefDataResult is null collectionName:%s, db:%s, key:%lld", collectionName.c_str(), dbName.c_str(), key);
        co_return false;
    }
    
    auto mongodbMgr = GetComp<KERNEL_NS::IMongoDbMgr>();
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    kv.emplace(keyName, KERNEL_NS::Variant(key));
    auto ret = co_await mongodbMgr->Query(dbName, collectionName, kv, fieldNameRefDataResult);
    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("query data fail db:%s, collection:%s, kv:%s", dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str());
    }

    co_return ret;
}


Int32 MongodbProxy::_OnGlobalSysInit()
{
    _closeServiceStub = _eventMgr->AddListener(EventEnums::QUIT_SERVICE_EVENT, this, &MongodbProxy::_CloseServiceEvent);

    if(!_options->Init(GetApp()->GetSourceWrap(), "MongoTestSuit"))
    {
        CLOG_ERROR("mongodb options init fail");
        return Status::ConfigError;
    }

    _purgeTimer->SetTimeOutHandler(this, &MongodbProxy::_OnPurgeTimer);
    const auto interval = _options->Current()->PurgeInterval;
    _purgeTimer->Schedule(interval);
    
    return Status::Success;
}
Int32 MongodbProxy::_OnGlobalSysCompsCreated()
{
    // 设置配置
    auto mongodbMgr = GetComp<KERNEL_NS::IMongoDbMgr>();
    mongodbMgr->SetConfigSource(*GetApp()->GetSourceWrap());
    mongodbMgr->SetConfigKeyName("MongoTestSuit");
    
    return Status::Success;
}

ALWAYS_INLINE static bool CheckValidStorage(const KERNEL_NS::CompHostObject *turnHost, const SERVICE_NS::IMongodbStorageInfo *storageComp, bool systemAsField, KERNEL_NS::LibString &errInfo)
{
    // 检查storageComp合法性

    // 必须检查项
    bool ret = true;
    if(storageComp->GetSystemName().empty())
    {
        errInfo.AppendFormat("logic :%s lack system name\n", turnHost->GetObjName().c_str());
        ret = false;
    }

    // 系统整体作为表的一个字段, 不用检查索引和_fieldNameRefStorageType, 以及dbName, 只需要检查_storageType
    if (!systemAsField)
    {
        if(storageComp->GetDbName().empty())
        {
            errInfo.AppendFormat("logic :%s lack db name\n", turnHost->GetObjName().c_str());
            ret = false;
        }
        
        if(storageComp->GetUniqueIndexFields().empty())
        {
            errInfo.AppendFormat("logic :%s lack unique index fields\n", turnHost->GetObjName().c_str());
            ret = false;
        }

        auto &fieldNameRefStorageType = storageComp->GetFieldNameRefStorageType();
        if(fieldNameRefStorageType.empty())
        {
            errInfo.AppendFormat("logic :%s lack _fieldNameRefStorageType and _storageType\n", turnHost->GetObjName().c_str());
            ret = false;
        }

        for(auto &iter : storageComp->GetUniqueIndexFields())
        {
            auto iterField = fieldNameRefStorageType.find(iter.first);
            if(iterField == fieldNameRefStorageType.end())
            {
                errInfo.AppendFormat("logic :%s lack _fieldNameRefStorageType lack unique field:%s\n", turnHost->GetObjName().c_str(), iter.first.c_str());
                ret = false;
            }
        }
    }
    else
    {
        if (storageComp->GetStorageType() == KERNEL_NS::MongoSerializeInfoType::UNKNOWN)
        {
            errInfo.AppendFormat("logic :%s lack sub comp:%s lack storage type, storageComp:%s\n", turnHost->GetObjName().c_str(), storageComp->GetOwner()->GetObjName().c_str(), storageComp->GetObjName().c_str());
            ret = false;
        }
    }

    return ret;
}

Int32 MongodbProxy::_OnGlobalSysWillStart()
{
    // 扫码所有的系统注册依赖
    auto &comps = GetService()->GetAllComps();
    KERNEL_NS::LibString errInfo;
    for(auto comp : comps)
    {
        if(UNLIKELY(!comp))
            continue;

        if(comp->GetType() != ServiceCompType::LOGIC_SYS)
            continue;

        auto turnLogic = comp->CastTo<SERVICE_NS::ILogicSys>();
        _CheckLogic(turnLogic, errInfo);
    }

    if(!errInfo.empty())
    {
        CLOG_ERROR("check logics storage fail\n :%s", errInfo.c_str());
        return Status::ConfigError;
    }

    auto mongodbMgr = GetComp<KERNEL_NS::IMongoDbMgr>();
    for(auto comp : comps)
    {
        if(UNLIKELY(!comp))
            continue;

        if(comp->GetType() != ServiceCompType::LOGIC_SYS)
            continue;

        auto turnLogic = comp->CastTo<SERVICE_NS::ILogicSys>();
        auto storageComp = turnLogic->GetComp<SERVICE_NS::IMongodbStorageInfo>();
        if(!storageComp)
            continue;

        RegisterDependence(turnLogic);

        // 索引信息(稳定排序)
        std::set<KERNEL_NS::LibString> fields;
        auto &uniqueIndexFields = storageComp->GetUniqueIndexFields();
        for(auto &item : uniqueIndexFields)
            fields.insert(item.first);
        
        KERNEL_NS::LibString indexName = "unique_index_";
        const Int32 count = static_cast<Int32>(fields.size());
        Int32 idx = 0;
        for(auto &item : fields)
        {
            auto &&strip = item.strip();
            strip = strip.tolower();
            indexName.AppendFormat("%s", strip.c_str());
            if(idx != (count - 1))
            {
                indexName.Append("_");
            }

            ++idx;
        }
        if(!mongodbMgr->CreateIndex(storageComp->GetDbName(), storageComp->GetSystemName(), indexName, storageComp->GetUniqueIndexFields(), true))
        {
            errInfo.AppendFormat("CreateIndex fail db:%s, collection:%s indexName:%s logic:%s\n", storageComp->GetDbName().c_str(), storageComp->GetSystemName().c_str(), indexName.c_str(), turnLogic->GetObjName().c_str());
            continue;
        }

        // 记录表信息
        _BuildSysFields(turnLogic);
    }

    if(!errInfo.empty())
    {
        CLOG_ERROR("mongodb proxy start fail errInfo:\n%s", errInfo.c_str());
        return Status::ConfigError;
    }
    
    return Status::Success;
}

Int32 MongodbProxy::_OnHostStart()
{
    MaskReady(true);

    return Status::Success;
}

void MongodbProxy::_OnGlobalSysClose()
{
    MaskReady(false);
    _Clear();
}

void MongodbProxy::_CloseServiceEvent(KERNEL_NS::LibEvent *ev)
{
    _CheckQuit();
}

void MongodbProxy::_OnPurgeTimer(KERNEL_NS::LibTimer *t)
{
    if(!_dirtyLogicRefIsNumberKey.empty())
    {
        KERNEL_NS::RunRightNow([this]()->KERNEL_NS::CoTask<>
        {
            co_await Purge();
        });
    }

    // 时间变更了
    auto currentConfig = _options->Current();
    if(t->GetData()->_period != currentConfig->PurgeInterval.GetTotalNanoSeconds())
    {
        t->Schedule(currentConfig->PurgeInterval);
    }
}

void MongodbProxy::_CheckQuit()
{
    if(_checkQuit)
        return;

    _checkQuit = true;
    
    KERNEL_NS::RunRightNow([this]()->KERNEL_NS::CoTask<>
    {
        // 先执行全部脏持久化
        co_await Purge();

        auto mongodbMgr = GetComp<KERNEL_NS::IMongoDbMgr>();

        // 检查依赖是否退出service
        auto service = GetService();
        if(mongodbMgr->HasDependence())
        {
            do
            {
                auto dependencies = mongodbMgr->GetDependenceComps();
                for(auto iter = dependencies.begin(); iter != dependencies.end();)
                {
                    if(!service->IsServiceModuleQuit(*iter))
                    {
                        ++iter;
                        continue;
                    }

                    UnRegisterDependence((*iter)->CastTo<ILogicSys>());
                    iter = dependencies.erase(iter);
                }

                // 依赖清空
                if(!dependencies.empty())
                {
                    const auto &str = KERNEL_NS::StringUtil::ToString(dependencies, ',');
                    CLOG_INFO("waiting mongodb dependencies quit service... :%s", str.c_str());
                    co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
                    continue;
                }

                CLOG_INFO("all mongodb dependencies quit service.");
                break;
            }
            while (true);
        }

        // 等待请求全部完成
        while(mongodbMgr->GetPendingRequestCount() != 0)
        {
            CLOG_INFO("waiting mongodb mgr pending request(%lld) complete...", mongodbMgr->GetPendingRequestCount());
            co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
        }

        // 避免遗漏再Purge一次
        if(!_dirtyLogicRefIsNumberKey.empty())
            co_await Purge();

        // 等待请求全部完成
        while(mongodbMgr->GetPendingRequestCount() != 0)
        {
            CLOG_INFO("waiting mongodb mgr pending request(%lld) complete...", mongodbMgr->GetPendingRequestCount());
            co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
        }
        
        CLOG_INFO("mongodb mgr quit completed.");
        
        _Clear();
        GetService()->MaskServiceModuleQuitFlag(this);
    });
}

void MongodbProxy::_Clear()
{
    if(_purgeTimer)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_purgeTimer);
        _purgeTimer = NULL;
    }
    
    CRYSTAL_DELETE_SAFE(_options);
}

void MongodbProxy::_InitDirtyHelper(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper)
{
    dirtyHelper->Init(StorageMode::MAX_TYPE);

    auto deleg = KERNEL_NS::DelegateFactory::Create(this, &MongodbProxy::_OnNumberAddDirtyHandler);
    dirtyHelper->SetHandler(StorageMode::ADD_DATA, deleg);
    
    deleg = KERNEL_NS::DelegateFactory::Create(this, &MongodbProxy::_OnNumberModifyDirtyHandler);
    dirtyHelper->SetHandler(StorageMode::UPDATE_DATA, deleg);

    deleg = KERNEL_NS::DelegateFactory::Create(this, &MongodbProxy::_OnNumberDeleteDirtyHandler);
    dirtyHelper->SetHandler(StorageMode::DEL_DATA, deleg);

    deleg = KERNEL_NS::DelegateFactory::Create(this, &MongodbProxy::_OnNumberReplaceDirtyHandler);
    dirtyHelper->SetHandler(StorageMode::REPLACE_DATA, deleg);
}

void MongodbProxy::_InitDirtyHelper(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper)
{
    dirtyHelper->Init(StorageMode::MAX_TYPE);

    auto deleg = KERNEL_NS::DelegateFactory::Create(this, &MongodbProxy::_OnStringAddDirtyHandler);
    dirtyHelper->SetHandler(StorageMode::ADD_DATA, deleg);
    
    deleg = KERNEL_NS::DelegateFactory::Create(this, &MongodbProxy::_OnStringModifyDirtyHandler);
    dirtyHelper->SetHandler(StorageMode::UPDATE_DATA, deleg);

    deleg = KERNEL_NS::DelegateFactory::Create(this, &MongodbProxy::_OnStringDeleteDirtyHandler);
    dirtyHelper->SetHandler(StorageMode::DEL_DATA, deleg);

    deleg = KERNEL_NS::DelegateFactory::Create(this, &MongodbProxy::_OnStringReplaceDirtyHandler);
    dirtyHelper->SetHandler(StorageMode::REPLACE_DATA, deleg);
}

// 多字段的脏回调
KERNEL_NS::CoTask<> MongodbProxy::_OnNumberAddDirtyHandler(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper, Int64 &key, KERNEL_NS::Variant *params)
{
    dirtyHelper->Clear(key, StorageMode::ADD_DATA);

    auto logic = (*params)[Params::VAR_LOGIC].AsPtr<ILogicSys>();
    if(UNLIKELY(!logic))
    {
        CLOG_ERROR("params is not logic sys please check, key:%lld", key);
        co_return;
    }

    KERNEL_NS::SmartPtr<std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *>, KERNEL_NS::AutoDelMethods::CustomDelete> dbInfo = new std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *>();
    dbInfo.SetClosureDelegate([](void *ptr)
    {
        auto p = KERNEL_NS::KernelCastTo<std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *>>(ptr);
        KERNEL_NS::ContainerUtil::DelContainer(*p, [](KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *ptr){
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(ptr);
        });
        CRYSTAL_DELETE_SAFE(p);
    });
    
    auto storageCom = logic->GetComp<IMongodbStorageInfo>();
    Int32 err = Status::Success;
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    if(storageCom->IsKvSystem())
    {
        // TODO:调用kv的OnSave接口  KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *
        // key:
        KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::Release> keyStream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
        keyStream->WriteInt64(key);
        KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::Release> valueStream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
        err = logic->OnSave(key, *valueStream);
        if (err != Status::Success)
        {
            CLOG_ERROR("logic save db fail err:%d, logic:%s, key:%lld", err, logic->GetObjName().c_str(), key);
            co_return;
        }

        dbInfo->emplace(storageCom->GetKeyFieldName(), keyStream.pop());
        dbInfo->emplace(storageCom->GetValueFieldName(), valueStream.pop());
        kv.emplace(storageCom->GetKeyFieldName(), KERNEL_NS::Variant(key));
    }
    else
    {
        err = logic->OnSave(key, *dbInfo.AsSelf());
        auto &uniqueIndexFields = storageCom->GetUniqueIndexFields();
        for(auto &iter : uniqueIndexFields)
        {
            kv.emplace(iter.first, KERNEL_NS::Variant(key));
            break;
        }
    }
    if(err != Status::Success)
    {
        CLOG_ERROR("logic save db fail err:%d, logic:%s, key:%lld", err, logic->GetObjName().c_str(), key);
        co_return;
    }

    if(kv.empty())
    {
        CLOG_ERROR("logic save db fail lack unique index define, logic:%s, key:%lld"
            , logic->GetObjName().c_str(), key);
        co_return;
    }

    KERNEL_NS::SmartMongoSerializeInfoWrapper dict;

    KERNEL_NS::LibString errInfo;
    for(auto iter = dbInfo->begin(); iter != dbInfo->end();)
    {
        auto &fieldName = iter->first;
        auto &fieldValue = iter->second;

        Int32 storageType = KERNEL_NS::MongoSerializeInfoType::UNKNOWN;
        auto &fieldNameRefStorageType = storageCom->GetFieldNameRefStorageType();
        auto iterFieldInfo = fieldNameRefStorageType.find(fieldName);
        if(iterFieldInfo != fieldNameRefStorageType.end())
        {
            storageType = iterFieldInfo->second;
        }

        // 没有定义该字段, 则从Proxy表信息找
        else
        {
            if(!_TryGetStorageType(logic, fieldName, storageType))
            {
                errInfo.AppendFormat("_TryGetStorageType fail, logic:%s, field name:%s when save data."
                    , logic->GetObjName().c_str(), fieldName.c_str());
                ++iter;
                continue;
            }
        }

        if(UNLIKELY(storageType == KERNEL_NS::MongoSerializeInfoType::UNKNOWN))
        {
            errInfo.AppendFormat("logic not define field storage type, logic:%s, field:%s when save data.", logic->GetObjName().c_str(), fieldName.c_str());
            ++iter;
            continue;
        }

        dict.Ptr->emplace(fieldName, KERNEL_NS::MongoSerializeInfo(storageType, fieldValue));
        iter = dbInfo->erase(iter);
    }

    if(UNLIKELY(!errInfo.empty()))
    {
        CLOG_ERROR("logic save data error logic:%s errInfo:\n%s", logic->GetObjName().c_str(), errInfo.c_str());
        co_return;
    }

    auto popDb = dict.Ptr.pop();
    const auto logicName = logic->GetObjName();
    auto ret = co_await GetComp<KERNEL_NS::IMongoDbMgr>()->AddData(storageCom->GetDbName(), storageCom->GetSystemName(), kv, popDb);

    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("logic AddData save data fail, logic:%s, kv:%s", logicName.c_str(), DictContainerToString(kv).c_str());
    }
}

KERNEL_NS::CoTask<> MongodbProxy::_OnNumberModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper, Int64 &key, KERNEL_NS::Variant *params)
{
    dirtyHelper->Clear(key, StorageMode::UPDATE_DATA);

    auto logic = (*params)[Params::VAR_LOGIC].AsPtr<ILogicSys>();
    if(UNLIKELY(!logic))
    {
        CLOG_ERROR("params is not logic sys please check, key:%lld", key);
        co_return;
    }

    KERNEL_NS::SmartPtr<std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *>, KERNEL_NS::AutoDelMethods::CustomDelete> dbInfo = new std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *>();
    dbInfo.SetClosureDelegate([](void *ptr)
    {
        auto p = KERNEL_NS::KernelCastTo<std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *>>(ptr);
        KERNEL_NS::ContainerUtil::DelContainer(*p, [](KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *ptr){
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(ptr);
        });
        CRYSTAL_DELETE_SAFE(p);
    });
    
    auto storageCom = logic->GetComp<IMongodbStorageInfo>();
    Int32 err = Status::Success;
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    if(storageCom->IsKvSystem())
    {
        // TODO:调用kv的OnSave接口  KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *
        // key:
        KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::Release> keyStream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
        keyStream->WriteInt64(key);
        KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::Release> valueStream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
        err = logic->OnSave(key, *valueStream);
        if (err != Status::Success)
        {
            CLOG_ERROR("logic save db fail err:%d, logic:%s, key:%lld", err, logic->GetObjName().c_str(), key);
            co_return;
        }

        dbInfo->emplace(storageCom->GetKeyFieldName(), keyStream.pop());
        dbInfo->emplace(storageCom->GetValueFieldName(), valueStream.pop());
        kv.emplace(storageCom->GetKeyFieldName(), KERNEL_NS::Variant(key));
    }
    else
    {
        err = logic->OnSave(key, *dbInfo.AsSelf());
        auto &uniqueIndexFields = storageCom->GetUniqueIndexFields();
        for(auto &iter : uniqueIndexFields)
        {
            kv.emplace(iter.first, KERNEL_NS::Variant(key));
            break;
        }
    }
    if(err != Status::Success)
    {
        CLOG_ERROR("logic save db fail err:%d, logic:%s, key:%lld", err, logic->GetObjName().c_str(), key);
        co_return;
    }

    if(kv.empty())
    {
        CLOG_ERROR("logic save db fail lack unique index define, logic:%s, key:%lld"
            , logic->GetObjName().c_str(), key);
        co_return;
    }

    KERNEL_NS::SmartMongoSerializeInfoWrapper dict;
    KERNEL_NS::LibString errInfo;
    for(auto iter = dbInfo->begin(); iter != dbInfo->end();)
    {
        auto &fieldName = iter->first;
        auto &fieldValue = iter->second;

        Int32 storageType = KERNEL_NS::MongoSerializeInfoType::UNKNOWN;
        auto &fieldNameRefStorageType = storageCom->GetFieldNameRefStorageType();
        auto iterFieldInfo = fieldNameRefStorageType.find(fieldName);
        if(iterFieldInfo != fieldNameRefStorageType.end())
        {
            storageType = iterFieldInfo->second;
        }

        // 没有定义该字段, 则从logic的组件找 TODO:在Proxy的表信息中找
        else
        {
            // Proxy表中找storageType
            if(!_TryGetStorageType(logic, fieldName, storageType))
            {
                errInfo.AppendFormat("_TryGetStorageType fail, logic:%s, field name:%s when save data."
                    , logic->GetObjName().c_str(), fieldName.c_str());
                ++iter;
                continue;
            }
        }

        if(UNLIKELY(storageType == KERNEL_NS::MongoSerializeInfoType::UNKNOWN))
        {
            errInfo.AppendFormat("logic not define field storage type, logic:%s, field:%s when save data.", logic->GetObjName().c_str(), fieldName.c_str());
            ++iter;
            continue;
        }

        dict.Ptr->emplace(fieldName, KERNEL_NS::MongoSerializeInfo(storageType, fieldValue));
        iter = dbInfo->erase(iter);
    }

    if(UNLIKELY(!errInfo.empty()))
    {
        CLOG_ERROR("logic save data error logic:%s errInfo:\n%s", logic->GetObjName().c_str(), errInfo.c_str());
        co_return;
    }

    auto popDb = dict.Ptr.pop();
    const auto logicName = logic->GetObjName();
    auto ret = co_await GetComp<KERNEL_NS::IMongoDbMgr>()->UpdateData(storageCom->GetDbName(), storageCom->GetSystemName(), kv, popDb, true);

    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("logic UpdateData save data fail, logic:%s, kv:%s", logicName.c_str(), DictContainerToString(kv).c_str());
    }
    co_return;
}

KERNEL_NS::CoTask<> MongodbProxy::_OnNumberDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper, Int64 &key, KERNEL_NS::Variant *params)
{
    dirtyHelper->Clear(key, StorageMode::DEL_DATA);
    auto logic = (*params)[Params::VAR_LOGIC].AsPtr<ILogicSys>();
    if(UNLIKELY(!logic))
    {
        CLOG_ERROR("params is not logic sys please check, key:%lld", key);
        co_return;
    }

    auto storageCom = logic->GetComp<IMongodbStorageInfo>();
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    auto &uniqueIndexFields = storageCom->GetUniqueIndexFields();
    for(auto &iter : uniqueIndexFields)
    {
        kv.emplace(iter.first, KERNEL_NS::Variant(key));
        break;
    }
    if(kv.empty())
    {
        CLOG_ERROR("logic save db fail lack unique index define, logic:%s, key:%lld"
            , logic->GetObjName().c_str(), key);
        co_return;
    }

    const auto logicName = logic->GetObjName();
    auto ret = co_await GetComp<KERNEL_NS::IMongoDbMgr>()->DelData(storageCom->GetDbName(), storageCom->GetSystemName(), kv);

    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("logic DelData save data fail, logic:%s, kv:%s", logicName.c_str(), DictContainerToString(kv).c_str());
    }
    co_return;
}

KERNEL_NS::CoTask<> MongodbProxy::_OnNumberReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper, Int64 &key, KERNEL_NS::Variant *params)
{
    dirtyHelper->Clear(key, StorageMode::REPLACE_DATA);
   auto logic = (*params)[Params::VAR_LOGIC].AsPtr<ILogicSys>();
    if(UNLIKELY(!logic))
    {
        CLOG_ERROR("params is not logic sys please check, key:%lld", key);
        co_return;
    }

    KERNEL_NS::SmartPtr<std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *>, KERNEL_NS::AutoDelMethods::CustomDelete> dbInfo = new std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *>();
    dbInfo.SetClosureDelegate([](void *ptr)
    {
        auto p = KERNEL_NS::KernelCastTo<std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *>>(ptr);
        KERNEL_NS::ContainerUtil::DelContainer(*p, [](KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *ptr){
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(ptr);
        });
        CRYSTAL_DELETE_SAFE(p);
    });
    auto storageCom = logic->GetComp<IMongodbStorageInfo>();
    Int32 err = Status::Success;
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    if(storageCom->IsKvSystem())
    {
        // TODO:调用kv的OnSave接口  KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *
        // key:
        KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::Release> keyStream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
        keyStream->WriteInt64(key);
        KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::Release> valueStream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
        err = logic->OnSave(key, *valueStream);
        if (err != Status::Success)
        {
            CLOG_ERROR("logic save db fail err:%d, logic:%s, key:%lld", err, logic->GetObjName().c_str(), key);
            co_return;
        }

        dbInfo->emplace(storageCom->GetKeyFieldName(), keyStream.pop());
        dbInfo->emplace(storageCom->GetValueFieldName(), valueStream.pop());
        kv.emplace(storageCom->GetKeyFieldName(), KERNEL_NS::Variant(key));
    }
    else
    {
        err = logic->OnSave(key, *dbInfo.AsSelf());
        auto &uniqueIndexFields = storageCom->GetUniqueIndexFields();
        for(auto &iter : uniqueIndexFields)
        {
            kv.emplace(iter.first, KERNEL_NS::Variant(key));
            break;
        }
    }
    if(err != Status::Success)
    {
        CLOG_ERROR("logic save db fail err:%d, logic:%s, key:%lld", err, logic->GetObjName().c_str(), key);
        co_return;
    }

    if(kv.empty())
    {
        CLOG_ERROR("logic save db fail lack unique index define, logic:%s, key:%lld"
            , logic->GetObjName().c_str(), key);
        co_return;
    }

    KERNEL_NS::SmartPtr<std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo>, KERNEL_NS::AutoDelMethods::CustomDelete> dict = new std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo>();
    dict.SetClosureDelegate([](void *arg)
    {
        auto dict = KERNEL_NS::KernelCastTo<std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo>>(arg);
        DelStreamContainer(dict);
    });

    KERNEL_NS::LibString errInfo;
    for(auto iter = dbInfo->begin(); iter != dbInfo->end();)
    {
        auto &fieldName = iter->first;
        auto &fieldValue = iter->second;

        Int32 storageType = KERNEL_NS::MongoSerializeInfoType::UNKNOWN;
        auto &fieldNameRefStorageType = storageCom->GetFieldNameRefStorageType();
        auto iterFieldInfo = fieldNameRefStorageType.find(fieldName);
        if(iterFieldInfo != fieldNameRefStorageType.end())
        {
            storageType = iterFieldInfo->second;
        }

        // 没有定义该字段, 则从logic的组件找
        else
        {
            // Proxy表中找storageType
            if(!_TryGetStorageType(logic, fieldName, storageType))
            {
                errInfo.AppendFormat("_TryGetStorageType fail, logic:%s, field name:%s when save data."
                    , logic->GetObjName().c_str(), fieldName.c_str());
                ++iter;
                continue;
            }
        }

        if(UNLIKELY(storageType == KERNEL_NS::MongoSerializeInfoType::UNKNOWN))
        {
            errInfo.AppendFormat("logic not define field storage type, logic:%s, field:%s when save data.", logic->GetObjName().c_str(), fieldName.c_str());
            ++iter;
            continue;
        }

        dict->emplace(fieldName, KERNEL_NS::MongoSerializeInfo(storageType, fieldValue));
        iter = dbInfo->erase(iter);
    }

    if(UNLIKELY(!errInfo.empty()))
    {
        CLOG_ERROR("logic save data error logic:%s errInfo:\n%s", logic->GetObjName().c_str(), errInfo.c_str());
        co_return;
    }

    auto popDb = dict.pop();
    const auto logicName = logic->GetObjName();
    auto ret = co_await GetComp<KERNEL_NS::IMongoDbMgr>()->ReplaceData(storageCom->GetDbName(), storageCom->GetSystemName(), kv, popDb);
    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("logic ReplaceData save data fail, logic:%s, kv:%s", logicName.c_str(), DictContainerToString(kv).c_str());
    }
    co_return;
}

KERNEL_NS::CoTask<> MongodbProxy::_OnStringAddDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    // TODO:
    co_return;
}

KERNEL_NS::CoTask<> MongodbProxy::_OnStringModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    // TODO:
    co_return;
}

KERNEL_NS::CoTask<> MongodbProxy::_OnStringDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    // TODO:
    co_return;
}

KERNEL_NS::CoTask<> MongodbProxy::_OnStringReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    // TODO:
    co_return;
}

KERNEL_NS::CoTask<> MongodbProxy::_DorPurgeNumber(const ILogicSys *sys)
{
    auto iterHelper = _logicRefNumberDirtyHelper.find(sys);
    if(iterHelper == _logicRefNumberDirtyHelper.end())
        co_return;

    auto dirtyHelper = iterHelper->second;
    auto &allMasks = dirtyHelper->GetAllMasks();
    for(auto iterMask : allMasks)
    {
        auto mask = iterMask.second;
        for(Int32 idx = StorageMode::BEGIN; idx < StorageMode::MAX_TYPE; ++idx)
        {
            if(!mask->IsDirty(idx))
                continue;

            auto var = mask->GetVar(idx);
            if(!var)
                var = mask->AddVar(idx);
        }
    }

    // 清洗
    KERNEL_NS::LibString err;
    auto handled = co_await dirtyHelper->PurgeCo(&err);
    if(!err.empty())
    {
        CLOG_ERROR("purge abnormal handled:%lld, err:%s, logic:%s, service:%s"
            , handled, err.c_str(), sys->GetObjName().c_str(), GetService()->ToString().c_str());
    }

    CLOG_DEBUG("purge logic:%s completed handled:%lld", sys->GetObjName().c_str(), handled);
    
    // 还有脏标记,放回去
    if(dirtyHelper->HasDirty())
    {
        if(_dirtyLogicRefIsNumberKey.find(sys) == _dirtyLogicRefIsNumberKey.end())
        {
            _dirtyLogicRefIsNumberKey.insert(std::make_pair(sys, true));
        }
    }
}

KERNEL_NS::CoTask<> MongodbProxy::_DorPurgeString(const ILogicSys *sys)
{
    auto iterHelper = _logicRefStringDirtyHelper.find(sys);
    if(iterHelper == _logicRefStringDirtyHelper.end())
        co_return;

    auto dirtyHelper = iterHelper->second;
    auto &allMasks = dirtyHelper->GetAllMasks();
    for(auto iterMask : allMasks)
    {
        auto mask = iterMask.second;
        for(Int32 idx = StorageMode::BEGIN; idx < StorageMode::MAX_TYPE; ++idx)
        {
            if(!mask->IsDirty(idx))
                continue;

            auto var = mask->GetVar(idx);
            if(!var)
                var = mask->AddVar(idx);
        }
    }

    // 清洗
    KERNEL_NS::LibString err;
    auto handled = co_await dirtyHelper->PurgeCo(&err);
    if(!err.empty())
    {
        CLOG_ERROR("purge abnormal handled:%lld, err:%s, logic:%s, service:%s"
            , handled, err.c_str(), sys->GetObjName().c_str(), GetService()->ToString().c_str());
    }

    CLOG_DEBUG("purge logic:%s completed handled:%lld", sys->GetObjName().c_str(), handled);

    // 还有脏标记,放回去
    if(dirtyHelper->HasDirty())
    {
        if(_dirtyLogicRefIsNumberKey.find(sys) == _dirtyLogicRefIsNumberKey.end())
        {
            _dirtyLogicRefIsNumberKey.insert(std::make_pair(sys, false));
        }
    }
}

bool MongodbProxy::_CheckLogic(const ILogicSys *logic, KERNEL_NS::LibString &err) const
{
    auto storageComp = logic->GetComp<SERVICE_NS::IMongodbStorageInfo>();
    if(!storageComp)
    {
        return false;
    }

    if (!CheckValidStorage(logic, storageComp, false, err))
    {
        return false;
    }

    // 如果是逻辑系统, 它的组件如果需要存储的需要验证下参数, 存储深度最多两个层级, 组件的子组件会作为其第一层存储的一个字段
    auto turnLogic = logic->CastTo<SERVICE_NS::ILogicSys>();
    auto &subComps = turnLogic->GetAllComps();
    for (auto &subComp : subComps)
    {
        if(UNLIKELY(!subComp))
            continue;

        if(!subComp->IsKernelObjType(KERNEL_NS::KernelObjectType::HOST_COMP))
            continue;

        auto subTurnHost = subComp->CastTo<KERNEL_NS::CompHostObject>();
        auto subStorageComp = subTurnHost->GetComp<SERVICE_NS::IMongodbStorageInfo>();
        if(!subStorageComp)
            continue;

        if (!CheckValidStorage(logic, subStorageComp, true, err))
            continue;
    }

    // 表信息建立在storageComp中, 比如UserMgr, User是运行时动态创建的, 其子系统在启动的时候还没建立智能放在UserMgr的StorageComp作为组件存在
    {
        auto &storageSubComps = storageComp->GetAllComps();
        for(auto subSubComp : storageSubComps)
        {
            // 是不是存储组件
            if(subSubComp->GetInterfaceTypeId() != KERNEL_NS::RttiUtil::GetTypeId<SERVICE_NS::IMongodbStorageInfo>())
                continue;

            auto subSubStorageComp = subSubComp->CastTo<SERVICE_NS::IMongodbStorageInfo>();
            CheckValidStorage(logic, subSubStorageComp, true, err);
        }
    }

    return err.empty();
}

void MongodbProxy::_BuildSysFields(const ILogicSys *logic)
{
    auto storageComp = logic->GetComp<SERVICE_NS::IMongodbStorageInfo>();
    if(UNLIKELY(!storageComp))
    {
        CLOG_WARN("_BuildSysFields fail logic:%s have no storage comp", logic->GetObjName().c_str());
        return;
    }
    
    auto iterFieldRefStorageType = _sysRefFieldStorageType.emplace(logic, std::unordered_map<KERNEL_NS::LibString, Int32>()).first;
    auto &fieldRefStorageType = iterFieldRefStorageType->second;

    auto &fieldNameRefStorageType = storageComp->GetFieldNameRefStorageType();
    for(auto &iterField : fieldNameRefStorageType)
    {
        fieldRefStorageType.emplace(iterField.first, iterField.second);
    }

    auto &subComps = logic->GetAllComps();
    for(auto &subComp : subComps)
    {
        if(UNLIKELY(!subComp))
            continue;

        if(!subComp->IsKernelObjType(KERNEL_NS::KernelObjectType::HOST_COMP))
            continue;

        auto subHost = subComp->CastTo<KERNEL_NS::CompHostObject>();
        auto subStorageComp = subHost->GetComp<SERVICE_NS::IMongodbStorageInfo>();
        if(!subStorageComp)
            continue;

        fieldRefStorageType[subStorageComp->GetSystemName()] = subStorageComp->GetStorageType();
    }

    // 表信息建立在storageComp中, 比如UserMgr, User是运行时动态创建的, 其子系统在启动的时候还没建立智能放在UserMgr的StorageComp作为组件存在
    {
        auto &storageSubComps = storageComp->GetAllComps();
        for(auto subSubComp : storageSubComps)
        {
            // 是不是存储组件
            if(subSubComp->GetInterfaceTypeId() != KERNEL_NS::RttiUtil::GetTypeId<SERVICE_NS::IMongodbStorageInfo>())
                continue;
            
            auto subSubStorageComp = subSubComp->CastTo<SERVICE_NS::IMongodbStorageInfo>();
            fieldRefStorageType[subSubStorageComp->GetSystemName()] = subSubStorageComp->GetStorageType();
        }
    }
}

bool MongodbProxy::_TryGetStorageType(const ILogicSys *logic, const KERNEL_NS::LibString &fieldName, Int32 &storageType) const
{
    auto iter = _sysRefFieldStorageType.find(logic);
    if(iter == _sysRefFieldStorageType.end())
        return false;

    auto &fieldRefStorageType = iter->second;
    auto iterField = fieldRefStorageType.find(fieldName);
    if(iterField == fieldRefStorageType.end())
        return false;

    storageType = iterField->second;
    return true;
}



SERVICE_END

