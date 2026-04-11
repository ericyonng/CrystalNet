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
 * Date: 2022-09-21 15:00:00
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/macro.h>
#include <service_common/ServiceCommon.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerConfig.h>
#include <kernel/comp/NetEngine/Defs/AddrIpConfig.h>
#include <unordered_map>
#include <vector>
#include <service_common/protocol/CrystalProtocol/CrystalProtocolStackType.h>


SERVICE_BEGIN




struct ServiceConfig
{
    POOL_CREATE_OBJ_DEFAULT(ServiceConfig);
    
    // 创建自己
    static ServiceConfig *CreateNewObj(ServiceConfig &&cfg)
    {
        return ServiceConfig::New_ServiceConfig(std::move(cfg));
    }

    // 释放自己
    void Release()
    {
        ServiceConfig::Delete_ServiceConfig(this);
    }
    
    // service层poller扫描间隔(不小于20ms)
    Int32 PollerMaxSleepMilliseconds = 20;
    // 帧更新间隔(调用组件update)
    Int32 FrameUpdateTimeMs = 50;
    // 是否开启网络协议打印
    bool ProtoStackOpenLog = true;
    // 配置表数据目录
    KERNEL_NS::LibString ConfigDataPath = "./Cfgs";
    // 密钥 公钥使用PKC8
    KERNEL_NS::LibString RsaPrivateKey;
    KERNEL_NS::LibString RsaPublicKey;
    // 协议加密key过期时间
    Int64 EncryptKeyExpireTime;
};

SERVICE_END


namespace YAML
{
    // 网络单元定义
    template<>
    struct convert<SERVICE_NS::ServiceConfig>
    {
        static Node encode(const SERVICE_NS::ServiceConfig& rhs)
        {
            Node node;
            node["PollerMaxSleepMilliseconds"] = rhs.PollerMaxSleepMilliseconds;
            node["FrameUpdateTimeMs"] = rhs.FrameUpdateTimeMs;
            // node["TcpListenList"] = rhs.TcpListenList;
            node["ProtoStackOpenLog"] = rhs.ProtoStackOpenLog;
            node["ConfigDataPath"] = rhs.ConfigDataPath;
            node["RsaPrivateKey"] = rhs.RsaPrivateKey;
            node["RsaPublicKey"] = rhs.RsaPublicKey;
            node["EncryptKeyExpireTime"] = rhs.EncryptKeyExpireTime;
            return node;
        }

        static bool decode(const Node& node,  SERVICE_NS::ServiceConfig& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }
            
            {
                auto &&value = node["PollerMaxSleepMilliseconds"];
                if(value.IsDefined())
                    rhs.PollerMaxSleepMilliseconds = value.as<Int32>();
            }

            {
                auto &&value = node["FrameUpdateTimeMs"];
                if(value.IsDefined())
                    rhs.FrameUpdateTimeMs = value.as<Int32>();
            }

            // {
            //     auto &&value = node["TcpListenList"];
            //     if(value.IsDefined())
            //         rhs.TcpListenList = value.as<std::vector<SERVICE_NS::TcpListenInfo>>();
            // }

            {
                auto &&value = node["ProtoStackOpenLog"];
                if(value.IsDefined())
                    rhs.ProtoStackOpenLog = value.as<bool>();
            }

            {
                auto &&value = node["ConfigDataPath"];
                if(value.IsDefined())
                    rhs.ConfigDataPath = value.as<KERNEL_NS::LibString>();
            }            
            {
                auto &&value = node["RsaPrivateKey"];
                if(value.IsDefined())
                    rhs.RsaPrivateKey = value.as<KERNEL_NS::LibString>();
            }                 
            {
                auto &&value = node["RsaPublicKey"];
                if(value.IsDefined())
                    rhs.RsaPublicKey = value.as<KERNEL_NS::LibString>();
            }  
            {
                auto &&value = node["EncryptKeyExpireTime"];
                if(value.IsDefined())
                    rhs.EncryptKeyExpireTime = value.as<Int32>();
            }             

            return true;
        }
    };
}