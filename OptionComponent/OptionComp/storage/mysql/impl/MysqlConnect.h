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
 * Date: 2023-06-09 11:16:00
 * Author: Eric Yonng
 * Description: mysql连接实例
 * 1.如果需要指定本地ip见配置
 * 2.MysqlConnect一开始连上去的时候是不指定数据库的, 在连上去后会根据配置切换到指定数据库，如果数据库不存在会自动创建
 * 3.启用断线重连后执行sql如果远端mysql断开, 则重连后不需要重新配置mysql选项，也不用重新切换到配置指定的数据库，直接执行sql即可
 * 4.MysqlConnect只操作配置指定的数据库期间不支持切换到其他数据库，因为这样会引入混乱, 如果要操作多个数据库应该从设计上着手创建多个MysqlConnect即可满足要求
 * 5.一张表只能有一个自增id字段
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_MYSQL_CONNECT_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_MYSQL_CONNECT_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/memory/memory.h>
#include <OptionComp/storage/mysql/impl/SqlBuilder.h>
#include <kernel/comp/Delegate/Delegate.h>

struct MYSQL;
struct MYSQL_RES;

KERNEL_BEGIN

class LibSocket;
class Record;

class MysqlOperateType
{
public:
    enum ENUMS
    {
        READ = 0,  // 查询等读数据操作
        WRITE = 1, // 写数据操作
    };
};

class MysqlOperateInfo
{
public:
    Int32 _type = 0;    // MysqlOperateType
    Int64 _count = 0;   // 操作次数
};

struct MysqlConfig
{
    POOL_CREATE_OBJ_DEFAULT(MysqlConfig);

    MysqlConfig();
    ~MysqlConfig(){}

    LibString ToString() const;

    // 连接参数
    LibString _host;        // 远端mysql ip
    LibString _user;        // 用户名
    LibString _pwd;         // 密码
    LibString _dbName;      // 数据库名
    UInt16 _port;           // 远端mysql的端口
    LibString _bindIp;      // 本地多张网卡可以绑定在某张网卡上

    // 可选配置
    LibString _charset;     // mysql操作时的编码字符集
    LibString _dbCharset;   // db库的字符集
    LibString _dbCollate;   // db库的字符集
    Int32 _autoReconnect;   // 自动重连
    UInt64 _maxPacketSize;  // mysql 单包缓冲区大小(涉及到从mysql回来的接收缓冲区大小) 最大1GB
    // bool _isOpenTableInfo;  // 开启表信息显示 一定是开启的
    bool _enableMultiStatements;    // 支持一次执行多条sql
};

// 建议一个线程一个Connect
class MysqlConnect
{
    POOL_CREATE_OBJ_DEFAULT(MysqlConnect);

public:
    // 空值(用于不需要回调时候匹配不同的Exe/UseTransAction函数)
    static constexpr IDelegate<void, MysqlConnect *, bool, MYSQL_RES *, bool> *DELG_NULL = NULL;
    static constexpr void (*FUNC_NULL)(MysqlConnect *, bool, MYSQL_RES *, bool)  = NULL;

    // 事务的
    static constexpr IDelegate<void, MysqlConnect *, bool, MYSQL_RES *, bool, bool &> *TA_DELG_NULL = NULL;
    static constexpr void (*TA_FUNC_NULL)(MysqlConnect *, bool, MYSQL_RES *, bool, bool &)  = NULL;

public:
    MysqlConnect(UInt64 id);
    ~MysqlConnect();

    void SetConfig(const MysqlConfig &cfg);
    const MysqlConfig &GetConfig() const;

    Int32 Init();
    Int32 Start();
    
    void Close();

    // 注意builder不能是开启事务的, 开启事务由MySQLConnect加
    // ExcuteSql时需要手动传入需不需要开启事务, 这样才可以准确的获取到AffectedRows和InsertId
    // CallbackType:params:MysqlConnect *, bool, MYSQL_RES *res, bool, 一个res调用一次cb, 第一个bool:sql语句是否成功发送到MysqlServer, 第二个bool表示Sql全部执行完成与否
    template<SqlBuilderType::ENUMS T, typename CallbackType>
    bool ExcuteSql(const SqlBuilder<T> &builder, CallbackType &&cb, bool doPing = true);
    template<typename CallbackType>
    bool ExcuteSql(const LibString &sql, CallbackType &&cb, bool doPing = true);
    template<SqlBuilderType::ENUMS T>
    bool ExcuteSql(const SqlBuilder<T> &builder, IDelegate<void, MysqlConnect *, bool, MYSQL_RES *, bool> *cb, bool doPing = true);
    bool ExcuteSql(const LibString &sql, IDelegate<void, MysqlConnect *, bool, MYSQL_RES *, bool> *cb, bool doPing = true);
    template<SqlBuilderType::ENUMS T>
    bool ExcuteSql(const SqlBuilder<T> &builder, void (*cb)(MysqlConnect *, bool, MYSQL_RES *, bool), bool doPing = true);
    bool ExcuteSql(const LibString &sql, void (*cb)(MysqlConnect *, bool, MYSQL_RES *, bool), bool doPing = true);
    template<SqlBuilderType::ENUMS T, typename ObjType>
    bool ExcuteSql(const SqlBuilder<T> &builder, ObjType *obj, void (ObjType::*cb)(MysqlConnect *, bool, MYSQL_RES *, bool), bool doPing = true);
    template<typename ObjType>
    bool ExcuteSql(const LibString &sql, ObjType *obj, void (ObjType::*cb)(MysqlConnect *, bool, MYSQL_RES *, bool), bool doPing = true);

    // 开启事务的方式执行sql
    // CallbackType:params:MysqlConnect *, bool, MYSQL_RES *res, bool, bool &, 一个res调用一次cb, 第一个bool:sql语句是否成功发送到MysqlServer, 第二个bool表示Sql全部执行完成与否, bool &:需不需要回滚
    template<SqlBuilderType::ENUMS T, typename CallbackType>
    bool UseTransActionExcuteSql(const SqlBuilder<T> &builder, CallbackType &&cb, bool doPing = true);
    template<typename CallbackType>
    bool UseTransActionExcuteSql(const LibString &sql, CallbackType &&cb, bool doPing = true);
    template<SqlBuilderType::ENUMS T>
    bool UseTransActionExcuteSql(const SqlBuilder<T> &builder, IDelegate<void, MysqlConnect *, bool, MYSQL_RES *, bool, bool &> *cb, bool doPing = true);
    bool UseTransActionExcuteSql(const LibString &sql, IDelegate<void, MysqlConnect *, bool, MYSQL_RES *, bool, bool &> *cb, bool doPing = true);
    template<SqlBuilderType::ENUMS T>
    bool UseTransActionExcuteSql(const SqlBuilder<T> &builder, void (*cb)(MysqlConnect *, bool, MYSQL_RES *, bool, bool &), bool doPing = true);
    bool UseTransActionExcuteSql(const LibString &sql, void (*cb)(MysqlConnect *, bool, MYSQL_RES *, bool, bool &), bool doPing = true);
    template<SqlBuilderType::ENUMS T, typename ObjType>
    bool UseTransActionExcuteSql(const SqlBuilder<T> &builder, ObjType *obj, void (ObjType::*cb)(MysqlConnect *, bool, MYSQL_RES *, bool, bool &), bool doPing = true);
    template<typename ObjType>
    bool UseTransActionExcuteSql(const LibString &sql, ObjType *obj, void (ObjType::*cb)(MysqlConnect *, bool, MYSQL_RES *, bool, bool &), bool doPing = true);

    // 返回多值属于结构化绑定,至少需要C++17
    // std::tuple<Int32, LibString> TestMulti();

    // 执行ping判断连接是否在
    bool Ping();

    // 获取上次新增一条数据操作产生的id(这个id是在标识了自增属性的字段, 一张表只能有一个自增id字段), 如果上次是其他非造成新增数据的sql则获取的结果是0
    // 如果表没有自增字段则返回0,开启事务时需要在最后一条Insert的时候获取InsertId的结果不然Commit等操作会影响InsertId结果
    Int64 GetLastInsertIdOfAutoIncField() const;

    // 最后一次sql后AffectedRow
    // 获取上次执行sql影响的行数(由执行完sql后自动返回的,有些sql不返回, 不返回的则此时是-1), 没有影响返回0
    Int64 GetLastAffectedRow() const;

    // 直接将结果集拿回本地再逐行的取数据 lambda[](KERNEL_NS::MysqlConnect *conn, MYSQL_RES *) ->void
    template<typename CallbackType>
    bool StoreResult(CallbackType &&cb);

    // 有可能不会拿到所有数据(尤其在网络不稳的情况下一部分一部分的取数据会取不全数据)
    // 需要fetch_row一次一次的从远程取会结果(需要注意中间可能会断开连接) 需要设置本地缓存为1K然后取多条数据到本地测试断线重连情况 lambda[](KERNEL_NS::MysqlConnect *conn, MYSQL_RES *) ->void
    template<typename CallbackType>
    bool UseResult(CallbackType &&cb);

    // 判断执行完sql有没有结果
    bool HasNextResult() const;

    // 逐行取数据(逐行回调)lambda[](MysqlConnect *, bool, SmartPtr<Record, AutoMethod::CustomDelete> &)->void bool:有没有数据, record:数据 FetchRow 
    // 需要配合 UseResult或者StoreResult使用, Fetch不负责释放res
    template<typename CallbackType>
    void FetchRow(MYSQL_RES *res, CallbackType &&cb);
    // 逐行取数据(回调时候是一次性多行)lambda[](MysqlConnect *, bool, bool, std::vector<SmartPtr<Record,  AutoMethod::CustomDelete>> &)->void bool:有没有数据
    // 需要配合 UseResult或者StoreResult使用, Fetch不负责释放res
    template<typename CallbackType>
    void FetchRows(MYSQL_RES *res, CallbackType &&cb);

    // 获取当前结果取到本地缓存中的总行数（对于UserResult来说是一行一行取, 每次FetchRow时行数会增加）
    Int64 GetCurrentResultRows(MYSQL_RES *res) const;

    LibString ToString() const;

    LibString GetCarefulOptionsInfo() const;

    const std::unordered_map<Int32, MysqlOperateInfo> &GetOperationInfos() const;

    // 上次ping的毫秒数
    UInt64 GetLastPingMs() const;

    // 获取mysql错误
    UInt32 GetMysqlErrno() const;
    LibString GetMysqlError() const;

private:
    MYSQL_RES *_StoreResult() const;
    MYSQL_RES *_UseResult() const;
    void _FreeRes(MYSQL_RES *res) const;

    // cb:返回值, connect对象, 是否有数据, 当前行数据, 第一个bool:执行是否成功, 第二个bool:有没有数据
    void _FetchRow(MYSQL_RES *res, IDelegate<void, MysqlConnect *, bool, SmartPtr<Record, AutoDelMethods::CustomDelete> &> *cb);
    void _FetchRows(MYSQL_RES *res, IDelegate<void, MysqlConnect *, bool, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb);

    bool _ExcuteSql(const LibString &sql) const;

    bool _Connect();
    bool _SelectDB();
    // bool _CreateTable();
    // bool _DropTable();
    bool _Ping(const LibString &content = "");

    void _AddOpCount(Int32 type, Int64 count = 1);

private:
    UInt64 _id;
    MysqlConfig _cfg;
    MYSQL *_mysql;
    bool _isConnected;
    std::unordered_map<Int32, MysqlOperateInfo> _typeRefOpInfo;
    UInt64 _lastPingMs;
};

ALWAYS_INLINE void MysqlConnect::SetConfig(const MysqlConfig &cfg)
{
    _cfg = cfg;
}

ALWAYS_INLINE const MysqlConfig &MysqlConnect::GetConfig() const
{
    return _cfg;
}

template<SqlBuilderType::ENUMS T, typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::ExcuteSql(const SqlBuilder<T> &builder, CallbackType &&cb, bool doPing)
{
    const auto &sql = builder.ToSql();
    return ExcuteSql(builder.ToSql(), cb, doPing);
}

template<typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::ExcuteSql(const LibString &sql, CallbackType &&cb, bool doPing)
{
    if(UNLIKELY(doPing))
        for(;!_Ping(sql););

    auto exeRet = _ExcuteSql(sql);
    if(!exeRet)
    {
        cb(this, false, NULL, true);
        return false;
    }

    do
    {
        bool hasError = false;
        auto res = _StoreResult();
        cb(this, true, res, false);

        if(LIKELY(res))
            _FreeRes(res);
    } while (HasNextResult());

    cb(this, true, NULL, true);

    return true;
}

template<SqlBuilderType::ENUMS T>
ALWAYS_INLINE bool MysqlConnect::ExcuteSql(const SqlBuilder<T> &builder, IDelegate<void, MysqlConnect *, bool, MYSQL_RES *, bool> *cb, bool doPing)
{
    return ExcuteSql(builder.ToSql(), cb, doPing);
}

ALWAYS_INLINE bool MysqlConnect::ExcuteSql(const LibString &sql, IDelegate<void, MysqlConnect *, bool, MYSQL_RES *, bool> *cb, bool doPing)
{
    if(UNLIKELY(doPing))
        for(;!_Ping(sql););

    auto exeRet = _ExcuteSql(sql);
    if(!exeRet)
    {
        if(LIKELY(cb))
        {
            cb->Invoke(this, false, NULL, true);
            cb->Release();
        }

        return false;
    }

    do
    {
        auto res = _StoreResult();
        if(LIKELY(cb))
            cb->Invoke(this, true, res, false);

        if(LIKELY(res))
            _FreeRes(res);
    } while (HasNextResult());

    if(LIKELY(cb))
    {
        cb->Invoke(this, true, NULL, true);
        cb->Release();
    }
    
    return true;
}

template<SqlBuilderType::ENUMS T>
ALWAYS_INLINE bool MysqlConnect::ExcuteSql(const SqlBuilder<T> &builder, void (*cb)(MysqlConnect *, bool, MYSQL_RES *, bool), bool doPing)
{
    return ExcuteSql(builder.ToSql(), cb, doPing);
}

ALWAYS_INLINE bool MysqlConnect::ExcuteSql(const LibString &sql, void (*cb)(MysqlConnect *, bool, MYSQL_RES *, bool), bool doPing)
{
    if(UNLIKELY(doPing))
        for(;!_Ping(sql););

    auto exeRet = _ExcuteSql(sql);
    if(!exeRet)
    {
        if(LIKELY(cb))
            (*cb)(this, false, NULL, true);

        return false;
    }

    do
    {
        auto res = _StoreResult();
        if(LIKELY(cb))
            (*cb)(this, true, res, false);

        if(LIKELY(res))
            _FreeRes(res);
    } while (HasNextResult());

    if(LIKELY(cb))
        (*cb)(this, true, NULL, true);

    return true;
}

template<SqlBuilderType::ENUMS T, typename ObjType>
ALWAYS_INLINE bool MysqlConnect::ExcuteSql(const SqlBuilder<T> &builder, ObjType *obj, void (ObjType::*cb)(MysqlConnect *, bool, MYSQL_RES *, bool), bool doPing)
{
    return ExcuteSql(builder.ToSql(), obj, cb, doPing);
}

template<typename ObjType>
ALWAYS_INLINE bool MysqlConnect::ExcuteSql(const LibString &sql, ObjType *obj, void (ObjType::*cb)(MysqlConnect *, bool, MYSQL_RES *, bool), bool doPing)
{
    if(UNLIKELY(doPing))
        for(;!_Ping(sql););

    auto exeRet = _ExcuteSql(sql);
    if(!exeRet)
    {
        if(LIKELY(obj && cb))
            (obj->*cb)(this, false, NULL, true);

        return false;
    }

    do
    {
        auto res = _StoreResult();
        if(LIKELY(obj && cb))
            (obj->*cb)(this, true, res, false);

        if(LIKELY(res))
            _FreeRes(res);
    } while (HasNextResult());

    if(LIKELY(obj && cb))
        (obj->*cb)(this, true, NULL, true);

    return true;
}

template<SqlBuilderType::ENUMS T, typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::UseTransActionExcuteSql(const SqlBuilder<T> &builder, CallbackType &&cb, bool doPing)
{
    return UseTransActionExcuteSql(builder.ToSql(), cb, doPing);
}

template<typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::UseTransActionExcuteSql(const LibString &sql, CallbackType &&cb, bool doPing)
{
    if(UNLIKELY(doPing))
        for(;!_Ping(sql););

    {// 开启事务与关闭自动提交
        std::vector<KERNEL_NS::LibString> sqls;
        sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::START_TRANSACTION>().ToSql());
        sqls.push_back(KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SET_AUTOCOMMIT>().SetAutoCommit(false).ToSql());
        auto exeRet = _ExcuteSql(StringUtil::ToString(sqls, ";"));
        if(!exeRet)
        {
            bool v = false;
            // (bool &)保证签名匹配
            cb(this, false, NULL, true, v);
            return false;
        }

        // 获取结果集
        do
        {
            auto res = _StoreResult();
            if(LIKELY(res))
                _FreeRes(res);
        } while (HasNextResult());
    }

    // 执行真正的sql
    auto exeRet = _ExcuteSql(sql);
    if(!exeRet)
    {
        bool v = false;
        cb(this, false, NULL, true, v);

        // 提交事务
        std::vector<KERNEL_NS::LibString> sqls;
        sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::COMMIT_TRANSACTION>().ToSql());
        sqls.push_back(KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SET_AUTOCOMMIT>().SetAutoCommit(true).ToSql());
        if(_ExcuteSql(StringUtil::ToString(sqls, ";")))
        {
            // 获取结果集
            do
            {
                auto res = _StoreResult();
                if(LIKELY(res))
                    _FreeRes(res);
            } while (HasNextResult());
        }

        return false;
    }

    // 获取结果集
    bool needRollback = false;
    do
    {
        auto res = _StoreResult();
        cb(this, true, res, false, needRollback);

        if(LIKELY(res))
            _FreeRes(res);
    } while (HasNextResult());

    // 完成后再调用
    cb(this, true, NULL, true, needRollback);

    // 处理回滚
    std::vector<KERNEL_NS::LibString> sqls;
    if(UNLIKELY(needRollback))
        sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::ROLLBACK>().ToSql());

    // 提交事务
    sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::COMMIT_TRANSACTION>().ToSql());
    sqls.push_back(KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SET_AUTOCOMMIT>().SetAutoCommit(true).ToSql());
    if(_ExcuteSql(StringUtil::ToString(sqls, ";")))
    {
        // 获取结果集
        do
        {
            auto res = _StoreResult();
            if(LIKELY(res))
                _FreeRes(res);
        } while (HasNextResult());
    }

    return true;
}

template<SqlBuilderType::ENUMS T>
ALWAYS_INLINE bool MysqlConnect::UseTransActionExcuteSql(const SqlBuilder<T> &builder, IDelegate<void, MysqlConnect *, bool, MYSQL_RES *, bool, bool &> *cb, bool doPing)
{
    return UseTransActionExcuteSql(builder.ToSql(), cb, doPing);
}

ALWAYS_INLINE bool MysqlConnect::UseTransActionExcuteSql(const LibString &sql, IDelegate<void, MysqlConnect *, bool, MYSQL_RES *, bool, bool &> *cb, bool doPing)
{
   if(UNLIKELY(doPing))
        for(;!_Ping(sql););

    {// 开启事务与关闭自动提交
        std::vector<KERNEL_NS::LibString> sqls;
        sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::START_TRANSACTION>().ToSql());
        sqls.push_back(KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SET_AUTOCOMMIT>().SetAutoCommit(false).ToSql());
        auto exeRet = _ExcuteSql(StringUtil::ToString(sqls, ";"));
        if(!exeRet)
        {
            if(LIKELY(cb))
            {
                bool v = false;
                cb->Invoke(this, false, NULL, true, v);
                cb->Release();
            }

            return false;
        }

        // 获取结果集
        do
        {
            auto res = _StoreResult();
            if(LIKELY(res))
                _FreeRes(res);
        } while (HasNextResult());
    }

    // 执行真正的sql
    auto exeRet = _ExcuteSql(sql);
    if(!exeRet)
    {
        if(LIKELY(cb))
        {
            bool v = false;
            cb->Invoke(this, false, NULL, true, v);
            cb->Release();
        }

        // 提交事务
        std::vector<KERNEL_NS::LibString> sqls;
        sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::COMMIT_TRANSACTION>().ToSql());
        sqls.push_back(KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SET_AUTOCOMMIT>().SetAutoCommit(true).ToSql());
        if(_ExcuteSql(StringUtil::ToString(sqls, ";")))
        {
            // 获取结果集
            do
            {
                auto res = _StoreResult();
                if(LIKELY(res))
                    _FreeRes(res);
            } while (HasNextResult());
        }

        return false;
    }

    // 获取结果集
    bool needRollback = false;
    do
    {
        auto res = _StoreResult();
        if(LIKELY(cb))
            cb->Invoke(this, true, res, false, needRollback);

        if(LIKELY(res))
            _FreeRes(res);
    } while (HasNextResult());

    if(LIKELY(cb))
    {
        cb->Invoke(this, true, NULL, true, needRollback);
        cb->Release();
    }

    // 处理回滚
    std::vector<KERNEL_NS::LibString> sqls;
    if(UNLIKELY(needRollback))
        sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::ROLLBACK>().ToSql());

    // 提交事务
    sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::COMMIT_TRANSACTION>().ToSql());
    sqls.push_back(KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SET_AUTOCOMMIT>().SetAutoCommit(true).ToSql());
    if(_ExcuteSql(StringUtil::ToString(sqls, ";")))
    {
        // 获取结果集
        do
        {
            auto res = _StoreResult();
            if(LIKELY(res))
                _FreeRes(res);
        } while (HasNextResult());
    }

    return true;
}

template<SqlBuilderType::ENUMS T>
ALWAYS_INLINE bool MysqlConnect::UseTransActionExcuteSql(const SqlBuilder<T> &builder, void (*cb)(MysqlConnect *, bool, MYSQL_RES *, bool, bool &), bool doPing)
{
    return UseTransActionExcuteSql(builder.ToSql(), cb, doPing);
}

ALWAYS_INLINE bool MysqlConnect::UseTransActionExcuteSql(const LibString &sql, void (*cb)(MysqlConnect *, bool, MYSQL_RES *, bool, bool &), bool doPing)
{
    if(UNLIKELY(doPing))
        for(;!_Ping(sql););

    {// 开启事务与关闭自动提交
        std::vector<KERNEL_NS::LibString> sqls;
        sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::START_TRANSACTION>().ToSql());
        sqls.push_back(KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SET_AUTOCOMMIT>().SetAutoCommit(false).ToSql());
        auto exeRet = _ExcuteSql(StringUtil::ToString(sqls, ";"));
        if(!exeRet)
        {
            if(LIKELY(cb))
            {
                bool v = false;
                (*cb)(this, false, NULL, true, v);
            }

            return false;
        }

        // 获取结果集
        do
        {
            auto res = _StoreResult();
            if(LIKELY(res))
                _FreeRes(res);
        } while (HasNextResult());
    }

    // 执行真正的sql
    auto exeRet = _ExcuteSql(sql);
    if(!exeRet)
    {
        if(LIKELY(cb))
        {
            bool v = false;
            (*cb)(this, false, NULL, true, v);
        }

        // 提交事务
        std::vector<KERNEL_NS::LibString> sqls;
        sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::COMMIT_TRANSACTION>().ToSql());
        sqls.push_back(KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SET_AUTOCOMMIT>().SetAutoCommit(true).ToSql());
        if(_ExcuteSql(StringUtil::ToString(sqls, ";")))
        {
            // 获取结果集
            do
            {
                auto res = _StoreResult();
                if(LIKELY(res))
                    _FreeRes(res);
            } while (HasNextResult());
        }

        return false;
    }

    // 获取结果集
    bool needRollback = false;
    do
    {
        auto res = _StoreResult();
        if(LIKELY(cb))
            (*cb)(this, true, res, false, needRollback);

        if(LIKELY(res))
            _FreeRes(res);
    } while (HasNextResult());

    if(LIKELY(cb))
        (*cb)(this, true, NULL, true, needRollback);

    // 处理回滚
    std::vector<KERNEL_NS::LibString> sqls;
    if(UNLIKELY(needRollback))
        sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::ROLLBACK>().ToSql());

    // 提交事务
    sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::COMMIT_TRANSACTION>().ToSql());
    sqls.push_back(KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SET_AUTOCOMMIT>().SetAutoCommit(true).ToSql());
    if(_ExcuteSql(StringUtil::ToString(sqls, ";")))
    {
        // 获取结果集
        do
        {
            auto res = _StoreResult();
            if(LIKELY(res))
                _FreeRes(res);
        } while (HasNextResult());
    }

    return true;
}

template<SqlBuilderType::ENUMS T, typename ObjType>
ALWAYS_INLINE bool MysqlConnect::UseTransActionExcuteSql(const SqlBuilder<T> &builder, ObjType *obj, void (ObjType::*cb)(MysqlConnect *, bool, MYSQL_RES *, bool, bool &), bool doPing)
{
    return UseTransActionExcuteSql(builder.ToSql(), obj, cb, doPing);
}

template<typename ObjType>
ALWAYS_INLINE bool MysqlConnect::UseTransActionExcuteSql(const LibString &sql, ObjType *obj, void (ObjType::*cb)(MysqlConnect *, bool, MYSQL_RES *, bool, bool &), bool doPing)
{
    if(UNLIKELY(doPing))
        for(;!_Ping(sql););

    {// 开启事务与关闭自动提交
        std::vector<KERNEL_NS::LibString> sqls;
        sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::START_TRANSACTION>().ToSql());
        sqls.push_back(KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SET_AUTOCOMMIT>().SetAutoCommit(false).ToSql());
        auto exeRet = _ExcuteSql(StringUtil::ToString(sqls, ";"));
        if(!exeRet)
        {
            if(LIKELY(obj && cb))
            {
                bool v = false;
                (obj->*cb)(this, false, NULL, true, v);
            }

            return false;
        }

        // 获取结果集
        do
        {
            auto res = _StoreResult();
            if(LIKELY(res))
                _FreeRes(res);
        } while (HasNextResult());
    }

    // 执行真正的sql
    auto exeRet = _ExcuteSql(sql);
    if(!exeRet)
    {
        if(LIKELY(obj && cb))
        {
            bool v = false;
            (obj->*cb)(this, false, NULL, true, v);
        }

        // 提交事务
        std::vector<KERNEL_NS::LibString> sqls;
        sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::COMMIT_TRANSACTION>().ToSql());
        sqls.push_back(KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SET_AUTOCOMMIT>().SetAutoCommit(true).ToSql());
        if(_ExcuteSql(StringUtil::ToString(sqls, ";")))
        {
            // 获取结果集
            do
            {
                auto res = _StoreResult();
                if(LIKELY(res))
                    _FreeRes(res);
            } while (HasNextResult());
        }

        return false;
    }

    // 获取结果集
    bool needRollback = false;
    do
    {
        auto res = _StoreResult();
        if(LIKELY(obj && cb))
            (obj->*cb)(this, true, res, false, needRollback);

        if(LIKELY(res))
            _FreeRes(res);
    } while (HasNextResult());

    if(LIKELY(obj && cb))
        (obj->*cb)(this, true, NULL, true, needRollback);

    // 处理回滚
    std::vector<KERNEL_NS::LibString> sqls;
    if(UNLIKELY(needRollback))
        sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::ROLLBACK>().ToSql());

    // 提交事务
    sqls.push_back(SqlBuilder<KERNEL_NS::SqlBuilderType::COMMIT_TRANSACTION>().ToSql());
    sqls.push_back(KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SET_AUTOCOMMIT>().SetAutoCommit(true).ToSql());
    if(_ExcuteSql(StringUtil::ToString(sqls, ";")))
    {
        // 获取结果集
        do
        {
            auto res = _StoreResult();
            if(LIKELY(res))
                _FreeRes(res);
        } while (HasNextResult());
    }

    return true;
}

ALWAYS_INLINE bool MysqlConnect::Ping()
{
    return _Ping(LibString());
}

template<typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::StoreResult(CallbackType &&cb)
{
    do
    {
        auto res = _StoreResult();
        cb(this, res);

        if(LIKELY(res))
            _FreeRes(res);
    } while (HasNextResult());
    
    return true;
}

template<typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::UseResult(CallbackType &&cb)
{
    do
    {
        auto res = _UseResult();
        cb(this, res);

        if(LIKELY(res))
            _FreeRes(res);
    } while (HasNextResult());
    
    return true;
}

template<typename CallbackType>
ALWAYS_INLINE void MysqlConnect::FetchRow(MYSQL_RES *res, CallbackType &&cb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void , MysqlConnect *, bool, SmartPtr<Record, AutoDelMethods::CustomDelete> &);
    _FetchRow(res, delg);
    delg->Release();
}

template<typename CallbackType>
ALWAYS_INLINE void MysqlConnect::FetchRows(MYSQL_RES *res, CallbackType &&cb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, MysqlConnect *, bool, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &);
    _FetchRows(res, delg);
    delg->Release();
}

ALWAYS_INLINE const std::unordered_map<Int32, MysqlOperateInfo> &MysqlConnect::GetOperationInfos() const
{
    return _typeRefOpInfo;
}

ALWAYS_INLINE UInt64 MysqlConnect::GetLastPingMs() const
{
    return _lastPingMs;
}

KERNEL_END

#endif
