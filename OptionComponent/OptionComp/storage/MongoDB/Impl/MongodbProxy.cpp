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

#include <OptionComp/storage/MongoDB/Impl/MongodbProxy.h>
#include <OptionComp/storage/MongoDB/Impl/MongodbProxyFactory.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <OptionComp/storage/MongoDB/Impl/MongoDbMgrFactory.h>
#include <OptionComp/storage/MongoDB/Interface/IMongoDbMgr.h>
#include <kernel/comp/Timer/LibTimer.h>
#include <kernel/comp/Event/EventManager.h>

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


KERNEL_BEGIN

MongodbProxy::MongodbProxy()
    :IMongodbProxy(KERNEL_NS::RttiUtil::GetTypeId<MongodbProxy>())
    ,_purgeTimer(KERNEL_NS::LibTimer::NewThreadLocal_LibTimer())
    ,_options(new KERNEL_NS::FileMonitor<KERNEL_NS::MongodbConfig, KERNEL_NS::YamlDeserializer>())
    ,_checkQuit(false)
    ,_mongodbMgr(NULL)
    ,_checkDependenceQuit(NULL)
    ,_maskSelfQuit(NULL)
    ,_registerFocus(NULL)
    ,_closeEventStub(INVALID_LISTENER_STUB)
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

// 标脏
void MongodbProxy::MaskLogicNumberKeyAddDirty(const CompHostObject *logic, Int64 key)
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
    var->BecomePtr() = logic;
}
void MongodbProxy::MaskLogicNumberKeyModifyDirty(const CompHostObject *logic, Int64 key)
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
    var->BecomePtr() = logic;

    if(_dirtyLogicRefIsNumberKey.find(logic) == _dirtyLogicRefIsNumberKey.end())
        _dirtyLogicRefIsNumberKey.insert(std::make_pair(logic, true));
}

void MongodbProxy::MaskLogicNumberKeyDeleteDirty(const CompHostObject *logic, Int64 key)
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
    var->BecomePtr() = logic;

    if(_dirtyLogicRefIsNumberKey.find(logic) == _dirtyLogicRefIsNumberKey.end())
        _dirtyLogicRefIsNumberKey.insert(std::make_pair(logic, true));
}
    
void MongodbProxy::MaskLogicStringKeyAddDirty(const CompHostObject *logic, const KERNEL_NS::LibString &key)
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
    var->BecomePtr() = logic;

    if(_dirtyLogicRefIsNumberKey.find(logic) == _dirtyLogicRefIsNumberKey.end())
        _dirtyLogicRefIsNumberKey.insert(std::make_pair(logic, false));
}

void MongodbProxy::MaskLogicStringKeyModifyDirty(const CompHostObject *logic, const KERNEL_NS::LibString &key)
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
    var->BecomePtr() = logic;
    
    if(_dirtyLogicRefIsNumberKey.find(logic) == _dirtyLogicRefIsNumberKey.end())
        _dirtyLogicRefIsNumberKey.insert(std::make_pair(logic, false));
}

void MongodbProxy::MaskLogicStringKeyDeleteDirty(const CompHostObject *logic, const KERNEL_NS::LibString &key)
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
    var->BecomePtr() = logic;

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
KERNEL_NS::CoTask<> MongodbProxy::Purge(const CompHostObject *logic)
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

KERNEL_NS::CoTask<bool> MongodbProxy::Query(const CompHostObject *logic, Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult)
{
    if(UNLIKELY(!logic || !fieldNameRefDataResult))
    {
        CLOG_ERROR("query fail logic or fieldNameRefDataResult is null logic:%s, key:%lld", logic ? logic->GetObjName().c_str() : "", key);
        co_return false;
    }
    
    auto storageComp = logic->GetComp<IMongodbStorageInfo>();
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
    auto ret = co_await _mongodbMgr->Query(dbName, storageComp->GetSystemName(), kv, fieldNameRefDataResult);
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
    
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    kv.emplace(keyName, KERNEL_NS::Variant(key));
    auto ret = co_await _mongodbMgr->Query(dbName, collectionName, kv, fieldNameRefDataResult);
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
    
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    kv.emplace(keyName, KERNEL_NS::Variant(key));
    auto ret = co_await _mongodbMgr->Query(dbName, collectionName, kv, fieldNameRefDataResult);
    if(UNLIKELY(!ret))
    {
        CLOG_DEBUG("query data fail db:%s, collection:%s, kv:%s", dbName.c_str(), collectionName.c_str(), DictContainerToString(kv).c_str());
    }

    co_return ret;
}

void MongodbProxy::SetMongodbMgr(KERNEL_NS::IMongoDbMgr *mongodbMgr)
{
    _mongodbMgr = mongodbMgr;
}

void MongodbProxy::ListenClose(KERNEL_NS::EventManager *eventMgr, Int32 eventType)
{
    _closeEventStub = eventMgr->AddListener(eventType, this, &MongodbProxy::_OnCloseEvent);
}

void MongodbProxy::SetCheckDependenceQuit(KERNEL_NS::IDelegate<bool, const KERNEL_NS::CompHostObject *> *cb)
{
    CRYSTAL_RELEASE_SAFE(_checkDependenceQuit);
    _checkDependenceQuit = cb;
}

void MongodbProxy::SetMongoProxyMaskQuit(KERNEL_NS::IDelegate<void, const KERNEL_NS::CompHostObject *> *cb)
{
    CRYSTAL_RELEASE_SAFE(_maskSelfQuit);
    _maskSelfQuit = cb;
}

void MongodbProxy::SetRegisterFocus(KERNEL_NS::IDelegate<void, const KERNEL_NS::CompHostObject *> *cb)
{
    CRYSTAL_RELEASE_SAFE(_registerFocus);
    _registerFocus = cb;
}


Int32 MongodbProxy::_OnHostInit()
{
    if(!_mongodbMgr)
    {
        CLOG_ERROR("mongodb mgr is null.");
        return Status::ConfigError;
    }

    if(!_checkDependenceQuit)
    {
        CLOG_ERROR("_checkDependenceQuit is null.");

        return Status::ConfigError;
    }

    if(!_maskSelfQuit)
    {
        CLOG_ERROR("_maskSelfQuit is null.");
        return Status::ConfigError;
    }

    if(!_registerFocus)
    {
        CLOG_ERROR("_registerFocus is null.");

        return Status::ConfigError;
    }

    if(_closeEventStub == INVALID_LISTENER_STUB)
    {
        CLOG_ERROR("_closeEventStub is invalid, please let proxy listen close.");
        return Status::ConfigError;
    }

    _configSource = _mongodbMgr->GetConfigSource();
    _keyName = _mongodbMgr->GetConfigSourceKeyName();
    
    if(_keyName.empty())
    {
        CLOG_ERROR("mongodb options key name empty fail");

        return Status::ConfigError;
    }

    if(!_options->Init(&_configSource, _keyName))
    {
        CLOG_ERROR("mongodb options init fail keyName:%s", _keyName.c_str());
        return Status::ConfigError;
    }

    _purgeTimer->SetTimeOutHandler(this, &MongodbProxy::_OnPurgeTimer);
    const auto interval = _options->Current()->PurgeInterval;
    _purgeTimer->Schedule(interval);

    // 让Host关注自己
    _registerFocus->Invoke(this);
    
    return Status::Success;
}
Int32 MongodbProxy::_OnCompsCreated()
{
    return Status::Success;
}

ALWAYS_INLINE static bool CheckValidStorage(const KERNEL_NS::CompHostObject *turnHost, const IMongodbStorageInfo *storageComp, bool systemAsField, KERNEL_NS::LibString &errInfo)
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

Int32 MongodbProxy::_OnHostWillStart()
{
    // 扫码所有的系统注册依赖
    auto &comps = GetOwner()->CastTo<KERNEL_NS::CompHostObject>()->GetAllComps();
    KERNEL_NS::LibString errInfo;
    for(auto comp : comps)
    {
        if(UNLIKELY(!comp))
            continue;

        if(!comp->IsKernelObjType(KernelObjectType::HOST_COMP))
            continue;

        auto turnHost = comp->CastTo<KERNEL_NS::CompHostObject>();
        _CheckLogic(turnHost, errInfo);
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

        if(!comp->IsKernelObjType(KernelObjectType::HOST_COMP))
            continue;

        auto turnHost = comp->CastTo<CompHostObject>();
        auto storageComp = turnHost->GetComp<IMongodbStorageInfo>();
        if(!storageComp)
            continue;

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
        // 如果是复制集模式, 则不设置分片键
        if(_options->Current()->ReplicaSetName.empty())
        {
            auto &shardKeyInfo = storageComp->GetShardKeyInfoGroup();
            if(!shardKeyInfo.ShardKeyInfos.empty())
            {
                if(!_mongodbMgr->SetShardKeyInfo(storageComp->GetDbName(), storageComp->GetSystemName(), shardKeyInfo.ShardKeyInfos, shardKeyInfo.IsUnique))
                {
                    errInfo.AppendFormat("SetShardKeyInfo fail db:%s, collection:%s shard key info:%s logic:%s\n"
                        , storageComp->GetDbName().c_str(), storageComp->GetSystemName().c_str(), shardKeyInfo.ToString().c_str(), turnHost->GetObjName().c_str());
                }
            }
        }

        // 索引
        if(!_mongodbMgr->CreateIndex(storageComp->GetDbName(), storageComp->GetSystemName(), indexName, storageComp->GetUniqueIndexFields(), true))
        {
            errInfo.AppendFormat("CreateIndex fail db:%s, collection:%s indexName:%s logic:%s\n", storageComp->GetDbName().c_str(), storageComp->GetSystemName().c_str(), indexName.c_str(), turnHost->GetObjName().c_str());
            continue;
        }
        
        // 普通索引
        auto &indexDict = storageComp->GetNormalIndexDict();
        for (auto &iterDict : indexDict)
        {
            auto &dictIndexName = iterDict.first;
            auto &indexInfo = iterDict.second;
            if(!_mongodbMgr->CreateIndex(storageComp->GetDbName(), storageComp->GetSystemName(), dictIndexName, indexInfo.Fields, indexInfo.Unique))
            {
                errInfo.AppendFormat("CreateIndex fail db:%s, collection:%s indexName:%s logic:%s\n", storageComp->GetDbName().c_str(), storageComp->GetSystemName().c_str(), dictIndexName.c_str(), turnHost->GetObjName().c_str());
            }
        }
        
        // 记录表信息
        _BuildSysFields(turnHost);

        // 注册依赖
        _dependence.insert(turnHost);
    }

    if(!errInfo.empty())
    {
        CLOG_ERROR("mongodb proxy start fail errInfo:\n%s", errInfo.c_str());
        return Status::ConfigError;
    }

    const auto &dependenceInfo = KERNEL_NS::StringUtil::ToStringBy(_dependence, ',', [](KERNEL_NS::CompHostObject *p)->KERNEL_NS::LibString
    {
        return p->GetObjName();
    });

    CLOG_INFO("mongodb proxy dependence:%s", dependenceInfo.c_str());
    
    return Status::Success;
}

Int32 MongodbProxy::_OnHostStart()
{
    return Status::Success;
}

void MongodbProxy::_OnHostClose()
{
    _Clear();
}

void MongodbProxy::_OnCloseEvent(KERNEL_NS::LibEvent *ev)
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

    CLOG_INFO("mongo proxy will check quit...");
    _checkQuit = true;
    
    KERNEL_NS::RunRightNow([this]()->KERNEL_NS::CoTask<>
    {
        // 先执行全部脏持久化
        co_await Purge();

        do
        {
            // 检查依赖是否退出
            for(auto iter = _dependence.begin(); iter != _dependence.end(); )
            {
                auto dependence = *iter;
                if(!_checkDependenceQuit->Invoke(dependence))
                {
                    CLOG_INFO("wait mongodb dependence:%s quit...", dependence->GetObjName().c_str());
                    ++iter;
                    continue;
                }
                iter = _dependence.erase(iter);
            }

            co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
        }
        while (!_dependence.empty());

        CLOG_INFO("all dependence quit completed will purge again...");
        if(!_dirtyLogicRefIsNumberKey.empty())
            co_await Purge();
        
        CLOG_INFO("all dependence quit completed purge completed, mongodb proxy do quit work completed.");
        _maskSelfQuit->Invoke(this);

        _Clear();
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
    CRYSTAL_RELEASE_SAFE(_checkDependenceQuit);
    CRYSTAL_RELEASE_SAFE(_maskSelfQuit);
    CRYSTAL_RELEASE_SAFE(_registerFocus);
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

    auto logic = params->AsPtr<KERNEL_NS::CompHostObject>();
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
        // key
        KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::Release> keyStream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
        keyStream->WriteInt64(key);
        // value
        KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::Release> valueStream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
        err = storageCom->OnSave(logic, key, *valueStream);
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
        err = storageCom->OnSave(logic, key, *dbInfo.AsSelf());
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
    auto ret = co_await _mongodbMgr->AddData(storageCom->GetDbName(), storageCom->GetSystemName(), kv, popDb);

    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("logic AddData save data fail, logic:%s, kv:%s", logicName.c_str(), DictContainerToString(kv).c_str());
    }
}

KERNEL_NS::CoTask<> MongodbProxy::_OnNumberModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper, Int64 &key, KERNEL_NS::Variant *params)
{
    dirtyHelper->Clear(key, StorageMode::UPDATE_DATA);

    auto logic = params->AsPtr<CompHostObject>();
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
        err = storageCom->OnSave(logic, key, *valueStream);
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
        err = storageCom->OnSave(logic, key, *dbInfo.AsSelf());
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
    auto ret = co_await _mongodbMgr->UpdateData(storageCom->GetDbName(), storageCom->GetSystemName(), kv, popDb, true);

    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("logic UpdateData save data fail, logic:%s, kv:%s", logicName.c_str(), DictContainerToString(kv).c_str());
    }
    co_return;
}

KERNEL_NS::CoTask<> MongodbProxy::_OnNumberDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper, Int64 &key, KERNEL_NS::Variant *params)
{
    dirtyHelper->Clear(key, StorageMode::DEL_DATA);
    auto logic = params->AsPtr<CompHostObject>();
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
    auto ret = co_await _mongodbMgr->DelData(storageCom->GetDbName(), storageCom->GetSystemName(), kv);

    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("logic DelData save data fail, logic:%s, kv:%s", logicName.c_str(), DictContainerToString(kv).c_str());
    }
    co_return;
}

KERNEL_NS::CoTask<> MongodbProxy::_OnNumberReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper, Int64 &key, KERNEL_NS::Variant *params)
{
   dirtyHelper->Clear(key, StorageMode::REPLACE_DATA);
   auto logic = params->AsPtr<CompHostObject>();
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
        err = storageCom->OnSave(logic, key, *valueStream);
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
        err = storageCom->OnSave(logic, key, *dbInfo.AsSelf());
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
    auto ret = co_await _mongodbMgr->ReplaceData(storageCom->GetDbName(), storageCom->GetSystemName(), kv, popDb);
    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("logic ReplaceData save data fail, logic:%s, kv:%s", logicName.c_str(), DictContainerToString(kv).c_str());
    }
    co_return;
}

KERNEL_NS::CoTask<> MongodbProxy::_OnStringAddDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    dirtyHelper->Clear(key, StorageMode::ADD_DATA);
    auto logic = params->AsPtr<CompHostObject>();
    if(UNLIKELY(!logic))
    {
        CLOG_ERROR("params is not logic sys please check, key:%s", key.c_str());
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
        keyStream->Write(key);
        KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::Release> valueStream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
        err = storageCom->OnSave(logic, key, *valueStream);
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
        err = storageCom->OnSave(logic, key, *dbInfo.AsSelf());
        auto &uniqueIndexFields = storageCom->GetUniqueIndexFields();
        for(auto &iter : uniqueIndexFields)
        {
            kv.emplace(iter.first, KERNEL_NS::Variant(key));
            break;
        }
    }
    if(err != Status::Success)
    {
        CLOG_ERROR("logic save db fail err:%d, logic:%s, key:%s", err, logic->GetObjName().c_str(), key.c_str());
        co_return;
    }

    if(kv.empty())
    {
        CLOG_ERROR("logic save db fail lack unique index define, logic:%s, key:%s"
            , logic->GetObjName().c_str(), key.c_str());
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
    auto ret = co_await _mongodbMgr->AddData(storageCom->GetDbName(), storageCom->GetSystemName(), kv, popDb);

    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("logic AddData save data fail, logic:%s, kv:%s", logicName.c_str(), DictContainerToString(kv).c_str());
    }
}

KERNEL_NS::CoTask<> MongodbProxy::_OnStringModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    dirtyHelper->Clear(key, StorageMode::UPDATE_DATA);
    auto logic = params->AsPtr<CompHostObject>();
    if(UNLIKELY(!logic))
    {
        CLOG_ERROR("params is not logic sys please check, key:%s", key.c_str());
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
        keyStream->Write(key);
        KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::Release> valueStream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
        err = storageCom->OnSave(logic, key, *valueStream);
        if (err != Status::Success)
        {
            CLOG_ERROR("logic save db fail err:%d, logic:%s, key:%s", err, logic->GetObjName().c_str(), key.c_str());
            co_return;
        }

        dbInfo->emplace(storageCom->GetKeyFieldName(), keyStream.pop());
        dbInfo->emplace(storageCom->GetValueFieldName(), valueStream.pop());
        kv.emplace(storageCom->GetKeyFieldName(), KERNEL_NS::Variant(key));
    }
    else
    {
        err = storageCom->OnSave(logic, key, *dbInfo.AsSelf());
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
        CLOG_ERROR("logic save db fail lack unique index define, logic:%s, key:%s"
            , logic->GetObjName().c_str(), key.c_str());
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
    auto ret = co_await _mongodbMgr->UpdateData(storageCom->GetDbName(), storageCom->GetSystemName(), kv, popDb, true);

    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("logic UpdateData save data fail, logic:%s, kv:%s", logicName.c_str(), DictContainerToString(kv).c_str());
    }
}

KERNEL_NS::CoTask<> MongodbProxy::_OnStringDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    dirtyHelper->Clear(key, StorageMode::DEL_DATA);
    auto logic = params->AsPtr<CompHostObject>();
    if(UNLIKELY(!logic))
    {
        CLOG_ERROR("params is not logic sys please check, key:%s", key.c_str());
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
        CLOG_ERROR("logic save db fail lack unique index define, logic:%s, key:%s"
            , logic->GetObjName().c_str(), key.c_str());
        co_return;
    }

    const auto logicName = logic->GetObjName();
    auto ret = co_await _mongodbMgr->DelData(storageCom->GetDbName(), storageCom->GetSystemName(), kv);

    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("logic DelData save data fail, logic:%s, kv:%s", logicName.c_str(), DictContainerToString(kv).c_str());
    }
}

KERNEL_NS::CoTask<> MongodbProxy::_OnStringReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    dirtyHelper->Clear(key, StorageMode::REPLACE_DATA);
   auto logic = params->AsPtr<CompHostObject>();
    if(UNLIKELY(!logic))
    {
        CLOG_ERROR("params is not logic sys please check, key:%s", key.c_str());
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
        keyStream->Write(key);
        KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::Release> valueStream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
        err = storageCom->OnSave(logic, key, *valueStream);
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
        err = storageCom->OnSave(logic, key, *dbInfo.AsSelf());
        auto &uniqueIndexFields = storageCom->GetUniqueIndexFields();
        for(auto &iter : uniqueIndexFields)
        {
            kv.emplace(iter.first, KERNEL_NS::Variant(key));
            break;
        }
    }
    if(err != Status::Success)
    {
        CLOG_ERROR("logic save db fail err:%d, logic:%s, key:%s", err, logic->GetObjName().c_str(), key.c_str());
        co_return;
    }

    if(kv.empty())
    {
        CLOG_ERROR("logic save db fail lack unique index define, logic:%s, key:%s"
            , logic->GetObjName().c_str(), key.c_str());
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
    auto ret = co_await _mongodbMgr->ReplaceData(storageCom->GetDbName(), storageCom->GetSystemName(), kv, popDb);
    if(UNLIKELY(!ret))
    {
        CLOG_ERROR("logic ReplaceData save data fail, logic:%s, kv:%s", logicName.c_str(), DictContainerToString(kv).c_str());
    }
}

KERNEL_NS::CoTask<> MongodbProxy::_DorPurgeNumber(const CompHostObject *sys)
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
        CLOG_ERROR("purge abnormal handled:%lld, err:%s, logic:%s, owner:%s"
            , handled, err.c_str(), sys->GetObjName().c_str(), GetOwner()->ToString().c_str());
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

KERNEL_NS::CoTask<> MongodbProxy::_DorPurgeString(const CompHostObject *sys)
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
        CLOG_ERROR("purge abnormal handled:%lld, err:%s, logic:%s, owner:%s"
            , handled, err.c_str(), sys->GetObjName().c_str(), GetOwner()->ToString().c_str());
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

bool MongodbProxy::_CheckLogic(const CompHostObject *hostObj, KERNEL_NS::LibString &err) const
{
    auto storageComp = hostObj->GetComp<IMongodbStorageInfo>();
    if(!storageComp)
    {
        return false;
    }

    if (!CheckValidStorage(hostObj, storageComp, false, err))
    {
        return false;
    }

    // 如果是逻辑系统, 它的组件如果需要存储的需要验证下参数, 存储深度最多两个层级, 组件的子组件会作为其第一层存储的一个字段
    auto &subComps = hostObj->GetAllComps();
    for (auto &subComp : subComps)
    {
        if(UNLIKELY(!subComp))
            continue;

        if(!subComp->IsKernelObjType(KERNEL_NS::KernelObjectType::HOST_COMP))
            continue;

        // 过滤掉存储组件
        if(subComp == storageComp)
            continue;

        auto subTurnHost = subComp->CastTo<KERNEL_NS::CompHostObject>();
        auto subStorageComp = subTurnHost->GetComp<IMongodbStorageInfo>();
        if(!subStorageComp)
            continue;

        if (!CheckValidStorage(hostObj, subStorageComp, true, err))
            continue;
    }

    // 表信息建立在storageComp中, 比如UserMgr, User是运行时动态创建的, 其子系统在启动的时候还没建立智能放在UserMgr的StorageComp作为组件存在
    {
        auto &storageSubComps = storageComp->GetAllComps();
        for(auto subSubComp : storageSubComps)
        {
            // 是不是存储组件
            if(subSubComp->GetInterfaceTypeId() != KERNEL_NS::RttiUtil::GetTypeId<IMongodbStorageInfo>())
                continue;

            auto subSubStorageComp = subSubComp->CastTo<IMongodbStorageInfo>();
            CheckValidStorage(hostObj, subSubStorageComp, true, err);
        }
    }

    return err.empty();
}

void MongodbProxy::_BuildSysFields(const CompHostObject *logic)
{
    auto storageComp = logic->GetComp<IMongodbStorageInfo>();
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

        if(subComp == storageComp)
            continue;

        auto subHost = subComp->CastTo<KERNEL_NS::CompHostObject>();
        auto subStorageComp = subHost->GetComp<IMongodbStorageInfo>();
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
            if(subSubComp->GetInterfaceTypeId() != KERNEL_NS::RttiUtil::GetTypeId<IMongodbStorageInfo>())
                continue;
            
            auto subSubStorageComp = subSubComp->CastTo<IMongodbStorageInfo>();
            fieldRefStorageType[subSubStorageComp->GetSystemName()] = subSubStorageComp->GetStorageType();
        }
    }
}

bool MongodbProxy::_TryGetStorageType(const CompHostObject *logic, const KERNEL_NS::LibString &fieldName, Int32 &storageType) const
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

KERNEL_END

