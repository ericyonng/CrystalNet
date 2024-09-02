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
*/

#include <pch.h>
#include <OptionComp/storage/mysql/impl/MysqlConnect.h>
#include <OptionComp/storage/mysql/impl/Record.h>
#include <OptionComp/storage/mysql/impl/PrepareStmt.h>

#include <mysql.h>
#include <kernel/comp/Utils/Utils.h>
#include <kernel/comp/Cpu/cpu.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlConnect);


MysqlConnect::MysqlConnect(UInt64 id)
:_id(id)
,_mysql(NULL)
,_isConnected(false)
,_lastPingMs(0)
,_lastErrno(0)
,_stmtId(0)
{

}

MysqlConnect::~MysqlConnect()
{
    Close();
}

void MysqlConnect::Close()
{
    if(_mysql)
    {
        mysql_close(_mysql);
        _mysql = NULL;
    }

    KERNEL_NS::ContainerUtil::DelContainer(_stmtIdRefPrepareStmt, [](PrepareStmt *p){
        PrepareStmt::DeleteThreadLocal_PrepareStmt(p);
    });
    _sqlRefPrepareStmt.clear();

    g_Log->Info(LOGFMT_OBJ_TAG("mysql connection closed %s"), ToString().c_str());
}

void MysqlConnect::OnMysqlDisconnect()
{
    for(auto iter : _stmtIdRefPrepareStmt)
        iter.second->OnMysqlDisconnect();
}

Int64 MysqlConnect::GetLastInsertIdOfAutoIncField() const
{
    return static_cast<Int64>(mysql_insert_id(_mysql));
}

Int64 MysqlConnect::GetLastAffectedRow() const
{
    return static_cast<Int64>(mysql_affected_rows(_mysql));
}

bool MysqlConnect::HasNextResult() const
{
    // 0表示有结果
    return mysql_next_result(_mysql) == 0;
}

Int64 MysqlConnect::GetCurrentResultRows(MYSQL_RES *res) const
{
    return static_cast<Int64>(mysql_num_rows(res));
}

LibString MysqlConnect::ToString() const
{
    LibString info;
    info.AppendFormat("connection id:%llu mysql host:%s, port:%hu, bind ip:%s, user:%s, db name:%s, is connected:%d, last ping:%llu ms"
                    , _id, _cfg._host.c_str(), _cfg._port, _cfg._bindIp.c_str(),
                     _cfg._user.c_str(), _cfg._dbName.c_str(), _isConnected, _lastPingMs);
    return info;
}

LibString MysqlConnect::GetCarefulOptionsInfo() const
{
    LibString info;
    {// MYSQL_SET_CHARSET_NAME
        LibString charset;
        charset.resize(256);
        auto err = mysql_get_option(_mysql, MYSQL_SET_CHARSET_NAME, charset.data());
        if(err != 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("get MYSQL_SET_CHARSET_NAME option fail err:%s"), mysql_error(_mysql));
            _UpdateLastMysqlErrno();
            return "";
        }

        info.AppendFormat("MYSQL_SET_CHARSET_NAME:%s\n", charset.c_str());
    }

    {// MYSQL_OPT_RECONNECT
        Int32 value = 0;
        auto err = mysql_get_option(_mysql, MYSQL_OPT_RECONNECT, &value);
        if(err != 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("get MYSQL_OPT_RECONNECT option fail err:%s"), mysql_error(_mysql));
            _UpdateLastMysqlErrno();
            return "";
        }

        info.AppendFormat("MYSQL_OPT_RECONNECT:%d\n", value);
    }

    {// MYSQL_OPT_MAX_ALLOWED_PACKET
        UInt64 value = 0;
        auto err = mysql_get_option(_mysql, MYSQL_OPT_MAX_ALLOWED_PACKET, &value);
        if(err != 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("get MYSQL_OPT_MAX_ALLOWED_PACKET option fail err:%s"), mysql_error(_mysql));
            _UpdateLastMysqlErrno();
            return "";
        }

        info.AppendFormat("MYSQL_OPT_MAX_ALLOWED_PACKET:%llu\n", value);
    }

    {// MYSQL_OPT_OPTIONAL_RESULTSET_METADATA
        bool value = false;
        auto err = mysql_get_option(_mysql, MYSQL_OPT_OPTIONAL_RESULTSET_METADATA, &value);
        if(err != 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("get MYSQL_OPT_OPTIONAL_RESULTSET_METADATA option fail err:%s"), mysql_error(_mysql));
            _UpdateLastMysqlErrno();
            return "";
        }

        info.AppendFormat("MYSQL_OPT_OPTIONAL_RESULTSET_METADATA:%d\n", value);
    }

    {// MYSQL_OPT_BIND
        LibString bindIp;
        bindIp.resize(256);
        auto err = mysql_get_option(_mysql, MYSQL_OPT_BIND, bindIp.data());
        if(err != 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("get MYSQL_OPT_BIND option fail err:%s"), mysql_error(_mysql));
            _UpdateLastMysqlErrno();
            return "";
        }

        info.AppendFormat("MYSQL_OPT_BIND:%s\n", bindIp.c_str());
    }

    return info;
}

UInt32 MysqlConnect::GetMysqlErrno() const
{
    return mysql_errno(_mysql);
}

LibString MysqlConnect::GetMysqlError() const
{
    return mysql_error(_mysql);
}

void MysqlConnect::AddOpCount(Int32 type, Int64 count) const
{
    auto iter = _typeRefOpInfo.find(type);
    if(iter == _typeRefOpInfo.end())
    {
        iter = _typeRefOpInfo.emplace(type, MysqlOperateInfo()).first;
        iter->second._type = type;
    }
    
    iter->second._count += count;
}

Int32 MysqlConnect::Init()
{
    if(UNLIKELY(_mysql))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("mysql is init before config:%s."), _cfg.ToString().c_str());
        return Status::Repeat;
    }

    _mysql = mysql_init(NULL);
    _isConnected = false;

    g_Log->Info(LOGFMT_OBJ_TAG("mysql connector init success id:%llu config:%s."), _id, _cfg.ToString().c_str());
    g_Log->Info(LOGFMT_OBJ_TAG("mysql connection simple info: %s."), ToString().c_str());

    return Status::Success;
}

Int32 MysqlConnect::Start()
{
    if(!_mysql)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("mysql is not init config:%s"), _cfg.ToString().c_str());
        return Status::Failed;
    }

    do
    {
        // 连接
        if(!_Connect())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("mysql_real_connect fail cfg:%s"), _cfg.ToString().c_str());
            return Status::Failed;
        }

        // 选择数据库
        if(!_SelectDB())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("select db fail cfg:%s"), _cfg.ToString().c_str());
            return Status::Failed;
        }

    } while (false);

    g_Log->Info(LOGFMT_OBJ_TAG("mysql connection start %s, carefule options:\n%s"), ToString().c_str(), GetCarefulOptionsInfo().c_str());
    
    return Status::Success;
}

MYSQL_RES *MysqlConnect::_StoreResult(bool &isSqlWithFieldsCountReturn) const
{
    AddOpCount(MysqlOperateType::Operate);
    auto res = mysql_store_result(_mysql);
    const auto fieldCount = mysql_field_count(_mysql);
    isSqlWithFieldsCountReturn = fieldCount != 0;
    
    if(!res)
    {
        auto errNo = _UpdateLastMysqlErrno();
        if(errNo == 0)
        {
            if(isSqlWithFieldsCountReturn)
            {// select等语句 有字段但是没有结果集
                g_Log->Warn(LOGFMT_OBJ_TAG("have some field but have no result(perhaps select sql): fieldCount:%u, err:%s, connection info:%s")
                            , fieldCount, mysql_error(_mysql), ToString().c_str());
                return NULL;
            }

            // update/insert/delete/create等 result是NULL属于正常的
            const auto affectedRow = GetLastAffectedRow();
            const auto lastInsertId = GetLastInsertIdOfAutoIncField();
            // update/insert/create/delete等没有结果集的sql
            if(g_Log->IsEnable(LogLevel::Debug))
                g_Log->Debug(LOGFMT_OBJ_TAG("perhaps update/insert/delete/create ... sql, have no result. store result affectedRow:%lld, lastInsertId:%lld err:%s, connection info:%s"), affectedRow, lastInsertId, mysql_error(_mysql), ToString().c_str());
        }
        else
        {
            g_Log->Error(LOGFMT_OBJ_TAG("store result fail errNo:%d error:%s, connection info :%s"), errNo, mysql_error(_mysql), ToString().c_str());
        }

        return NULL;
    }

    return res;
}

MYSQL_RES *MysqlConnect::_UseResult(bool &isSqlWithFieldsCountReturn) const
{
    auto res = mysql_use_result(_mysql);
    const auto fieldCount = mysql_field_count(_mysql);
    isSqlWithFieldsCountReturn = fieldCount != 0;

    if(!res)
    {
        auto errNo = _UpdateLastMysqlErrno();
        if(errNo == 0)
        {
            if(isSqlWithFieldsCountReturn)
            {// select等语句 有字段但是没有结果集
                g_Log->Warn(LOGFMT_OBJ_TAG("have some field but have no result(perhaps select sql): fieldCount:%u, err:%s, connection info:%s")
                            , fieldCount, mysql_error(_mysql), ToString().c_str());
                return NULL;
            }

            const auto affectedRow = GetLastAffectedRow();
            const auto lastInsertId = GetLastInsertIdOfAutoIncField();
            // update/insert/create/delete等没有结果集的sql
            if(g_Log->IsEnable(LogLevel::Debug))
                g_Log->Debug(LOGFMT_OBJ_TAG("perhaps update/insert/delete/create ... sql, have no result. store result affectedRow:%lld, lastInsertId:%lld err:%s, connection info:%s"), affectedRow, lastInsertId, mysql_error(_mysql), ToString().c_str());
        }
        else
        {
            g_Log->Error(LOGFMT_OBJ_TAG("use result fail errNo:%d error:%s, connection info :%s"), errNo, mysql_error(_mysql), ToString().c_str());
        }

        return NULL;
    }

    return res;
}

void MysqlConnect::_FreeRes(MYSQL_RES *res) const
{
    mysql_free_result(res);
}

void MysqlConnect::_FetchRows(MYSQL_RES *res, UInt64 seqId, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb)
{
    mysql_data_seek(res, 0);

    MYSQL_ROW row;
    auto fieldNum = mysql_num_fields(res);
    // Int32 rowCount = 0;
    std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> records;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        auto lens = mysql_fetch_lengths(res);
        SmartPtr<Record, AutoDelMethods::CustomDelete> record = Record::NewThreadLocal_Record();
        record.SetClosureDelegate([](void *p){
            auto ptr = KernelCastTo<Record>(p);
            Record::DeleteThreadLocal_Record(ptr);
        });

        record->SetFieldAmount(fieldNum);
        // ++rowCount;

        for(UInt32 idx = 0; idx < fieldNum; ++idx)
        {
            // 取当前字段信息并打印字段信息与数据
            auto curField = mysql_fetch_field_direct(res, idx);
            record->AddField(idx, curField->table, curField->name, curField->type, static_cast<UInt64>(curField->flags), row[idx], lens[idx]);

            // if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
            //     g_Log->Info(LOGFMT_OBJ_TAG("field:%s"), newField->ToString().c_str());
        }

        // if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
        //      g_Log->Info(LOGFMT_OBJ_TAG("row count:%d row info:%s"), rowCount, record->ToString().c_str());

        records.push_back(record);
    }

    auto mysqlErr = _UpdateLastMysqlErrno();
    if(LIKELY(cb))
        cb->Invoke(this, seqId, mysqlErr == 0 ?Status::Success : Status::Failed, mysqlErr, true, 0, 0, records);
}

bool MysqlConnect::_ExcuteSql(UInt64 seqId, const LibString &sql) const
{
    // mysql需要指定一个日志文件
    if(g_Log->IsEnable(LogLevel::DumpSql))
        g_Log->DumpSql(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("mysql connection id:%llu, excute sql seqId:%llu size:%llu excute sql:", _id, seqId, static_cast<UInt64>(sql.size())), sql);

    // 当没有开启CLIENT_MULTI_STATEMENTS时候会返回结果, 如果开启CLIENT_MULTI_STATEMENTS则语句并没有全部执行好,需要获取结果, 会有多个结果
    auto ret = mysql_real_query(_mysql, sql.c_str(), static_cast<ULong>(sql.length()));
    if(ret != 0)
    {
        _UpdateLastMysqlErrno();
        g_Log->FailSql(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("connection info:%s, excute sql fail err:%s, seqId:%llu sql size:%llu sql:", ToString().c_str(), mysql_error(_mysql), seqId, static_cast<UInt64>(sql.size())), sql);
        return false;
    }

    // 打印出影响的行数
    const Int64 count = GetLastAffectedRow();
    if(g_Log->IsEnable(LogLevel::DumpSql))
        g_Log->DumpSql(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("mysql connection id:%llu, seqId:%llu excute sql affected row:%lld, mysql_error:%s", _id, seqId, count, (count >= 0) ? "NONE" : mysql_error(_mysql)));

    AddOpCount(MysqlOperateType::Operate);
    
    return true;
}

bool MysqlConnect::_ExecuteSqlUsingStmt(const LibString &sql, UInt64 seqId, const std::vector<Field *> &fields, IDelegate<void, MysqlConnect *, UInt64, Int32, UInt32, bool, Int64, Int64, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &> *cb)
{
    auto stmt = _GetStmt(sql);
    if(!stmt)
    {
        stmt = _CreateStmt(sql);
        if(UNLIKELY(!stmt))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("stmt create fail"));
            return false;
        }
    }

    if(UNLIKELY(!stmt->IsConnected()))
    {
        if(UNLIKELY(!stmt->OnMysqlReconnect()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("stmt reconnect fail seqId:%llu"), seqId);
            return false;
        }
    }

    auto ret = stmt->Execute(seqId, fields, cb);
    if(ret != 0)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("stmt excute fail ret:%u"), ret);
        return false;
    }

    AddOpCount(MysqlOperateType::CompleteQuery);

    return true;
}

bool MysqlConnect::_Connect()
{
    // 连接
    _isConnected = false;

    // 设置mysql选项
    Int32 err = 0;
    do
    {
        // 操作字符集
        if(!_cfg._charset.empty())
        {
            err = mysql_options(_mysql, MYSQL_SET_CHARSET_NAME, _cfg._charset.c_str());
            if(err != 0)
                break;
        }

        // TODO:需要判断断线重连是否会自动选择上次的数据库
        // 自动重连
        err = mysql_options(_mysql, MYSQL_OPT_RECONNECT, &_cfg._autoReconnect);
        if(err != 0)
            break;

        // 设置mysql的最大包大小
        err = mysql_options(_mysql, MYSQL_OPT_MAX_ALLOWED_PACKET, &_cfg._maxPacketSize);
        if(err != 0)
            break;

        // 表信息打开
        bool isOpenTableInfo = true;
        err = mysql_options(_mysql, MYSQL_OPT_OPTIONAL_RESULTSET_METADATA, &isOpenTableInfo);
        if(err != 0)
            break;

        // 绑定本地指定的ip
        if(!_cfg._bindIp.empty())
        {
            err = mysql_options(_mysql, MYSQL_OPT_BIND, _cfg._bindIp.c_str());
            if(err != 0)
                break;
        }
    }while(false);
    
    if(err != 0)
    {
        _UpdateLastMysqlErrno();
        g_Log->Error(LOGFMT_OBJ_TAG("mysql_options fail error:%s, mysql connection info:%s"), mysql_error(_mysql), ToString().c_str());
        return false;
    }

    // 启用一次执行多条sql
    ULong flag = 0;
    if(_cfg._enableMultiStatements)
        flag = CLIENT_MULTI_STATEMENTS;

    if(!mysql_real_connect(_mysql, _cfg._host.c_str(), _cfg._user.c_str(), _cfg._pwd.c_str(), NULL, _cfg._port, 0, flag))
    {
        _UpdateLastMysqlErrno();
        g_Log->Error(LOGFMT_OBJ_TAG("mysql_real_connect fail connection info:%s, mysql error:%s"), ToString().c_str(), mysql_error(_mysql));
        return false;
    }

    _isConnected = true;

    return true;
}

bool MysqlConnect::_SelectDB()
{
    bool isExists = false;
    auto res = mysql_list_dbs(_mysql, _cfg._dbName.c_str());
    AddOpCount(MysqlOperateType::Operate);

    if(UNLIKELY(res))
    {
        isExists = mysql_num_rows(res) > 0;
        mysql_free_result(res);
    }

    if(!isExists)
    {// 数据库不存在则创建
        CreateDBSqlBuilder builder;
        const auto &sql = builder.DB(_cfg._dbName).Charset(_cfg._dbCharset).Collate(_cfg._dbCollate).ToSql();

        if(!_ExcuteSql(0, sql))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("db:%s, _ExcuteSql fail err:%s"), _cfg._dbName.c_str(), mysql_error(_mysql));
            return false;
        }

        g_Log->Info(LOGFMT_OBJ_TAG( "DB:%s create success."), _cfg._dbName.c_str());
    }

    auto ret = mysql_select_db(_mysql, _cfg._dbName.c_str());
    AddOpCount(MysqlOperateType::Operate);
    if(ret != 0)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("select db:%s fail err:%s"), _cfg._dbName.c_str(), mysql_error(_mysql));
        return false;
    }

    return true;
}

bool MysqlConnect::_Ping(const LibString &content)
{
    g_Log->Info(LOGFMT_OBJ_TAG("will ping content:%s, _mysql:%p, connection:%s"), content.c_str(), _mysql, ToString().c_str());

    const auto &counter = LibCpuCounter::Current();
    auto ret = mysql_ping(_mysql);
    _lastPingMs = LibCpuCounter::Current().ElapseMilliseconds(counter);
    
    AddOpCount(MysqlOperateType::Operate);
    if(ret == 0)
    {
        _isConnected = true;
        if(g_Log->IsEnable(LogLevel::DumpSql))
            g_Log->DumpSql(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("mysql ping success connection id:%llu, content:%s, last ping ms:%llu", _id, content.c_str(), _lastPingMs));

        return true;
    }

    _UpdateLastMysqlErrno();
    g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("mysql disconnected connection info:%s, mysql error:%s, %u, last ping ms:%llu,\ncontent:."
    , ToString().c_str(), _lastErrString.c_str(), _lastErrno, _lastPingMs)
                , content);

    _isConnected = false;

    return false;
}

UInt32 MysqlConnect::_UpdateLastMysqlErrno() const
{
    _lastErrString = mysql_error(_mysql);
    return _lastErrno = mysql_errno(_mysql);
}

PrepareStmt *MysqlConnect::_CreateStmt(const LibString &sql)
{
    ++_stmtId;

    SmartPtr<PrepareStmt, KERNEL_NS::AutoDelMethods::CustomDelete> newStmt = PrepareStmt::NewThreadLocal_PrepareStmt(this, sql, _stmtId);
    newStmt.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<PrepareStmt *>(p);
        PrepareStmt::DeleteThreadLocal_PrepareStmt(ptr);
    });

    if(!newStmt->Init())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("prepare stmt init fail connection: %s."), ToString().c_str());
        return NULL;
    }

    _stmtIdRefPrepareStmt.insert(std::make_pair(_stmtId, newStmt.AsSelf()));
    _sqlRefPrepareStmt.insert(std::make_pair(sql, newStmt.AsSelf()));
    return newStmt.pop();
}

PrepareStmt *MysqlConnect::_GetStmt(const LibString &sql)
{
    auto iter = _sqlRefPrepareStmt.find(sql);
    return iter == _sqlRefPrepareStmt.end() ? NULL : iter->second;
}

KERNEL_END