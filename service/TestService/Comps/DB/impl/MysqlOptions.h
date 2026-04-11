// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-04-11 23:04:43
// Author: Eric Yonng
// Description:

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <service/common/macro.h>
#include <kernel/comp/LibStringYaml.h>

SERVICE_BEGIN

struct MysqlOptions
{
    POOL_CREATE_OBJ_DEFAULT(MysqlOptions);
    
    // 创建自己
    static MysqlOptions *CreateNewObj(MysqlOptions &&cfg)
    {
        return MysqlOptions::New_MysqlOptions(std::move(cfg));
    }

    // 释放自己
    void Release()
    {
        MysqlOptions::Delete_MysqlOptions(this);
    }

    // 默认的存储引擎(可以选择默认的存储引擎作为主要存储)
    KERNEL_NS::LibString DefaultStorage;
    // 数据库版本号, 跨版本号清档数据
    Int32 DbVersion = 0;
    // 系统使用operator uid, operator uid用于多线程下的负载均衡
    Int32 SystemOperatorUid = 0;
    // 数据清洗时间间隔
    Int32 PurgeIntervalMs = 3000;
    // 系统表MysqlMgr表禁用自动清库
    bool DisableSystemTableAutoDrop = true;
    // 用清库功能 用于线上, 线上不可以自动清库，线上必须设置成1, 只能修数据 TODO:
    bool DisableAutoDropDB = true;
};

struct MysqlCommonOptions
{
    POOL_CREATE_OBJ_DEFAULT(MysqlCommonOptions);
    
    // 创建自己
    static MysqlCommonOptions *CreateNewObj(MysqlCommonOptions &&cfg)
    {
        return MysqlCommonOptions::New_MysqlCommonOptions(std::move(cfg));
    }

    // 释放自己
    void Release()
    {
        MysqlCommonOptions::Delete_MysqlCommonOptions(this);
    }

    // 默认初始blob字段大小 64字节
    Int64 DefaultBlobOriginSize = 64;

    // 初始string key大小 VARCHAR(256)
    Int32 DefaultStringKeyOriginSize = 64;
};

SERVICE_END


namespace YAML
{
    template<>
    struct convert<SERVICE_NS::MysqlOptions>
    {
        static Node encode(const SERVICE_NS::MysqlOptions& rhs)
        {
            Node node;
            node["DefaultStorage"] = rhs.DefaultStorage;
            node["DbVersion"] = rhs.DbVersion;
            node["SystemOperatorUid"] = rhs.SystemOperatorUid;
            node["PurgeIntervalMs"] = rhs.PurgeIntervalMs;
            node["DisableSystemTableAutoDrop"] = rhs.DisableSystemTableAutoDrop;
            node["DisableAutoDropDB"] = rhs.DisableAutoDropDB;
            
            return node;
        }

        static bool decode(const Node& node, SERVICE_NS::MysqlOptions& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            {
                auto &&value = node["DefaultStorage"];
                if(value.IsDefined())
                {
                    rhs.DefaultStorage = value.as<KERNEL_NS::LibString>();
                }
            }

            {
                auto &&value = node["DbVersion"];
                if(value.IsDefined())
                {
                    rhs.DbVersion = value.as<Int32>();
                }
            }

            {
                auto &&value = node["SystemOperatorUid"];
                if(value.IsDefined())
                {
                    rhs.SystemOperatorUid = value.as<Int32>();
                }
            }

            {
                auto &&value = node["PurgeIntervalMs"];
                if(value.IsDefined())
                {
                    rhs.PurgeIntervalMs = value.as<Int32>();
                }
            }

            {
                auto &&value = node["DisableSystemTableAutoDrop"];
                if(value.IsDefined())
                {
                    rhs.DisableSystemTableAutoDrop = value.as<bool>();
                }
            }

            {
                auto &&value = node["DisableAutoDropDB"];
                if(value.IsDefined())
                {
                    rhs.DisableAutoDropDB = value.as<bool>();
                }
            }

            return true;
        }
    };

    
    template<>
    struct convert<SERVICE_NS::MysqlCommonOptions>
    {
        static Node encode(const SERVICE_NS::MysqlCommonOptions& rhs)
        {
            Node node;
            node["DefaultBlobOriginSize"] = rhs.DefaultBlobOriginSize;
            node["DefaultStringKeyOriginSize"] = rhs.DefaultStringKeyOriginSize;
            
            return node;
        }

        static bool decode(const Node& node, SERVICE_NS::MysqlCommonOptions& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            {
                auto &&value = node["DefaultBlobOriginSize"];
                if(value.IsDefined())
                {
                    rhs.DefaultBlobOriginSize = value.as<Int64>();
                }
            }

            {
                auto &&value = node["DefaultStringKeyOriginSize"];
                if(value.IsDefined())
                {
                    rhs.DefaultStringKeyOriginSize = value.as<Int32>();
                }
            }

            return true;
        }
    };
}
