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

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlConfig);

POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlConnect);

MysqlConfig::MysqlConfig()
:_port(3306)
,_charset("utf8mb4")
,_autoReconnect(1)
,_maxPacketSize(1024*1024*1024)
,_isOpenTableInfo(true)
{

}

LibString MysqlConfig::ToString() const
{
    LibString info;
    info.AppendFormat("host:%s user:%s pwd:%s db:%s port:%llu charset:%s autoreconnect:%d maxpacketsize:%llu isopentableinfo:%d"
        , _host.c_str(), _user.c_str(), _pwd.c_str(), _dbName.c_str(), _port, _charset.c_str(), _autoReconnect, _maxPacketSize, _isOpenTableInfo);

    return info;
}

MysqlConnect::MysqlConnect()
:_mysql(NULL)
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

    }while(false);
    
    if(err != 0)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("mysql error:%s"), mysql_error(_mysql));
        return Status::Failed;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("mysql connector init success config:%s."), _cfg.ToString().c_str());

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
        if(!mysql_real_connect(_mysql, _cfg._host.c_str(), _cfg._user.c_str(), _cfg._pwd.c_str(), NULL, _cfg._port, 0, 0))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("mysql_real_connect fail cfg:%s, mysql error:%s"), _cfg.ToString().c_str(), mysql_error(_mysql));
            return Status::Failed;
        }

        
    } while (false);
    
    return Status::Success;
}

bool MysqlConnect::_Connect()
{
    // 连接
    if(!mysql_real_connect(_mysql, _cfg._host.c_str(), _cfg._user.c_str(), _cfg._pwd.c_str(), NULL, _cfg._port, 0, 0))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("mysql_real_connect fail cfg:%s, mysql error:%s"), _cfg.ToString().c_str(), mysql_error(_mysql));
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
        KERNEL_NS::LibString sql;
        sql.AppendFormat("CREATE DATABASE IF NOT EXISTS `%s` DEFAULT CHARACTER SET utf8mb4 DEFAULT COLLATE utf8mb4_bin"
                        , _cfg._dbName.c_str());

        auto ret = mysql_real_query(_mysql, sql.c_str(), static_cast<ULong>(sql.length()));
        _AddOpCount(MysqlOperateType::WRITE);
        if(ret != 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("db:%s, create fail err:%s"), _cfg._dbName.c_str(), mysql_error(_mysql));
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

bool MysqlConnect::_Ping()
{
    auto ret = mysql_ping(_mysql);
    _AddOpCount(MysqlOperateType::READ);

    if(ret == 0)
    {
        _isConnected = true;
        return true;
    }

    g_Log->Warn(LOGFMT_OBJ_TAG("mysql disconnected config:%s, mysql error:%s.")
                ,_cfg.ToString().c_str(), mysql_error(_mysql));

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