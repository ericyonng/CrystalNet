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
    LibString _host;
    LibString _user;
    LibString _pwd;
    LibString _dbName;
    UInt16 _port;

    // 可选配置
    LibString _charset;     // mysql操作时的编码字符集
    LibString _dbCharset;     // db库的字符集
    LibString _dbCollate;     // db库的字符集
    Int32 _autoReconnect;    // 自动重连
    UInt64 _maxPacketSize;  // mysql 单包缓冲区大小(涉及到从mysql回来的接收缓冲区大小)
    bool _isOpenTableInfo;  // 开启表信息显示
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

    // 直接将结果集拿回本地
    template<typename CallbackType>
    bool StoreResult(CallbackType &&cb);

    // 需要fetch_row一次一次的从远程取会结果
    template<typename CallbackType>
    bool UseResult(CallbackType &&cb);

    LibString ToString() const;

private:
    MYSQL_RES *_StoreResult();
    MYSQL_RES *_UseResult();
    void _FreeRes(MYSQL_RES *res);
    bool _ExcuteSql(const LibString &sql);
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
ALWAYS_INLINE bool MysqlConnect::StoreResult(CallbackType &&cb)
{
    auto res = _StoreResult();
    if(!res)
        return false;

    cb(this, res);

    _FreeRes(res);

    return true;
}

template<typename CallbackType>
ALWAYS_INLINE bool MysqlConnect::UseResult(CallbackType &&cb)
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
