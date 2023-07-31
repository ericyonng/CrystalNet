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
#include <service/TestService/ServiceCompHeader.h>
#include <mysql.h>
#include <service/TestService/Comps/DB/impl/MysqlDefs.h>
#include <OptionComp/storage/mysql/mysqlcomp.h>

SERVICE_BEGIN

std::vector<UInt64> StorageSizeType::multis_than_16 = GenMultisThan16();

Int32 MysqlFieldTypeHelper::FieldTypeToMysqlType(const KERNEL_NS::LibString &fieldType)
{
    const auto &upper = fieldType.toupper();
    if(upper == "TINYINT")
        return MYSQL_TYPE_TINY;
        
    if(upper == "SMALLINT")
        return MYSQL_TYPE_SHORT;

    if(upper == "MEDIUMINT")
        return MYSQL_TYPE_INT24;

    if(upper == "INT")
        return MYSQL_TYPE_LONG;

    if(upper == "BIGINT")
        return MYSQL_TYPE_LONGLONG;

    if(upper == "FLOAT")
        return MYSQL_TYPE_FLOAT;

    if(upper == "DOUBLE")
        return MYSQL_TYPE_DOUBLE;

    if(upper == "DATETIME")
        return MYSQL_TYPE_DATETIME;

    if(upper == "VARCHAR")
        return MYSQL_TYPE_VAR_STRING;

    if((upper == "BLOB") || (upper == "TEXT") || (upper == "VARBINARY"))
        return MYSQL_TYPE_BLOB;

    if((upper == "MEDIUMBLOB") || (upper == "MEDIUMTEXT"))
        return MYSQL_TYPE_MEDIUM_BLOB;

    if((upper == "LONGBLOB") || (upper == "LONGTEXT"))
        return MYSQL_TYPE_LONG_BLOB;

    return -1;
}

bool MysqlFieldTypeHelper::GetFieldTypeBySize(UInt64 sz, KERNEL_NS::LibString &fieldType)
{
    if(sz <= StorageSizeType::VERY_SUPER_BIG_SIZE)
    {
        sz = StorageSizeType::GetCapacityType(sz);
        sz = sz > StorageSizeType::VERY_SUPER_BIG_SIZE ? StorageSizeType::VERY_SUPER_BIG_SIZE : sz;
        fieldType.AppendFormat("VARBINARY(%llu)", sz);
        return true;
    }

    if(sz <= StorageSizeType::VERY_HUGE_SIZE)
    {
        fieldType = "BLOB";
        return true;
    }

    if(sz <= StorageSizeType::SUPER_HUGE_SIZE)
    {
        fieldType = "MEDIUMBLOB";
        return true;
    }

    if(sz <= StorageSizeType::SERIOUS_SIZE)
    {
        fieldType = "LONGBLOB";
        return true;
    }

    return false;
}

bool MysqlFieldTypeHelper::GetStringPrimaryDataTypeBySize(UInt64 sz, KERNEL_NS::LibString &dataType)
{
    // 主键最大只支持1024字节长度
    if(sz <= 256)
    {
        dataType = "VARCHAR(256)";
        return true;
    }

    if(sz <= 1024)
    {
        dataType = "VARCHAR(1024)";
        return true;
    }

    return false;
}

bool MysqlFieldTypeHelper::GetStringDataTypeBySize(UInt64 sz, bool onlyText, bool onlyVarchar, KERNEL_NS::LibString &dataType)
{
    if(onlyVarchar)
    {
        if(sz <= 64)
        {
            dataType = "VARCHAR(64)";
            return true;
        }

        if(sz <= 256)
        {
            dataType = "VARCHAR(256)";
            return true;
        }

        if(sz <= 512)
        {
            dataType = "VARCHAR(512)";
            return true;
        }

        if(sz <= 1024)
        {
            dataType = "VARCHAR(1024)";
            return true;
        }

        if(sz <= 2048)
        {
            dataType = "VARCHAR(2048)";
            return true;
        }

        return false;
    }

    if(onlyText)
    {
        if(sz <= StorageSizeType::VERY_HUGE_SIZE)
        {
            dataType = "TEXT";
            return true;
        }

        if(sz <= StorageSizeType::SUPER_HUGE_SIZE)
        {
            dataType = "MEDIUMTEXT";
            return true;
        }

        if(sz <= StorageSizeType::SERIOUS_SIZE)
        {
            dataType = "LONGTEXT";
            return true;
        }
    }

    if(sz <= 64)
    {
        dataType = "VARCHAR(64)";
        return true;
    }

    if(sz <= 256)
    {
        dataType = "VARCHAR(256)";
        return true;
    }

    if(sz <= 512)
    {
        dataType = "VARCHAR(512)";
        return true;
    }

    if(sz <= 1024)
    {
        dataType = "VARCHAR(1024)";
        return true;
    }

    if(sz <= 2048)
    {
        dataType = "VARCHAR(2048)";
        return true;
    }

    if(sz <= StorageSizeType::VERY_HUGE_SIZE)
    {
        dataType = "TEXT";
        return true;
    }

    if(sz <= StorageSizeType::SUPER_HUGE_SIZE)
    {
        dataType = "MEDIUMTEXT";
        return true;
    }

    if(sz <= StorageSizeType::SERIOUS_SIZE)
    {
        dataType = "LONGTEXT";
        return true;
    }

    return false;
}

bool MysqlFieldTypeHelper::MakeFieldDescribe(const IStorageInfo *fieldDefine, KERNEL_NS::LibString &fieldDesc, bool isAddColumn)
{
    if(!isAddColumn)
        fieldDesc.AppendFormat("%s ", fieldDefine->GetFieldName().c_str());

    if(fieldDefine->IsPrimaryField())
    {
        if(fieldDefine->IsStringField())
        {
            if(UNLIKELY(fieldDefine->GetCapacitySize() == 0))
            {
                g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "field define capacity cant be zero field define:%s"), fieldDefine->ToString().c_str());
                return false;
            }

            KERNEL_NS::LibString dataType;
            if(UNLIKELY(!GetStringPrimaryDataTypeBySize(fieldDefine->GetCapacitySize(), dataType)))
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "GetPrimaryDataTypeBySize fail, field define:%s"), fieldDefine->ToString().c_str());
                return false;
            }

            fieldDesc.AppendFormat("%s NOT NULL COMMENT '%s'", dataType.c_str(), fieldDefine->GetComment().c_str());
            return true;
        }

        if(fieldDefine->IsNumberField())
        {
            if(fieldDefine->IsInt8NumberField())
            {
                fieldDesc.AppendFormat("TINYINT ");
            }
            if(fieldDefine->IsInt16NumberField())
            {
                fieldDesc.AppendFormat("SMALLINT ");
            }
            if(fieldDefine->IsInt32NumberField())
            {
                fieldDesc.AppendFormat("INT ");
            }
            else if(fieldDefine->IsInt64NumberField())
            {
                fieldDesc.AppendFormat("BIGINT ");
            }
            else
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "unknown number, field define:%s"), fieldDefine->ToString().c_str());
                return false;
            }

            if(fieldDefine->IsUnsignedField())
            {
                fieldDesc.AppendFormat("UNSIGNED ");
            }

            fieldDesc.AppendFormat("NOT NULL AUTO_INCREMENT COMMENT '%s'", fieldDefine->GetComment().c_str());
            return true;
        }

        g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "primary key not number nor string, bad field type field define:%s"), fieldDefine->ToString().c_str());
        return false;
    }

    if(fieldDefine->IsBinaryField())
    {
        if(UNLIKELY(fieldDefine->GetCapacitySize() == 0))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "field define capacity cant be zero field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        KERNEL_NS::LibString dataType;
        if(UNLIKELY(!GetFieldTypeBySize(fieldDefine->GetCapacitySize(), dataType)))
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "GetFieldTypeBySize fail, field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        fieldDesc.AppendFormat("%s COMMENT '%s'", dataType.c_str(), fieldDefine->GetComment().c_str());
        return true;
    }

    if(fieldDefine->IsNumberField())
    {
        if(fieldDefine->IsInt8NumberField())
        {
            fieldDesc.AppendFormat("TINYINT ");
        }
        if(fieldDefine->IsInt16NumberField())
        {
            fieldDesc.AppendFormat("SMALLINT ");
        }
        if(fieldDefine->IsInt32NumberField())
        {
            fieldDesc.AppendFormat("INT ");
        }
        else if(fieldDefine->IsInt64NumberField())
        {
            fieldDesc.AppendFormat("BIGINT ");
        }
        else if(fieldDefine->IsFloatNumberField())
        {
            fieldDesc.AppendFormat("FLOAT");
        }
        else if(fieldDefine->IsDoubleNumberField())
        {
            fieldDesc.AppendFormat("DOUBLE");
        }
        else
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "unknown number, field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        if(fieldDefine->IsUnsignedField())
        {
            fieldDesc.AppendFormat("UNSIGNED ");
        }

        fieldDesc.AppendFormat("NOT NULL DEFAULT 0 COMMENT '%s'", fieldDefine->GetComment().c_str());
        return true;
    }

    if(fieldDefine->IsStringField())
    {
        if(UNLIKELY(fieldDefine->GetCapacitySize() == 0))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "field define capacity cant be zero field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        KERNEL_NS::LibString dataType;
        if(UNLIKELY(!GetStringDataTypeBySize(fieldDefine->GetCapacitySize(), fieldDefine->IsTextField(), false, dataType)))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "GetStringDataTypeBySize fail field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        if(fieldDefine->IsTextField())
        {
            fieldDesc.AppendFormat("%s COMMENT '%s'", dataType.c_str(), fieldDefine->GetComment().c_str());
        }
        else
        {
            fieldDesc.AppendFormat("%s NOT NULL DEFAULT '' COMMENT '%s'", dataType.c_str(), fieldDefine->GetComment().c_str());
        }

        return true;
    }

    g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "not number, string, binary, bad field type field define:%s"), fieldDefine->ToString().c_str());
    return false;
}

bool MysqlFieldTypeHelper::MakeFieldDataType(const IStorageInfo *fieldDefine, KERNEL_NS::LibString &dataType)
{
    if(fieldDefine->IsPrimaryField())
    {
        if(fieldDefine->IsStringField())
        {
            if(UNLIKELY(fieldDefine->GetCapacitySize() == 0))
            {
                g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "field define capacity cant be zero field define:%s"), fieldDefine->ToString().c_str());
                return false;
            }

            if(UNLIKELY(!GetStringPrimaryDataTypeBySize(fieldDefine->GetCapacitySize(), dataType)))
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "GetPrimaryDataTypeBySize fail, field define:%s"), fieldDefine->ToString().c_str());
                return false;
            }

            return true;
        }

        if(fieldDefine->IsNumberField())
        {
            if(fieldDefine->IsInt8NumberField())
            {
                dataType.AppendFormat("TINYINT");
            }
            if(fieldDefine->IsInt16NumberField())
            {
                dataType.AppendFormat("SMALLINT");
            }
            if(fieldDefine->IsInt32NumberField())
            {
                dataType.AppendFormat("INT");
            }
            else if(fieldDefine->IsInt64NumberField())
            {
                dataType.AppendFormat("BIGINT");
            }
            else
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "unknown number, field define:%s"), fieldDefine->ToString().c_str());
                return false;
            }

            if(fieldDefine->IsUnsignedField())
            {
                dataType.AppendFormat(" UNSIGNED");
            }

            return true;
        }

        g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "primary key not number nor string, bad field type field define:%s"), fieldDefine->ToString().c_str());
        return false;
    }

    if(fieldDefine->IsBinaryField())
    {
        if(UNLIKELY(fieldDefine->GetCapacitySize() == 0))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "field define sz cant be zero field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        if(UNLIKELY(!GetFieldTypeBySize(fieldDefine->GetCapacitySize(), dataType)))
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "GetFieldTypeBySize fail, field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        return true;
    }

    if(fieldDefine->IsNumberField())
    {
        if(fieldDefine->IsInt8NumberField())
        {
            dataType.AppendFormat("TINYINT");
        }
        if(fieldDefine->IsInt16NumberField())
        {
            dataType.AppendFormat("SMALLINT");
        }
        if(fieldDefine->IsInt32NumberField())
        {
            dataType.AppendFormat("INT");
        }
        else if(fieldDefine->IsInt64NumberField())
        {
            dataType.AppendFormat("BIGINT");
        }
        else if(fieldDefine->IsFloatNumberField())
        {
            dataType.AppendFormat("FLOAT");
        }
        else if(fieldDefine->IsDoubleNumberField())
        {
            dataType.AppendFormat("DOUBLE");
        }
        else
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "unknown number, field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        if(fieldDefine->IsUnsignedField())
        {
            dataType.AppendFormat(" UNSIGNED");
        }

        return true;
    }

    if(fieldDefine->IsStringField())
    {
        if(UNLIKELY(fieldDefine->GetCapacitySize() == 0))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "field define capacity cant be zero field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }
        
        if(UNLIKELY(!GetStringDataTypeBySize(fieldDefine->GetCapacitySize(), fieldDefine->IsTextField(), false, dataType)))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "GetStringDataTypeBySize fail field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        return true;
    }

    g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "not number, string, binary, bad field type field define:%s"), fieldDefine->ToString().c_str());
    return false;
}

bool MysqlFieldTypeHelper::TurnToMysqlDataType(const IStorageInfo *fieldDefine, KERNEL_NS::LibString &dataType)
{
    if(fieldDefine->IsPrimaryField())
    {
        if(fieldDefine->IsStringField())
        {
            if(UNLIKELY(fieldDefine->GetCapacitySize() == 0))
            {
                g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "field define capacity cant be zero field define:%s"), fieldDefine->ToString().c_str());
                return false;
            }

            if(UNLIKELY(!GetStringPrimaryDataTypeBySize(fieldDefine->GetCapacitySize(), dataType)))
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "GetPrimaryDataTypeBySize fail, field define:%s"), fieldDefine->ToString().c_str());
                return false;
            }

            if(dataType.Contain("("))
                dataType = dataType.DragBefore("(");

            return true;
        }

        if(fieldDefine->IsNumberField())
        {
            if(fieldDefine->IsInt8NumberField())
            {
                dataType.AppendFormat("TINYINT");
            }
            if(fieldDefine->IsInt16NumberField())
            {
                dataType.AppendFormat("SMALLINT");
            }
            if(fieldDefine->IsInt32NumberField())
            {
                dataType.AppendFormat("INT");
            }
            else if(fieldDefine->IsInt64NumberField())
            {
                dataType.AppendFormat("BIGINT");
            }
            else
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "unknown number, field define:%s"), fieldDefine->ToString().c_str());
                return false;
            }

            return true;
        }

        g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "primary key not number nor string, bad field type field define:%s"), fieldDefine->ToString().c_str());
        return false;
    }

    if(fieldDefine->IsBinaryField())
    {
        if(UNLIKELY(fieldDefine->GetCapacitySize() == 0))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "field define sz cant be zero field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        if(UNLIKELY(!GetFieldTypeBySize(fieldDefine->GetCapacitySize(), dataType)))
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "GetFieldTypeBySize fail, field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        if(dataType.Contain("("))
            dataType = dataType.DragBefore("(");

        return true;
    }

    if(fieldDefine->IsNumberField())
    {
        if(fieldDefine->IsInt8NumberField())
        {
            dataType.AppendFormat("TINYINT");
        }
        if(fieldDefine->IsInt16NumberField())
        {
            dataType.AppendFormat("SMALLINT");
        }
        if(fieldDefine->IsInt32NumberField())
        {
            dataType.AppendFormat("INT");
        }
        else if(fieldDefine->IsInt64NumberField())
        {
            dataType.AppendFormat("BIGINT");
        }
        else if(fieldDefine->IsFloatNumberField())
        {
            dataType.AppendFormat("FLOAT");
        }
        else if(fieldDefine->IsDoubleNumberField())
        {
            dataType.AppendFormat("DOUBLE");
        }
        else
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "unknown number, field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        return true;
    }

    if(fieldDefine->IsStringField())
    {
        if(UNLIKELY(fieldDefine->GetCapacitySize() == 0))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "field define capacity cant be zero field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }
        
        if(UNLIKELY(!GetStringDataTypeBySize(fieldDefine->GetCapacitySize(), fieldDefine->IsTextField(), false, dataType)))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "GetStringDataTypeBySize fail field define:%s"), fieldDefine->ToString().c_str());
            return false;
        }

        if(dataType.Contain("("))
            dataType = dataType.DragBefore("(");

        return true;
    }

    g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "not number, string, binary, bad field type field define:%s"), fieldDefine->ToString().c_str());
    return false;
}

KERNEL_NS::CreateTableSqlBuilder *MysqlFieldTypeHelper::NewCreateTableSqlBuilder(const KERNEL_NS::LibString &dbName, const IStorageInfo *storageInfo)
{
    const auto &tableName = storageInfo->GetTableName();
    KERNEL_NS::SmartPtr<KERNEL_NS::CreateTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> createBuilder = KERNEL_NS::CreateTableSqlBuilder::NewThreadLocal_CreateTableSqlBuilder();
    createBuilder.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<KERNEL_NS::CreateTableSqlBuilder *>(p);
        KERNEL_NS::CreateTableSqlBuilder::DeleteThreadLocal_CreateTableSqlBuilder(ptr);
    });

    createBuilder->DB(dbName).Table(tableName);
    auto &fieldInfos = storageInfo->GetSubStorageInfos();
    for(auto subStorageInfo : fieldInfos)
    {
        KERNEL_NS::LibString dataType;
        if(UNLIKELY(!MysqlFieldTypeHelper::MakeFieldDescribe(subStorageInfo, dataType)))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(MysqlFieldTypeHelper, "MakeFieldDescribe fail storage info :%s"), storageInfo->ToString().c_str());
            return NULL;
        }

        createBuilder->Field(dataType);

        if(subStorageInfo->IsPrimaryField())
            createBuilder->PrimaryKey(subStorageInfo->GetFieldName());
    }
    createBuilder->Comment(storageInfo->GetComment());

    return createBuilder.pop();
}

bool MysqlFieldTypeHelper::CheckCanSupportMysqlDataType(const KERNEL_NS::LibString &dataType)
{
    return StorageFlagType::CheckCanSupportMysqlDataType(dataType);
}

SERVICE_END