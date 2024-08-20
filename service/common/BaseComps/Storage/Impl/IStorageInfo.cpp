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
 * Date: 2023-07-23 15:55:11
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <service/common/BaseComps/Storage/Impl/IStorageInfo.h>
#include <service/common/BaseComps/ServiceCompType.h>

SERVICE_BEGIN

bool StorageFlagType::GetStringFieldFlagsBySize(UInt64 sz, bool onlyText, bool onlyVarchar, UInt64 &flags)
{
    if(onlyVarchar)
    {
        if(sz <= StorageCapacityType::Cap256)
        {
            flags = StorageFlagType::NORMAL_STRING_FIELD_FLAG;
            return true;
        }

        return false;
    }

    if(onlyText)
    {
        if(sz <= StorageCapacityType::Cap64K)
        {
            flags = NORMAL_TEXT_STRING_FIELD_FLAG;
            return true;
        }

        if(sz <= StorageCapacityType::Cap16M)
        {
            flags = MEDIUM_TEXT_STRING_FIELD_FLAG;
            return true;
        }

        if(sz <= StorageCapacityType::Cap4GB)
        {
            flags = LONG_TEXT_STRING_FIELD_FLAG;
            return true;
        }
        return false;
    }

    // 其他情况
    if(sz <= StorageCapacityType::Cap256)
    {
        flags = StorageFlagType::NORMAL_STRING_FIELD_FLAG;
        return true;
    }

    if(sz <= StorageCapacityType::Cap64K)
    {
        flags = NORMAL_TEXT_STRING_FIELD_FLAG;
        return true;
    }

    if(sz <= StorageCapacityType::Cap16M)
    {
        flags = MEDIUM_TEXT_STRING_FIELD_FLAG;
        return true;
    }

    if(sz <= StorageCapacityType::Cap4GB)
    {
        flags = LONG_TEXT_STRING_FIELD_FLAG;
        return true;
    }

    return false;
}

bool StorageFlagType::GetBinaryFieldFlagsBySize(UInt64 sz, UInt64 &flags)
{
    if(sz <= StorageCapacityType::Cap4K)
    {
        flags = VARBINARY_FIELD_FLAG;
        return true;
    }

    if(sz <= StorageCapacityType::Cap64K)
    {
        flags = BLOB_BINARY_FIELD_FLAG;
        return true;
    }

    if(sz <= StorageCapacityType::Cap16M)
    {
        flags = MEDIUM_BLOB_BINARY_FIELD_FLAG;
        return true;
    }

    if(sz <= StorageCapacityType::Cap4GB)
    {
        flags = LONG_BLOB_BINARY_FIELD_FLAG;
        return true;
    }

    return false;
}

bool StorageFlagType::GenFlagsByMysqlDataType(const KERNEL_NS::LibString &dataType, UInt64 &flags)
{
    // 数值类型
    if(dataType == "TINYINT")     
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::INT8_NUMBER_FIELD_FLAG);
        return true;
    }

    if(dataType == "SMALLINT")     
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::INT16_NUMBER_FIELD_FLAG);
        return true;
    }

    if(dataType == "INT")     
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::INT32_NUMBER_FIELD_FLAG);
        return true;
    }

    if(dataType == "BIGINT")     
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::INT64_NUMBER_FIELD_FLAG);
        return true;
    }

    if(dataType == "FLOAT")     
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::FLOAT_NUMBER_FIELD_FLAG);
        return true;
    }

    if(dataType == "DOUBLE")     
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::DOUBLE_NUMBER_FIELD_FLAG);
        return true;
    }

    if(dataType == "VARBINARY")
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::VARBINARY_FIELD_FLAG);
        return true;
    }

    if(dataType == "BLOB")
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::BLOB_BINARY_FIELD_FLAG);
        return true;
    }

    if(dataType == "MEDIUMBLOB")
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::MEDIUM_BLOB_BINARY_FIELD_FLAG);
        return true;
    }

    if(dataType == "LONGBLOB")
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::LONG_BLOB_BINARY_FIELD_FLAG);
        return true;
    }

    if(dataType == "VARCHAR")
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::NORMAL_STRING_FIELD_FLAG);
        return true;
    }

    if(dataType == "TEXT")
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::NORMAL_TEXT_STRING_FIELD_FLAG);
        return true;
    }

    if(dataType == "MEDIUMTEXT")
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::MEDIUM_TEXT_STRING_FIELD_FLAG);
        return true;
    }

    if(dataType == "LONGTEXT")
    {
        flags = StorageFlagType::AddFlags(flags, StorageFlagType::LONG_TEXT_STRING_FIELD_FLAG);
        return true;
    }

    return false;
}

bool IStorageInfo::AddStorageInfo(IStorageInfo *storageInfo)
{
    if(UNLIKELY(storageInfo == this))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("cant add self storage info system name:%s"), storageInfo->GetSystemName().c_str());
        return false;
    }

    auto iter = _objNameRefStorageInfo.find(storageInfo->GetSystemName());
    if(UNLIKELY(iter != _objNameRefStorageInfo.end()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("storage info already exists system name:%s, current system name:%s"), storageInfo->GetSystemName().c_str(), GetSystemName().c_str());
        return false;
    }

    auto iterFieldName = _fieldNameRefStorageInfo.find(storageInfo->GetFieldName());
    if(UNLIKELY(iterFieldName != _fieldNameRefStorageInfo.end()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("storage info already exists, current system name:%s, exists system name:%s, will add system name:%s field name:%s")
            , GetSystemName().c_str(), iterFieldName->second->GetSystemName().c_str(), storageInfo->GetSystemName().c_str(), storageInfo->GetFieldName().c_str());
        return false;
    }

    // 不可以有两个primary key
    if(_primaryKeyStorage && storageInfo->IsPrimaryField())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("primary key is already exists, current system name:%s, will add system name:%s field name:%s")
            , GetSystemName().c_str(), storageInfo->GetSystemName().c_str(), storageInfo->GetFieldName().c_str());
        return false;
    }

    _subStorageInfos.push_back(storageInfo);
    _objNameRefStorageInfo.insert(std::make_pair(storageInfo->GetSystemName(), storageInfo));
    _fieldNameRefStorageInfo.insert(std::make_pair(storageInfo->GetFieldName(), storageInfo));

    AddFlags(StorageFlagType::MULTI_FIELD_SYSTEM_FLAG);

    // table有了索引了, 那么就是value, 因为有了primarykey后, 可能value根据需求也会是unique key
    if(IsKvSystem())
    {
        if(_primaryKeyStorage || !_uniqueKeyStorages.empty())
        {
            _kvModeValueStorage = storageInfo;
        }
    }

    if(storageInfo->IsPrimaryField())
        _primaryKeyStorage = storageInfo;
    if(storageInfo->IsUniqueKeyField())
        _uniqueKeyStorages.push_back(storageInfo);

    if(storageInfo->IsIndexField())
        _indexKeyStorages.push_back(storageInfo);

    // kv系统, 既不是主键, 也不是uniquekey, 那就是value
    // if(IsKvSystem() && 
    // (!storageInfo->IsPrimaryField()) && 
    // (!storageInfo->IsUniqueKeyField()))
    //     _kvModeValueStorage = storageInfo;

    return true;
}

bool IStorageInfo::AddStorageInfo(const std::vector<IStorageInfo *> &storageInfos)
{
    std::vector<IStorageInfo *> toAdd;
    for(auto storageInfo : storageInfos)
    {
        if(storageInfo == this)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("cant add self system name:%s"), storageInfo->GetSystemName().c_str());
            continue;
        }

        toAdd.push_back(storageInfo);
    }

    const Int32 count = static_cast<Int32>(toAdd.size());
    if(UNLIKELY(count == 0))
        return false;

    for(Int32 idx = count - 1; idx >= 0; --idx)
    {
        auto storageInfo = toAdd[idx];
        if(!AddStorageInfo(storageInfo))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("add storage fail system name:%s, current system name:%s")
                    , storageInfo->GetSystemName().c_str(), GetSystemName().c_str());
            toAdd.erase(toAdd.begin() + idx);
        }
    }
    
    return !toAdd.empty();
}

bool StorageFlagType::UpdateNumberStorageInfo(IStorageInfo *storageInfo, const KERNEL_NS::LibString &dataType)
{
    if(!CheckCanSupportMysqlDataType(dataType))
        return false;

    do
    {
        if(dataType == "TINYINT")
        {
            storageInfo->ClearFlags(INT8_NUMBER_FIELD_FLAG | INT16_NUMBER_FIELD_FLAG | INT32_NUMBER_FIELD_FLAG | INT64_NUMBER_FIELD_FLAG | FLOAT_NUMBER_FIELD_FLAG | DOUBLE_NUMBER_FIELD_FLAG);
            storageInfo->AddFlags(INT8_NUMBER_FIELD_FLAG);
            break;
        }

        if(dataType == "SMALLINT")
        {
            storageInfo->ClearFlags(INT8_NUMBER_FIELD_FLAG | INT16_NUMBER_FIELD_FLAG | INT32_NUMBER_FIELD_FLAG | INT64_NUMBER_FIELD_FLAG | FLOAT_NUMBER_FIELD_FLAG | DOUBLE_NUMBER_FIELD_FLAG);
            storageInfo->AddFlags(INT16_NUMBER_FIELD_FLAG);
            break;
        }

        if(dataType == "INT")
        {
            storageInfo->ClearFlags(INT8_NUMBER_FIELD_FLAG | INT16_NUMBER_FIELD_FLAG | INT32_NUMBER_FIELD_FLAG | INT64_NUMBER_FIELD_FLAG | FLOAT_NUMBER_FIELD_FLAG | DOUBLE_NUMBER_FIELD_FLAG);
            storageInfo->AddFlags(INT32_NUMBER_FIELD_FLAG);
            break;
        }

        if(dataType == "BIGINT")
        {
            storageInfo->ClearFlags(INT8_NUMBER_FIELD_FLAG | INT16_NUMBER_FIELD_FLAG | INT32_NUMBER_FIELD_FLAG | INT64_NUMBER_FIELD_FLAG | FLOAT_NUMBER_FIELD_FLAG | DOUBLE_NUMBER_FIELD_FLAG);
            storageInfo->AddFlags(INT64_NUMBER_FIELD_FLAG);
            break;
        }

        if(dataType == "FLOAT")
        {
            storageInfo->ClearFlags(INT8_NUMBER_FIELD_FLAG | INT16_NUMBER_FIELD_FLAG | INT32_NUMBER_FIELD_FLAG | INT64_NUMBER_FIELD_FLAG | FLOAT_NUMBER_FIELD_FLAG | DOUBLE_NUMBER_FIELD_FLAG);
            storageInfo->AddFlags(FLOAT_NUMBER_FIELD_FLAG);
            break;
        }

        if(dataType == "DOUBLE")
        {
            storageInfo->ClearFlags(INT8_NUMBER_FIELD_FLAG | INT16_NUMBER_FIELD_FLAG | INT32_NUMBER_FIELD_FLAG | INT64_NUMBER_FIELD_FLAG | FLOAT_NUMBER_FIELD_FLAG | DOUBLE_NUMBER_FIELD_FLAG);
            storageInfo->AddFlags(DOUBLE_NUMBER_FIELD_FLAG);
            break;
        }
    } while (false);
    
    return true;
}

bool StorageFlagType::UpdateNormalStringStorageInfo(IStorageInfo *storageInfo, UInt64 newCapacitySize)
{
    storageInfo->SetCapacitySize(newCapacitySize);
    return true;
}

bool StorageFlagType::UpdateTextStringStorageInfo(IStorageInfo *storageInfo, const KERNEL_NS::LibString &dataType)
{
    if(!CheckCanSupportMysqlDataType(dataType))
        return false;

    do
    {
        if(dataType == "TEXT")
        {
            storageInfo->ClearFlags(TEXT_STRING_FIELD_FLAG | NORMAL_STRING_FIELD_FLAG | NORMAL_TEXT_STRING_FIELD_FLAG | MEDIUM_TEXT_STRING_FIELD_FLAG | LONG_TEXT_STRING_FIELD_FLAG);
            storageInfo->AddFlags(NORMAL_TEXT_STRING_FIELD_FLAG);
            break;
        }   

        if(dataType == "MEDIUMTEXT")
        {
            storageInfo->ClearFlags(TEXT_STRING_FIELD_FLAG | NORMAL_STRING_FIELD_FLAG | NORMAL_TEXT_STRING_FIELD_FLAG | MEDIUM_TEXT_STRING_FIELD_FLAG | LONG_TEXT_STRING_FIELD_FLAG);
            storageInfo->AddFlags(MEDIUM_TEXT_STRING_FIELD_FLAG);
            break;
        }   

        if(dataType == "LONGTEXT")
        {
            storageInfo->ClearFlags(TEXT_STRING_FIELD_FLAG | NORMAL_STRING_FIELD_FLAG | NORMAL_TEXT_STRING_FIELD_FLAG | MEDIUM_TEXT_STRING_FIELD_FLAG | LONG_TEXT_STRING_FIELD_FLAG);
            storageInfo->AddFlags(LONG_TEXT_STRING_FIELD_FLAG);
            break;
        }   
    } while (false);

    return true;
}

bool StorageFlagType::UpdateNormalBinaryStorageInfo(IStorageInfo *storageInfo, UInt64 newCapacitySize)
{
    storageInfo->SetCapacitySize(newCapacitySize);
    return true;
}

bool StorageFlagType::UpdateBlobTypeBinaryStorageInfo(IStorageInfo *storageInfo, const KERNEL_NS::LibString &dataType)
{
    if(!CheckCanSupportMysqlDataType(dataType))
        return false;

    do
    {
        if(dataType == "BLOB")
        {
            storageInfo->ClearFlags(BLOB_TYPE_BINARY_FIELD_FLAG | VARBINARY_FIELD_FLAG | BLOB_BINARY_FIELD_FLAG | MEDIUM_BLOB_BINARY_FIELD_FLAG | LONG_BLOB_BINARY_FIELD_FLAG );
            storageInfo->AddFlags(BLOB_BINARY_FIELD_FLAG);
            break;
        }

        if(dataType == "MEDIUMBLOB")
        {
            storageInfo->ClearFlags(BLOB_TYPE_BINARY_FIELD_FLAG | VARBINARY_FIELD_FLAG | BLOB_BINARY_FIELD_FLAG | MEDIUM_BLOB_BINARY_FIELD_FLAG | LONG_BLOB_BINARY_FIELD_FLAG );
            storageInfo->AddFlags(MEDIUM_BLOB_BINARY_FIELD_FLAG);
            break;
        }

        if(dataType == "LONGBLOB")
        {
            storageInfo->ClearFlags(BLOB_TYPE_BINARY_FIELD_FLAG | VARBINARY_FIELD_FLAG | BLOB_BINARY_FIELD_FLAG | MEDIUM_BLOB_BINARY_FIELD_FLAG | LONG_BLOB_BINARY_FIELD_FLAG );
            storageInfo->AddFlags(LONG_BLOB_BINARY_FIELD_FLAG);
            break;
        }
    } while (false);
    
    return true;
}

bool StorageFlagType::CheckCanSupportMysqlDataType(const KERNEL_NS::LibString &dataType)
{
    if((dataType == "TINYINT") || 
        (dataType == "SMALLINT") ||
        (dataType == "INT") ||
        (dataType == "BIGINT") ||
        (dataType == "FLOAT") ||
        (dataType == "DOUBLE") ||
        (dataType == "VARCHAR") ||
        (dataType == "TEXT") ||
        (dataType == "MEDIUMTEXT") ||
        (dataType == "LONGTEXT") ||
        (dataType == "VARBINARY") ||
        (dataType == "BLOB") ||
        (dataType == "MEDIUMBLOB") ||
        (dataType == "LONGBLOB")
        )
    {
        return true;
    }

    return false;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(IStorageInfo);

IStorageInfo::IStorageInfo(const KERNEL_NS::LibString &systemName)
:_systemName(systemName)
,_flags(0)
,_primaryKeyStorage(NULL)
,_kvModeValueStorage(NULL)
,_capacitySize(0)
,_inputMysqlDataType(0)
,_outputMysqlDataType(0)
,_releaseCb(NULL)
,_dataCountLimit(-1)
{
    _SetType(ServiceCompType::STORAGE_COMP);

    // 兼容windows/linux 表名使用小写
    _tableName = KERNEL_NS::StringUtil::RemoveNameSpace(_systemName).tolower();
    _fieldName = KERNEL_NS::StringUtil::RemoveNameSpace(_systemName);
}

IStorageInfo::IStorageInfo(Byte8 const * const &systemName)
:_systemName(systemName)
,_flags(0)
,_primaryKeyStorage(NULL)
,_kvModeValueStorage(NULL)
,_capacitySize(0)
,_inputMysqlDataType(0)
,_outputMysqlDataType(0)
,_releaseCb(NULL)
,_dataCountLimit(-1)
{
    _SetType(ServiceCompType::STORAGE_COMP);

    // 兼容windows/linux 表名使用小写
    _tableName = KERNEL_NS::StringUtil::RemoveNameSpace(_systemName).tolower();
    _fieldName = KERNEL_NS::StringUtil::RemoveNameSpace(_systemName);
}

IStorageInfo::~IStorageInfo()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_subStorageInfos);

    if(_releaseCb)
        _releaseCb->Release();

    _releaseCb = NULL;
}

void IStorageInfo::Release()
{
    if(_releaseCb)
    {
        _releaseCb->Invoke();
        return;
    }

    g_Log->Error(LOGFMT_OBJ_TAG("cant invoke IStorageInfo:Release without release callback derectly please check system name:%s"), GetSystemName().c_str());
}

Int32 IStorageInfo::ManualStart()
{
    auto err = Init();
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("init fail err:%d, system:%s")
            , err, _systemName.c_str());
        return err;
    }

    for(auto v : _subStorageInfos)
    {
        err = v->Init();
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("init fail err:%d, sub system:%s, cur system:%s")
                , err, v->GetSystemName().c_str(), _systemName.c_str());
            return err;
        }
    }

    err = Start();
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("start fail err:%d"), err);
        return err;
    }

    for(auto v : _subStorageInfos)
    {
        err = v->Start();
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("start fail err:%d, sub system:%s, cur system:%s")
                , err, v->GetSystemName().c_str(), _systemName.c_str());
            return err;
        }
    }

    return Status::Success;
}

void IStorageInfo::ManualClose()
{
    WillClose();

    Close();
}

KERNEL_NS::LibString IStorageInfo::ToString() const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("base info: system:%s, table name:%s, comment:%s, flags:%llx, sub storage info count:%d, has primary key storage:%d, primary key name:%s, primary key field name:%s, capacity size:%llu, _dataCountlimit:%d\n"
            ,_systemName.c_str(), _tableName.c_str(), _comment.c_str(),  _flags
            , static_cast<Int32>(_subStorageInfos.size()), _primaryKeyStorage != NULL
            , _primaryKeyStorage ? _primaryKeyStorage->GetFieldName().c_str() : ""
            , _fieldName.c_str(), _capacitySize, _dataCountLimit);

    info.AppendFormat("sub storage info:[\n");
    for(auto storageInfo : _subStorageInfos)
        info.AppendData(storageInfo->ToString()).AppendFormat("\n");

    info.AppendFormat("]\n");

    return info;
}

Int32 IStorageInfo::_OnInit()
{
    if(!RegisterStorages())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("RegisterStorages fail"));
        return Status::Failed;
    }

    for(auto v : _subStorageInfos)
    {
        auto err = v->Init();
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("init fail err:%d, sub system:%s, cur system:%s")
                , err, v->GetSystemName().c_str(), _systemName.c_str());
            return err;
        }
    }

    return Status::Success;
}

Int32 IStorageInfo::_OnStart()
{
    for(auto v : _subStorageInfos)
    {
        auto err = v->Start();
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("start fail err:%d, sub system:%s, cur system:%s")
                , err, v->GetSystemName().c_str(), _systemName.c_str());
            return err;
        }
    }

    return Status::Success;
}

void IStorageInfo::_OnWillClose()
{
    for(auto v : _subStorageInfos)
        v->WillClose();
}

void IStorageInfo::_OnClose()
{
    for(auto v : _subStorageInfos)
        v->Close();
}

// TODO:需要测试存储以及取数据对序列化反序列化的影响
void IStorageInfo::_UpadteCapacityAndMysqlDataType()
{
    if(IsNormalStringField())
    {
        _inputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_STRING;
        _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_VARCHAR;
        return;
    }

    // 只有Text, Blob才需要考虑更新capacity(在addflags时候)
    if(IsTextField())
    {
         _inputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_STRING;
        if(IsNormalTextStringField())
        {
            _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_BLOB;
            _capacitySize = StorageCapacityType::Cap64K;
            return;
        }

        if(IsMediumTextStringField())
        {
            _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_MEDIUM_BLOB;
            _capacitySize = StorageCapacityType::Cap16M;
            return;
        }

        if(IsLongTextStringField())
        {
            _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_LONG_BLOB;
            _capacitySize = StorageCapacityType::Cap4GB;
            return;
        }

        return;
    }

    if(IsVarBinaryField())
    {
        _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_VARCHAR;
        _inputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_BLOB;
        return;
    }

    if(IsBlobTypeBinaryField())
    {
        if(IsBlobBinaryField())
        {
            _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_BLOB;
            _inputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_BLOB;
            _capacitySize = StorageCapacityType::Cap64K;
            return;
        }

        if(IsMediumBlobBinaryField())
        {
            _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_MEDIUM_BLOB;
            _inputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_BLOB;
            _capacitySize = StorageCapacityType::Cap16M;
            return;
        }

        if(IsLongBlobBinaryField())
        {
            _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_LONG_BLOB;
            _inputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_BLOB;
            _capacitySize = StorageCapacityType::Cap4GB;
            return;
        }

        return;
    }

    if(IsNumberField())
    {
        if(IsInt8NumberField())
        {
            _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_TINY;
            _inputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_TINY;
            _capacitySize = static_cast<UInt64>(sizeof(Byte8));
            return;
        }

        if(IsInt16NumberField())
        {
            _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_SHORT;
            _inputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_SHORT;
            _capacitySize = static_cast<UInt64>(sizeof(Int16));
            return;
        }

        if(IsInt32NumberField())
        {
            _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_LONG;
            _inputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_LONG;
            _capacitySize = static_cast<UInt64>(sizeof(Int32));
            return;
        }

        if(IsInt64NumberField())
        {
            _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_LONGLONG;
            _inputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_LONGLONG;
            _capacitySize = static_cast<UInt64>(sizeof(Int64));
            return;
        }

        if(IsFloatNumberField())
        {
            _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_FLOAT;
            _inputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_FLOAT;
            _capacitySize = static_cast<UInt64>(sizeof(Float));
            return;
        }

        if(IsDoubleNumberField())
        {
            _outputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_DOUBLE;
            _inputMysqlDataType = StorageFlagType::STORAGE_MYSQL_TYPE_DOUBLE;
            _capacitySize = static_cast<UInt64>(sizeof(Double));
            return;
        }
    }
}

OBJ_GET_OBJ_TYPEID_IMPL(IStorageInfo)

SERVICE_END
