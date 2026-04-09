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
 * Date: 2023-07-14 13:17:00
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_MYSQL_CONFIG_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_MYSQL_CONFIG_H__

#pragma once

#include <kernel/comp/LibString.h>
#include <kernel/comp/memory/ObjPoolMacro.h>

KERNEL_BEGIN

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
    UInt64 _maxPacketSize;  // mysql 单包缓冲区大小(涉及到从mysql回来的接收缓冲区大小) 最大2GB
    // bool _isOpenTableInfo;  // 开启表信息显示 一定是开启的
    bool _enableMultiStatements;    // 支持一次执行多条sql
    Int32 _retryWhenError;      // sql执行失败(网络断开)重试次数
    Int32 _dbThreadNum;         // db线程数量
    Int64 _pingIntervalSeconds;    // 间隔多长时间Ping一次, 维持mysql 连接活跃
};

KERNEL_END

namespace YAML
{
    template<>
    struct convert<KERNEL_NS::MysqlConfig>
    {
        static Node encode(const KERNEL_NS::MysqlConfig& rhs)
        {
            Node node;
            node["Host"] = rhs._host;
            node["DB"] = rhs._dbName;
            node["User"] = rhs._user;
            node["Pwd"] = rhs._pwd;
            node["Port"] = rhs._port;
            node["BindIp"] = rhs._bindIp;
            node["CharacterSet"] = rhs._dbCharset;
            node["COLLATE"] = rhs._dbCollate;
            node["AutoReconnect"] = rhs._autoReconnect;
            node["MaxPacketSize"] = rhs._maxPacketSize;
            node["EnableMultiStatements"] = rhs._enableMultiStatements;
            node["RetryTimesWhenNetInterrupt"] = rhs._retryWhenError;
            node["DbThreadNum"] = rhs._dbThreadNum;
            node["PingIntervalSeconds"] = rhs._pingIntervalSeconds;
            return node;
        }
        
        static bool decode(const Node& node, KERNEL_NS::MysqlConfig& rhs)
        {
            if (!node.IsMap())
                return false;

            {
                auto &&value = node["Host"];
                if (value.IsDefined())
                {
                    rhs._host = value.as<KERNEL_NS::LibString>();
                }
            }

            {
                auto &&value = node["DB"];
                if (value.IsDefined())
                {
                    rhs._dbName = value.as<KERNEL_NS::LibString>();
                }
            }

            {
                auto &&value = node["User"];
                if (value.IsDefined())
                {
                    rhs._user = value.as<KERNEL_NS::LibString>();
                }
            }

            {
                auto &&value = node["Pwd"];
                if (value.IsDefined())
                {
                    rhs._pwd = value.as<KERNEL_NS::LibString>();
                }
            }

            {
                auto &&value = node["Port"];
                if (value.IsDefined())
                {
                    rhs._port = value.as<UInt16>();
                }
            }

            {
                auto &&value = node["BindIp"];
                if (value.IsDefined())
                {
                    rhs._bindIp = value.as<KERNEL_NS::LibString>();
                }
            }

            {
                auto &&value = node["CharacterSet"];
                if (value.IsDefined())
                {
                    rhs._dbCharset = value.as<KERNEL_NS::LibString>();

                    // mysql操作时的编码需要和数据库编码一致
                    rhs._charset = rhs._dbCharset;
                }
            }

            {
                auto &&value = node["COLLATE"];
                if (value.IsDefined())
                {
                    rhs._dbCollate = value.as<KERNEL_NS::LibString>();
                }
            }

            {
                auto &&value = node["AutoReconnect"];
                if (value.IsDefined())
                {
                    rhs._autoReconnect = value.as<Int32>();
                }
            }

            {
                auto &&value = node["MaxPacketSize"];
                if (value.IsDefined())
                {
                    rhs._maxPacketSize = value.as<UInt64>();
                }
            }

            {
                auto &&value = node["EnableMultiStatements"];
                if (value.IsDefined())
                {
                    rhs._enableMultiStatements = value.as<bool>();
                }
            }
            
            {
                auto &&value = node["RetryTimesWhenNetInterrupt"];
                if (value.IsDefined())
                {
                    rhs._retryWhenError = value.as<Int32>();
                }
            }

            {
                auto &&value = node["DbThreadNum"];
                if (value.IsDefined())
                {
                    rhs._dbThreadNum = value.as<Int32>();
                }
            }

            {
                auto &&value = node["PingIntervalSeconds"];
                if (value.IsDefined())
                {
                    rhs._pingIntervalSeconds = value.as<Int64>();
                }
            }
            return true;
        }
    };
}


#endif
