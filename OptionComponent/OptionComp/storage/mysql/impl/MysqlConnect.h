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
#include <kernel/comp/memory/memory.h>
#include <OptionComp/storage/mysql/impl/SqlBuilder.h>

struct MYSQL;
struct MYSQL_RES;

KERNEL_BEGIN

class LibSocket;

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
    UInt64 _maxPacketSize;  // mysql 单包缓冲区大小(涉及到从mysql回来的接收缓冲区大小)
    bool _isOpenTableInfo;  // 开启表信息显示
    bool _enableMultiStatements;    // 支持一次执行多条sql
};

class MysqlConnect
{
    POOL_CREATE_OBJ_DEFAULT(MysqlConnect);

public:
    MysqlConnect(UInt64 id);
    ~MysqlConnect();

    void SetConfig(const MysqlConfig &cfg);
    const MysqlConfig &GetConfig() const;

    Int32 Init();
    Int32 Start();
    
    void Close();

    template<SqlBuilderType::ENUMS T>
    bool ExcuteSql(const SqlBuilder<T> &builder);

    // 获取上次新增一条数据操作产生的id(这个id是在标识了自增属性的字段, 一张表只能有一个自增id字段), 如果上次是其他非造成新增数据的sql则获取的结果是0
    // 如果表没有自增字段则返回0
    UInt64 GetLastInsertIdOfAutoIncField() const;

    // 获取上次执行sql影响的行数(由执行完sql后自动返回的,有些sql不返回, 不返回的则此时是-1), 没有影响返回0
    Int64 GetLastAffectedRow() const;

    // 直接将结果集拿回本地
    template<typename CallbackType>
    bool StoreResult(CallbackType &&cb) const;

    // 需要fetch_row一次一次的从远程取会结果
    template<typename CallbackType>
    bool UseResult(CallbackType &&cb) const;

    // 判断执行完sql有没有结果
    bool HasNextResult() const;

    // 

    LibString ToString() const;

private:
    MYSQL_RES *_StoreResult() const;
    MYSQL_RES *_UseResult() const;
    void _FreeRes(MYSQL_RES *res) const;
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
};

ALWAYS_INLINE void MysqlConnect::SetConfig(const MysqlConfig &cfg)
{
    _cfg = cfg;
}

ALWAYS_INLINE const MysqlConfig &MysqlConnect::GetConfig() const
{
    return _cfg;
}

template<SqlBuilderType::ENUMS T>
ALWAYS_INLINE bool MysqlConnect::ExcuteSql(const SqlBuilder<T> &builder)
{
    const auto &sql = builder.ToSql();
    for(;!_Ping(sql););

    return _ExcuteSql(sql);
}

template<typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::StoreResult(CallbackType &&cb) const
{
    auto res = _StoreResult();
    if(!res)
        return false;

    cb(this, res);

    _FreeRes(res);

    return true;
}

template<typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::UseResult(CallbackType &&cb) const
{
    auto res = _UseResult();
    if(!res)
        return false;

    cb(this, res);

    _FreeRes(res);

    return true;
}

KERNEL_END

#endif
