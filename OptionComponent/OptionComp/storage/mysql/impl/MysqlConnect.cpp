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
#include <mysql.h>
#include <kernel/comp/Utils/Utils.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlConfig);

POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlConnect);

MysqlConfig::MysqlConfig()
:_port(3306)
,_charset("utf8mb4")
,_dbCharset("utf8mb4")
,_dbCollate("utf8mb4_bin")
,_autoReconnect(1)
,_maxPacketSize(4 * 1024 * 1024 * 1024) // 4GB
,_isOpenTableInfo(true)
,_enableMultiStatements(true)
{

}

LibString MysqlConfig::ToString() const
{
    LibString info;
    info.AppendFormat("host:%s user:%s pwd:%s db:%s port:%llu bind ip:%s \ncharset:%s db charset:%s db collate:%s autoreconnect:%d \nmaxpacketsize:%llu isopentableinfo:%d, _enableMultiStatements:%d"
        , _host.c_str(), _user.c_str(), _pwd.c_str(), _dbName.c_str(), _port, _bindIp.c_str(), _charset.c_str(), _dbCharset.c_str(), _dbCollate.c_str(),
         _autoReconnect, _maxPacketSize, _isOpenTableInfo, _enableMultiStatements);

    return info;
}

MysqlConnect::MysqlConnect(UInt64 id)
:_id(id)
,_mysql(NULL)
,_isConnected(false)
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

    g_Log->Info(LOGFMT_OBJ_TAG("mysql connection closed %s"), ToString().c_str());
}

UInt64 MysqlConnect::GetLastInsertIdOfAutoIncField() const
{
    return mysql_insert_id(_mysql);
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

LibString MysqlConnect::ToString() const
{
    LibString info;
    info.AppendFormat("connection id:%llu mysql host:%s, port:%hu, bind ip:%s, user:%s, db name:%s, is connected:%d"
                    , _id, _cfg._host.c_str(), _cfg._port, _cfg._bindIp.c_str(),
                     _cfg._user.c_str(), _cfg._dbName.c_str(), _isConnected);
    return info;
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

    Int32 err = 0;
    do
    {
        // 操作字符集
        err = mysql_options(_mysql, MYSQL_SET_CHARSET_NAME, _cfg._charset.c_str());
        if(err != 0)
            break;

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
        err = mysql_options(_mysql, MYSQL_OPT_OPTIONAL_RESULTSET_METADATA, &_cfg._isOpenTableInfo);
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
        g_Log->Error(LOGFMT_OBJ_TAG("mysql error:%s"), mysql_error(_mysql));
        return Status::Failed;
    }

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

    Int32 err = 0;
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

    g_Log->Info(LOGFMT_OBJ_TAG("mysql connection start %s"), ToString().c_str());
    
    return Status::Success;
}

MYSQL_RES *MysqlConnect::_StoreResult() const
{
    auto res = mysql_store_result(_mysql);
    if(!res)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("store result fail err:%s, connection info:%s"), mysql_error(_mysql), ToString().c_str());
        return NULL;
    }

    return res;
}

MYSQL_RES *MysqlConnect::_UseResult() const
{
    auto res = mysql_use_result(_mysql);
    if(!res)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("use result fail err:%s, connection info:%s"), mysql_error(_mysql), ToString().c_str());
        return NULL;
    }

    return res;
}

void MysqlConnect::_FreeRes(MYSQL_RES *res) const
{
    mysql_free_result(res);
}

bool MysqlConnect::_ExcuteSql(const LibString &sql) const
{
    g_Log->Info2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("mysql connection id:%llu, excute sql:", _id), sql);

    // 当没有开启CLIENT_MULTI_STATEMENTS时候会返回结果, 如果开启CLIENT_MULTI_STATEMENTS则语句并没有全部执行好,需要获取结果, 会有多个结果
    auto ret = mysql_real_query(_mysql, sql.c_str(), static_cast<ULong>(sql.length()));
    if(ret != 0)
    {
        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("connection info:%s, excute sql fail err:%s, sql:", ToString().c_str(), mysql_error(_mysql)), sql);
        return false;
    }

    // 打印出影响的行数
    const Int64 count = GetLastAffectedRow();
    g_Log->Info2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("mysql connection id:%llu, excute sql affected row:%lld, mysql_error:%s", _id, count, (count >= 0) ? "NONE" : mysql_error(_mysql)));

    return true;
}

bool MysqlConnect::_Connect()
{
    // 连接
    _isConnected = false;
    ULong flag = 0;
    if(_cfg._enableMultiStatements)
        flag = CLIENT_MULTI_STATEMENTS;

    if(!mysql_real_connect(_mysql, _cfg._host.c_str(), _cfg._user.c_str(), _cfg._pwd.c_str(), NULL, _cfg._port, 0, flag))
    {
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
    _AddOpCount(MysqlOperateType::READ);

    if(UNLIKELY(res))
    {
        isExists = mysql_num_rows(res) > 0;
        mysql_free_result(res);
    }

    if(!isExists)
    {// 数据库不存在则创建
        SqlBuilder<SqlBuilderType::CREATE_DB> builder;
        const auto &sql = builder.DB(_cfg._dbName).Charset(_cfg._dbCharset).Collate(_cfg._dbCollate).ToSql();

        _AddOpCount(MysqlOperateType::WRITE);
        if(!_ExcuteSql(sql))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("db:%s, _ExcuteSql fail err:%s"), _cfg._dbName.c_str(), mysql_error(_mysql));
            return false;
        }

        g_Log->Info(LOGFMT_OBJ_TAG( "DB:%s create success."), _cfg._dbName.c_str());
    }

    auto ret = mysql_select_db(_mysql, _cfg._dbName.c_str());
    _AddOpCount(MysqlOperateType::READ);
    if(ret != 0)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("select db:%s fail err:%s"), _cfg._dbName.c_str(), mysql_error(_mysql));
        return false;
    }

    return true;
}

bool MysqlConnect::_Ping(const LibString &content)
{
    auto ret = mysql_ping(_mysql);
    _AddOpCount(MysqlOperateType::READ);

    if(ret == 0)
    {
        _isConnected = true;
        return true;
    }

    g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("mysql disconnected connection info:%s, mysql error:%s\ncontent:.", ToString().c_str(), mysql_error(_mysql))
                , content);

    _isConnected = false;

    return false;
}

void MysqlConnect::_AddOpCount(Int32 type, Int64 count)
{
    auto iter = _typeRefOpInfo.find(type);
    if(iter == _typeRefOpInfo.end())
    {
        iter = _typeRefOpInfo.emplace(type, MysqlOperateInfo()).first;
        iter->second._type = type;
    }
    
    iter->second._count += count;
}


KERNEL_END