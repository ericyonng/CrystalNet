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

ALWAYS_INLINE static bool CheckValidStorage(KERNEL_NS::CompHostObject *turnHost, SERVICE_NS::IMongodbStorageInfo *storageComp, bool systemAsField, KERNEL_NS::LibString &errInfo)
{
    // 检查storageComp合法性

    // if(storageComp->_collectionName.empty())
    // {
    //     storageComp->_collectionName = turnHost->GetObjName();
    // }

    // 系统整体作为表的一个字段, 不用检查索引和_fieldNameRefStorageType, 以及dbName, 只需要检查_storageType
    bool ret = true;
    if (!systemAsField)
    {
        if(storageComp->_dbName.empty())
        {
            errInfo.AppendFormat("logic :%s lack db name\n", turnHost->GetObjName().c_str());
            ret = false;
        }
        
        if(storageComp->_uniqueIndexFields.empty())
        {
            errInfo.AppendFormat("logic :%s lack unique index fields\n", turnHost->GetObjName().c_str());
            ret = false;
        }

        if(storageComp->_fieldNameRefStorageType.empty() && storageComp->_storageType == 0)
        {
            errInfo.AppendFormat("logic :%s lack _fieldNameRefStorageType and _storageType\n", turnHost->GetObjName().c_str());
            ret = false;
        }
    }
    else
    {
        if (storageComp->_storageType == KERNEL_NS::MongoSerializeInfoType::UNKNOWN)
        {
            errInfo.AppendFormat("logic :%s lack sub comp:%s lack storage type, storageComp:%s\n", turnHost->GetObjName().c_str(), storageComp->GetOwner()->GetObjName().c_str(), storageComp->GetObjName().c_str());
            ret = false;
        }
    }

    return ret;
}

Int32 MongodbProxy::_OnHostStart()
{
    auto mongodbMgr = GetComp<KERNEL_NS::IMongoDbMgr>();
    // 扫码所有的系统注册依赖
    auto &comps = GetService()->GetAllComps();
    KERNEL_NS::LibString errInfo;
    for(auto comp : comps)
    {
        if(UNLIKELY(!comp))
            continue;

        if(!comp->IsKernelObjType(KERNEL_NS::KernelObjectType::HOST_COMP))
            continue;

        auto turnHost = comp->CastTo<KERNEL_NS::CompHostObject>();
        auto storageComp = turnHost->GetComp<SERVICE_NS::IMongodbStorageInfo>();
        if(!storageComp)
            continue;

        if (!CheckValidStorage(turnHost, storageComp, false, errInfo))
            continue;

        if(turnHost->GetType() != ServiceCompType::LOGIC_SYS)
            continue;

        // 如果是逻辑系统, 它的组件如果需要存储的需要验证下参数, 存储深度最多两个层级, 组件的子组件会作为其第一层存储的一个字段
        auto turnLogic = turnHost->CastTo<SERVICE_NS::ILogicSys>();
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

            if (!CheckValidStorage(turnHost, subStorageComp, true, errInfo))
                continue;
        }
    }

    if(!errInfo.empty())
    {
        CLOG_ERROR("check logics storage fail\n :%s", errInfo.c_str());
        return Status::ConfigError;
    }
    
    for(auto comp : comps)
    {
        if(UNLIKELY(!comp))
            continue;

        if(!comp->IsKernelObjType(KERNEL_NS::KernelObjectType::HOST_COMP))
            continue;

        auto turnHost = comp->CastTo<KERNEL_NS::CompHostObject>();
        auto storageComp = turnHost->GetComp<SERVICE_NS::IMongodbStorageInfo>();
        if(!storageComp)
            continue;

        // 是逻辑系统
        if(turnHost->GetType() != ServiceCompType::LOGIC_SYS)
            continue;

        auto turnLogic = turnHost->CastTo<SERVICE_NS::ILogicSys>();
        RegisterDependence(turnLogic);

        // 索引信息(稳定排序)
        std::set<KERNEL_NS::LibString> fields;
        for(auto &item : storageComp->_uniqueIndexFields)
            fields.insert(item.first);
        
        KERNEL_NS::LibString indexName = "unique_index_";
        const Int32 count = static_cast<Int32>(fields.size());
        Int32 idx = 0;
        for(auto &item : fields)
        {
            auto &&strip = item.strip();
            strip = strip.tolower();
            ++idx;
            indexName.AppendFormat("%s", strip.c_str());
            if(idx != (count - 1))
            {
                indexName.Append("_");
            }
        }
        // 没填写collection的用类名
        if(storageComp->_collectionName.empty())
            storageComp->_collectionName = turnLogic->GetObjName();
        mongodbMgr->CreateIndex(storageComp->_dbName, storageComp->_collectionName, indexName, storageComp->_uniqueIndexFields, true);
    }

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
    KERNEL_NS::RunRightNow([this]()->KERNEL_NS::CoTask<>
    {
        co_await Purge();
    });

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
    auto err = logic->OnSave(key, *dbInfo.AsSelf());
    if(err != Status::Success)
    {
        CLOG_ERROR("logic save db fail err:%d, logic:%s, key:%lld", err, logic->GetObjName().c_str(), key);
        co_return;
    }

    auto storageCom = logic->GetComp<IMongodbStorageInfo>();
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    for(auto &iter : storageCom->_uniqueIndexFields)
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
        auto iterFieldInfo = storageCom->_fieldNameRefStorageType.find(fieldName);
        if(iterFieldInfo != storageCom->_fieldNameRefStorageType.end())
        {
            storageType = iterFieldInfo->second;
        }

        // 没有定义该字段, 则从logic的组件找
        else
        {
            // 默认认为是ObjName
            auto comp = logic->GetCompByName(fieldName);
            if(UNLIKELY(comp == NULL))
            {
                errInfo.AppendFormat("logic unknown field, logic:%s, field name:%s when save data."
                    , logic->GetObjName().c_str(), fieldName.c_str());
                ++iter;
                continue;
            }

            if(UNLIKELY(!comp->IsKernelObjType(KERNEL_NS::KernelObjectType::HOST_COMP)))
            {
                errInfo.AppendFormat("logic comp:%s is not host comp, logic:%s, field name:%s when save data."
                , comp->GetObjName().c_str(), logic->GetObjName().c_str(), fieldName.c_str());
                ++iter;
                continue;
            }

            // 子系统默认只作为母系统的一个字段存储
            auto subCompStorageComp = comp->CastTo<KERNEL_NS::CompHostObject>()->GetComp<SERVICE_NS::IMongodbStorageInfo>();
            storageType = subCompStorageComp->_storageType;
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
    auto ret = co_await GetComp<KERNEL_NS::IMongoDbMgr>()->AddData(storageCom->_dbName, storageCom->_collectionName, kv, popDb);

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
    auto err = logic->OnSave(key, *dbInfo.AsSelf());
    if(err != Status::Success)
    {
        CLOG_ERROR("logic save db fail err:%d, logic:%s, key:%lld", err, logic->GetObjName().c_str(), key);
        co_return;
    }

    auto storageCom = logic->GetComp<IMongodbStorageInfo>();
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    for(auto &iter : storageCom->_uniqueIndexFields)
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
        auto iterFieldInfo = storageCom->_fieldNameRefStorageType.find(fieldName);
        if(iterFieldInfo != storageCom->_fieldNameRefStorageType.end())
        {
            storageType = iterFieldInfo->second;
        }

        // 没有定义该字段, 则从logic的组件找
        else
        {
            // 默认认为是ObjName
            auto comp = logic->GetCompByName(fieldName);
            if(UNLIKELY(comp == NULL))
            {
                errInfo.AppendFormat("logic unknown field, logic:%s, field name:%s when save data."
                    , logic->GetObjName().c_str(), fieldName.c_str());
                ++iter;
                continue;
            }

            if(UNLIKELY(!comp->IsKernelObjType(KERNEL_NS::KernelObjectType::HOST_COMP)))
            {
                errInfo.AppendFormat("logic comp:%s is not host comp, logic:%s, field name:%s when save data."
                , comp->GetObjName().c_str(), logic->GetObjName().c_str(), fieldName.c_str());
                ++iter;
                continue;
            }

            // 子系统默认只作为母系统的一个字段存储
            auto subCompStorageComp = comp->CastTo<KERNEL_NS::CompHostObject>()->GetComp<SERVICE_NS::IMongodbStorageInfo>();
            storageType = subCompStorageComp->_storageType;
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
    auto ret = co_await GetComp<KERNEL_NS::IMongoDbMgr>()->UpdateData(storageCom->_dbName, storageCom->_collectionName, kv, popDb, true);

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
    for(auto &iter : storageCom->_uniqueIndexFields)
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
    auto ret = co_await GetComp<KERNEL_NS::IMongoDbMgr>()->DelData(storageCom->_dbName, storageCom->_collectionName, kv);

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
    auto err = logic->OnSave(key, *dbInfo.AsSelf());
    if(err != Status::Success)
    {
        CLOG_ERROR("logic save db fail err:%d, logic:%s, key:%lld", err, logic->GetObjName().c_str(), key);
        co_return;
    }

    auto storageCom = logic->GetComp<IMongodbStorageInfo>();
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    for(auto &iter : storageCom->_uniqueIndexFields)
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
        auto iterFieldInfo = storageCom->_fieldNameRefStorageType.find(fieldName);
        if(iterFieldInfo != storageCom->_fieldNameRefStorageType.end())
        {
            storageType = iterFieldInfo->second;
        }

        // 没有定义该字段, 则从logic的组件找
        else
        {
            // 默认认为是ObjName
            auto comp = logic->GetCompByName(fieldName);
            if(UNLIKELY(comp == NULL))
            {
                errInfo.AppendFormat("logic unknown field, logic:%s, field name:%s when save data."
                    , logic->GetObjName().c_str(), fieldName.c_str());
                ++iter;
                continue;
            }

            if(UNLIKELY(!comp->IsKernelObjType(KERNEL_NS::KernelObjectType::HOST_COMP)))
            {
                errInfo.AppendFormat("logic comp:%s is not host comp, logic:%s, field name:%s when save data."
                , comp->GetObjName().c_str(), logic->GetObjName().c_str(), fieldName.c_str());
                ++iter;
                continue;
            }

            // 子系统默认只作为母系统的一个字段存储
            auto subCompStorageComp = comp->CastTo<KERNEL_NS::CompHostObject>()->GetComp<SERVICE_NS::IMongodbStorageInfo>();
            storageType = subCompStorageComp->_storageType;
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
    auto ret = co_await GetComp<KERNEL_NS::IMongoDbMgr>()->ReplaceData(storageCom->_dbName, storageCom->_collectionName, kv, popDb);
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

SERVICE_END

