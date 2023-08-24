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
 * Date: 2023-07-08 17:27:00
 * Author: Eric Yonng
 * Description: Mysql预处理
*/

#include <pch.h>
#include <OptionComp/storage/mysql/impl/PrepareStmt.h>

#include <mysql.h>
#include <OptionComp/storage/mysql/impl/MysqlConnect.h>
#include <OptionComp/storage/mysql/impl/Defs.h>
#include <OptionComp/storage/mysql/impl/Field.h>

KERNEL_BEGIN
POOL_CREATE_OBJ_DEFAULT_IMPL(PrepareStmt);

PrepareStmt::PrepareStmt(MysqlConnect *conn, const LibString &sql, UInt64 id)
:_conn(conn)
,_sql(sql)
,_id(id)
,_stmt(NULL)
,_isError(false)
,_isInit(false)
,_isConnected(false)
,_paramCount(0)
,_bindParams(NULL)
,_curBindParamIndex(0)
,_paramValues(NULL)
,_seqId(0)
,_resultFieldCount(0)
,_resultBinds(NULL)
,_resultFieldIsNulls(NULL)
,_resultFieldLengths(NULL)
,_hasRecords(false)
,_isSucObtainResultSetMetadata(false)
{

}

PrepareStmt::~PrepareStmt()
{
    if(_stmt)
    {
        mysql_stmt_close(_stmt);
        _stmt = NULL;
    }

    _ClearBindParamValue();
    _ClearBindResult();
}

bool PrepareStmt::Init()
{
    if(_isInit)
        return true;

    _conn->AddOpCount(MysqlOperateType::Operate);

    auto mysqlObj = _conn->GetMysql();
    _stmt = mysql_stmt_init(mysqlObj);
    
    if(mysql_stmt_prepare(_stmt, _sql.c_str(), static_cast<ULong>(_sql.length() + 1)) != 0)
    {
        auto errNo = mysql_errno(mysqlObj);
        auto stmtErrNo = mysql_stmt_errno(_stmt);
        if(stmtErrNo != 0)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("fail mysql_stmt_prepare stmt err:%s, %u"), mysql_stmt_error(_stmt), mysql_stmt_errno(_stmt));
        }

        _conn->_UpdateLastMysqlErrno();
        if(!IS_MYSQL_NETWORK_ERROR(errNo))
        {
            _isError = true;
            g_Log->Error(LOGFMT_OBJ_TAG("fail mysql_stmt_prepare error:%s"), mysql_error(mysqlObj));
        }
        else
        {
            _isError = false;
            g_Log->Warn(LOGFMT_OBJ_TAG("fail to mysql_stmt_prepare, mysql network error:%s"), mysql_error(mysqlObj));
        }

        return false;
    }

    _paramCount = static_cast<Int64>(mysql_stmt_param_count(_stmt));

    _isInit = true;
    _isConnected = true;
    return true;
}

bool PrepareStmt::OnMysqlReconnect()
{
    if(_stmt)
        mysql_stmt_close(_stmt);

    _stmt = NULL;
    _paramCount = 0;
    _seqId = 0;

    _ClearBindResult();
    _ClearBindParamValue();
    _isInit = false;
    _isError = false;

    auto ret = Init();

    g_Log->Info(LOGFMT_OBJ_TAG("stmt reconnect ret:%d, stmt info:%s"), ret, ToString().c_str());

    return ret;
}

void PrepareStmt::OnMysqlDisconnect()
{
    _isConnected = false;
}

bool PrepareStmt::IsConnected() const
{
    return _isConnected;
}

void PrepareStmt::StartParam(UInt64 seqId)
{
    _ClearBindParamValue();

    _seqId = seqId;
    _curBindParamIndex = 0;
    if(_paramCount > 0)
    {
        if(!_bindParams)
            _bindParams = KERNEL_NS::KernelCastTo<MYSQL_BIND>(KERNEL_ALLOC_MEMORY_TL(sizeof(MYSQL_BIND) * _paramCount));

        if(!_paramValues)
            _paramValues = KERNEL_NS::KernelCastTo<Field*>(KERNEL_ALLOC_MEMORY_TL(sizeof(Field*) * _paramCount));

        ::memset(_bindParams, 0, sizeof(MYSQL_BIND) * _paramCount);
        ::memset(_paramValues, 0, sizeof(Field*) * _paramCount);
    }
}

void PrepareStmt::BindParam(Field *field)
{
    auto curIndex = _curBindParamIndex++;
    auto &bindParam = _bindParams[curIndex];
    auto newField = Field::NewThreadLocal_Field(*field);
    _paramValues[curIndex] = newField;
    bindParam.buffer_type = static_cast<enum_field_types>(newField->GetType());
    
    // 绑定数据
    if(newField->GetType() != static_cast<Int32>(MYSQL_TYPE_NULL))
    {
        // TODO:数据长度需要根据实际的Type做限制 例如int只有4个字节如果外部写入一个UInt64进来就是8个字节了
        bindParam.buffer = newField->GetData()->GetReadBegin();
        bindParam.buffer_length = static_cast<ULong>(newField->GetData()->GetReadableSize());
    }
}

UInt32 PrepareStmt::CommitParam()
{
    Int32 ret = 0;
    if(_paramCount > 0)
    {
        _conn->AddOpCount(MysqlOperateType::Operate);
        ret = mysql_stmt_bind_param(_stmt, _bindParams);
        if(ret != 0)
        {
            UInt32 errNo = mysql_errno(_conn->GetMysql());
            auto stmtErr = mysql_stmt_errno(_stmt);
            if(IS_MYSQL_NETWORK_ERROR(stmtErr))
                _OnMysqlDisconnect();

            _conn->_UpdateLastMysqlErrno();
            g_Log->Warn(LOGFMT_OBJ_TAG("mysql_stmt_bind_param fail error:%s, errNo:%u, mysql stmt:%s,%u"), mysql_error(_conn->GetMysql()), errNo, mysql_stmt_error(_stmt), stmtErr);
            return stmtErr != 0 ? stmtErr : errNo;
        }
    }

    // blob或者text数据需要mysql_stmt_send_long_data 见官方文档
    for(Int32 idx = 0; idx < _paramCount; ++idx)
    {
        auto &bindParam = _bindParams[idx];
        if(IS_MYSQL_NEED_SEND_LONG_DATA(bindParam.buffer_type))
        {
            _conn->AddOpCount(MysqlOperateType::Operate);
            ret = mysql_stmt_send_long_data(_stmt, idx, reinterpret_cast<const Byte8 *>(bindParam.buffer), bindParam.buffer_length);
            if(ret != 0)
            {
                auto field = _paramValues[idx];
                UInt32 errNo = mysql_errno(_conn->GetMysql());
                _conn->_UpdateLastMysqlErrno();
                auto stmtErr = mysql_stmt_errno(_stmt);
                if(IS_MYSQL_NETWORK_ERROR(stmtErr))
                    _OnMysqlDisconnect();

                g_Log->Warn(LOGFMT_OBJ_TAG("mysql_stmt_send_long_data fail error:%s, errNo:%u, stmt mysql err:%s, stmtErrNo:%u bind field:%s")
                            , mysql_error(_conn->GetMysql()), errNo, mysql_stmt_error(_stmt), stmtErr, field ? field->ToString().c_str() : "");

                return stmtErr != 0 ? stmtErr : errNo;
            }
        }
    }

    return 0;
}

UInt32 PrepareStmt::Execute(UInt64 seqId, const std::vector<Field *> &fields, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb)
{
    if(g_Log->IsEnable(LogLevel::DumpSql))
    {
        std::vector<KERNEL_NS::LibString> fieldsStr;
        for(auto field:fields)
            fieldsStr.push_back(field->Dump());

        g_Log->DumpSql(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("seq id:%llu, sql:%s, fields:\n", seqId, _sql.c_str()), KERNEL_NS::StringUtil::ToString(fieldsStr, "\n"));
    }

    StartParam(seqId);

    for(auto field : fields)
        BindParam(field);

    auto ret = CommitParam();
    if(ret != 0)
    {
        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("commit param fail ret:%u", ret), ToString());
        if(LIKELY(cb))
        {
            std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> emptyRecords;
            cb->Invoke(_conn, _seqId, Status::Failed, ret, false, 0, 0, emptyRecords);
        }

        return ret;
    }

    ret = Execute();
    if(ret != 0)
    {
        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("execute fail ret:%u", ret), ToString());
        if(LIKELY(cb))
        {
            std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> emptyRecords;
            cb->Invoke(_conn, _seqId, Status::Failed, ret, false, 0, 0, emptyRecords);
        }

        return ret;   
    }

    ret = FetchRows(cb);
    if(ret != 0)
    {
        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("FetchRows fail ret:%u", ret), ToString());
        return ret;
    }

    return 0;
}

UInt32 PrepareStmt::Execute()
{
    _conn->AddOpCount(MysqlOperateType::Operate);
    auto ret = mysql_stmt_execute(_stmt);
    if(ret != 0)
    {
        int errNo = mysql_errno(_conn->GetMysql());
        _conn->_UpdateLastMysqlErrno();
        auto stmtErr = mysql_stmt_errno(_stmt);
        if(IS_MYSQL_NETWORK_ERROR(stmtErr))
            _OnMysqlDisconnect();

        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("mysql_stmt_execute fail error:%s, stmt err:%s,%u prepare stmt info:"
        , mysql_error(_conn->GetMysql()), mysql_stmt_error(_stmt), stmtErr)
                    , ToString());

        return stmtErr != 0 ? stmtErr : errNo;
    }

    return 0;
}

UInt32 PrepareStmt::FetchRows(IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb)
{
    UInt32 ret = _ObtainResultSetMetadata();
    if(ret != 0)
    {
        if(LIKELY(cb))
        {
            std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> emptyRecords;
            cb->Invoke(_conn, _seqId, Status::Failed, ret, false, 0, 0, emptyRecords);
        }

        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("_ObtainResultSetMetadata fail ret:%d, prepare stmt info:", ret), ToString());
        return ret;
    }

    // 成功
    if(!_hasRecords)
    {// 没有返回数据的sql 如insert, create等
        Int64 insertId = static_cast<Int64>(mysql_stmt_insert_id(_stmt));
        Int64 affectedRows = static_cast<Int64>(mysql_stmt_affected_rows(_stmt));
        if(LIKELY(cb))
        {
            std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> emptyRecords;
            cb->Invoke(_conn, _seqId, Status::Success, ret, true, insertId, affectedRows, emptyRecords);
        }

        mysql_stmt_free_result(_stmt);
        return 0;
    }
    
    // 有返回数据的sql 如select等
    _conn->AddOpCount(MysqlOperateType::Operate);
    ret = mysql_stmt_store_result(_stmt);
    if(ret != 0)
    {
        ret = mysql_errno(_conn->GetMysql());
        _conn->_UpdateLastMysqlErrno();
        auto stmtErr = mysql_stmt_errno(_stmt);
        if(IS_MYSQL_NETWORK_ERROR(stmtErr))
            _OnMysqlDisconnect();

        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT()
        , LibString().AppendFormat("mysql_stmt_store_result fail error:%s, ret:%u, stmt err:%s, %u", mysql_error(_conn->GetMysql()), ret, mysql_stmt_error(_stmt), stmtErr)
        , ToString());

        if(LIKELY(cb))
        {
            std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> emptyRecords;
            cb->Invoke(_conn, _seqId, Status::Failed, stmtErr != 0 ? stmtErr : ret, true, 0, 0, emptyRecords);
        }

        mysql_stmt_free_result(_stmt);
        return stmtErr != 0 ? stmtErr : ret;
    }

    std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> allRecords;
    auto rowCount = static_cast<UInt64>(mysql_stmt_num_rows(_stmt));
    Int32 retFetch = mysql_stmt_fetch(_stmt);
    const Int32 resultFieldCount = static_cast<Int32>(_resultFieldCount);
    while((retFetch == 0) || (retFetch == MYSQL_DATA_TRUNCATED))
    {
        SmartPtr<Record, AutoDelMethods::CustomDelete> record = Record::NewThreadLocal_Record();
        record.SetClosureDelegate([](void *p){
            auto ptr = KernelCastTo<Record>(p);
            Record::DeleteThreadLocal_Record(ptr);
        });

        // 一行数据填充
        record->SetFieldAmount(resultFieldCount);
        for(Int32 idx = 0; idx < resultFieldCount; ++idx)
        {
            auto &bindField = _resultBinds[idx];
            auto &flags = _indexOfFlags[idx];

            const auto &tableName = _indexRefTableName[idx];
            const auto &fieldName = _indexRefFieldName[idx];

            // const bool isNull = (bindField.is_null != 0);
            Int64 len = static_cast<Int64>(bindField.length ? *(bindField.length) : 0);
            record->AddField(idx, tableName, fieldName, bindField.buffer_type, flags, (len == 0) ? NULL : bindField.buffer, len);
        }
        allRecords.push_back(record);

        retFetch = mysql_stmt_fetch(_stmt);
    }

    if(retFetch == 1)
    {
        ret = mysql_errno(_conn->GetMysql());
        _conn->_UpdateLastMysqlErrno();
        auto stmtErr = mysql_stmt_errno(_stmt);
        if(IS_MYSQL_NETWORK_ERROR(stmtErr))
            _OnMysqlDisconnect();

        if(LIKELY(cb))
        {
            std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> emptyRecords;
            cb->Invoke(_conn, _seqId, Status::Failed, stmtErr != 0 ? stmtErr : ret, true, 0, 0, emptyRecords);
        }

        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("mysql_stmt_fetch error stmt err:%s,%u mysql err:%s,%u allRecords:%llu"
        , mysql_stmt_error(_stmt), stmtErr, mysql_error(_conn->GetMysql()), mysql_errno(_conn->GetMysql()), static_cast<UInt64>(allRecords.size())), ToString());
        mysql_stmt_free_result(_stmt);
        return stmtErr != 0 ? stmtErr : ret;
    }

    if(LIKELY(cb))
    {
        cb->Invoke(_conn, _seqId, Status::Success, 0, true, 0, 0, allRecords);
    }
    mysql_stmt_free_result(_stmt);

    return 0;
}

LibString PrepareStmt::ToString() const
{
    LibString info;
    info.AppendFormat("seq id:%llu, has error:%d, is init:%d, param count:%lld, result field count:%u, result has records%d, do success obtain result set metadata:%d, sql:"
                    , _seqId, _isError, _isInit, _paramCount, _resultFieldCount, _hasRecords, _isSucObtainResultSetMetadata)
        .AppendData(_sql);

    return info;
}

void PrepareStmt::_ClearBindResult()
{
    _hasRecords = false;
    _resultFieldCount = 0;
    _isSucObtainResultSetMetadata = false;
    if(_resultBinds)
    {
        // 清理buffer
        for(Int32 idx = 0; idx < _paramCount; ++idx)
        {
            auto &result = _resultBinds[idx];
            if(result.buffer)
            {
                KERNEL_FREE_MEMORY_TL(result.buffer);
                result.buffer = NULL;
            }
        }
        
        KERNEL_FREE_MEMORY_TL(_resultBinds);
        _resultBinds = NULL;
    }

    if(_resultFieldIsNulls)
    {
        KERNEL_FREE_MEMORY_TL(_resultFieldIsNulls);
        _resultFieldIsNulls = NULL;
    }

    if(_resultFieldLengths)
    {
        KERNEL_FREE_MEMORY_TL(_resultFieldLengths);
        _resultFieldLengths = NULL;
    }

    _fieldNameRefIndex.clear();
    _indexRefFieldName.clear();
    _indexRefTableName.clear();
    _indexOfFlags.clear();
}

void PrepareStmt::_ClearBindParamValue()
{
    if(_bindParams)
        ::memset(_bindParams, 0, sizeof(MYSQL_BIND) * _paramCount);

    if(_paramValues)
    {
        for(Int32 idx = 0; idx < _paramCount; ++idx)
        {
            auto field = _paramValues[idx];
            if(field)
                field->Release();

            _paramValues[idx] = NULL;
        }
    }

    _curBindParamIndex = 0;
}

UInt32 PrepareStmt::_ObtainResultSetMetadata()
{
    // 绑定元数据信息, 只执行一次 因为绑定之后,不需要再绑定, mysql那边会一直保留 除非重连
    if(_isSucObtainResultSetMetadata)
        return 0;

    _conn->AddOpCount(MysqlOperateType::Operate);

    _hasRecords = false;
    UInt32 ret = 0;
    auto prepareMetaResult = mysql_stmt_result_metadata(_stmt);
    if(!prepareMetaResult)
    {
        ret = mysql_errno(_conn->GetMysql());
        auto stmtErr = mysql_stmt_errno(_stmt);
        if(IS_MYSQL_NETWORK_ERROR(stmtErr))
            _OnMysqlDisconnect();

        if(ret == 0 && stmtErr == 0)
        {
            auto fieldCount = mysql_stmt_field_count(_stmt);
            if(fieldCount == 0)
            {// 没有字段信息说明不是查询等相关sql操作
                _isSucObtainResultSetMetadata = true;
                return 0;
            }

            // 有字段信息, 但是没结果 一般是select等
            g_Log->Warn(LOGFMT_OBJ_TAG("have some field info but have no result, error:%s"), mysql_error(_conn->GetMysql()));
            return ret;
        }

        _conn->_UpdateLastMysqlErrno();

        // 错误发生
        g_Log->Error2(LOGFMT_OBJ_TAG_NO_FMT()
        , LibString().AppendFormat("mysql_stmt_result_metadata fail mysql error:%s,%u, stmt err:%s,%u prepare stmt info:",  mysql_error(_conn->GetMysql()), ret, mysql_stmt_error(_stmt), stmtErr)
        , ToString());

        return stmtErr != 0 ? stmtErr : ret;
    }
    
    _hasRecords = true;

    _resultFieldCount = mysql_num_fields(prepareMetaResult);
    _resultBinds = reinterpret_cast<MYSQL_BIND *>(KERNEL_ALLOC_MEMORY_TL(sizeof(MYSQL_BIND) * _resultFieldCount));
    _resultFieldIsNulls = reinterpret_cast<bool *>(KERNEL_ALLOC_MEMORY_TL(sizeof(bool) * _resultFieldCount));
    _resultFieldLengths = reinterpret_cast<ULong *>(KERNEL_ALLOC_MEMORY_TL(sizeof(ULong) * _resultFieldCount));
    ::memset(_resultBinds, 0, sizeof(MYSQL_BIND) * _resultFieldCount);

    Int32 curFieldIndex = 0;
    auto field = mysql_fetch_field(prepareMetaResult);
    for(;field;)
    {
        const UInt64 sz = static_cast<UInt64>(field->length);
        _fieldNameRefIndex[field->name] = curFieldIndex;
        _indexRefFieldName[curFieldIndex] = field->name;
        _indexRefTableName[curFieldIndex] = field->table;
        _indexOfFlags[curFieldIndex] = static_cast<UInt64>(field->flags);

        _resultBinds[curFieldIndex].buffer_type = field->type;
        if(sz != 0)
        {
            _resultBinds[curFieldIndex].buffer = KERNEL_ALLOC_MEMORY_TL(sz);
            ::memset(_resultBinds[curFieldIndex].buffer, 0, sz);
        }
        else
        {
            _resultBinds[curFieldIndex].buffer = NULL;
        }

        _resultBinds[curFieldIndex].buffer_length = static_cast<ULong>(sz);
        _resultBinds[curFieldIndex].length = &_resultFieldLengths[curFieldIndex];
        _resultBinds[curFieldIndex].is_null = &_resultFieldIsNulls[curFieldIndex];
        _resultBinds[curFieldIndex].error = NULL;
        _resultBinds[curFieldIndex].is_unsigned = field->flags & UNSIGNED_FLAG;

        field = mysql_fetch_field(prepareMetaResult);

        ++curFieldIndex;
    }

    ::memset(_resultFieldIsNulls, 0, sizeof(bool) * _resultFieldCount);
    ::memset(_resultFieldLengths, 0, sizeof(ULong) * _resultFieldCount);

    ret = mysql_stmt_bind_result(_stmt, _resultBinds);
    if(ret != 0)
    {
        ret = mysql_errno(_conn->GetMysql());
        _conn->_UpdateLastMysqlErrno();
        auto stmtErr = mysql_stmt_errno(_stmt);
        if(IS_MYSQL_NETWORK_ERROR(stmtErr))
            _OnMysqlDisconnect();

        _ClearBindResult();
        
        g_Log->Error2(LOGFMT_OBJ_TAG_NO_FMT()
        , LibString().AppendFormat("mysql_stmt_bind_result fail stmt errNo:%u, stmt error:%s, mysql err:%s,%u prepare stmt info:"
                    , stmtErr, mysql_stmt_error(_stmt), mysql_error(_conn->GetMysql()), mysql_errno(_conn->GetMysql()))
        , ToString());

        mysql_free_result(prepareMetaResult);
        return stmtErr != 0 ? stmtErr : ret;
    }

    mysql_free_result(prepareMetaResult);
    _isSucObtainResultSetMetadata = true;

    return 0;
}

void PrepareStmt::_OnMysqlDisconnect()
{
    OnMysqlDisconnect();
    
    _conn->OnMysqlDisconnect();
}

KERNEL_END
