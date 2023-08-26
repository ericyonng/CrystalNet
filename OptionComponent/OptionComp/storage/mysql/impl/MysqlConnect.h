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
 * 6.MysqlConnect 不建议一次性批量执行多条语句, 因为有可能会因为网络问题导致从中间断开失败, 导致数据不一致, 所以默认是不打开批量执行
 * 7.mysql connect 提交一个sql执行一定要保证其操作的原子性, 尤其是开启事务, 必须要批量发送过去, 做到要么全部发送, 要么不发送, 因为mysql连接是有可能会断开的
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
#include <OptionComp/storage/mysql/impl/MysqlConfig.h>

struct MYSQL;
struct MYSQL_RES;

KERNEL_BEGIN

class Record;
class PrepareStmt;
class Field;

class MysqlOperateType
{
public:
    enum ENUMS
    {
        Operate = 0,  // 查询等读数据操作
        CompleteQuery,  // 完成一次sql操作
    };
};

class MysqlOperateInfo
{
public:
    Int32 _type = 0;    // MysqlOperateType
    Int64 _count = 0;   // 操作次数
};

// 建议一个线程一个Connect
class MysqlConnect
{
    friend class PrepareStmt;

    POOL_CREATE_OBJ_DEFAULT(MysqlConnect);

public:
    // 空值(用于不需要回调时候匹配不同的Exe/UseTransAction函数)
    static constexpr IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *DELG_NULL = NULL;
    static constexpr void (*FUNC_NULL)(MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &)  = NULL;

    // 事务的
    static constexpr IDelegate<void,MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *TA_DELG_NULL = NULL;
    static constexpr void (*TA_FUNC_NULL)(MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &)  = NULL;

public:
    MysqlConnect(UInt64 id);
    ~MysqlConnect();

    void SetConfig(const MysqlConfig &cfg);
    const MysqlConfig &GetConfig() const;

    Int32 Init();
    Int32 Start();
    
    void Close();

    void OnMysqlDisconnect();

    // stmt执行sql结果: MysqlConnect*, UInt64(seqId), Int32(runErrCode), UInt32(mysqlerr), bool(是否发送到mysqlserver), Int64(InsertId), Int64(AffectedRows), std::vector<SmartPtr<Record>>(数据),
    bool ExecuteSqlUsingStmt(const SqlBuilder &builder, UInt64 seqId, const std::vector<Field *> &fields, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb = NULL);
    template<typename CallbackType>
    bool ExecuteSqlUsingStmt(const SqlBuilder &builder, UInt64 seqId, const std::vector<Field *> &fields, CallbackType &&cb);

    // 执行普通sql
    bool ExecuteSql(const SqlBuilder &builder, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb = NULL);
    template<typename CallbackType>
    bool ExecuteSql(const SqlBuilder &builder, UInt64 seqId, CallbackType &&cb);
    
    // 开启事务执行sql
    bool ExecuteSqlUsingTransAction(const SqlBuilder &builder, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb = NULL);
    bool ExecuteSqlUsingTransAction(const std::vector<SqlBuilder *> &builder, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb = NULL);
    bool ExecuteSqlUsingTransAction(const LibString &sql, Int32 sqlCount, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb = NULL);
    template<typename CallbackType>
    bool ExecuteSqlUsingTransAction(const SqlBuilder &builder, UInt64 seqId, CallbackType &&cb);
    template<typename CallbackType>
    bool ExecuteSqlUsingTransAction(const std::vector<SqlBuilder *> &builder, UInt64 seqId, CallbackType &&cb);
    
    // 返回多值属于结构化绑定,至少需要C++17
    // std::tuple<Int32, LibString> TestMulti();

    // 执行ping判断连接是否在
    bool Ping(const LibString &content);

    // 获取上次新增一条数据操作产生的id(这个id是在标识了自增属性的字段, 一张表只能有一个自增id字段), 如果上次是其他非造成新增数据的sql则获取的结果是0
    // 如果表没有自增字段则返回0,开启事务时需要在最后一条Insert的时候获取InsertId的结果不然Commit等操作会影响InsertId结果
    Int64 GetLastInsertIdOfAutoIncField() const;

    // 最后一次sql后AffectedRow
    // 获取上次执行sql影响的行数(由执行完sql后自动返回的,有些sql不返回, 不返回的则此时是-1), 没有影响返回0
    Int64 GetLastAffectedRow() const;

    // 判断执行完sql有没有结果
    bool HasNextResult() const;

    // 获取当前结果取到本地缓存中的总行数（对于UserResult来说是一行一行取, 每次FetchRow时行数会增加）
    Int64 GetCurrentResultRows(MYSQL_RES *res) const;

    LibString ToString() const;

    LibString GetCarefulOptionsInfo() const;

    const std::unordered_map<Int32, MysqlOperateInfo> &GetOperationInfos() const;
    void ClearOperationInfos();

    // 上次ping的毫秒数
    UInt64 GetLastPingMs() const;

    // 实时获取mysql错误
    UInt32 GetMysqlErrno() const;
    LibString GetMysqlError() const;

    // 获取最近一次出错的错误码
    UInt32 GetLastMysqlErrno() const;
    const LibString &GetLastMysqlErrString() const;

    const MYSQL *GetMysql() const;
    MYSQL *GetMysql();

    void AddOpCount(Int32 type, Int64 count = 1) const;

private:
    MYSQL_RES *_StoreResult(bool &isSqlWithFieldsCountReturn) const;
    MYSQL_RES *_UseResult(bool &isSqlWithFieldsCountReturn) const;
    void _FreeRes(MYSQL_RES *res) const;

    void _FetchRows(MYSQL_RES *res, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb);

    bool _ExcuteSql(UInt64 seqId, const LibString &sql) const;

    bool _ExecuteSqlUsingStmt(const LibString &sql, UInt64 seqId, const std::vector<Field *> &fields, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb);
    bool _ExcuteSql(const LibString &sql, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb);
    bool _ExcuteSqlUsingTransAction(const LibString &sqls, Int32 sqlCount, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb);

    bool _Connect();
    bool _SelectDB();
    // bool _CreateTable();
    // bool _DropTable();
    bool _Ping(const LibString &content = "");


    UInt32 _UpdateLastMysqlErrno() const;

    // stmt
    PrepareStmt *_CreateStmt(const LibString &sql);
    PrepareStmt *_GetStmt(const LibString &sql);

private:
    UInt64 _id;
    MysqlConfig _cfg;
    MYSQL *_mysql;
    bool _isConnected;
    mutable std::unordered_map<Int32, MysqlOperateInfo> _typeRefOpInfo;
    UInt64 _lastPingMs;
    mutable UInt32 _lastErrno;
    mutable LibString _lastErrString;

    // 预处理的sql
    UInt64 _stmtId;
    std::map<UInt64, PrepareStmt *> _stmtIdRefPrepareStmt;
    std::map<LibString, PrepareStmt *> _sqlRefPrepareStmt;
};

ALWAYS_INLINE void MysqlConnect::SetConfig(const MysqlConfig &cfg)
{
    _cfg = cfg;
}

ALWAYS_INLINE const MysqlConfig &MysqlConnect::GetConfig() const
{
    return _cfg;
}

ALWAYS_INLINE bool MysqlConnect::ExecuteSqlUsingStmt(const SqlBuilder &builder, UInt64 seqId, const std::vector<Field *> &fields, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb)
{
    return _ExecuteSqlUsingStmt(builder.ToSql(), seqId, fields, cb);
}

template<typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::ExecuteSqlUsingStmt(const SqlBuilder &builder, UInt64 seqId, const std::vector<Field *> &fields, CallbackType &&cb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &);
    auto ret = _ExecuteSqlUsingStmt(builder.ToSql(), seqId, fields, delg);
    delg->Release();
    return ret;
}

ALWAYS_INLINE bool MysqlConnect::ExecuteSql(const SqlBuilder &builder, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb)
{
    return _ExcuteSql(builder.ToSql(), seqId, cb);
}

template<typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::ExecuteSql(const SqlBuilder &builder, UInt64 seqId, CallbackType &&cb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &);
    auto ret = _ExcuteSql(builder.ToSql(), seqId, delg);
    delg->Release();
    return ret;
}

ALWAYS_INLINE bool MysqlConnect::ExecuteSqlUsingTransAction(const SqlBuilder &builder, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb)
{
    return _ExcuteSqlUsingTransAction(builder.ToSql(), 1, seqId, cb);
}

ALWAYS_INLINE bool MysqlConnect::ExecuteSqlUsingTransAction(const std::vector<SqlBuilder *> &builder, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb)
{
    std::vector<LibString> sqls;
    for(auto &b : builder)
        sqls.push_back(b->ToSql());

    return _ExcuteSqlUsingTransAction(StringUtil::ToString(sqls, ";"), static_cast<Int32>(builder.size()), seqId, cb);
}

ALWAYS_INLINE bool MysqlConnect::ExecuteSqlUsingTransAction(const LibString &sql, Int32 sqlCount, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb)
{
    return _ExcuteSqlUsingTransAction(sql, sqlCount, seqId, cb);
}

template<typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::ExecuteSqlUsingTransAction(const SqlBuilder &builder, UInt64 seqId, CallbackType &&cb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &);
    auto ret = _ExcuteSqlUsingTransAction(builder.ToSql(), 1, seqId, delg);
    delg->Release();

    return ret;
}

template<typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::ExecuteSqlUsingTransAction(const std::vector<SqlBuilder *> &builder, UInt64 seqId, CallbackType &&cb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &);
    std::vector<LibString> sqls;
    for(auto &b : builder)
        sqls.push_back(b->ToSql());

    auto ret = _ExcuteSqlUsingTransAction(StringUtil::ToString(sqls, ";"), static_cast<Int32>(builder.size()), seqId, delg);
    delg->Release();

    return ret;
}

ALWAYS_INLINE bool MysqlConnect::Ping(const LibString &content)
{
    return _Ping(content);
}

ALWAYS_INLINE const std::unordered_map<Int32, MysqlOperateInfo> &MysqlConnect::GetOperationInfos() const
{
    return _typeRefOpInfo;
}

ALWAYS_INLINE void MysqlConnect::ClearOperationInfos()
{
    _typeRefOpInfo.clear();
}

ALWAYS_INLINE UInt64 MysqlConnect::GetLastPingMs() const
{
    return _lastPingMs;
}

ALWAYS_INLINE UInt32 MysqlConnect::GetLastMysqlErrno() const
{
    return _lastErrno;
}

ALWAYS_INLINE const LibString &MysqlConnect::GetLastMysqlErrString() const
{
    return _lastErrString;
}

ALWAYS_INLINE const MYSQL *MysqlConnect::GetMysql() const
{
    return _mysql;
}

ALWAYS_INLINE MYSQL *MysqlConnect::GetMysql()
{
    return _mysql;
}

ALWAYS_INLINE bool MysqlConnect::_ExcuteSql(const LibString &sql, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb)
{
    static std::vector<KERNEL_NS::SmartPtr<KERNEL_NS::Record, KERNEL_NS::AutoDelMethods::CustomDelete>> s_empty;

    auto exeRet = _ExcuteSql(seqId, sql);
    if(!exeRet)
    {
        if(LIKELY(cb))
            cb->Invoke(this, seqId, Status::Failed, _lastErrno, false, 0, 0, s_empty);

        return false;
    }

    do
    {
        bool hasFieldsCount = false;
        auto res = _StoreResult(hasFieldsCount);

        // 可能有数据回来
        if(hasFieldsCount)
        {
            if(res)
                _FetchRows(res, seqId, cb);
            else
            {// 有字段信息但是没有数据结果
                auto ret = _UpdateLastMysqlErrno();
                if(LIKELY(cb))
                    cb->Invoke(this, seqId, ret == 0 ? Status::Success : Status::Failed, ret, true, 0, 0, s_empty);
            }
        }
        else
        {// 没有数据可回来, 那么回传InsertId, 和AffectedRow
            auto lastInsertId = GetLastInsertIdOfAutoIncField();
            auto lastAffectedRows = GetLastAffectedRow();
            auto ret = _UpdateLastMysqlErrno();

            if(LIKELY(cb))
                cb->Invoke(this, seqId, ret == 0 ? Status::Success : Status::Failed, ret, true, lastInsertId, lastAffectedRows, s_empty);
        }

        if(LIKELY(res))
            _FreeRes(res);
    } while (HasNextResult());

    AddOpCount(MysqlOperateType::CompleteQuery);

    return true;
}

ALWAYS_INLINE bool MysqlConnect::_ExcuteSqlUsingTransAction(const LibString &sqls, Int32 sqlCount, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb)
{
    if(UNLIKELY(sqls.empty()))
        return false;

    static std::vector<KERNEL_NS::SmartPtr<KERNEL_NS::Record, KERNEL_NS::AutoDelMethods::CustomDelete>> s_empty;

    // 必须开启事务
    if(UNLIKELY(!_cfg._enableMultiStatements))
    {
        if(LIKELY(cb))
            cb->Invoke(this, seqId, Status::Failed, 0, false, 0, 0, s_empty);

        return false;
    }

    // 开启事务（网络断开会自动rollback）
    {
        std::vector<KERNEL_NS::LibString> sqlsParts;
        sqlsParts.push_back(StartTransActionSqlBuilder().ToSql());
        sqlsParts.push_back(SetAutoCommitSqlBuilder().SetAutoCommit(false).ToSql());

        // 发送到mysql执行
        if(!_ExcuteSql(seqId, StringUtil::ToString(sqlsParts, ";")))
        {
            if(LIKELY(cb))
                cb->Invoke(this, seqId, Status::Failed, _lastErrno, false, 0, 0, s_empty);

            return false;
        }

        do
        {
            bool hasFieldsCount = false;
            auto res = _StoreResult(hasFieldsCount);

            if(LIKELY(res))
                _FreeRes(res);
        } while (HasNextResult());

        AddOpCount(MysqlOperateType::CompleteQuery);
    }

    bool isFailed = false;
    {
        // 执行sql

        // 发送到mysql执行
        if(!_ExcuteSql(seqId, sqls))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("excute sqls fail seq id:%llu"), seqId);

            if(LIKELY(cb))
                cb->Invoke(this, seqId, Status::Failed, _lastErrno, false, 0, 0, s_empty);

            isFailed = true;
        }
        else
        {
            // 获取结果集
            do
            {
                bool hasFieldsCount = false;
                auto res = _StoreResult(hasFieldsCount);

                // 只有在 resultStart 和 resultEnd之间才是sql执行的结果
                if(hasFieldsCount)
                {// 查询
                    auto ret = _UpdateLastMysqlErrno();
                    if(UNLIKELY(ret != 0))
                        isFailed = true;

                    if(res)
                        _FetchRows(res, seqId, cb);
                    else
                    {// 有字段信息但是没有数据结果
                        if(LIKELY(cb))
                            cb->Invoke(this, seqId, ret == 0 ? Status::Success : Status::Failed, ret, true, 0, 0, s_empty);
                    }
                }
                else
                {// 其他物结果的
                    auto lastInsertId = GetLastInsertIdOfAutoIncField();
                    auto lastAffectedRows = GetLastAffectedRow();
                    auto ret = _UpdateLastMysqlErrno();
                    if(UNLIKELY(ret != 0))
                        isFailed = true;

                    if(LIKELY(cb))
                        cb->Invoke(this, seqId, ret == 0 ? Status::Success : Status::Failed, ret, true, lastInsertId, lastAffectedRows, s_empty);
                } 

                if(LIKELY(res))
                    _FreeRes(res);

            } while (HasNextResult());

            AddOpCount(MysqlOperateType::CompleteQuery, sqlCount);
        }

        if(UNLIKELY(isFailed))
        {
            // rollback
            std::vector<KERNEL_NS::LibString> sqlsParts;
            sqlsParts.push_back(RollbackSqlBuilder().ToSql());
            if(!_ExcuteSql(seqId, StringUtil::ToString(sqlsParts, ";")))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("rollback fail seq id:%llu"), seqId);
                return false;
            }

            do
            {
                bool hasFieldsCount = false;
                auto res = _StoreResult(hasFieldsCount);

                if(LIKELY(res))
                    _FreeRes(res);
            } while (HasNextResult());

            AddOpCount(MysqlOperateType::CompleteQuery);
        }
    }

    // 提交事务
    {
        std::vector<KERNEL_NS::LibString> sqlsParts;
        sqlsParts.push_back(CommitTransActionSqlBuilder().ToSql());
        sqlsParts.push_back(SetAutoCommitSqlBuilder().SetAutoCommit(true).ToSql());

        // 发送到mysql执行
        if(!_ExcuteSql(seqId, StringUtil::ToString(sqlsParts, ";")))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("commit sqls fail seq id:%llu"), seqId);
            if(LIKELY(cb))
                cb->Invoke(this, seqId, Status::Failed, _lastErrno, false, 0, 0, s_empty);

            return false;
        }

        do
        {
            bool hasFieldsCount = false;
            auto res = _StoreResult(hasFieldsCount);

            if(LIKELY(res))
                _FreeRes(res);
        } while (HasNextResult());

        AddOpCount(MysqlOperateType::CompleteQuery);
    }

    return !isFailed;
}

KERNEL_END

#endif
