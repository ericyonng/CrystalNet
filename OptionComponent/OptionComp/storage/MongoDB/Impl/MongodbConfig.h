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
// Date: 2026-05-24 18:05:56
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGODB_CONFIG_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGODB_CONFIG_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/LibStringYaml.h>
#include <yaml-cpp/yaml.h>


KERNEL_BEGIN

// 配置
struct MongodbConfig
{
    POOL_CREATE_OBJ_DEFAULT(MongodbConfig);

    // 创建自己
    static MongodbConfig *CreateNewObj(MongodbConfig &&cfg)
    {
        return MongodbConfig::New_MongodbConfig(std::move(cfg));
    }

    // 释放自己
    void Release()
    {
        MongodbConfig::Delete_MongodbConfig(this);
    }

    KERNEL_NS::LibString SrvHostName;
    KERNEL_NS::LibString Account;
    KERNEL_NS::LibString Pwd;
    // 若指定复制集名, 则运行在复制集模式, 如果没有默认分片集群模式
    KERNEL_NS::LibString ReplicaSetName;
    // PurgeInterval
    KERNEL_NS::TimeSlice PurgeInterval;
};

KERNEL_END


namespace YAML
{
    // LogFileDefine序列化反序列化
    template<>
   struct convert<KERNEL_NS::MongodbConfig>
    {
        static Node encode(const KERNEL_NS::MongodbConfig& rhs)
        {
            Node node;
            node["SrvHostName"] = rhs.SrvHostName;
            node["Account"] = rhs.Account;
            node["Pwd"] = rhs.Pwd;
            node["ReplicaSetName"] = rhs.ReplicaSetName;
            node["PurgeInterval"] = rhs.PurgeInterval;
            return node;
        }

        static bool decode(const Node& node,  KERNEL_NS::MongodbConfig& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            auto &srvHostNameNode = node["SrvHostName"];
            if(srvHostNameNode.IsDefined() && (!srvHostNameNode.IsNull()))
                rhs.SrvHostName = srvHostNameNode.as<std::string>();

            auto &accountNode = node["Account"];
            if(accountNode.IsDefined() && (!accountNode.IsNull()))
                rhs.Account = accountNode.as<std::string>();
            
            auto &pwdNode = node["Pwd"];
            if(pwdNode.IsDefined() && (!pwdNode.IsNull()))
                rhs.Pwd = pwdNode.as<std::string>();

            auto &replicaSetNameNode = node["ReplicaSetName"];
            if(replicaSetNameNode.IsDefined() && (!replicaSetNameNode.IsNull()))
                rhs.ReplicaSetName = replicaSetNameNode.as<std::string>();

            auto &purgeIntervalNode = node["PurgeInterval"];
            if(purgeIntervalNode.IsDefined() && (!purgeIntervalNode.IsNull()))
                rhs.PurgeInterval = purgeIntervalNode.as<KERNEL_NS::TimeSlice>();
            
            return true;
        }
    };
}

#endif