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
 * Date: 2023-11-19 00:03:49
 * Author: Eric Yonng
 * Description: 
*/


#include <pch.h>
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/TestService/Common/ServiceCommon.h>

#include <Comps/SystemLog/Impl/SystemLogGlobal.h>
#include <Comps/SystemLog/Impl/SystemLogGlobalStorageFactory.h>
#include <Comps/SystemLog/Impl/SystemLogGlobalStorage.h>
#include <Comps/SystemLog/Impl/SystemLogGlobalFactory.h>
#include <Comps/config/config.h>
#include <protocols/protocols.h>
#include <Comps/User/User.h>
#include <Comps/Library/library.h>
#include <Comps/UserSys/UserSys.h>
#include <Comps/DB/db.h>
#include <OptionComp/storage/mysql/mysqlcomp.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ISystemLogGlobal);
POOL_CREATE_OBJ_DEFAULT_IMPL(SystemLogGlobal);

SystemLogGlobal::SystemLogGlobal()
{

}

SystemLogGlobal::~SystemLogGlobal()
{

}

void SystemLogGlobal::Release()
{
    SystemLogGlobal::DeleteByAdapter_SystemLogGlobal(SystemLogGlobalFactory::_buildType.V, this);
}

void SystemLogGlobal::OnRegisterComps()
{
    RegisterComp<SystemLogGlobalStorageFactory>();
}

Int32 SystemLogGlobal::_OnGlobalSysInit()
{
    Subscribe(Opcodes::OpcodeConst::OPCODE_SystemLogDataListReq, this, &SystemLogGlobal::_OnSystemLogDataListReq);
    
    return Status::Success;
}

Int32 SystemLogGlobal::OnSave(UInt64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const
{
    auto iter = _idRefSystemLogData.find(key);
    if(iter == _idRefSystemLogData.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system log not exists key:%llu"), key);
        return Status::NotFound;
    }

    auto data = iter->second;

    {// id
        auto newStream = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        newStream->Init(sizeof(UInt64));
        newStream->WriteUInt64(key);
        fieldRefdb.insert(std::make_pair(SystemLogGlobalStorage::ID, newStream));
    }

    {// libraryId
        auto newStream = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        newStream->Init(sizeof(UInt64));
        newStream->WriteUInt64(data->libraryid());
        fieldRefdb.insert(std::make_pair(SystemLogGlobalStorage::LIBRARY_ID_NAME, newStream));
    }

    {// data

        auto newStream = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        fieldRefdb.insert(std::make_pair(SystemLogGlobalStorage::LOG_DATA_NAME, newStream));
        if(!data->Encode(*newStream))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("encode fail key:%llu"), key);
            return Status::SerializeFail;
        }
    }

    _idRefSystemLogData.erase(iter);

    data->Release();

    return Status::Success;
}

void SystemLogGlobal::AddLog(UInt64 libraryId, const KERNEL_NS::LibString &titleWordId, const std::vector<VariantParam> &titleParams, const KERNEL_NS::LibString &contentWordId, const std::vector<VariantParam> &contentParams)
{
    auto uid = GetGlobalSys<IGlobalUidMgr>()->NewGuid();
    auto newData = new SystemLogData();
    newData->set_id(uid);
    newData->set_libraryid(libraryId);
    newData->set_titlewordid(titleWordId.GetRaw());
    newData->set_createtime(KERNEL_NS::LibTime::NowMilliTimestamp());

    for(auto &param : titleParams)
        *newData->add_titleparams() = param;

    newData->set_contentwordid(contentWordId.GetRaw());

    for(auto &param : contentParams)
        *newData->add_contentparams() = param;

    _idRefSystemLogData.insert(std::make_pair(uid, newData));

    MaskNumberKeyAddDirty(uid);
}

void SystemLogGlobal::_OnSystemLogDataListReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<SystemLogDataListReq>();
    auto libraryMgr = user->GetSys<ILibraryMgr>();
    const auto libraryId = libraryMgr->GetMyLibraryId();
    auto libraryGlobal = GetGlobalSys<ILibraryGlobal>();
    std::vector<SystemLogData> logDatas;

    do
    {
        if(libraryId == 0)
            break;

        if(!libraryGlobal->IsManager(libraryId, user->GetUserId()))
            break;

        const auto baseId = req->baselogid();
        const auto absBookCount = std::abs(req->count());

        UInt64 stub = 0;
        auto mysqlMgr = GetGlobalSys<IMysqlMgr>();
        std::vector<KERNEL_NS::MysqlSqlBuilderInfo *> builders;

        // builder构建
        auto builder = KERNEL_NS::MysqlSqlBuilderInfo::Create();
        builders.push_back(builder);

        auto selectBuilder = KERNEL_NS::SelectSqlBuilder::NewThreadLocal_SelectSqlBuilder();
        builder->_builder = selectBuilder;
        auto storageInfo = GetComp<SystemLogGlobalStorage>();
        selectBuilder->DB(mysqlMgr->GetCurrentServiceDbName()).From(storageInfo->GetTableName());
        if(req->count() >= 0)
        {
            selectBuilder->Where(KERNEL_NS::LibString().AppendFormat("`%s` = ? and `%s` > ?", SystemLogGlobalStorage::LIBRARY_ID_NAME.c_str(), SystemLogGlobalStorage::ID.c_str()))
            .Limit(absBookCount)
            .OrderBy(KERNEL_NS::LibString().AppendFormat("%s asc", SystemLogGlobalStorage::ID.c_str()));
        }
        else
        {
            selectBuilder->Where(KERNEL_NS::LibString().AppendFormat("`%s` = ? and `%s` < ?", SystemLogGlobalStorage::LIBRARY_ID_NAME.c_str(), SystemLogGlobalStorage::ID.c_str()))
            .Limit(absBookCount)
            .OrderBy(KERNEL_NS::LibString().AppendFormat("%s desc", SystemLogGlobalStorage::ID.c_str()));
        }

        // stmt的fields绑定
        std::vector<KERNEL_NS::Field *> fields;
        fields.resize(2);
        {
            KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), SystemLogGlobalStorage::LIBRARY_ID_NAME.c_str(), MYSQL_TYPE_LONGLONG, 0);
            v->Write(&libraryId, static_cast<Int64>(sizeof(libraryId)));
            fields[0] = v;
        }
        {
            KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), SystemLogGlobalStorage::ID.c_str(), MYSQL_TYPE_LONGLONG, 0);
            v->Write(&baseId, static_cast<Int64>(sizeof(baseId)));
            fields[1] = v;
        }
        builder->_fields = fields;

        Int32 err = Status::Success;
        err = mysqlMgr->NewRequestAndWaitResponseBy2(stub, mysqlMgr->GetCurrentServiceDbName(), user->GetStorageOperatorId(), builders
            ,[&err, this, &logDatas](KERNEL_NS::MysqlResponse *res)
            {
                if(res->_errCode != Status::Success)
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("NewRequestAndWaitResponseBy2 fail db name:%s res seqId:%llu, mysqlError:%u")
                            , res->_dbName.c_str(), res->_seqId, res->_mysqlErrno);
                    err = res->_errCode;
                    return;
                }

                auto &records = res->_datas;
                for(auto &record : records)
                {
                    auto field = record->GetField(SystemLogGlobalStorage::LOG_DATA_NAME);
                    auto data = field->GetData();
                    if(!data)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("have no data, record:%s"), record->ToString().c_str());
                        continue;
                    }

                    SystemLogData logData;
                    if(!logData.Decode(*data))
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("decode data fail, record:%s"), record->ToString().c_str());
                        continue;
                    }

                    logDatas.push_back(logData);
                }
            });

        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestAndWaitResponseBy2 fail user:%s, err:%d, req:%s")
            , user->ToString().c_str(), err, req->ToJsonString().c_str());
            break;
        }
        
    }while(false);

    SystemLogDataListRes res;
    if(!logDatas.empty())
    {
        for(auto &data : logDatas)
            *res.add_loglist() = data;
    }

    user->Send(Opcodes::OpcodeConst::OPCODE_SystemLogDataListRes, res, packet->GetPacketId());
}

SERVICE_END
