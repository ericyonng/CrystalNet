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
 * Date: 2023-07-16 14:37:39
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <service/TestService/Comps/DB/impl/MysqlMgr.h>
#include <service/TestService/Comps/DB/impl/MysqlMgrFactory.h>
#include <service/TestService/Comps/StubHandle/StubHandle.h>
#include <OptionComp/storage/mysql/mysqlcomp.h>
#include <service/TestService/Comps/DB/impl/MysqlDefs.h>
#include <service/TestService/Comps/DB/impl/TableInfo.h>
#include <service/TestService/Comps/DB/impl/MysqlMgrStorage.h>
#include <service/TestService/Comps/DB/impl/MysqlMgrStorageFactory.h>

SERVICE_BEGIN
POOL_CREATE_OBJ_DEFAULT_IMPL(IMysqlMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlMgr);

MysqlMgr::MysqlMgr()
:_closeServiceStub(INVALID_LISTENER_STUB)
,_purgeTimer(NULL)
,_poller(NULL)
,_defaultBlobOriginSize(64)
,_defaultStringKeyOriginSize(256)
,_curVersionNo(0)
,_systemOperatorUid(0)
,_purgeIntervalMs(3000)
{
}

MysqlMgr::~MysqlMgr()
{
    _Clear();
}

void MysqlMgr::Release()
{
    return MysqlMgr::DeleteByAdapter_MysqlMgr(MysqlMgrFactory::_buildType.V, this);
}

void MysqlMgr::OnRegisterComps()
{
    RegisterComp<KERNEL_NS::MysqlDBMgrFactory>();
    RegisterComp<MysqlMgrStorageFactory>();
}

void MysqlMgr::RegisterDependence(ILogicSys *obj)
{
    auto dbMgr = GetComp<KERNEL_NS::MysqlDBMgr>();
    dbMgr->RegisterDependence(obj);

    // 兼容windows/linux表名使用小写
    _tableNameRefLogic.insert(std::make_pair(obj->GetStorageInfo()->GetTableName(), obj));
}

void MysqlMgr::UnRegisterDependence(const ILogicSys *obj)
{
    auto dbMgr = GetComp<KERNEL_NS::MysqlDBMgr>();
    dbMgr->UnRegisterDependence(obj);

    _tableNameRefLogic.erase(obj->GetStorageInfo()->GetTableName());
}

Int32 MysqlMgr::NewRequest(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &fields, bool isDestroyHandler, KERNEL_NS::IDelegate<void, KERNEL_NS::MysqlResponse *> *cb, KERNEL_NS::Variant **var)
{
    stub = 0;
    auto dbMgr = GetComp<KERNEL_NS::MysqlDBMgr>();
    auto db = dbMgr->GetDB(_currentServiceDBName);
    if(UNLIKELY(!db))
    {
        std::vector<KERNEL_NS::LibString> sqls;
        for(auto builder : builders)
            sqls.push_back(builder->Dump());

        std::vector<KERNEL_NS::LibString> fieldsDump;
        for(auto field : fields)
            fieldsDump.push_back(field->Dump());

        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString().AppendFormat("db not found db name:%s, sqls:\n", dbName.c_str())
                    , KERNEL_NS::StringUtil::ToString(sqls, ";"), KERNEL_NS::LibString().AppendFormat("\nfields:\n"), KERNEL_NS::StringUtil::ToString(fieldsDump, ","));

        KERNEL_NS::ContainerUtil::DelContainer2(builders);
        KERNEL_NS::ContainerUtil::DelContainer2(fields);

        if(var)
        {
            KERNEL_NS::Variant::DeleteThreadLocal_Variant(*var);
            *var = NULL;
        }

        return Status::NotFound;
    }

    auto stubMgr = GetGlobalSys<IStubHandleMgr>();
    stub = stubMgr->NewStub();

    auto req = KERNEL_NS::MysqlRequest::New_MysqlRequest();
    req->_dbOperatorId = dbOperatorId;
    req->_seqId = dbMgr->NewSeqId();
    req->_stub = stub;
    req->_msgType = KERNEL_NS::MysqlMsgType::Stmt;

    req->_builders = builders;
    builders.clear();

    req->_fields = fields;
    fields.clear();

    req->_handler = cb;
    req->_isDestroyHandler = isDestroyHandler;
    req->_dbName = dbName;

    if(var)
    {
        req->_var = *var;
        *var = NULL;
    }

    if(!dbMgr->PushRequest(db, req))
    {
        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString().AppendFormat("push request fail req:\n"), req->Dump());
        KERNEL_NS::MysqlRequest::Delete_MysqlRequest(req);
        return Status::Failed;
    }

    return Status::Success;
}

void MysqlMgr::MaskLogicNumberKeyAddDirty(const ILogicSys *logic, UInt64 key, bool isRightRow)
{
    auto storageInfo = logic->GetStorageInfo();
    if(UNLIKELY(!storageInfo))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no stoarage info cant mask dirty logic:%s, key:%llu"), logic->GetObjName().c_str(), key);
        return;
    }

    if(UNLIKELY(!storageInfo->IsUsingNumberKey()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("storage key type not number type cant mask number dirty logic:%s, key:%llu, storage info:%s")
                , logic->GetObjName().c_str(), key, storageInfo->ToString().c_str());
        return;
    }

    auto iter = _logicRefNumberDirtyHelper.find(logic);
    if(iter == _logicRefNumberDirtyHelper.end())
    {
        iter = _logicRefNumberDirtyHelper.insert(std::make_pair(logic, KERNEL_NS::LibDirtyHelper<UInt64, UInt64>::NewThreadLocal_LibDirtyHelper())).first;
        auto dirtyHelper = iter->second;
        _InitNumberDirtyHelper(storageInfo, dirtyHelper);
    }

    // 标脏
    auto dirtyHelper = iter->second;

    // 如果有其他脏标记则移除
    if(dirtyHelper->HasDirty())
        dirtyHelper->Clear(key);

    auto var = dirtyHelper->MaskDirty(key, MysqlDirtyType::REPLACE_TYPE, true);
    var->BecomePtr() = logic;
    _dirtyLogics.insert(logic);

    if(isRightRow)
    {
        auto &&nowCounter = KERNEL_NS::LibCpuCounter::Current();
        const auto &startCounter = KERNEL_NS::LibCpuCounter::Current();
        const auto &deadLine = nowCounter + _poller->GetMaxPieceTime();
        KERNEL_NS::LibString err;
        dirtyHelper->Purge(deadLine, &err);
        if(UNLIKELY(!err.empty()))
            g_Log->Warn(LOGFMT_OBJ_TAG("purge right now err:%s, logic:%s"), err.c_str(), logic->GetObjName().c_str());
            
        nowCounter.Update();
        g_Log->Info(LOGFMT_OBJ_TAG("logic:%s purge cost time %llu (ms)"), logic->GetObjName().c_str(), (nowCounter - startCounter).GetTotalMilliseconds());

        if(!dirtyHelper->HasDirty())
            _dirtyLogics.erase(logic);
    }
}

void MysqlMgr::MaskLogicNumberKeyModifyDirty(const ILogicSys *logic, UInt64 key, bool isRightRow)
{
    auto storageInfo = logic->GetStorageInfo();
    if(UNLIKELY(!storageInfo))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no stoarage info cant mask dirty logic:%s, key:%llu"), logic->GetObjName().c_str(), key);
        return;
    }

    if(UNLIKELY(!storageInfo->IsUsingNumberKey()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("storage key type not number type cant mask number dirty logic:%s, key:%llu, storage info:%s")
                , logic->GetObjName().c_str(), key, storageInfo->ToString().c_str());
        return;
    }

    auto iter = _logicRefNumberDirtyHelper.find(logic);
    if(iter == _logicRefNumberDirtyHelper.end())
    {
        iter = _logicRefNumberDirtyHelper.insert(std::make_pair(logic, KERNEL_NS::LibDirtyHelper<UInt64, UInt64>::NewThreadLocal_LibDirtyHelper())).first;
        auto dirtyHelper = iter->second;
        _InitNumberDirtyHelper(storageInfo, dirtyHelper);
    }

    // 标脏
    auto dirtyHelper = iter->second;

    // 移除其他脏标记
    const auto isDelDirty = dirtyHelper->IsDirty(key, MysqlDirtyType::DEL_TYPE);

    // 已经被删除了不可以modify
    if(UNLIKELY(isDelDirty))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("data already mask delete before please check logic:%s, key:%llu"), logic->GetObjName().c_str(), key);
        return;
    }

    // 已经标脏过不需要再标脏(增加会覆盖最新的数据)
    if(dirtyHelper->HasDirty())
        return;

    auto var = dirtyHelper->MaskDirty(key, MysqlDirtyType::MODIFY_TYPE, true);
    var->BecomePtr() = logic;
    _dirtyLogics.insert(logic);

    if(isRightRow)
    {
        auto &&nowCounter = KERNEL_NS::LibCpuCounter::Current();
        const auto &startCounter = KERNEL_NS::LibCpuCounter::Current();
        const auto &deadLine = nowCounter + _poller->GetMaxPieceTime();
        KERNEL_NS::LibString err;
        dirtyHelper->Purge(deadLine, &err);
        if(UNLIKELY(!err.empty()))
            g_Log->Warn(LOGFMT_OBJ_TAG("purge right now err:%s, logic:%s"), err.c_str(), logic->GetObjName().c_str());
            
        nowCounter.Update();
        g_Log->Info(LOGFMT_OBJ_TAG("logic:%s purge cost time %llu (ms)"), logic->GetObjName().c_str(), (nowCounter - startCounter).GetTotalMilliseconds());

        if(!dirtyHelper->HasDirty())
            _dirtyLogics.erase(logic);
    }
}

void MysqlMgr::MaskLogicNumberKeyDeleteDirty(const ILogicSys *logic, UInt64 key, bool isRightRow)
{
    auto storageInfo = logic->GetStorageInfo();
    if(UNLIKELY(!storageInfo))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no stoarage info cant mask dirty logic:%s, key:%llu"), logic->GetObjName().c_str(), key);
        return;
    }

    if(UNLIKELY(!storageInfo->IsUsingNumberKey()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("storage key type not number type cant mask number dirty logic:%s, key:%llu, storage info:%s")
                , logic->GetObjName().c_str(), key, storageInfo->ToString().c_str());
        return;
    }

    auto iter = _logicRefNumberDirtyHelper.find(logic);
    if(iter == _logicRefNumberDirtyHelper.end())
    {
        iter = _logicRefNumberDirtyHelper.insert(std::make_pair(logic, KERNEL_NS::LibDirtyHelper<UInt64, UInt64>::NewThreadLocal_LibDirtyHelper())).first;
        auto dirtyHelper = iter->second;
        _InitNumberDirtyHelper(storageInfo, dirtyHelper);
    }

    // 标脏
    auto dirtyHelper = iter->second;
    dirtyHelper->Clear(key);
    auto var = dirtyHelper->MaskDirty(key, MysqlDirtyType::DEL_TYPE, true);
    var->BecomePtr() = logic;

    _dirtyLogics.insert(logic);

    if(isRightRow)
    {
        auto &&nowCounter = KERNEL_NS::LibCpuCounter::Current();
        const auto &startCounter = KERNEL_NS::LibCpuCounter::Current();
        const auto &deadLine = nowCounter + _poller->GetMaxPieceTime();
        KERNEL_NS::LibString err;
        dirtyHelper->Purge(deadLine, &err);
        if(UNLIKELY(!err.empty()))
            g_Log->Warn(LOGFMT_OBJ_TAG("purge right now err:%s, logic:%s"), err.c_str(), logic->GetObjName().c_str());
            
        nowCounter.Update();
        g_Log->Info(LOGFMT_OBJ_TAG("logic:%s purge cost time %llu (ms)"), logic->GetObjName().c_str(), (nowCounter - startCounter).GetTotalMilliseconds());

        if(!dirtyHelper->HasDirty())
            _dirtyLogics.erase(logic);
    }
}

void MysqlMgr::MaskLogicStringKeyAddDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key, bool isRightRow)
{
    auto storageInfo = logic->GetStorageInfo();
    if(UNLIKELY(!storageInfo))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no stoarage info cant mask dirty logic:%s, key:%s")
            , logic->GetObjName().c_str(), key.c_str());
        return;
    }

    if(UNLIKELY(!storageInfo->IsUsingStringKey()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("storage key type not string type cant mask number dirty logic:%s, key:%s, storage info:%s")
                , logic->GetObjName().c_str(), key.c_str(), storageInfo->ToString().c_str());
        return;
    }

    auto iter = _logicRefStringDirtyHelper.find(logic);
    if(iter == _logicRefStringDirtyHelper.end())
    {
        iter = _logicRefStringDirtyHelper.insert(std::make_pair(logic, KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64>::NewThreadLocal_LibDirtyHelper())).first;
        auto dirtyHelper = iter->second;
        _InitStringDirtyHelper(storageInfo, dirtyHelper);
    }

    // 标脏
    auto dirtyHelper = iter->second;

    // 如果有其他脏标记则移除
    if(dirtyHelper->HasDirty())
        dirtyHelper->Clear(key);

    auto var = dirtyHelper->MaskDirty(key, MysqlDirtyType::REPLACE_TYPE, true);
    var->BecomePtr() = logic;
    _dirtyLogics.insert(logic);

    if(isRightRow)
    {
        auto &&nowCounter = KERNEL_NS::LibCpuCounter::Current();
        const auto &startCounter = KERNEL_NS::LibCpuCounter::Current();
        const auto &deadLine = nowCounter + _poller->GetMaxPieceTime();
        KERNEL_NS::LibString err;
        dirtyHelper->Purge(deadLine, &err);
        if(UNLIKELY(!err.empty()))
            g_Log->Warn(LOGFMT_OBJ_TAG("purge right now err:%s, logic:%s"), err.c_str(), logic->GetObjName().c_str());
            
        nowCounter.Update();
        g_Log->Info(LOGFMT_OBJ_TAG("logic:%s purge cost time %llu (ms)"), logic->GetObjName().c_str(), (nowCounter - startCounter).GetTotalMilliseconds());

        if(!dirtyHelper->HasDirty())
            _dirtyLogics.erase(logic);
    }
}

void MysqlMgr::MaskLogicStringKeyModifyDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key, bool isRightRow)
{
    auto storageInfo = logic->GetStorageInfo();
    if(UNLIKELY(!storageInfo))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no stoarage info cant mask dirty logic:%s, key:%s")
            , logic->GetObjName().c_str(), key.c_str());
        return;
    }

    if(UNLIKELY(!storageInfo->IsUsingStringKey()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("storage key type not string type cant mask number dirty logic:%s, key:%s, storage info:%s")
                , logic->GetObjName().c_str(), key.c_str(), storageInfo->ToString().c_str());
        return;
    }

    auto iter = _logicRefStringDirtyHelper.find(logic);
    if(iter == _logicRefStringDirtyHelper.end())
    {
        iter = _logicRefStringDirtyHelper.insert(std::make_pair(logic, KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64>::NewThreadLocal_LibDirtyHelper())).first;
        auto dirtyHelper = iter->second;
        _InitStringDirtyHelper(storageInfo, dirtyHelper);
    }

    // 标脏
    auto dirtyHelper = iter->second;

    // 移除其他脏标记
    const auto isDelDirty = dirtyHelper->IsDirty(key, MysqlDirtyType::DEL_TYPE);

    // 已经被删除了不可以modify
    if(UNLIKELY(isDelDirty))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("data already mask delete before please check logic:%s, key:%llu"), logic->GetObjName().c_str(), key);
        return;
    }

    // 已经标脏过不需要再标脏(增加会覆盖最新的数据)
    if(dirtyHelper->HasDirty())
        return;

    auto var = dirtyHelper->MaskDirty(key, MysqlDirtyType::MODIFY_TYPE, true);
    var->BecomePtr() = logic;
    _dirtyLogics.insert(logic);

    if(isRightRow)
    {
        auto &&nowCounter = KERNEL_NS::LibCpuCounter::Current();
        const auto &startCounter = KERNEL_NS::LibCpuCounter::Current();
        const auto &deadLine = nowCounter + _poller->GetMaxPieceTime();
        KERNEL_NS::LibString err;
        dirtyHelper->Purge(deadLine, &err);
        if(UNLIKELY(!err.empty()))
            g_Log->Warn(LOGFMT_OBJ_TAG("purge right now err:%s, logic:%s"), err.c_str(), logic->GetObjName().c_str());
            
        nowCounter.Update();
        g_Log->Info(LOGFMT_OBJ_TAG("logic:%s purge cost time %llu (ms)"), logic->GetObjName().c_str(), (nowCounter - startCounter).GetTotalMilliseconds());

        if(!dirtyHelper->HasDirty())
            _dirtyLogics.erase(logic);
    }
}

void MysqlMgr::MaskLogicStringKeyDeleteDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key, bool isRightRow)
{
    auto storageInfo = logic->GetStorageInfo();
    if(UNLIKELY(!storageInfo))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no stoarage info cant mask dirty logic:%s, key:%s")
            , logic->GetObjName().c_str(), key.c_str());
        return;
    }

    if(UNLIKELY(!storageInfo->IsUsingStringKey()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("storage key type not string type cant mask number dirty logic:%s, key:%s, storage info:%s")
                , logic->GetObjName().c_str(), key.c_str(), storageInfo->ToString().c_str());
        return;
    }

    auto iter = _logicRefStringDirtyHelper.find(logic);
    if(iter == _logicRefStringDirtyHelper.end())
    {
        iter = _logicRefStringDirtyHelper.insert(std::make_pair(logic, KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64>::NewThreadLocal_LibDirtyHelper())).first;
        auto dirtyHelper = iter->second;
        _InitStringDirtyHelper(storageInfo, dirtyHelper);
    }

    // 标脏
    auto dirtyHelper = iter->second;
    dirtyHelper->Clear(key);
    auto var = dirtyHelper->MaskDirty(key, MysqlDirtyType::DEL_TYPE, true);
    var->BecomePtr() = logic;

    _dirtyLogics.insert(logic);

    if(isRightRow)
    {
        auto &&nowCounter = KERNEL_NS::LibCpuCounter::Current();
        const auto &startCounter = KERNEL_NS::LibCpuCounter::Current();
        const auto &deadLine = nowCounter + _poller->GetMaxPieceTime();
        KERNEL_NS::LibString err;
        dirtyHelper->Purge(deadLine, &err);
        if(UNLIKELY(!err.empty()))
            g_Log->Warn(LOGFMT_OBJ_TAG("purge right now err:%s, logic:%s"), err.c_str(), logic->GetObjName().c_str());
            
        nowCounter.Update();
        g_Log->Info(LOGFMT_OBJ_TAG("logic:%s purge cost time %llu (ms)"), logic->GetObjName().c_str(), (nowCounter - startCounter).GetTotalMilliseconds());

        if(!dirtyHelper->HasDirty())
            _dirtyLogics.erase(logic);
    }
}

Int32 MysqlMgr::GetSystemDBOperatorId() const
{
    return _systemOperatorUid;
}

Int32 MysqlMgr::NewDbOperatorId()
{
    auto dbMgr = GetComp<KERNEL_NS::MysqlDBMgr>();
    return dbMgr->NewOperatorUid(_currentServiceDBName);
}

const KERNEL_NS::LibString &MysqlMgr::GetCurrentServiceDbOption() const
{
    return _currentServiceDBOption;
}

const KERNEL_NS::LibString &MysqlMgr::GetCurrentServiceDbName() const
{
    return _currentServiceDBName;
}

Int32 MysqlMgr::OnSave(const KERNEL_NS::LibString &key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const
{
    auto tableInfo = _GetTableInfo(key);
    if(UNLIKELY(!tableInfo))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("table info not found key:%s, logic:%s"), key.c_str(), GetObjName().c_str());
        return Status::NotFound;
    }

    // 转化成json
    std::string data;
    if(!::google::protobuf::util::MessageToJsonString(*tableInfo->_simpleInfo, &data).ok())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("SerializeFail key:%s"), key.c_str());
        return Status::SerializeFail;
    }

    auto simpleInfoData = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
    simpleInfoData->Init(1);
    simpleInfoData->Write(data.data(), static_cast<Int64>(data.size()));
    fieldRefdb.insert(std::make_pair(MysqlMgrStorage::SIMPLE_INFO, simpleInfoData));
    
    auto keyData = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
    keyData->Init(1);
    keyData->Write(key.data(), static_cast<Int64>(key.size()));
    fieldRefdb.insert(std::make_pair(MysqlMgrStorage::TABLE_NAME, keyData));

    return Status::Success;
}

Int32 MysqlMgr::OnLoaded(const KERNEL_NS::LibString &key, const std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb)
{
    KERNEL_NS::SmartPtr<TableInfo, KERNEL_NS::AutoDelMethods::CustomDelete> tableInfo = TableInfo::NewThreadLocal_TableInfo();
    tableInfo.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<TableInfo *>(p);
        TableInfo::DeleteThreadLocal_TableInfo(ptr);
    });

    for(auto iter : fieldRefdb)
    {
        if(iter.first == MysqlMgrStorage::SIMPLE_INFO)
        {
            auto data = iter.second;
            KERNEL_NS::SmartPtr<SimpleInfo, KERNEL_NS::AutoDelMethods::Release> simpleInfo = CRYSTAL_NEW(SimpleInfo);
            auto &&jsonString = ::google::protobuf::StringPiece(data->GetReadBegin(), data->GetReadableSize());
            if(!::google::protobuf::util::JsonStringToMessage(jsonString, simpleInfo.AsSelf()).ok())
            {
                g_Log->Error(LOGFMT_OBJ_TAG("SimpleInfo field JsonStringToMessage fail key:%s, jsonString:%s"), key.c_str(), jsonString.as_string().c_str());
                return Status::ParseFail;
            }

            tableInfo->_simpleInfo = simpleInfo.pop();
            tableInfo->_tableName = key;
        }
    }

    _tableNameRefTableInfo.insert(std::make_pair(key, tableInfo.pop()));

    return Status::Success;
}

Int32 MysqlMgr::_OnGlobalSysInit()
{
    _poller = GetService()->GetComp<KERNEL_NS::Poller>();
    _closeServiceStub = GetEventMgr()->AddListener(EventEnums::QUIT_SERVICE_EVENT, this, &MysqlMgr::_CloseServiceEvent);

    auto ini = GetApp()->GetIni();
    if(!ini->CheckReadNumber("MysqlCommon", "DefaultBlobOriginSize", _defaultBlobOriginSize))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("lack of MysqlCommon:DefaultBlobOriginSize config in ini:%s"), ini->GetPath().c_str());
        return Status::Failed;
    }

    if(!ini->CheckReadNumber("MysqlCommon", "DefaultStringKeyOriginSize", _defaultStringKeyOriginSize))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("lack of MysqlCommon:DefaultStringKeyOriginSize config in ini:%s"), ini->GetPath().c_str());
        return Status::Failed;
    }

    if(!ini->ReadStr(GetService()->GetServiceName().c_str(), "CurrentServiceDB", _currentServiceDBOption))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("lack of %s:CurrentServiceDB config in ini:%s"), GetService()->GetServiceName().c_str(), ini->GetPath().c_str());
        return Status::Failed;
    }

    if(!ini->ReadStr(_currentServiceDBOption.c_str(), "DB", _currentServiceDBName))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("lack of %s:DB config in ini:%s"), _currentServiceDBName.c_str(), ini->GetPath().c_str());
        return Status::Failed;
    }

    if(!ini->CheckReadNumber(GetService()->GetServiceName().c_str(), "DbVersion", _curVersionNo))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("lack of MysqlCommon:DbVersion config in ini:%s"), ini->GetPath().c_str());
        return Status::Failed;
    }

    if(!ini->CheckReadNumber(GetService()->GetServiceName().c_str(), "SystemOperatorUid", _systemOperatorUid))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("lack of MysqlCommon:SystemOperatorUid config in ini:%s"), ini->GetPath().c_str());
        return Status::Failed;
    }

    if(!ini->CheckReadNumber(GetService()->GetServiceName().c_str(), "PurgeIntervalMs", _purgeIntervalMs))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("lack of %s:PurgeIntervalMs config in ini:%s"), _currentServiceDBName.c_str(), ini->GetPath().c_str());
        return Status::Failed;
    }

    // 设置操作id
    SetStorageOperatorId(_systemOperatorUid);

    return Status::Success;
}

Int32 MysqlMgr::_OnGlobalSysCompsCreated()
{
    auto service = GetService();
    auto ini = service->GetApp()->GetIni();
    
    // 获取需要加载的数据库段名列表
    KERNEL_NS::LibString dbList;
    if(!ini->ReadStr(service->GetServiceName().c_str(), "DbConfigList", dbList))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("service DbConfigList config not exists, service name:%s."), service->GetServiceName().c_str());
        return Status::Failed;
    }

    if(UNLIKELY(dbList.empty()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no DbConfigList please check service name:%s"), service->GetServiceName().c_str());
        return Status::Failed;
    }

    const auto &dbs = dbList.Split(",");
    if(UNLIKELY(dbs.empty()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("DbConfigList config error:%s check service name:%s"), dbList.c_str(), service->GetServiceName().c_str());
        return Status::Failed;
    }

    for(auto &db : dbs)
    {
        if(UNLIKELY(db.empty()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("DbConfigList config error:%s check service name:%s"), dbList.c_str(), service->GetServiceName().c_str());
            return Status::Failed;
        }
    }

    auto dbMgr = GetComp<KERNEL_NS::MysqlDBMgr>();
    dbMgr->SetMsgBackPoller(service->GetPoller());

    const Int32 dbLevel = static_cast<Int32>(PriorityLevelDefine::DB);
    auto level = service->GetMaxPriorityLevel() > dbLevel ? dbLevel : service->GetMaxPriorityLevel();
    dbMgr->SetMsgLevel(level);

    dbMgr->SetDbEventType(ServicePollerEvent::MysqlDbEvent);

    dbMgr->SetIniFile(ini);
    dbMgr->SetDbSegmentList(dbs);

    // 其他系统使用数据库时候跳过系统的操作id, 避免干扰系统行为
    dbMgr->SkipOperatorId(_currentServiceDBName, _systemOperatorUid);

    // 3分钟清洗一次数据
    _purgeTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    _purgeTimer->SetTimeOutHandler(this, &MysqlMgr::_OnPurge);
    _purgeTimer->Schedule(_purgeIntervalMs);

    g_Log->Info(LOGFMT_OBJ_TAG("mysql mgr init success."));

    return Status::Success;
}

Int32 MysqlMgr::_OnHostStart()
{
    // 所有支持数据存储的注册依赖
    auto service = GetService();
    auto &comps = service->GetCompsByType(ServiceCompType::LOGIC_SYS);
    for(auto &comp : comps)
    {
        auto logicSys = comp->CastTo<ILogicSys>();
        auto comp = logicSys->GetCompByType(ServiceCompType::STORAGE_COMP);
        if(!comp)
            continue;

        auto storageComp = comp->CastTo<IStorageInfo>();
        if(!storageComp->IsUsingMysql())
            continue;

        // kv系统但是没有填充表信息需要自动填充
        if(storageComp->IsMultiFieldSystem())
        {
            if(!_FillMultiFieldStorageInfo(storageComp))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("_FillMultiFieldStorageInfo fail storage comp:%s"), storageComp->GetSystemName().c_str());
                return Status::MysqlMgrFillStorageInfoFail;
            }
        }

        // 检查表
        if(!_CheckStorageInfo(storageComp))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("check logic fail."));
            return Status::MysqlMgrCheckLogicFail;
        }

        // 注册依赖
        RegisterDependence(logicSys);
    }

    // 依赖存储的组件
    auto dbMgr = GetComp<KERNEL_NS::MysqlDBMgr>();
    auto &dependence = dbMgr->GetDependence();
    std::vector<KERNEL_NS::LibString> dependenceNames;
    for(auto comp : dependence)
        dependenceNames.push_back(comp->GetObjName());

    MaskReady(true);

    // 加载表信息 key是每个依赖系统的Name
    if(!_LoadSystemTable())
    {
        MaskReady(false);

        g_Log->Error(LOGFMT_OBJ_TAG("load system table fail %s"), GetObjName().c_str());
        return Status::Failed;
    }
    
    g_Log->Info(LOGFMT_OBJ_TAG("mysql mgr start success dependenceNames:[%s]."), KERNEL_NS::StringUtil::ToString(dependenceNames, ","));
    return Status::Success;
}

void MysqlMgr::_OnGlobalSysClose()
{
    MaskReady(false);

    _Clear();
    if (_closeServiceStub != INVALID_LISTENER_STUB)
        GetEventMgr()->RemoveListenerX(_closeServiceStub);
}

void MysqlMgr::_CloseServiceEvent(KERNEL_NS::LibEvent *ev)
{
    // 持久化所有数据
    auto dirtyLogics = _dirtyLogics;
    for(auto logic : dirtyLogics)
        _PurgeDirty(logic);

    auto dbMgr = GetComp<KERNEL_NS::MysqlDBMgr>();
    dbMgr->CloseMysqlAll();

    // 定时器等待db mgr 退出
    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer->SetTimeOutHandler([this, dbMgr](KERNEL_NS::LibTimer *t) mutable 
    {
        auto service = GetService();

        // 必须没有脏
        if(!_dirtyLogics.empty())
        {
            auto dirtyLogics = _dirtyLogics;
            for(auto logic : dirtyLogics)
                _PurgeDirty(logic);

            return;
        }

        // 必须没有需要处理的
        if(dbMgr->HasPendings())
            return;

        // 判断依赖是否退出，退出则从依赖列表中移除
        auto dependence = dbMgr->GetDependence();
        if(!dependence.empty())
        {
            dependence.erase(this);
            for(auto iter = dependence.begin(); iter != dependence.end();)
            {
                if(!service->IsServiceModuleQuit(*iter))
                {
                    ++iter;
                    continue;
                }

                UnRegisterDependence((*iter)->CastTo<ILogicSys>());
                iter = dependence.erase(iter);
            }

            // 开始自己落地
            if(dependence.empty())
            {
                _SaveMe();
                service->MaskServiceModuleQuitFlag(this);
                UnRegisterDependence(this);
            }
        }

        g_Log->Warn(LOGFMT_OBJ_TAG("waiting mysql db mgr quit..."));
        if(dbMgr->IsReady())
            return;

        g_Log->Warn(LOGFMT_OBJ_TAG("waiting mysql db mgr had quit..."));
        

        // 注意释放t会同时释放lambda对象, 导致lambda捕获的对象也失效,至于为什么会失效可以看lambda的内存布局
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    });

    timer->Schedule(3000);
    g_Log->Info(LOGFMT_OBJ_TAG("start waiting for mysql db mgr quit..."));
}

bool MysqlMgr::_LoadSystemTable()
{
    // TODO:不应该load自己的表, 而应该从db中获取所有现有的表, 一系列建表,修改表结构后再load全部公共表, 运行时改字段是安全的, 只是性能会变差, 不过由于只会往大的改, 所以最终会趋于稳定
    // 表不存在则创建
    auto service = GetService();
    auto ini = service->GetApp()->GetIni();
    auto dbMgr = GetComp<KERNEL_NS::MysqlDBMgr>();
    auto &configs = dbMgr->GetConfigs();
    auto &config = dbMgr->GetConfig(_currentServiceDBName);
    auto storageInfo = GetStorageInfo();
    const auto &systemTableName = storageInfo->GetTableName();
    UInt64 stub = 0;

    std::vector<KERNEL_NS::SqlBuilder *> builders;

    // 1.不存在则建立表
    auto createBuilder = MysqlFieldTypeHelper::NewCreateTableSqlBuilder(_currentServiceDBName, storageInfo);
    if(!createBuilder)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("NewCreateTableSqlBuilder fail logic:%s"), GetObjName().c_str());
        return false;
    }
    builders.push_back(createBuilder);

    // 2.查询数据
    KERNEL_NS::SelectSqlBuilder *selectBuilder = KERNEL_NS::SelectSqlBuilder::NewThreadLocal_SelectSqlBuilder();
    selectBuilder->DB(_currentServiceDBName).From(systemTableName);
    builders.push_back(selectBuilder);
    auto err = NewRequestBy(stub, _currentServiceDBName, GetStorageOperatorId(), builders, {}, this, &MysqlMgr::_OnSystemTableBack);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d"), err);
        return false;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("load system table success stub:%llu"), stub);
    return true;
}

void MysqlMgr::_SaveMe()
{
    KERNEL_NS::LibString db;
    _PurgeDirty(this);
}

void MysqlMgr::_OnSystemTableBack(KERNEL_NS::MysqlResponse *res)
{
    g_Log->Info(LOGFMT_OBJ_TAG("mysql res:%s"), res->ToString().c_str());

    const auto &systemTableName = GetStorageInfo()->GetTableName();
    if(res->_errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("load system table %s fail db name:%s res seqId:%llu, mysqlError:%u")
                , systemTableName.c_str(), res->_dbName.c_str(), res->_seqId, res->_mysqlErrno);

        GetService()->GetApp()->SinalFinish(Status::LoadSystemTableFail);
        return;
    }

    std::vector<KERNEL_NS::LibString> moduleNames;
    for(auto &record : res->_datas)
    {
        auto primaryKey = record->GetPrimaryKey();
        if(UNLIKELY(!primaryKey))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("system table:%s error db name:%s have no primary key")
            , systemTableName.c_str(), res->_dbName.c_str());
            GetService()->GetApp()->SinalFinish(Status::DBTableError);
            return;
        }

        auto value = record->GetField(MysqlMgrStorage::TABLE_NAME);
        auto &fieldDatas = record->GetFieldDatas();

        KERNEL_NS::LibString tableName;
        primaryKey->GetString(tableName);

        OnLoaded(tableName, fieldDatas);
        moduleNames.push_back(tableName);
    }

    g_Log->Info(LOGFMT_OBJ_TAG("load system table success db name:%s module count:%llu,\nmoduleNames:[%s] ")
                , res->_dbName.c_str(), static_cast<UInt64>(moduleNames.size()), KERNEL_NS::StringUtil::ToString(moduleNames, ",").c_str());

    // 查询库中所有表和字段: 需要校准:tbl_system_data, 1.表校准, 2.表字段校准
    std::vector<KERNEL_NS::SqlBuilder *> builders;
    KERNEL_NS::SelectSqlBuilder *selectBuilder = KERNEL_NS::SelectSqlBuilder::NewThreadLocal_SelectSqlBuilder();
    const KERNEL_NS::LibString &specifyDb = "information_schema";
    selectBuilder->DB(specifyDb).From(KERNEL_NS::LibString().AppendFormat("`COLUMNS`"));
    selectBuilder->WithFields({"TABLE_NAME", "COLUMN_NAME", "DATA_TYPE", "CHARACTER_MAXIMUM_LENGTH"})
                .Where(KERNEL_NS::LibString().AppendFormat("TABLE_SCHEMA='%s'", _currentServiceDBName.c_str()));
    builders.push_back(selectBuilder);
    UInt64 stub = 0;
    auto err = NewRequestBy(stub, specifyDb, GetStorageOperatorId(), builders, {}, this, &MysqlMgr::_OnLoadDbTableColumns);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d"), err);
        GetService()->GetApp()->SinalFinish(Status::DBNewRequestFail);
        return;
    }
    
    g_Log->Info(LOGFMT_OBJ_TAG("load information_schema columns success stub:%llu"), stub);
}

void MysqlMgr::_OnLoadDbTableColumns(KERNEL_NS::MysqlResponse *res)
{
    // 查询库中所有表和字段: 需要校准:tbl_system_data, 1.表校准, 2.表字段校准
    g_Log->Info(LOGFMT_OBJ_TAG("mysql res:%s"), res->ToString().c_str());

    const auto &systemTableName = GetStorageInfo()->GetTableName();
    if(res->_errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("load db table columns fail db name:%s res seqId:%llu, mysqlError:%u")
                , res->_dbName.c_str(), res->_seqId, res->_mysqlErrno);

        GetService()->GetApp()->SinalFinish(Status::DBLoadDataFail);
        return;
    }

    // 1.key:tablename, value:map<fieldName, pair:fieldType, fieldLen>
    std::map<KERNEL_NS::LibString, std::map<KERNEL_NS::LibString, std::pair<KERNEL_NS::LibString, Int64>>> currentTotalTableInfo;
    for(auto &record : res->_datas)
    {
        KERNEL_NS::LibString tblName;
        KERNEL_NS::LibString columnName;
        KERNEL_NS::LibString dataType;
        Int64 fieldLen = 0;
        for(auto field : *record)
        {
            if(field->GetName() == "TABLE_NAME")
            {
                field->GetString(tblName);
            }
            else if(field->GetName() == "COLUMN_NAME")
            {
                field->GetString(columnName);
            }
            else if(field->GetName() == "DATA_TYPE")
            {
                field->GetString(dataType);
                dataType = dataType.toupper();
            }
            else if(field->GetName() == "CHARACTER_MAXIMUM_LENGTH")
            {
                fieldLen = field->GetInt64();
            }
        }

        if(UNLIKELY(tblName.empty() || columnName.empty() || dataType.empty()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("bad tbl name:%s, columnName:%s, dataType:%s"), tblName.c_str(), columnName.c_str(), dataType.c_str());
            continue;
        }

        auto iter = currentTotalTableInfo.find(tblName);
        if(iter == currentTotalTableInfo.end())
            iter = currentTotalTableInfo.insert(std::make_pair(tblName, std::map<KERNEL_NS::LibString, std::pair<KERNEL_NS::LibString, Int64>>())).first;

        iter->second.insert(std::make_pair(columnName, std::make_pair(dataType, fieldLen)));
    }

    // 2.使用数据库原生表校准旧表
    std::vector<KERNEL_NS::SqlBuilder *> builders;
    for(auto iter : currentTotalTableInfo)
    {
        auto tableInfo = _GetTableInfo(iter.first);
        if(!tableInfo)
        {// 原来有的,系统表却没有可能需要新增
            auto dependenceLogic = _GetDependenceLogic(iter.first);
            if(!dependenceLogic)
            {// 当前不需要的logic, 则该表保留, 不删除
                continue;
            }

            tableInfo = _CreateNewTableInfo(dependenceLogic);
            if(!tableInfo)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("_CreateNewTableInfo fail table name:%s, dependenceLogic:%s"), iter.first.c_str(), dependenceLogic->ToString().c_str());
                GetService()->GetApp()->SinalFinish(Status::CreateTableInfoInSystemTableFail);
                return;
            }
        }
    }

    // 3.使用系统数据表校准数据库原生表:a.系统数据表有的表但是数据库中没有的需要删除系统数据表的, b.系统数据表中有的字段但是数据库没有的需要删除
    for(auto iterTableInfo = _tableNameRefTableInfo.begin(); iterTableInfo != _tableNameRefTableInfo.end();)
    {
        auto tableInfo = iterTableInfo->second;

        auto iterRawTableInfo = currentTotalTableInfo.find(tableInfo->_tableName);
        if(iterRawTableInfo == currentTotalTableInfo.end())
        {// 数据库没有该表, 需要移除
            MaskStringKeyDeleteDirty(tableInfo->_tableName);
            TableInfo::DeleteThreadLocal_TableInfo(tableInfo);
            iterTableInfo = _tableNameRefTableInfo.erase(iterTableInfo);
            continue;
        }

        ++iterTableInfo;
    }

    // 4.找出需要修改表结构的
    for(auto iter : currentTotalTableInfo)
    {
        auto logic = _GetDependenceLogic(iter.first);
        if(!logic)
            continue;

        auto storageInfo = logic->GetStorageInfo();
        if(_GetModifyTableInfo(storageInfo, iter.second, builders))
            g_Log->Info(LOGFMT_OBJ_TAG("storage info changed storageInfo:%s"), storageInfo->ToString().c_str());
        else
        {
            g_Log->Error(LOGFMT_OBJ_TAG("_GetModifyTableInfo fail storageInfo:%s"), storageInfo->ToString().c_str());
        }
    }

    // 5.找出需要新建的表,（表只能新增不能删除, 因为有可能是暂时下架某个功能, 要删除表需要人工手动删除） 然后创建表, 并在创建完成后更新系统表(数据需要透传过去Variant), 以及需要altertable的表
    auto dbMgr = GetComp<KERNEL_NS::MysqlDBMgr>();
    auto &&dependence = dbMgr->GetDependence({this});
    std::vector<const ILogicSys *> logics;
    for(auto comp : dependence)
    {
        auto logic = comp->CastTo<ILogicSys>();
        auto storageInfo = logic->GetStorageInfo();

        if(currentTotalTableInfo.find(storageInfo->GetTableName()) != currentTotalTableInfo.end())
            continue;

        auto createBuilder = MysqlFieldTypeHelper::NewCreateTableSqlBuilder(_currentServiceDBName, storageInfo);
        if(!createBuilder)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("NewCreateTableSqlBuilder fail logic:%s, storageInfo:%s"), GetObjName().c_str(), storageInfo->ToString().c_str());
            GetService()->GetApp()->SinalFinish(Status::DBCreateTableSqlBuilderFail);

            return;
        }

        builders.push_back(createBuilder);
        logics.push_back(logic);
        g_Log->Info(LOGFMT_OBJ_TAG("will add new table info storage info:%s"), storageInfo->ToString().c_str());
    }

    if(builders.empty())
    {
        _PurgeDirty(this);

        // 加载所有Global系统表
        _LoadAllPublicData();
        return;
    }

    UInt64 stub = 0;
    auto var = KERNEL_NS::Variant::NewThreadLocal_Variant();
    *var = logics;
    auto err = NewRequestBy(stub, _currentServiceDBName, GetStorageOperatorId(), builders, {}, this, &MysqlMgr::_OnAddNewTableBack, &var);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d"), err);
        GetService()->GetApp()->SinalFinish(Status::DBNewRequestFail);
        return;
    }
}

void MysqlMgr::_OnAddNewTableBack(KERNEL_NS::MysqlResponse *res)
{
    auto &logicSeq = res->_var->AsSequence();
    const Int64 originAddNewTableCount = static_cast<Int64>(logicSeq.size());
    g_Log->Info(LOGFMT_OBJ_TAG("add  table back:%s, originAddNewCount:%lld"), res->ToString().c_str(), originAddNewTableCount);

    std::vector<KERNEL_NS::LibString> tables;
    for(auto &logicVar : logicSeq)
    {
        auto logic = logicVar.AsPtr<ILogicSys>();
        auto storageInfo = logic->GetStorageInfo();
        tables.push_back(storageInfo->GetTableName());
    }

    if(res->_errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("add new table fail errCode:%d, mysql errno:%u, db name:%s, tables:[%s], originAddNewTableCount:%lld")
                , res->_errCode, res->_mysqlErrno, res->_dbName.c_str(), KERNEL_NS::StringUtil::ToString(tables, ",").c_str(), originAddNewTableCount);

        GetService()->GetApp()->SinalFinish(Status::DBAddDataFail);
        return;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("add new table info success db name:%s, tables:%s affected rows:%lld"), res->_dbName.c_str(), KERNEL_NS::StringUtil::ToString(tables, ",").c_str(), res->_affectedRows);

    // 加载数据
    _LoadAllPublicData();
}

bool MysqlMgr::_CheckTruncateTables()
{
    // 比对表version, 进行清库操作
    bool hasMe = false;
    std::vector<KERNEL_NS::SqlBuilder *> builders;
    for(auto iter : _tableNameRefTableInfo)
    {
        auto tableInfo = iter.second;
        auto logic = _GetDependenceLogic(tableInfo->_tableName);
        if(tableInfo->_simpleInfo->versionno() != _curVersionNo)
        {
            tableInfo->_simpleInfo->set_versionno(_curVersionNo);
            MaskStringKeyModifyDirty(tableInfo->_tableName);

            // 系统表也需要清理, 清理后需要立即创建
            if(logic == this)
                hasMe = true;

            // sql
            auto newBuilder = KERNEL_NS::TruncateTableSqlBuilder::NewThreadLocal_TruncateTableSqlBuilder();
            newBuilder->DB(_currentServiceDBName).Table(tableInfo->_tableName);
            builders.push_back(newBuilder);
        }
    }

    if(builders.empty())
        return true;

    UInt64 stub = 0;
    auto err = NewRequestBy(stub, _currentServiceDBName, GetStorageOperatorId(), builders, {});
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d"), err);
        GetService()->GetApp()->SinalFinish(Status::DBNewRequestFail);
        return false;
    }

    // 立即添加数据
    if(hasMe)
    {
        std::map<KERNEL_NS::LibString, TableInfo *> tmp;
        tmp.swap(_tableNameRefTableInfo);
        for(auto iter : tmp)
        {
            auto logic = _GetDependenceLogic(iter.first);
            _CreateNewTableInfo(logic);
        }
    }

    return true;
}

void MysqlMgr::_LoadAllPublicData()
{
    if(!_CheckTruncateTables())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("_CheckTruncateTables fail."));
        return;
    }

    // TODO:加载所有公共数据
    std::vector<KERNEL_NS::LibString> logics;
    for(auto iter : _tableNameRefLogic)
    {
        // 跳过自己
        auto logic = iter.second;
        if(logic == this)
            continue;

        auto storageInfo = logic->GetStorageInfo();

        // 查询库中所有表和字段: 需要校准:tbl_system_data, 1.表校准, 2.表字段校准
        KERNEL_NS::SelectSqlBuilder *selectBuilder = KERNEL_NS::SelectSqlBuilder::NewThreadLocal_SelectSqlBuilder();
        selectBuilder->DB(_currentServiceDBName).From(storageInfo->GetTableName());
        std::vector<KERNEL_NS::SqlBuilder *> builders;
        builders.push_back(selectBuilder);
        UInt64 stub = 0;
        auto var = KERNEL_NS::Variant::NewThreadLocal_Variant();
        *var = logic;
        auto err = NewRequestBy(stub, _currentServiceDBName, GetStorageOperatorId(), builders, {}, this, &MysqlMgr::_OnLoadPublicData, &var);
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d"), err);
            GetService()->GetApp()->SinalFinish(Status::DBNewRequestFail);
            return;
        }

        _loadPublicDataPending.insert(logic);
        logics.push_back(logic->GetObjName());
    }

    // 启动定时器监测
    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer->SetTimeOutHandler([this, logics](KERNEL_NS::LibTimer *t){
        if(_loadPublicDataPending.empty())
        {
            g_Log->Info(LOGFMT_OBJ_TAG("All public data load finish."));
            _PurgeDirty(this);

            // 抛加载结束事件
            auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::DB_LOADED_FINISH_ON_STARTUP);
            GetEventMgr()->FireEvent(ev);

            g_Log->Info(LOGFMT_OBJ_TAG("fire db loaded finish event."));

            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);

            return;
        }

        g_Log->Info(LOGFMT_OBJ_TAG("public data load left count:%llu, left system name:[%s]")
        , static_cast<UInt64>(logics.size()), KERNEL_NS::StringUtil::ToString(logics, ",").c_str());
    });

    timer->Schedule(3000);

    g_Log->Info(LOGFMT_OBJ_TAG("will load all public data count:%llu, list:[%s]"), static_cast<UInt64>(logics.size()), KERNEL_NS::StringUtil::ToString(logics, ",").c_str());
}

void MysqlMgr::_OnLoadPublicData(KERNEL_NS::MysqlResponse *res)
{
    g_Log->Info(LOGFMT_OBJ_TAG("mysql res:%s"), res->ToString().c_str());

    auto var = res->_var;
    auto logic = var->AsPtr<ILogicSys>();
    _loadPublicDataPending.erase(logic);

    if(res->_errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("_OnLoadPublicData fail db name:%s res seqId:%llu, mysqlError:%u")
                , res->_dbName.c_str(), res->_seqId, res->_mysqlErrno);

        GetService()->GetApp()->SinalFinish(Status::DBLoadDataFail);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    if(storageInfo->IsKvSystem())
    {
        const auto &keyName = storageInfo->GetKeyStorage()->GetFieldName();
        if(storageInfo->IsUsingNumberKey())
        {
            for(auto &record : res->_datas)
            {
                auto keyField = record->GetField(keyName);
                auto valueField = record->GetField(StorageCommonDefine::SYSTEM_DATA);
                auto k = keyField->GetUInt64();
                auto err = logic->OnLoaded(k, *valueField->GetData());
                if(err != Status::Success)
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("OnLoaded fail db name:%s res seqId:%llu, logic:%s, err:%d,k:%llu")
                            , res->_dbName.c_str(), res->_seqId, logic->GetObjName().c_str(), err, k);

                    GetService()->GetApp()->SinalFinish(Status::DBLoadDataFail);
                    return;
                }
            }
        }
        else
        {
            for(auto &record : res->_datas)
            {
                auto keyField = record->GetField(keyName);
                auto valueField = record->GetField(StorageCommonDefine::SYSTEM_DATA);
                KERNEL_NS::LibString k;
                keyField->GetString(k);
                auto err = logic->OnLoaded(k, *valueField->GetData());
                if(err != Status::Success)
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("OnLoaded fail db name:%s res seqId:%llu, logic:%s, err:%d, k:%s")
                            , res->_dbName.c_str(), res->_seqId, logic->GetObjName().c_str(), err, k.c_str());

                    GetService()->GetApp()->SinalFinish(Status::DBLoadDataFail);
                    return;
                }
            }
        }
    }
    else if(storageInfo->IsAsField())
    {
        for(auto &record : res->_datas)
        {
            auto v = record->GetField(0);
            auto err = logic->OnLoaded(*v->GetData());
            if(err != Status::Success)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("OnLoaded fail db name:%s res seqId:%llu, logic:%s, err:%d")
                        , res->_dbName.c_str(), res->_seqId, logic->GetObjName().c_str(), err);

                GetService()->GetApp()->SinalFinish(Status::DBLoadDataFail);
                return;
            }
        }
    }
    else
    {
        const auto &keyName = storageInfo->GetKeyStorage()->GetFieldName();
        if(storageInfo->IsUsingNumberKey())
        {
            for(auto &record : res->_datas)
            {
                auto keyField = record->GetField(keyName);
                auto &fieldDatas = record->GetFieldDatas();
                auto k = keyField->GetUInt64();
                auto err = logic->OnLoaded(k, fieldDatas);
                if(err != Status::Success)
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("OnLoaded fail db name:%s res seqId:%llu, logic:%s, err:%d, k:%llu")
                            , res->_dbName.c_str(), res->_seqId, logic->GetObjName().c_str(), err, k);

                    GetService()->GetApp()->SinalFinish(Status::DBLoadDataFail);
                    return;
                }
            }
        }
        else
        {
            for(auto &record : res->_datas)
            {
                auto keyField = record->GetField(keyName);
                KERNEL_NS::LibString k;
                keyField->GetString(k);
                auto &fieldDatas = record->GetFieldDatas();
                auto err = logic->OnLoaded(k, fieldDatas);
                if(err != Status::Success)
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("OnLoaded fail db name:%s res seqId:%llu, logic:%s, err:%d, k:%s")
                            , res->_dbName.c_str(), res->_seqId, logic->GetObjName().c_str(), err, k.c_str());

                    GetService()->GetApp()->SinalFinish(Status::DBLoadDataFail);
                    return;
                }
            }
        }
    }

    // 判断是否在系统表中有
    auto tableInfo = _GetTableInfo(storageInfo->GetTableName());
    if(!tableInfo)
        _CreateNewTableInfo(logic);
}

void MysqlMgr::_OnKvSystemNumberAddDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::ADD_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%llu"), key);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
    auto valueStorageInfo = storageInfo->GetSubStorageByFieldName(StorageCommonDefine::SYSTEM_DATA);
    data->Init(valueStorageInfo->GetCapacitySize());
    err = logic->OnSave(key, *data);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("OnSave fail err:%d, logic:%s, key:%llu"), err, logic->GetObjName().c_str(), key);
        KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
        return;
    }

    if(UNLIKELY(valueStorageInfo->GetCapacitySize() < static_cast<UInt64>(data->GetReadableSize())))
    {
        auto alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
        alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());
        valueStorageInfo->SetCapacitySize(data->GetReadableSize());

        KERNEL_NS::LibString newDescribe;
        if(!MysqlFieldTypeHelper::MakeFieldDescribe(valueStorageInfo, newDescribe))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                        , valueStorageInfo->ToString().c_str(), valueStorageInfo->GetFieldName().c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            return;
        }

        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {alterModifyColumn};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, {});
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d"), err);
            *dirtyHelper->MaskDirty(key, MysqlDirtyType::ADD_TYPE, true) = logic;
            return;
        }
    }

    // 保存数据sql
    auto newAddSqlBuilder = KERNEL_NS::InsertSqlBuilder::NewThreadLocal_InsertSqlBuilder();
    newAddSqlBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName()).Fields({StorageCommonDefine::NUMBER_KEY, StorageCommonDefine::SYSTEM_DATA})
    .Values({"?", "?"});

    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(2);

    {// key
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::NUMBER_KEY, MYSQL_TYPE_LONGLONG, 0);
        v->Write(&key, static_cast<Int64>(sizeof(key)));
        fields[0] = v;
    }

    {// value
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::SYSTEM_DATA , MYSQL_TYPE_BLOB, 0);
        v->SetData(data);
        fields[1] = v;
    }
    
    UInt64 stub = 0;
    std::vector<KERNEL_NS::SqlBuilder *> builders = {newAddSqlBuilder};
    err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fields);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%s"), err, logic->GetObjName().c_str(), key);
        *dirtyHelper->MaskDirty(key, MysqlDirtyType::ADD_TYPE, true) = logic;
        return;
    }
}

void MysqlMgr::_OnKvSystemNumberModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::MODIFY_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%llu"), key);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
    auto valueStorageInfo = storageInfo->GetSubStorageByFieldName(StorageCommonDefine::SYSTEM_DATA);
    data->Init(valueStorageInfo->GetCapacitySize());
    err = logic->OnSave(key, *data);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("OnSave fail err:%d, logic:%s, key:%llu"), err, logic->GetObjName().c_str(), key);
        KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
        return;
    }

    if(UNLIKELY(valueStorageInfo->GetCapacitySize() < static_cast<UInt64>(data->GetReadableSize())))
    {
        auto alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
        alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());
        valueStorageInfo->SetCapacitySize(data->GetReadableSize());

        KERNEL_NS::LibString newDescribe;
        if(!MysqlFieldTypeHelper::MakeFieldDescribe(valueStorageInfo, newDescribe))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                        , valueStorageInfo->ToString().c_str(), valueStorageInfo->GetFieldName().c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            return;
        }

        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {alterModifyColumn};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, {});
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d"), err);
            *dirtyHelper->MaskDirty(key, MysqlDirtyType::MODIFY_TYPE, true) = logic;
            return;
        }
    }

    // 保存数据sql
    auto newUpdateSqlBuilder = KERNEL_NS::UpdateSqlBuilder::NewThreadLocal_UpdateSqlBuilder();
    newUpdateSqlBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName())
    .Set(StorageCommonDefine::SYSTEM_DATA, "?")
    .Where(KERNEL_NS::LibString().AppendFormat("`%s`=?", StorageCommonDefine::NUMBER_KEY));

    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(2);

    {// value
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::SYSTEM_DATA , MYSQL_TYPE_BLOB, 0);
        v->SetData(data);
        fields[0] = v;
    }

    {// key
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::NUMBER_KEY , MYSQL_TYPE_LONGLONG, 0);
        v->Write(&key, static_cast<Int64>(sizeof(key)));
        fields[1] = v;
    }

    UInt64 stub = 0;
    std::vector<KERNEL_NS::SqlBuilder *> builders = {newUpdateSqlBuilder};
    err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fields);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%llu"), err, logic->GetObjName().c_str(), key);
        *dirtyHelper->MaskDirty(key, MysqlDirtyType::MODIFY_TYPE, true) = logic;
        return;
    }
}

void MysqlMgr::_OnKvSystemNumberDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::DEL_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%llu"), key);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    // 保存数据sql
    auto newDeleteSqlBuilder = KERNEL_NS::DeleteSqlBuilder::NewThreadLocal_DeleteSqlBuilder();
    newDeleteSqlBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName())
    .Where(KERNEL_NS::LibString().AppendFormat("`%s`=?", StorageCommonDefine::NUMBER_KEY));

    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(1);

    {// key
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::NUMBER_KEY, MYSQL_TYPE_LONGLONG, 0);
        v->Write(&key, static_cast<Int64>(sizeof(key)));
        fields[0] = v;
    }

    UInt64 stub = 0;
    std::vector<KERNEL_NS::SqlBuilder *> builders = {newDeleteSqlBuilder};
    err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fields);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%llu"), err, logic->GetObjName().c_str(), key);
        *dirtyHelper->MaskDirty(key, MysqlDirtyType::DEL_TYPE, true) = logic;
        return;
    }
}

void MysqlMgr::_OnKvSystemNumberReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::REPLACE_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%llu"), key);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
    auto valueStorageInfo = storageInfo->GetSubStorageByFieldName(StorageCommonDefine::SYSTEM_DATA);
    data->Init(valueStorageInfo->GetCapacitySize());
    err = logic->OnSave(key, *data);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("OnSave fail err:%d, logic:%s, key:%llu"), err, logic->GetObjName().c_str(), key);
        KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
        return;
    }

    if(UNLIKELY(valueStorageInfo->GetCapacitySize() < static_cast<UInt64>(data->GetReadableSize())))
    {
        auto alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
        alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());
        valueStorageInfo->SetCapacitySize(data->GetReadableSize());

        KERNEL_NS::LibString newDescribe;
        if(!MysqlFieldTypeHelper::MakeFieldDescribe(valueStorageInfo, newDescribe))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                        , valueStorageInfo->ToString().c_str(), valueStorageInfo->GetFieldName().c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            return;
        }

        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {alterModifyColumn};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, {});
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d"), err);
            *dirtyHelper->MaskDirty(key, MysqlDirtyType::REPLACE_TYPE, true) = logic;
            return;
        }
    }

    // 保存数据sql
    auto newReplaceSqlBuilder = KERNEL_NS::ReplaceIntoSqlBuilder::NewThreadLocal_ReplaceIntoSqlBuilder();
    newReplaceSqlBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName())
    .Fields({StorageCommonDefine::NUMBER_KEY, StorageCommonDefine::SYSTEM_DATA})
    .Values({"?", "?"});

    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(2);

    {// key
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::NUMBER_KEY, MYSQL_TYPE_LONGLONG, 0);
        v->Write(&key, static_cast<Int64>(sizeof(key)));
        fields[0] = v;
    }

    {// value
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::SYSTEM_DATA , MYSQL_TYPE_BLOB, 0);
        v->SetData(data);
        fields[1] = v;
    }
    
    UInt64 stub = 0;
    std::vector<KERNEL_NS::SqlBuilder *> builders = {newReplaceSqlBuilder};
    err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fields);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%llu"), err, logic->GetObjName().c_str(), key);
        *dirtyHelper->MaskDirty(key, MysqlDirtyType::REPLACE_TYPE, true) = logic;
        return;
    }
}

void MysqlMgr::_OnKvSystemStringAddDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
   dirtyHelper->Clear(key, MysqlDirtyType::ADD_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%llu"), key);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
    auto valueStorageInfo = storageInfo->GetSubStorageByFieldName(StorageCommonDefine::SYSTEM_DATA);
    data->Init(valueStorageInfo->GetCapacitySize());
    err = logic->OnSave(key, *data);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("OnSave fail err:%d, logic:%s, key:%s"), err, logic->GetObjName().c_str(), key.c_str());
        KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
        return;
    }

    if(UNLIKELY(valueStorageInfo->GetCapacitySize() < static_cast<UInt64>(data->GetReadableSize())))
    {
        auto alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
        alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());
        valueStorageInfo->SetCapacitySize(data->GetReadableSize());

        KERNEL_NS::LibString newDescribe;
        if(!MysqlFieldTypeHelper::MakeFieldDescribe(valueStorageInfo, newDescribe))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                        , valueStorageInfo->ToString().c_str(), valueStorageInfo->GetFieldName().c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            return;
        }

        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {alterModifyColumn};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, {});
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d"), err);
            *dirtyHelper->MaskDirty(key, MysqlDirtyType::ADD_TYPE, true) = logic;
            return;
        }
    }

    // 保存数据sql
    auto newAddSqlBuilder = KERNEL_NS::InsertSqlBuilder::NewThreadLocal_InsertSqlBuilder();
    newAddSqlBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName()).Fields({StorageCommonDefine::STRING_KEY, StorageCommonDefine::SYSTEM_DATA})
    .Values({"?", "?"});

    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(2);

    {// key
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::STRING_KEY , MYSQL_TYPE_VAR_STRING, 0);
        v->Write(key.data(), static_cast<Int64>(key.size()));
        fields[0] = v;
    }

    {// value
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::SYSTEM_DATA , MYSQL_TYPE_BLOB, 0);
        v->SetData(data);
        fields[1] = v;
    }
    
    UInt64 stub = 0;
    std::vector<KERNEL_NS::SqlBuilder *> builders = {newAddSqlBuilder};
    err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fields);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%s"), err, logic->GetObjName().c_str(), key.c_str());
        *dirtyHelper->MaskDirty(key, MysqlDirtyType::ADD_TYPE, true) = logic;
        return;
    }
}

void MysqlMgr::_OnKvSystemStringModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::MODIFY_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%llu"), key);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
    auto valueStorageInfo = storageInfo->GetSubStorageByFieldName(StorageCommonDefine::SYSTEM_DATA);
    data->Init(valueStorageInfo->GetCapacitySize());
    err = logic->OnSave(key, *data);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("OnSave fail err:%d, logic:%s, key:%s"), err, logic->GetObjName().c_str(), key.c_str());
        KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
        return;
    }
    if(UNLIKELY(valueStorageInfo->GetCapacitySize() < static_cast<UInt64>(data->GetReadableSize())))
    {
        auto alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
        alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());
        valueStorageInfo->SetCapacitySize(data->GetReadableSize());

        KERNEL_NS::LibString newDescribe;
        if(!MysqlFieldTypeHelper::MakeFieldDescribe(valueStorageInfo, newDescribe))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                        , valueStorageInfo->ToString().c_str(), valueStorageInfo->GetFieldName().c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            return;
        }

        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {alterModifyColumn};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, {});
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d"), err);
            *dirtyHelper->MaskDirty(key, MysqlDirtyType::MODIFY_TYPE, true) = logic;
            return;
        }
    }

    // 保存数据sql
    auto newUpdateSqlBuilder = KERNEL_NS::UpdateSqlBuilder::NewThreadLocal_UpdateSqlBuilder();
    newUpdateSqlBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName())
    .Set(StorageCommonDefine::SYSTEM_DATA, "?")
    .Where(KERNEL_NS::LibString().AppendFormat("`%s`=?", StorageCommonDefine::STRING_KEY));

    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(2);

    {// value
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::SYSTEM_DATA ,  MYSQL_TYPE_BLOB, 0);
        v->SetData(data);
        fields[0] = v;
    }

    {// key
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::STRING_KEY , MYSQL_TYPE_VAR_STRING, 0);
        v->Write(key.data(), static_cast<Int64>(key.size()));
        fields[1] = v;
    }

    UInt64 stub = 0;
    std::vector<KERNEL_NS::SqlBuilder *> builders = {newUpdateSqlBuilder};
    err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fields);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%s"), err, logic->GetObjName().c_str(), key.c_str());
        *dirtyHelper->MaskDirty(key, MysqlDirtyType::MODIFY_TYPE, true) = logic;
        return;
    }
}

void MysqlMgr::_OnKvSystemStringDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
   dirtyHelper->Clear(key, MysqlDirtyType::DEL_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%llu"), key);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    // 保存数据sql
    auto newDeleteSqlBuilder = KERNEL_NS::DeleteSqlBuilder::NewThreadLocal_DeleteSqlBuilder();
    newDeleteSqlBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName())
    .Where(KERNEL_NS::LibString().AppendFormat("`%s`=?", StorageCommonDefine::STRING_KEY));

    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(1);

    {// key
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::STRING_KEY, MYSQL_TYPE_VAR_STRING, 0);
        v->Write(key.data(), static_cast<Int64>(key.size()));
        fields[0] = v;
    }

    UInt64 stub = 0;
    std::vector<KERNEL_NS::SqlBuilder *> builders = {newDeleteSqlBuilder};
    err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fields);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%s"), err, logic->GetObjName().c_str(), key.c_str());
        *dirtyHelper->MaskDirty(key, MysqlDirtyType::DEL_TYPE, true) = logic;
        return;
    }
}

void MysqlMgr::_OnKvSystemStringReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::REPLACE_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%llu"), key);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
    auto valueStorageInfo = storageInfo->GetSubStorageByFieldName(StorageCommonDefine::SYSTEM_DATA);
    data->Init(valueStorageInfo->GetCapacitySize());
    err = logic->OnSave(key, *data);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("OnSave fail err:%d, logic:%s, key:%s"), err, logic->GetObjName().c_str(), key.c_str());
        KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
        return;
    }
    if(UNLIKELY(valueStorageInfo->GetCapacitySize() < static_cast<UInt64>(data->GetReadableSize())))
    {
        auto alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
        alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());
        valueStorageInfo->SetCapacitySize(data->GetReadableSize());

        KERNEL_NS::LibString newDescribe;
        if(!MysqlFieldTypeHelper::MakeFieldDescribe(valueStorageInfo, newDescribe))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                        , valueStorageInfo->ToString().c_str(), valueStorageInfo->GetFieldName().c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            return;
        }

        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {alterModifyColumn};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, {});
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, key:%s"), err, key.c_str());
            *dirtyHelper->MaskDirty(key, MysqlDirtyType::REPLACE_TYPE, true) = logic;
            return;
        }
    }

    // 保存数据sql
    auto newReplaceSqlBuilder = KERNEL_NS::ReplaceIntoSqlBuilder::NewThreadLocal_ReplaceIntoSqlBuilder();
    newReplaceSqlBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName())
    .Fields({StorageCommonDefine::STRING_KEY, StorageCommonDefine::SYSTEM_DATA})
    .Values({"?", "?"});

    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(2);

    {// key
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::STRING_KEY ,  MYSQL_TYPE_VAR_STRING, 0);
        v->Write(key.data(), static_cast<Int64>(key.size()));
        fields[0] = v;
    }

    {// value
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), StorageCommonDefine::SYSTEM_DATA ,  MYSQL_TYPE_BLOB, 0);
        v->SetData(data);
        fields[1] = v;
    }
    
    UInt64 stub = 0;
    std::vector<KERNEL_NS::SqlBuilder *> builders = {newReplaceSqlBuilder};
    err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fields);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s,  key:%s"), err, logic->GetObjName().c_str(), key.c_str());
        *dirtyHelper->MaskDirty(key, MysqlDirtyType::REPLACE_TYPE, true) = logic;
        return;
    }
}

void MysqlMgr::_OnNumberAddDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::ADD_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%llu"), key);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    // 多字段但是不是kv系统的: 1.获取持久化数据, 2.根据持久化数据大小判断是否需要modify字段(由于是运行时不考虑增删字段, 只考虑修改大小, 而且必须比当前系统StorageInfo中的容量大)
    std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> dbInfo;
    err = logic->OnSave(key, dbInfo);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("logic save db data fail err:%d, logic:%s, key:%llu"), err, logic->GetObjName().c_str(), key);
        KERNEL_NS::ContainerUtil::DelContainer(dbInfo, [](KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *ptr){
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(ptr);
        });
        return;
    }

    KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
    alterModifyColumn.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::AlterTableSqlBuilder *>(p);
            KERNEL_NS::AlterTableSqlBuilder::DeleteThreadLocal_AlterTableSqlBuilder(ptr);
    });
    alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());

    KERNEL_NS::SmartPtr<KERNEL_NS::InsertSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> insertBuilder = KERNEL_NS::InsertSqlBuilder::NewThreadLocal_InsertSqlBuilder();
    insertBuilder.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::InsertSqlBuilder *>(p);
            KERNEL_NS::InsertSqlBuilder::DeleteThreadLocal_InsertSqlBuilder(ptr);
    });
    insertBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName());
    
    bool hasModify = false;
    std::vector<KERNEL_NS::LibString> fields;
    std::vector<KERNEL_NS::LibString> values;
    std::vector<KERNEL_NS::Field *> fieldDatas;
    for(auto iter = dbInfo.begin(); iter != dbInfo.end();)
    {
        auto &fieldName = iter->first;
        auto data = iter->second;
        auto subStorage = storageInfo->GetSubStorageByFieldName(fieldName);
        if(UNLIKELY(!subStorage))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("unknown field name when onsave fieldName:%s, table name:%s, system name:%s"), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            iter = dbInfo.erase(iter);
            continue;
        }

        // 固定大小的不需要判断
        if(subStorage->IsNumberField())
        {
            fields.push_back(fieldName);
            values.push_back("?");

            KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                            ,fieldName , subStorage->GetInputMysqlDataType(), 0);

            v->SetData(data);
            fieldDatas.push_back(v);

            ++iter;
            continue;
        }

        // 需要alter table
        if(UNLIKELY(static_cast<UInt64>(data->GetReadableSize()) > subStorage->GetCapacitySize()))
        {
            subStorage->SetCapacitySize(data->GetReadableSize());

            KERNEL_NS::LibString newDescribe;
            if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorage, newDescribe))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                            , subStorage->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
                
                KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
                iter = dbInfo.erase(iter);
                continue;
            }
            
            hasModify = true;
            alterModifyColumn->Modify(fieldName, newDescribe);
            g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: add dirty modify field, will modify column => new field describe:%s table name:%s, field name:%s, system name:%s")
                        , newDescribe.c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());
        }

        fields.push_back(fieldName);
        values.push_back("?");
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                        ,fieldName , subStorage->GetInputMysqlDataType(), 0);

        v->SetData(data);
        fieldDatas.push_back(v);
        ++iter;
    }

    if(UNLIKELY(hasModify))
    {
        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {alterModifyColumn.pop()};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, {});
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%llu"), err, logic->GetObjName().c_str(), key);
            return;
        }
    }

    if(LIKELY(!fields.empty()))
    {
        insertBuilder->Fields(fields);
        insertBuilder->Values(values);
        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {insertBuilder.pop()};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fieldDatas);
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s, key:%llu"), err, logic->GetObjName().c_str(), key);
            *dirtyHelper->MaskDirty(key, MysqlDirtyType::ADD_TYPE, true) = logic;
        }
    }
}

void MysqlMgr::_OnNumberModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::MODIFY_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%llu"), key);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    // 多字段但是不是kv系统的: 1.获取持久化数据, 2.根据持久化数据大小判断是否需要modify字段(由于是运行时不考虑增删字段, 只考虑修改大小, 而且必须比当前系统StorageInfo中的容量大)
    std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> dbInfo;
    err = logic->OnSave(key, dbInfo);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("logic save db data fail err:%d, logic:%s, key:%llu"), err, logic->GetObjName().c_str(), key);
        KERNEL_NS::ContainerUtil::DelContainer(dbInfo, [](KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *ptr){
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(ptr);
        });
        return;
    }

    KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
    alterModifyColumn.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::AlterTableSqlBuilder *>(p);
            KERNEL_NS::AlterTableSqlBuilder::DeleteThreadLocal_AlterTableSqlBuilder(ptr);
    });
    alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());

    KERNEL_NS::SmartPtr<KERNEL_NS::UpdateSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> updateBuilder = KERNEL_NS::UpdateSqlBuilder::NewThreadLocal_UpdateSqlBuilder();
    updateBuilder.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::UpdateSqlBuilder *>(p);
            KERNEL_NS::UpdateSqlBuilder::DeleteThreadLocal_UpdateSqlBuilder(ptr);
    });
    updateBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName());
    auto keyStorageInfo = storageInfo->GetKeyStorage();
    
    bool hasModify = false;
    std::vector<KERNEL_NS::Field *> fieldDatas;
    KERNEL_NS::SmartPtr<KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>, KERNEL_NS::AutoDelMethods::CustomDelete> keyData = NULL;
    keyData.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *>(p);
        KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(ptr);
    });

    for(auto iter = dbInfo.begin(); iter != dbInfo.end();)
    {
        auto &fieldName = iter->first;
        auto data = iter->second;
        auto subStorage = storageInfo->GetSubStorageByFieldName(fieldName);
        if(UNLIKELY(!subStorage))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("unknown field name when onsave fieldName:%s, table name:%s, system name:%s"), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            iter = dbInfo.erase(iter);
            continue;
        }

        // 不需要key
        if(fieldName == keyStorageInfo->GetFieldName())
        {
            keyData = data;
            iter = dbInfo.erase(iter);
            continue;
        }

        // 固定大小的不需要判断
        if(subStorage->IsNumberField())
        {
            updateBuilder->Set(fieldName, "?");
            KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                            ,fieldName , subStorage->GetInputMysqlDataType(), 0);

            v->SetData(data);
            fieldDatas.push_back(v);

            ++iter;
            continue;
        }

        // 需要alter table
        if(UNLIKELY(static_cast<UInt64>(data->GetReadableSize()) > subStorage->GetCapacitySize()))
        {
            subStorage->SetCapacitySize(data->GetReadableSize());

            KERNEL_NS::LibString newDescribe;
            if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorage, newDescribe))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                            , subStorage->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
                
                KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
                iter = dbInfo.erase(iter);
                continue;
            }
            
            hasModify = true;
            alterModifyColumn->Modify(fieldName, newDescribe);
            g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: add dirty modify field, will modify column => new field describe:%s table name:%s, field name:%s, system name:%s")
                        , newDescribe.c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());
        }

        updateBuilder->Set(fieldName, "?");
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                        ,fieldName , subStorage->GetInputMysqlDataType(), 0);

        v->SetData(data);
        fieldDatas.push_back(v);
        ++iter;
    }

    if(UNLIKELY(hasModify))
    {
        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {alterModifyColumn.pop()};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, {});
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%llu"), err, logic->GetObjName().c_str(), key);
            return;
        }
    }

    if(LIKELY(!fieldDatas.empty()))
    {
        updateBuilder->Where(KERNEL_NS::LibString().AppendFormat("`%s` = ?", keyStorageInfo->GetFieldName().c_str()));
        if(!keyData)
        {
            keyData = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
            keyData->Init(static_cast<Int64>(sizeof(key)));
            keyData->Write(&key, static_cast<Int64>(sizeof(key)));
        }

        // key
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                        , keyStorageInfo->GetFieldName() , keyStorageInfo->GetInputMysqlDataType(), 0);

        v->SetData(keyData.pop());
        fieldDatas.push_back(v);

        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {updateBuilder.pop()};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fieldDatas);
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s, key:%llu"), err, logic->GetObjName().c_str(), key);
            *dirtyHelper->MaskDirty(key, MysqlDirtyType::MODIFY_TYPE, true) = logic;
        }
    }
}

void MysqlMgr::_OnNumberDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::DEL_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%llu"), key);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }


    // 保存数据sql
    auto keyStorageInfo = storageInfo->GetKeyStorage();
    auto newDeleteSqlBuilder = KERNEL_NS::DeleteSqlBuilder::NewThreadLocal_DeleteSqlBuilder();
    newDeleteSqlBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName())
    .Where(KERNEL_NS::LibString().AppendFormat("`%s`=?", keyStorageInfo->GetFieldName().c_str()));

    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(1);

    {// key
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), keyStorageInfo->GetFieldName(), keyStorageInfo->GetInputMysqlDataType(), 0);
        v->Write(&key, static_cast<Int64>(sizeof(key)));
        fields[0] = v;
    }

    UInt64 stub = 0;
    std::vector<KERNEL_NS::SqlBuilder *> builders = {newDeleteSqlBuilder};
    err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fields);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%llu"), err, logic->GetObjName().c_str(), key);
        *dirtyHelper->MaskDirty(key, MysqlDirtyType::DEL_TYPE, true) = logic;
        return;
    }
}

void MysqlMgr::_OnNumberReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::REPLACE_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%llu"), key);
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    // 多字段但是不是kv系统的: 1.获取持久化数据, 2.根据持久化数据大小判断是否需要modify字段(由于是运行时不考虑增删字段, 只考虑修改大小, 而且必须比当前系统StorageInfo中的容量大)
    std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> dbInfo;
    err = logic->OnSave(key, dbInfo);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("logic save db data fail err:%d, logic:%s, key:%llu"), err, logic->GetObjName().c_str(), key);
        KERNEL_NS::ContainerUtil::DelContainer(dbInfo, [](KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *ptr){
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(ptr);
        });
        return;
    }

    KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
    alterModifyColumn.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::AlterTableSqlBuilder *>(p);
            KERNEL_NS::AlterTableSqlBuilder::DeleteThreadLocal_AlterTableSqlBuilder(ptr);
    });
    alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());

    KERNEL_NS::SmartPtr<KERNEL_NS::ReplaceIntoSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> replaceBuilder = KERNEL_NS::ReplaceIntoSqlBuilder::NewThreadLocal_ReplaceIntoSqlBuilder();
    replaceBuilder.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::ReplaceIntoSqlBuilder *>(p);
            KERNEL_NS::ReplaceIntoSqlBuilder::DeleteThreadLocal_ReplaceIntoSqlBuilder(ptr);
    });
    replaceBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName());
    
    bool hasModify = false;
    std::vector<KERNEL_NS::LibString> fields;
    std::vector<KERNEL_NS::LibString> values;
    std::vector<KERNEL_NS::Field *> fieldDatas;
    for(auto iter = dbInfo.begin(); iter != dbInfo.end();)
    {
        auto &fieldName = iter->first;
        auto data = iter->second;
        auto subStorage = storageInfo->GetSubStorageByFieldName(fieldName);
        if(UNLIKELY(!subStorage))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("unknown field name when onsave fieldName:%s, table name:%s, system name:%s"), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            iter = dbInfo.erase(iter);
            continue;
        }

        // 固定大小的不需要判断
        if(subStorage->IsNumberField())
        {
            fields.push_back(fieldName);
            values.push_back("?");

            KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                            ,fieldName , subStorage->GetInputMysqlDataType(), 0);

            v->SetData(data);
            fieldDatas.push_back(v);

            ++iter;
            continue;
        }

        // 需要alter table
        if(UNLIKELY(static_cast<UInt64>(data->GetReadableSize()) > subStorage->GetCapacitySize()))
        {
            subStorage->SetCapacitySize(data->GetReadableSize());

            KERNEL_NS::LibString newDescribe;
            if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorage, newDescribe))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                            , subStorage->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
                
                KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
                iter = dbInfo.erase(iter);
                continue;
            }
            
            hasModify = true;
            alterModifyColumn->Modify(fieldName, newDescribe);
            g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: add dirty modify field, will modify column => new field describe:%s table name:%s, field name:%s, system name:%s")
                        , newDescribe.c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());
        }

        fields.push_back(fieldName);
        values.push_back("?");
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                        ,fieldName , subStorage->GetInputMysqlDataType(), 0);

        v->SetData(data);
        fieldDatas.push_back(v);
        ++iter;
    }

    if(UNLIKELY(hasModify))
    {
        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {alterModifyColumn.pop()};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, {});
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%llu"), err, logic->GetObjName().c_str(), key);
            return;
        }
    }

    if(LIKELY(!fields.empty()))
    {
        replaceBuilder->Fields(fields);
        replaceBuilder->Values(values);
        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {replaceBuilder.pop()};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fieldDatas);
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s, key:%llu"), err, logic->GetObjName().c_str(), key);
            *dirtyHelper->MaskDirty(key, MysqlDirtyType::REPLACE_TYPE, true) = logic;
        }
    }
}

void MysqlMgr::_OnStringAddDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::ADD_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%s"), key.c_str());
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    // 多字段但是不是kv系统的: 1.获取持久化数据, 2.根据持久化数据大小判断是否需要modify字段(由于是运行时不考虑增删字段, 只考虑修改大小, 而且必须比当前系统StorageInfo中的容量大)
    std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> dbInfo;
    err = logic->OnSave(key, dbInfo);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("logic save db data fail err:%d, logic:%s, key:%s"), err, logic->GetObjName().c_str(), key.c_str());
        KERNEL_NS::ContainerUtil::DelContainer(dbInfo, [](KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *ptr){
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(ptr);
        });
        return;
    }

    KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
    alterModifyColumn.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::AlterTableSqlBuilder *>(p);
            KERNEL_NS::AlterTableSqlBuilder::DeleteThreadLocal_AlterTableSqlBuilder(ptr);
    });
    alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());

    KERNEL_NS::SmartPtr<KERNEL_NS::InsertSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> insertBuilder = KERNEL_NS::InsertSqlBuilder::NewThreadLocal_InsertSqlBuilder();
    insertBuilder.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::InsertSqlBuilder *>(p);
            KERNEL_NS::InsertSqlBuilder::DeleteThreadLocal_InsertSqlBuilder(ptr);
    });
    insertBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName());
    
    bool hasModify = false;
    std::vector<KERNEL_NS::LibString> fields;
    std::vector<KERNEL_NS::LibString> values;
    std::vector<KERNEL_NS::Field *> fieldDatas;
    for(auto iter = dbInfo.begin(); iter != dbInfo.end();)
    {
        auto &fieldName = iter->first;
        auto data = iter->second;
        auto subStorage = storageInfo->GetSubStorageByFieldName(fieldName);
        if(UNLIKELY(!subStorage))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("unknown field name when onsave fieldName:%s, table name:%s, system name:%s"), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            iter = dbInfo.erase(iter);
            continue;
        }

        // 固定大小的不需要判断
        if(subStorage->IsNumberField())
        {
            fields.push_back(fieldName);
            values.push_back("?");

            KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                            ,fieldName , subStorage->GetInputMysqlDataType(), 0);

            v->SetData(data);
            fieldDatas.push_back(v);
            ++iter;
            continue;
        }

        // 需要alter table
        if(UNLIKELY(static_cast<UInt64>(data->GetReadableSize()) > subStorage->GetCapacitySize()))
        {
            subStorage->SetCapacitySize(data->GetReadableSize());

            KERNEL_NS::LibString newDescribe;
            if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorage, newDescribe))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                            , subStorage->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
                
                KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
                iter = dbInfo.erase(iter);
                continue;
            }
            
            hasModify = true;
            alterModifyColumn->Modify(fieldName, newDescribe);
            g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: add dirty modify field, will modify column => new field describe:%s table name:%s, field name:%s, system name:%s")
                        , newDescribe.c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());
        }

        fields.push_back(fieldName);
        values.push_back("?");
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                        ,fieldName , subStorage->GetInputMysqlDataType(), 0);
        v->SetData(data);
        fieldDatas.push_back(v);
        ++iter;
    }

    if(UNLIKELY(hasModify))
    {
        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {alterModifyColumn.pop()};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, {});
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%s"), err, logic->GetObjName().c_str(), key.c_str());
            return;
        }
    }

    if(LIKELY(!fields.empty()))
    {
        insertBuilder->Fields(fields);
        insertBuilder->Values(values);
        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {insertBuilder.pop()};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fieldDatas);
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s, key:%s"), err, logic->GetObjName().c_str(), key.c_str());
            *dirtyHelper->MaskDirty(key, MysqlDirtyType::ADD_TYPE, true) = logic;
        }
    }
}

void MysqlMgr::_OnStringModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::MODIFY_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%s"), key.c_str());
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    // 多字段但是不是kv系统的: 1.获取持久化数据, 2.根据持久化数据大小判断是否需要modify字段(由于是运行时不考虑增删字段, 只考虑修改大小, 而且必须比当前系统StorageInfo中的容量大)
    std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> dbInfo;
    err = logic->OnSave(key, dbInfo);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("logic save db data fail err:%d, logic:%s, key:%s"), err, logic->GetObjName().c_str(), key.c_str());
        KERNEL_NS::ContainerUtil::DelContainer(dbInfo, [](KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *ptr){
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(ptr);
        });
        return;
    }

    KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
    alterModifyColumn.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::AlterTableSqlBuilder *>(p);
            KERNEL_NS::AlterTableSqlBuilder::DeleteThreadLocal_AlterTableSqlBuilder(ptr);
    });
    alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());

    KERNEL_NS::SmartPtr<KERNEL_NS::UpdateSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> updateBuilder = KERNEL_NS::UpdateSqlBuilder::NewThreadLocal_UpdateSqlBuilder();
    updateBuilder.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::UpdateSqlBuilder *>(p);
            KERNEL_NS::UpdateSqlBuilder::DeleteThreadLocal_UpdateSqlBuilder(ptr);
    });
    updateBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName());
    auto keyStorageInfo = storageInfo->GetKeyStorage();
    
    bool hasModify = false;
    std::vector<KERNEL_NS::Field *> fieldDatas;
    KERNEL_NS::SmartPtr<KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>, KERNEL_NS::AutoDelMethods::CustomDelete> keyData = NULL;
    keyData.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *>(p);
        KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(ptr);
    });
    for(auto iter = dbInfo.begin(); iter != dbInfo.end();)
    {
        auto &fieldName = iter->first;
        auto data = iter->second;
        auto subStorage = storageInfo->GetSubStorageByFieldName(fieldName);
        if(UNLIKELY(!subStorage))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("unknown field name when onsave fieldName:%s, table name:%s, system name:%s"), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            iter = dbInfo.erase(iter);
            continue;
        }

        // 不需要key
        if(fieldName == keyStorageInfo->GetFieldName())
        {
            keyData = data;
            iter = dbInfo.erase(iter);
            continue;
        }

        // 固定大小的不需要判断
        if(subStorage->IsNumberField())
        {
            updateBuilder->Set(fieldName, "?");
            KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                            ,fieldName , subStorage->GetInputMysqlDataType(), 0);

            v->SetData(data);
            fieldDatas.push_back(v);

            ++iter;
            continue;
        }

        // 需要alter table
        if(UNLIKELY(static_cast<UInt64>(data->GetReadableSize()) > subStorage->GetCapacitySize()))
        {
            subStorage->SetCapacitySize(data->GetReadableSize());

            KERNEL_NS::LibString newDescribe;
            if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorage, newDescribe))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                            , subStorage->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
                
                KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
                iter = dbInfo.erase(iter);
                continue;
            }
            
            hasModify = true;
            alterModifyColumn->Modify(fieldName, newDescribe);
            g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: add dirty modify field, will modify column => new field describe:%s table name:%s, field name:%s, system name:%s")
                        , newDescribe.c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());
        }

        updateBuilder->Set(fieldName, "?");
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                        ,fieldName , subStorage->GetInputMysqlDataType(), 0);

        v->SetData(data);
        fieldDatas.push_back(v);
        ++iter;
    }

    if(UNLIKELY(hasModify))
    {
        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {alterModifyColumn.pop()};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, {});
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%s"), err, logic->GetObjName().c_str(), key.c_str());
            return;
        }
    }

    if(LIKELY(!fieldDatas.empty()))
    {
        updateBuilder->Where(KERNEL_NS::LibString().AppendFormat("`%s` like '?'", keyStorageInfo->GetFieldName().c_str()));
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                        ,keyStorageInfo->GetFieldName() , keyStorageInfo->GetInputMysqlDataType(), 0);

        if(UNLIKELY(!keyData))
        {
            keyData = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
            keyData->Init(static_cast<Int64>(key.size()));
            keyData->Write(key.data(), static_cast<Int64>(key.size()));
        }
        v->SetData(keyData.pop());
        fieldDatas.push_back(v);

        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {updateBuilder.pop()};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fieldDatas);
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s, key:%s"), err, logic->GetObjName().c_str(), key.c_str());
            *dirtyHelper->MaskDirty(key, MysqlDirtyType::MODIFY_TYPE, true) = logic;
        }
    }
}

void MysqlMgr::_OnStringDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::DEL_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%s"), key.c_str());
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    // 保存数据sql
    auto keyStorageInfo = storageInfo->GetKeyStorage();
    auto newDeleteSqlBuilder = KERNEL_NS::DeleteSqlBuilder::NewThreadLocal_DeleteSqlBuilder();
    newDeleteSqlBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName())
    .Where(KERNEL_NS::LibString().AppendFormat("`%s`=?",  keyStorageInfo->GetFieldName().c_str()));

    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(1);

    {// key
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), keyStorageInfo->GetFieldName(), MYSQL_TYPE_VAR_STRING, 0);
        v->Write(key.data(), static_cast<Int64>(key.size()));
        fields[0] = v;
    }

    UInt64 stub = 0;
    std::vector<KERNEL_NS::SqlBuilder *> builders = {newDeleteSqlBuilder};
    err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fields);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%s"), err, logic->GetObjName().c_str(), key.c_str());
        *dirtyHelper->MaskDirty(key, MysqlDirtyType::DEL_TYPE, true) = logic;
        return;
    }
}

void MysqlMgr::_OnStringReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params)
{
    auto logic = params->AsPtr<ILogicSys>();
    dirtyHelper->Clear(key, MysqlDirtyType::REPLACE_TYPE);
    if(UNLIKELY(!logic))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("params is not logic sys please check, key:%s"), key.c_str());
        return;
    }

    auto storageInfo = logic->GetStorageInfo();
    Int32 err = Status::Success;

    // 如果oid没有指定, 需要告警
    auto oid = logic->GetStorageOperatorId();
    if(!storageInfo->IsLoadDataOnStartup() && oid == _systemOperatorUid)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system is not load on startup, cant use system operator uid, need get a new oid from mysql mgr for mysql balance table name:%s, system name:%s")
                    ,storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
    }

    // 多字段但是不是kv系统的: 1.获取持久化数据, 2.根据持久化数据大小判断是否需要modify字段(由于是运行时不考虑增删字段, 只考虑修改大小, 而且必须比当前系统StorageInfo中的容量大)
    std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> dbInfo;
    err = logic->OnSave(key, dbInfo);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("logic save db data fail err:%d, logic:%s, key:%s"), err, logic->GetObjName().c_str(), key.c_str());
        KERNEL_NS::ContainerUtil::DelContainer(dbInfo, [](KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *ptr){
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(ptr);
        });
        return;
    }

    KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
    alterModifyColumn.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::AlterTableSqlBuilder *>(p);
            KERNEL_NS::AlterTableSqlBuilder::DeleteThreadLocal_AlterTableSqlBuilder(ptr);
    });
    alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());

    KERNEL_NS::SmartPtr<KERNEL_NS::ReplaceIntoSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> replaceBuilder = KERNEL_NS::ReplaceIntoSqlBuilder::NewThreadLocal_ReplaceIntoSqlBuilder();
    replaceBuilder.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::ReplaceIntoSqlBuilder *>(p);
            KERNEL_NS::ReplaceIntoSqlBuilder::DeleteThreadLocal_ReplaceIntoSqlBuilder(ptr);
    });
    replaceBuilder->DB(_currentServiceDBName).Table(storageInfo->GetTableName());
    
    bool hasModify = false;
    std::vector<KERNEL_NS::LibString> fields;
    std::vector<KERNEL_NS::LibString> values;
    std::vector<KERNEL_NS::Field *> fieldDatas;
    for(auto iter = dbInfo.begin(); iter != dbInfo.end();)
    {
        auto &fieldName = iter->first;
        auto data = iter->second;
        auto subStorage = storageInfo->GetSubStorageByFieldName(fieldName);
        if(UNLIKELY(!subStorage))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("unknown field name when onsave fieldName:%s, table name:%s, system name:%s"), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            iter = dbInfo.erase(iter);
            continue;
        }

        // 固定大小的不需要判断
        if(subStorage->IsNumberField())
        {
            fields.push_back(fieldName);
            values.push_back("?");

            KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                            ,fieldName , subStorage->GetInputMysqlDataType(), 0);

            v->SetData(data);
            fieldDatas.push_back(v);

            ++iter;
            continue;
        }

        // 需要alter table
        if(UNLIKELY(static_cast<UInt64>(data->GetReadableSize()) > subStorage->GetCapacitySize()))
        {
            subStorage->SetCapacitySize(data->GetReadableSize());

            KERNEL_NS::LibString newDescribe;
            if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorage, newDescribe))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                            , subStorage->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
                
                KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
                iter = dbInfo.erase(iter);
                continue;
            }
            
            hasModify = true;
            alterModifyColumn->Modify(fieldName, newDescribe);
            g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: add dirty modify field, will modify column => new field describe:%s table name:%s, field name:%s, system name:%s")
                        , newDescribe.c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());
        }

        fields.push_back(fieldName);
        values.push_back("?");
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName()
                        ,fieldName , subStorage->GetInputMysqlDataType(), 0);

        v->SetData(data);
        fieldDatas.push_back(v);
        ++iter;
    }

    if(UNLIKELY(hasModify))
    {
        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {alterModifyColumn.pop()};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, {});
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s key:%s"), err, logic->GetObjName().c_str(), key.c_str());
            return;
        }
    }

    if(LIKELY(!fields.empty()))
    {
        replaceBuilder->Fields(fields);
        replaceBuilder->Values(values);
        UInt64 stub = 0;
        std::vector<KERNEL_NS::SqlBuilder *> builders = {replaceBuilder.pop()};
        err =  NewRequestBy(stub, _currentServiceDBName, logic->GetStorageOperatorId(), builders, fieldDatas);
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, logic:%s, key:%s"), err, logic->GetObjName().c_str(), key.c_str());
            *dirtyHelper->MaskDirty(key, MysqlDirtyType::REPLACE_TYPE, true) = logic;
        }
    }
}

void MysqlMgr::_InitNumberDirtyHelper(const IStorageInfo *storageInfo, KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper) 
{
    dirtyHelper->Init(MysqlDirtyType::MAX_TYPE);
    if(storageInfo->IsKvSystem())
    {
        auto deleg =  KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnKvSystemNumberAddDirtyHandler);
        dirtyHelper->SetHandler(MysqlDirtyType::ADD_TYPE, deleg);

        deleg = KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnKvSystemNumberModifyDirtyHandler);
        dirtyHelper->SetHandler(MysqlDirtyType::MODIFY_TYPE, deleg);

        deleg = KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnKvSystemNumberDeleteDirtyHandler);
        dirtyHelper->SetHandler(MysqlDirtyType::DEL_TYPE, deleg);

        deleg = KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnKvSystemNumberReplaceDirtyHandler);
        dirtyHelper->SetHandler(MysqlDirtyType::REPLACE_TYPE, deleg);
        return;
    }

    auto deleg =  KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnNumberAddDirtyHandler);
    dirtyHelper->SetHandler(MysqlDirtyType::ADD_TYPE, deleg);

    deleg = KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnNumberModifyDirtyHandler);
    dirtyHelper->SetHandler(MysqlDirtyType::MODIFY_TYPE, deleg);

    deleg = KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnNumberDeleteDirtyHandler);
    dirtyHelper->SetHandler(MysqlDirtyType::DEL_TYPE, deleg);

    deleg = KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnNumberReplaceDirtyHandler);
    dirtyHelper->SetHandler(MysqlDirtyType::REPLACE_TYPE, deleg);
}

void MysqlMgr::_InitStringDirtyHelper(const IStorageInfo *storageInfo, KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper)
{
    dirtyHelper->Init(MysqlDirtyType::MAX_TYPE);
    if(storageInfo->IsKvSystem())
    {
        auto deleg =  KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnKvSystemStringAddDirtyHandler);
        dirtyHelper->SetHandler(MysqlDirtyType::ADD_TYPE, deleg);

        deleg = KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnKvSystemStringModifyDirtyHandler);
        dirtyHelper->SetHandler(MysqlDirtyType::MODIFY_TYPE, deleg);

        deleg = KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnKvSystemStringDeleteDirtyHandler);
        dirtyHelper->SetHandler(MysqlDirtyType::DEL_TYPE, deleg);

        deleg = KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnKvSystemStringReplaceDirtyHandler);
        dirtyHelper->SetHandler(MysqlDirtyType::REPLACE_TYPE, deleg);
        return;   
    }

    auto deleg =  KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnStringAddDirtyHandler);
    dirtyHelper->SetHandler(MysqlDirtyType::ADD_TYPE, deleg);

    deleg = KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnStringModifyDirtyHandler);
    dirtyHelper->SetHandler(MysqlDirtyType::MODIFY_TYPE, deleg);

    deleg = KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnStringDeleteDirtyHandler);
    dirtyHelper->SetHandler(MysqlDirtyType::DEL_TYPE, deleg);

    deleg = KERNEL_NS::DelegateFactory::Create(this, &MysqlMgr::_OnStringReplaceDirtyHandler);
    dirtyHelper->SetHandler(MysqlDirtyType::REPLACE_TYPE, deleg);
}

void MysqlMgr::_OnPurge(KERNEL_NS::LibTimer *t)
{
    const Int64 dirtyCount = static_cast<Int64>(_dirtyLogics.size());

    auto &&nowCounter = KERNEL_NS::LibCpuCounter::Current();
    const auto &startCounter = KERNEL_NS::LibCpuCounter::Current();
    const auto &deadLine =  KERNEL_NS::LibCpuCounter(0);
    auto poller = GetService()->GetPoller();
    for(auto iter = _dirtyLogics.begin(); iter != _dirtyLogics.end();)
    {
        auto logic = *iter;
        auto storageInfo = logic->GetStorageInfo();
        if(storageInfo->IsUsingNumberKey())
        {
            auto iterDirtyHelper = _logicRefNumberDirtyHelper.find(logic);
            if(LIKELY(iterDirtyHelper != _logicRefNumberDirtyHelper.end()))
            {
                KERNEL_NS::LibString err;
                iterDirtyHelper->second->Purge(deadLine, &err);
                if(UNLIKELY(!err.empty()))
                    g_Log->Warn(LOGFMT_OBJ_TAG("purge err:%s, logic:%s"), err.c_str(), logic->GetObjName().c_str());
            
                if(!iterDirtyHelper->second->HasDirty())
                {
                    iter = _dirtyLogics.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
            else
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("not init dirtyhelper before storage info:%s, logic:%s"), storageInfo->ToString().c_str(), logic->GetObjName().c_str());
                iter = _dirtyLogics.erase(iter);
            }

            continue;
        }

        if(storageInfo->IsUsingStringKey())
        {
            auto iterDirtyHelper = _logicRefStringDirtyHelper.find(logic);
            if(LIKELY(iterDirtyHelper != _logicRefStringDirtyHelper.end()))
            {
                KERNEL_NS::LibString err;
                iterDirtyHelper->second->Purge(deadLine, &err);
                if(UNLIKELY(!err.empty()))
                    g_Log->Warn(LOGFMT_OBJ_TAG("purge err:%s, logic:%s"), err.c_str(), logic->GetObjName().c_str());
            
                if(!iterDirtyHelper->second->HasDirty())
                {
                    iter = _dirtyLogics.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
            else
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("not init dirtyhelper before storage info:%s, logic:%s"), storageInfo->ToString().c_str(), logic->GetObjName().c_str());
                iter = _dirtyLogics.erase(iter);
            }

            continue;
        }


        g_Log->Warn(LOGFMT_OBJ_TAG("unknown storage flag storage info:%s, logic:%s"), storageInfo->ToString().c_str(), logic->GetObjName().c_str());
        iter = _dirtyLogics.erase(iter);
    }

    nowCounter.Update();
    g_Log->Info(LOGFMT_OBJ_TAG("purge cost time %llu (ms), all logic count:%lld"), (nowCounter - startCounter).GetTotalMilliseconds(), dirtyCount);
}

void MysqlMgr::_Clear()
{
    KERNEL_NS::ContainerUtil::DelContainer(_logicRefNumberDirtyHelper, [](KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *ptr){
        KERNEL_NS::LibDirtyHelper<UInt64, UInt64>::DeleteThreadLocal_LibDirtyHelper(ptr);
    });

    KERNEL_NS::ContainerUtil::DelContainer(_logicRefStringDirtyHelper, [](KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *ptr){
        KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64>::DeleteThreadLocal_LibDirtyHelper(ptr);
    });

    _dirtyLogics.clear();

    if(_purgeTimer)
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_purgeTimer);

    _purgeTimer = NULL;

    KERNEL_NS::ContainerUtil::DelContainer(_tableNameRefTableInfo, [](TableInfo *ptr)
    {
        TableInfo::DeleteThreadLocal_TableInfo(ptr);
    });
}

TableInfo *MysqlMgr::_CreateNewTableInfo(const ILogicSys *logic)
{
    auto storageInfo = logic->GetStorageInfo();
    KERNEL_NS::SmartPtr<TableInfo, KERNEL_NS::AutoDelMethods::CustomDelete> newTableInfo = TableInfo::NewThreadLocal_TableInfo();
    newTableInfo.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<TableInfo *>(p);
        TableInfo::DeleteThreadLocal_TableInfo(ptr);
    });

    newTableInfo->_tableName = storageInfo->GetTableName();

    // 简要信息
    auto simpleInfo = newTableInfo->_simpleInfo;
    simpleInfo->set_versionno(_curVersionNo);

    auto newSt = newTableInfo.pop();
    _tableNameRefTableInfo.insert(std::make_pair(storageInfo->GetTableName(), newSt));

    MaskLogicStringKeyAddDirty(this, storageInfo->GetTableName());
    return newSt;
}

void MysqlMgr::_RemoveTableInfo(const KERNEL_NS::LibString &tableName)
{
    auto iter = _tableNameRefTableInfo.find(tableName);
    if(iter == _tableNameRefTableInfo.end())
        return;

    TableInfo::DeleteThreadLocal_TableInfo(iter->second);
    _tableNameRefTableInfo.erase(tableName);

    MaskStringKeyDeleteDirty(tableName);
}

void MysqlMgr::_PurgeDirty(const ILogicSys *logic)
{
    if(UNLIKELY(_dirtyLogics.find(logic) == _dirtyLogics.end()))
        return;

    auto &&nowCounter = KERNEL_NS::LibCpuCounter::Current();
    auto &&startCounter = KERNEL_NS::LibCpuCounter::Current();
    auto &&deadLine = KERNEL_NS::LibCpuCounter(0);
    KERNEL_NS::LibString err;
    do
    {
        auto iterNumberKey = _logicRefNumberDirtyHelper.find(logic);
        if(iterNumberKey != _logicRefNumberDirtyHelper.end())
        {
            iterNumberKey->second->Purge(deadLine, &err);

            if(!iterNumberKey->second->HasDirty())
                _dirtyLogics.erase(logic);
            break;
        }

        auto iterStringKey = _logicRefStringDirtyHelper.find(logic);
        if(iterStringKey != _logicRefStringDirtyHelper.end())
        {
            iterStringKey->second->Purge(deadLine, &err);

            if(!iterStringKey->second->HasDirty())
                _dirtyLogics.erase(logic);

            break;
        }
    }
    while(false);

    if(UNLIKELY(!err.empty()))
        g_Log->Warn(LOGFMT_OBJ_TAG("purge err:%s, logic:%s"), err.c_str(), logic->GetObjName().c_str());
}

bool MysqlMgr::_CheckStorageInfo(const IStorageInfo *storageInfo)
{
    // 校验表名
    if(storageInfo->GetTableName().size() > KERNEL_NS::MysqlLimit::_tableNameLimit)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("table name over mysql limit(%llu), current len:%llu, table name:%s, logic:%s")
                , KERNEL_NS::MysqlLimit::_tableNameLimit, storageInfo->GetTableName().size()
                , storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
        return false;
    }

    // 校验字段名
    auto &allSubStorageInfo = storageInfo->GetSubStorageInfos();
    for(auto subStorageInfo : allSubStorageInfo)
    {
        if(!subStorageInfo->IsAsField())
            continue;

        if(subStorageInfo->GetFieldName().size() > KERNEL_NS::MysqlLimit::_fieldNameLimit)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("field name over mysql limit(%llu), current len:%llu, field name:%s, table name:%s, logic:%s")
                    , KERNEL_NS::MysqlLimit::_fieldNameLimit, subStorageInfo->GetFieldName().size(), subStorageInfo->GetFieldName().c_str()
                    , storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            return false;
        }
    }

    // 多字段的必须至少有两个字段
    if(storageInfo->IsMultiFieldSystem())
    {
        const Int32 curCount = static_cast<Int32>(storageInfo->GetSubStorageInfos().size());
        // 必须至少一个字段
        if(curCount < 1)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("logic storage table must have at least one visible column, table name:%s, logic:%s"),
                    storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            return false;
        }

        if(curCount > KERNEL_NS::MysqlLimit::_tableFieldsCountLimit)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("table column count limit:%d, current logic over limit count:%d table name:%s, logic:%s")
                    , KERNEL_NS::MysqlLimit::_tableFieldsCountLimit, curCount, storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            return false;
        }
    }

    return true;
}

bool MysqlMgr::_FillMultiFieldStorageInfo(IStorageInfo *storageInfo)
{
    if(storageInfo->GetTableName().empty())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no table name please check storage info system name:%s"), storageInfo->GetSystemName().c_str());
        return false;
    }

    if(storageInfo->IsKvSystem())
        return _FillKvSystemStorageInfo(storageInfo);

    return true;
}

bool MysqlMgr::_FillKvSystemStorageInfo(IStorageInfo *storageInfo)
{
    if(storageInfo->IsNeedStringKey())
    {
        storageInfo->RemoveAllSubStorage();

        // STRING key
        auto newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(StorageCommonDefine::STRING_KEY);
        newStorageInfo->SetRelease([newStorageInfo](){
            IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
        });
        newStorageInfo->AddFlags(StorageFlagType::NORMAL_STRING_FIELD_FLAG | 
        StorageFlagType::MYSQL_FLAG | 
        StorageFlagType::PRIMARY_FIELD_FLAG);
        newStorageInfo->SetCapacitySize(StorageCapacityType::Cap256);
        newStorageInfo->SetComment("自动创建字符串主键");
        storageInfo->AddStorageInfo(newStorageInfo);

        // BINARY
        newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(StorageCommonDefine::SYSTEM_DATA);
        newStorageInfo->SetRelease([newStorageInfo](){
            IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
        });
        newStorageInfo->AddFlags(StorageFlagType::BINARY_FIELD_FLAG | 
        StorageFlagType::MYSQL_FLAG);
        newStorageInfo->SetCapacitySize(_defaultBlobOriginSize);
        newStorageInfo->SetComment("自动创建系统数据字段");
        storageInfo->AddStorageInfo(newStorageInfo);

        return true;
    }

    if(storageInfo->IsNeedNumberKey())
    {
        storageInfo->RemoveAllSubStorage();

        // BIGINT key
        auto newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(StorageCommonDefine::NUMBER_KEY);
        newStorageInfo->SetRelease([newStorageInfo](){
            IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
        });
        newStorageInfo->AddFlags(StorageFlagType::INT64_NUMBER_FIELD_FLAG | 
        StorageFlagType::UNSIGNED_NUMBER_FIELD_FLAG |
        StorageFlagType::MYSQL_FLAG | 
        StorageFlagType::PRIMARY_FIELD_FLAG);
        newStorageInfo->SetComment("自动创建64位数值主键");
        storageInfo->AddStorageInfo(newStorageInfo);

        // BINARY
        newStorageInfo = IStorageInfo::NewThreadLocal_IStorageInfo(StorageCommonDefine::SYSTEM_DATA);
        newStorageInfo->SetRelease([newStorageInfo](){
            IStorageInfo::DeleteThreadLocal_IStorageInfo(newStorageInfo);
        });
        newStorageInfo->AddFlags(StorageFlagType::BINARY_FIELD_FLAG | 
        StorageFlagType::MYSQL_FLAG);
        newStorageInfo->SetCapacitySize(_defaultBlobOriginSize);
        newStorageInfo->SetComment("自动创建系统数据字段");
        storageInfo->AddStorageInfo(newStorageInfo);

        return true;
    }

    return true;
}

bool MysqlMgr::_GetModifyTableInfo(IStorageInfo *storageInfo, std::map<KERNEL_NS::LibString, std::pair<KERNEL_NS::LibString, Int64>> &originDbTableInfo, std::vector<KERNEL_NS::SqlBuilder *> &builders)
{
    // 代码没有,但是数据库还残留需要drop,drop和其他互斥所以不需要担心先后, 当然需要先drop字段再做其他的
    KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> alterDropColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
    alterDropColumn.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::AlterTableSqlBuilder *>(p);
            KERNEL_NS::AlterTableSqlBuilder::DeleteThreadLocal_AlterTableSqlBuilder(ptr);
    });
    alterDropColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());

    KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> alterModifyColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
    alterModifyColumn.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::AlterTableSqlBuilder *>(p);
            KERNEL_NS::AlterTableSqlBuilder::DeleteThreadLocal_AlterTableSqlBuilder(ptr);
    });
    alterModifyColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());

    KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> alterAddColumn = KERNEL_NS::AlterTableSqlBuilder::NewThreadLocal_AlterTableSqlBuilder();
    alterAddColumn.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<KERNEL_NS::AlterTableSqlBuilder *>(p);
            KERNEL_NS::AlterTableSqlBuilder::DeleteThreadLocal_AlterTableSqlBuilder(ptr);
    });
    alterAddColumn->DB(_currentServiceDBName).Table(storageInfo->GetTableName());

    // 拿旧表对照
    bool hasDrop = false;
    bool hasModify = false;
    bool hasAdd = false;
    for(auto iter : originDbTableInfo)
    {
        // 旧字段信息
        auto &fieldName = iter.first;
        auto &fieldInfo = iter.second;
        const auto &oldFieldType = fieldInfo.first.toupper();
        const auto oldFieldLen = fieldInfo.second;

        // 暂不支持的数据类型, 需要管理员手动修改做出决策, 避免误删
        if(!MysqlFieldTypeHelper::CheckCanSupportMysqlDataType(oldFieldType))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("cant support data type:%s, storage info:%s, fieldName:%s")
                        , oldFieldType.c_str(), storageInfo->ToString().c_str(), fieldName.c_str());
            return false;
        }

        UInt64 oldFieldFlags = 0;
        if(!StorageFlagType::GenFlagsByMysqlDataType(oldFieldType, oldFieldFlags))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("old field type not support:%s, storage info:%s, fieldName:%s")
                        , oldFieldType.c_str(), storageInfo->ToString().c_str(), fieldName.c_str());
            return false;
        }

        auto subStorageInfo = storageInfo->GetSubStorageByFieldName(fieldName);
        if(!subStorageInfo)
        {// 旧表有但是新系统没有, 需要删除字段
            alterDropColumn->Drop(fieldName);
            hasDrop = true;

            g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: db has field to drop when system have no this field:%s, table name:%s, storage system name:%s")
                        , fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());

            continue;
        }

        KERNEL_NS::LibString dataType;
        if(!MysqlFieldTypeHelper::TurnToMysqlDataType(subStorageInfo, dataType))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("TurnToMysqlDataType fail, subStorageInfo:%s, storageInfo:%s")
                        , subStorageInfo->ToString().c_str(), storageInfo->ToString().c_str());
            return false;
        }

        if(!MysqlFieldTypeHelper::CheckCanSupportMysqlDataType(dataType))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("cant support data type from sub storage info data type:%s, subStorageInfo:%s, table name:%s, system name:%s fieldName:%s")
                        , dataType.c_str(), subStorageInfo->ToString().c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str(), fieldName.c_str());
            return false;
        }     

        // 旧的是数值型
        if(StorageFlagType::IsNumber(oldFieldFlags))
        {
            if(!_ModifyDbNumberDataType(storageInfo, subStorageInfo, dataType, oldFieldType, fieldName, alterDropColumn, alterModifyColumn, alterAddColumn, hasModify, hasAdd, hasDrop))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("_ModifyDbNumberDataType fail."));
                return false;
            }

            continue;
        }

        // 旧的是字符串类型
        if(StorageFlagType::IsString(oldFieldFlags))
        {
            if(!_ModifyDbStringDataType(storageInfo, subStorageInfo, dataType, oldFieldType, oldFieldLen, fieldName, alterDropColumn, alterModifyColumn, alterAddColumn, hasModify, hasAdd, hasDrop))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("_ModifyDbStringDataType fail."));
                return false;
            }

            continue;
        }

        // 旧的是二进制
        if(StorageFlagType::IsBinary(oldFieldFlags))
        {
            if(!_ModifyDbBinaryDataType(storageInfo, subStorageInfo, dataType, oldFieldType, oldFieldLen, fieldName, alterDropColumn, alterModifyColumn, alterAddColumn, hasModify, hasAdd, hasDrop))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("_ModifyDbBinaryDataType fail."));
                return false;
            }
            continue;
        }
    }    

    if(hasDrop)
        builders.push_back(alterDropColumn.pop());

    if(hasAdd)
        builders.push_back(alterAddColumn.pop());

    if(hasModify)
        builders.push_back(alterModifyColumn.pop());

    return true;
}

bool MysqlMgr::_ModifyDbNumberDataType(IStorageInfo *storageInfo, IStorageInfo *subStorageInfo, const KERNEL_NS::LibString &dataType
, const KERNEL_NS::LibString &oldFieldType,  const KERNEL_NS::LibString &fieldName
, KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterDropColumn 
, KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterModifyColumn 
, KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterAddColumn 
, bool &hasModify
, bool &hasAdd
, bool &hasDrop
)
{
    if(subStorageInfo->IsNumberField())
    {// 两个同时是数值需要取大的
        // 代码定义的若比数据库的大则需要改表字段
        if(StorageFlagType::IsMysqlDataTypeNumberBiggerEq(dataType, oldFieldType))
        {
            if(dataType != oldFieldType)
            {
                KERNEL_NS::LibString newDescribe;
                if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorageInfo, newDescribe))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                                , subStorageInfo->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
                    return false;
                }
                
                alterModifyColumn->Modify(fieldName, newDescribe);
                g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: number field, db field not same with system field, will modify column old column type:%s => new field describe:%s table name:%s, field name:%s, system name:%s")
                            , oldFieldType.c_str(), newDescribe.c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());

                hasModify = true;
            }

            return true;

        }
        else
        {// 代码定义的比数据表的小, 则更新代码的定义
            if(dataType != oldFieldType)
            {
                g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: number field sub storage info will change from %s, to data base old field type:%s table name:%s, field name:%s, system name:%s")
                , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());

                if(!StorageFlagType::UpdateNumberStorageInfo(subStorageInfo, oldFieldType))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("UpdateNumberStorageInfo fail subStorageInfo:%s, oldFieldType:%s, table name:%s, field name:%s, system name:%s")
                            , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
                    return false;
                }

                g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: number field sub storage info final change to %s success. data base old field type:%s table name:%s, field name:%s, system name:%s")
                , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            }

            return true;
        }
    }
    else 
    {// 数据库是数值但是代码却是其他类型
        // 1.先drop掉字段, 再新增字段
        KERNEL_NS::LibString newDescribe;
        if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorageInfo, newDescribe))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                        , subStorageInfo->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            return false;
        }

        alterDropColumn->Drop(fieldName);
        hasDrop = true;
        alterAddColumn->Add(fieldName, newDescribe);
        hasAdd = true;
    }

    return true;
}

bool MysqlMgr::_ModifyDbStringDataType(IStorageInfo *storageInfo, IStorageInfo *subStorageInfo, const KERNEL_NS::LibString &dataType
, const KERNEL_NS::LibString &oldFieldType, UInt64 oldCapacitySize,  const KERNEL_NS::LibString &fieldName
, KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterDropColumn 
, KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterModifyColumn 
, KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterAddColumn 
, bool &hasModify
, bool &hasAdd
, bool &hasDrop)
{
    if(subStorageInfo->IsStringField())
    {
        if((!subStorageInfo->IsNormalStringField()) && (!subStorageInfo->IsTextField()))
        {// 既不是普通的字符串, 又不是可扩展的字符串
            g_Log->Error(LOGFMT_OBJ_TAG("not normal string and text field please check, subStorageInfo:%s, table name:%s, field name:%s, system name:%s")
            , subStorageInfo->ToString().c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());

            return false;
        }

        if(subStorageInfo->IsNormalStringField())
        {
            if(StorageFlagType::IsMysqlDataTypeNormalStringBiggerEq(subStorageInfo->GetCapacitySize(), oldCapacitySize))
            {
                if(oldCapacitySize != subStorageInfo->GetCapacitySize())
                {
                    KERNEL_NS::LibString newDescribe;
                    if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorageInfo, newDescribe))
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                                    , subStorageInfo->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
                        return false;
                    }
                    
                    alterModifyColumn->Modify(fieldName, newDescribe);
                    g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: string field, db field not same with system field, will modify column old column type:%s => new field describe:%s table name:%s, field name:%s, system name:%s")
                                , oldFieldType.c_str(), newDescribe.c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());

                    hasModify = true;
                }
            }
            else
            {// 代码定义的比数据表的小, 则更新代码的定义
                if(oldCapacitySize != subStorageInfo->GetCapacitySize())
                {
                    g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: string field sub storage info will change from %s, to data base old field type:%s table name:%s, field name:%s, system name:%s")
                    , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), subStorageInfo->GetFieldName().c_str(), storageInfo->GetSystemName().c_str());

                    if(!StorageFlagType::UpdateNormalStringStorageInfo(subStorageInfo, oldCapacitySize))
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("UpdateNormalStringStorageInfo fail subStorageInfo:%s, oldFieldType:%s, table name:%s, field name:%s, system name:%s")
                                , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), subStorageInfo->GetFieldName().c_str(), storageInfo->GetSystemName().c_str());
                        return false;
                    }

                    g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: string field sub storage info final change to %s, oldCapacitySize:%llu success. data base old field type:%s table name:%s, field name:%s, system name:%s")
                    , subStorageInfo->ToString().c_str(), oldCapacitySize, oldFieldType.c_str(), storageInfo->GetTableName().c_str(), subStorageInfo->GetFieldName().c_str(), storageInfo->GetSystemName().c_str());
                }
            }

            return true;
        }

        if(StorageFlagType::IsMysqlDataTypeTextBiggerEq(dataType, oldFieldType))
        {
            if(dataType != oldFieldType)
            {
                KERNEL_NS::LibString newDescribe;
                if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorageInfo, newDescribe))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                                , subStorageInfo->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
                    return false;
                }
                
                alterModifyColumn->Modify(fieldName, newDescribe);
                g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: text string field, db field not same with system field, will modify column old column type:%s => new field describe:%s table name:%s, field name:%s, system name:%s")
                            , oldFieldType.c_str(), newDescribe.c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());

                hasModify = true;
            }
        }
        else
        {// 代码定义的比数据表的小, 则更新代码的定义
            if(dataType != oldFieldType)
            {
                g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: text string sub storage info will change from %s, to data base old field type:%s table name:%s, field name:%s, system name:%s")
                , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());

                if(!StorageFlagType::UpdateTextStringStorageInfo(subStorageInfo, oldFieldType))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("UpdateTextStringStorageInfo fail subStorageInfo:%s, oldFieldType:%s, table name:%s, field name:%s, system name:%s")
                            , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), subStorageInfo->GetFieldName().c_str(), storageInfo->GetSystemName().c_str());
                    return false;
                }

                g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: text string sub storage info final change to %s success. data base old field type:%s table name:%s, field name:%s, system name:%s")
                , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), subStorageInfo->GetFieldName().c_str(), storageInfo->GetSystemName().c_str());
            }
        }

        return true;
    }
    else
    {// 数据库是数值但是代码却是其他类型
        // 1.先drop掉字段, 再新增字段
        KERNEL_NS::LibString newDescribe;
        if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorageInfo, newDescribe))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                        , subStorageInfo->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            return false;
        }

        alterDropColumn->Drop(fieldName);
        alterAddColumn->Add(fieldName, newDescribe);
        hasDrop = true;
        hasAdd = true;
    }

    return true;
}


bool MysqlMgr::_ModifyDbBinaryDataType(IStorageInfo *storageInfo, IStorageInfo *subStorageInfo, const KERNEL_NS::LibString &dataType
, const KERNEL_NS::LibString &oldFieldType, UInt64 oldCapacitySize,  const KERNEL_NS::LibString &fieldName
, KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterDropColumn 
, KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterModifyColumn 
, KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterAddColumn 
, bool &hasModify
, bool &hasAdd
, bool &hasDrop)
{
    if(subStorageInfo->IsBinaryField())
    {
        if((!subStorageInfo->IsVarBinaryField()) && (!subStorageInfo->IsBlobTypeBinaryField()))
        {// 既不是varbinary, 又不是可扩展blob族类型
            g_Log->Error(LOGFMT_OBJ_TAG("not var binary and blob type field please check, subStorageInfo:%s, table name:%s, field name:%s, system name:%s")
            , subStorageInfo->ToString().c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());

            return false;
        }

        if(subStorageInfo->IsVarBinaryField())
        {
            if(StorageFlagType::IsMysqlDataTypeNoamalBinaryBiggerEq(subStorageInfo->GetCapacitySize(), oldCapacitySize))
            {
                if(oldCapacitySize != subStorageInfo->GetCapacitySize())
                {
                    KERNEL_NS::LibString newDescribe;
                    if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorageInfo, newDescribe))
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                                    , subStorageInfo->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
                        return false;
                    }
                    
                    alterModifyColumn->Modify(fieldName, newDescribe);
                    g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: var binary field, db field not same with system field, will modify column old column type:%s => new field describe:%s table name:%s, field name:%s, system name:%s")
                                , oldFieldType.c_str(), newDescribe.c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());

                    hasModify = true;
                }
            }
            else
            {// 代码定义的比数据表的小, 则更新代码的定义
                if(oldCapacitySize != subStorageInfo->GetCapacitySize())
                {
                    g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: var binary sub storage info will change from %s, to data base old field type:%s table name:%s, field name:%s, system name:%s")
                    , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());

                    if(!StorageFlagType::UpdateNormalBinaryStorageInfo(subStorageInfo, oldCapacitySize))
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("UpdateNormalBinaryStorageInfo fail subStorageInfo:%s, oldFieldType:%s, table name:%s, field name:%s, system name:%s")
                                , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), subStorageInfo->GetFieldName().c_str(), storageInfo->GetSystemName().c_str());
                        return false;
                    }

                    g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: var binary sub storage info final change to %s, oldCapacitySize:%llu success. data base old field type:%s table name:%s, field name:%s, system name:%s")
                    , subStorageInfo->ToString().c_str(), oldCapacitySize, oldFieldType.c_str(), storageInfo->GetTableName().c_str(), subStorageInfo->GetFieldName().c_str(), storageInfo->GetSystemName().c_str());
                }
            }

            return true;
        }

        if(StorageFlagType::IsMysqlDataTypeBlobTypeBinaryBiggerEq(dataType, oldFieldType))
        {
            if(dataType != oldFieldType)
            {
                KERNEL_NS::LibString newDescribe;
                if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorageInfo, newDescribe))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                                , subStorageInfo->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
                    return false;
                }
                
                alterModifyColumn->Modify(fieldName, newDescribe);
                g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: blob type field, db field not same with system field, will modify column old column type:%s => new field describe:%s table name:%s, field name:%s, system name:%s")
                            , oldFieldType.c_str(), newDescribe.c_str(), storageInfo->GetTableName().c_str(), fieldName.c_str(), storageInfo->GetSystemName().c_str());

                hasModify = true;
            }
        }
        else
        {// 代码定义的比数据表的小, 则更新代码的定义
            if(dataType != oldFieldType)
            {
                g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: blob type field sub storage info will change from %s, to data base old field type:%s table name:%s, field name:%s, system name:%s")
                , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());

                if(!StorageFlagType::UpdateBlobTypeBinaryStorageInfo(subStorageInfo, oldFieldType))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("UpdateBlobTypeBinaryStorageInfo fail subStorageInfo:%s, oldFieldType:%s, table name:%s, field name:%s, system name:%s")
                            , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), subStorageInfo->GetFieldName().c_str(), storageInfo->GetSystemName().c_str());
                    return false;
                }

                g_Log->Info(LOGFMT_OBJ_TAG("[MYSQL MGR ALTER TABLE]: blob type field sub storage info final change to %s success. data base old field type:%s table name:%s, field name:%s, system name:%s")
                , subStorageInfo->ToString().c_str(), oldFieldType.c_str(), storageInfo->GetTableName().c_str(), subStorageInfo->GetFieldName().c_str(), storageInfo->GetSystemName().c_str());
            }
        }

        return true;
    }
    else
    {// 数据库是二进制但是代码却是其他类型
        // 1.先drop掉字段, 再新增字段
        KERNEL_NS::LibString newDescribe;
        if(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorageInfo, newDescribe))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("MakeFieldDescribe fail sub storage info:%s, fieldName:%s, table name:%s, system name:%s")
                        , subStorageInfo->ToString().c_str(), fieldName.c_str(), storageInfo->GetTableName().c_str(), storageInfo->GetSystemName().c_str());
            return false;
        }

        alterDropColumn->Drop(fieldName);
        alterAddColumn->Add(fieldName, newDescribe);
        hasDrop = true;
        hasAdd = true;
    }

    return true;
}

SERVICE_END