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

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_PREPARE_STMT_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_PREPARE_STMT_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <OptionComp/storage/mysql/impl/Record.h>

struct MYSQL_STMT;
struct MYSQL_BIND;

KERNEL_BEGIN

class MysqlConnect;
class Field;

class PrepareStmt
{
    POOL_CREATE_OBJ_DEFAULT(PrepareStmt);

public:
    PrepareStmt(MysqlConnect *conn, const LibString &sql, UInt64 id);
    ~PrepareStmt();

    bool Init();

    // 重置, 重新预编译等
    bool OnMysqlReconnect();

    void StartParam(UInt64 seqId);
    void BindParam(Field *field);
    // return 0 if success.
    UInt32 CommitParam();

    // return 0 if success, 执行sql结果: MysqlConnect*, UInt64(seqId), Int32(runErrCode), UInt32(mysqlerr), bool(是否发送到mysqlserver), Int64(InsertId), Int64(AffectedRows), std::vector<SmartPtr<Record>>(数据),
    UInt32 Execute(UInt64 seqId, const std::vector<Field *> &fields, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb);
    // return 0 if success
    UInt32 Execute();

    // return 0 if success 执行sql结果: MysqlConnect*, UInt64(seqId), Int32(runErrCode), UInt32(mysqlerr), bool(是否发送到mysqlserver), Int64(InsertId), Int64(AffectedRows), std::vector<SmartPtr<Record>>(数据),
    UInt32 FetchRows(IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb);

    bool IsInit() const;
    bool IsError() const;

    LibString ToString() const;

private:
    void _ClearBindResult();
    void _ClearBindParamValue();

    // 绑定字段信息, 只执行一次，除非重连 Fetch result set meta information
    UInt32 _ObtainResultSetMetadata();

private:
    MysqlConnect *_conn;
    const LibString _sql;
    const UInt64 _id;
    MYSQL_STMT *_stmt;
    bool _isError;
    bool _isInit;
    Int64 _paramCount;
    MYSQL_BIND *_bindParams;        // 绑定的参数数组
    Int32 _curBindParamIndex;       // 当前可绑定的索引位置
    Field* *_paramValues;           // 缓存绑定的数据, 最后成功后释放
    UInt64 _seqId;                  // 执行的唯一id

    // 结果
    UInt32 _resultFieldCount;
    MYSQL_BIND *_resultBinds;
    bool *_resultFieldIsNulls;
    ULong *_resultFieldLengths;
    bool _hasRecords;
    bool _isSucObtainResultSetMetadata;

    // 字段与索引信息
    std::map<LibString, Int32> _fieldNameRefIndex;
    std::map<Int32, LibString> _indexRefFieldName;
    std::map<Int32, LibString> _indexRefTableName;
    std::map<Int32, bool> _indexRefIsAutoIncField;
    std::map<Int32, bool> _indexRefIsUnsigned;
};

ALWAYS_INLINE bool PrepareStmt::IsInit() const
{
    return _isInit;
}

ALWAYS_INLINE bool PrepareStmt::IsError() const
{
    return _isError;   
}

KERNEL_END

#endif
