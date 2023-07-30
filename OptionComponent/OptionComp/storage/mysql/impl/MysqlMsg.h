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
 * Date: 2023-07-14 13:46:00
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_MYSQL_MSG_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_MYSQL_MSG_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Delegate/Delegate.h>

KERNEL_BEGIN

class SqlBuilder;
class Field;
class Record;
class MysqlResponse;
class Variant;

class MysqlMsgType
{
public:
    enum ENUMS
    {
        Stmt = 0,           // 默认类型
        NormalSql,          // 普通sql
        SqlWithTransAction, // 开启事务sql批量
        Max,
    };

    static const Byte8 *ToString(Int32 type)
    {
        switch (type)
        {
        case Stmt: return "Stmt";
        case NormalSql: return "NormalSql";
        case SqlWithTransAction: return "SqlWithTransAction";
        default:
            break;
        }

        return "UNKNOWN MYSQL MSG TYPE";
    }
};

class MysqlRequest
{
    POOL_CREATE_OBJ_DEFAULT(MysqlRequest);

public:
    MysqlRequest();
    ~MysqlRequest();

    LibString ToString() const;

    LibString Dump() const;

    // 操作id
    Int32 _dbOperatorId;
    // 唯一序列号
    UInt64 _seqId;
    // 存根
    UInt64 _stub;
    // 请求类型 MysqlMsgType
    Int32 _msgType;
    // sql
    std::vector<SqlBuilder *> _builders;
    // 参数
    std::vector<Field *> _fields;
    // 回调
    IDelegate<void, MysqlResponse *> *_handler;
    // 是否释放handler
    bool _isDestroyHandler;
    // db name
    LibString _dbName;
    // 透传参数
    Variant *_var;
};

class MysqlResponse
{
    POOL_CREATE_OBJ_DEFAULT(MysqlResponse);

public:
    MysqlResponse();
    ~MysqlResponse();

    LibString ToString() const;

    // 操作id
    Int32 _dbOperatorId;
    // 唯一序列号
    UInt64 _seqId;
    // 存根
    UInt64 _stub;
    // 请求类型 MysqlMsgType
    Int32 _msgType;
    // 当前插入的最大insertId
    Int64 _maxInsertId;
    // 总的影响的行数
    Int64 _affectedRows;
    // 错误码
    Int32 _errCode;
    // mysql的错误码
    UInt32 _mysqlErrno;
    // 是否已发送到Mysql
    bool _isRequestSendToMysql;
    // 数据
    std::vector<SmartPtr<Record, KERNEL_NS::AutoDelMethods::CustomDelete>> _datas;
    // 回调
    IDelegate<void, MysqlResponse *> *_handler;
    // 是否释放handler
    bool _isDestroyHandler;
    // db name
    LibString _dbName;
    // 透传参数
    Variant *_var;
};

KERNEL_END

#endif
