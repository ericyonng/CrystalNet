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
// Date: 2026-02-25 22:02:29
// Author: Eric Yonng
// Description:


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CONFIG_KERNEL_CONFIG_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CONFIG_KERNEL_CONFIG_H__

#pragma once

#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/kernel_export.h>
#include <yaml-cpp/yaml.h>

KERNEL_BEGIN

// 网络处理单元定义
struct KERNEL_EXPORT NetUnitDefine
{
    Int32 Id = 0;
    Int32 Count = 1;
};

// 黑白名单模式
struct KERNEL_EXPORT BlackWhiteListMode
{
    // 是否启用黑名单
    bool BlackList = true;
    // 是否启用白名单
    bool WhiteList = true;
};

struct KERNEL_EXPORT NetConfig
{
    // 是否启用网络
    bool Enable = true;

    // 连接器配置
    NetUnitDefine Linker;

    // 数据传输器配置
    NetUnitDefine DataTransfer;

    // 黑白名单模式
    BlackWhiteListMode BlackWhiteListMode;

    // 最大会话数量限制 0表示无限制
    Int32 MaxSessionQuantity = 100000;

    /** 收发包参数 */
    // 收包单包大小限制, 0表示不限制, 默认16MB 如果传参不指定, 则默认使用配置, 如果手动指定参数, 则使用参数的
    Int32 SessionRecvPacketContentLimit = 16777216;
    // 发包单包大小限制 0表示不限制,如果传参不指定, 则默认使用配置, 如果手动指定参数, 则使用参数的
    Int32 SessionSendPacketContentLimit = 0;
    // 单帧最大接收数据量 0表示不限制
    Int32 MaxRecvBytesPerFrame = 0;
    // 单帧最大发送数据量 0表示不限制
    Int32 MaxSendBytesPerFrame = 0;
    // 单帧最大处理连接数 0表示不限制
    Int32 MaxAcceptCountPerFrame = 1024;
};

// 内核配置
struct KERNEL_EXPORT KernelConfig
{
    POOL_CREATE_OBJ_DEFAULT(KernelConfig);
    
    /** 网络配置 */
    NetConfig NetConfig;

    /** 垃圾回收参数 */
    // 所有分配器内存使用上限控制 4GB
    Int64 AllMemoryAlloctorTotalUpper = 4294967296;
    // GC 时间间隔
    Int32 GCIntervalMs = 100;
    // 设置中央内存收集器工作时间间隔
    Int32 CenterCollectorIntervalMs = 100;
    // 设置中央收集器跨线程block合并数量达到限制唤醒收集器工作的数量 128 * 1024
    Int32 WakeupCenterCollectorMinBlockNum = 131072;
    // 设置MergeMemoryBufferInfo队列达到数量时唤醒中央收集器 128 * 1024
    Int32 WakeupCenterCollectorMinMergeBufferInfoNum = 131072;
    // 定时合并tls内存块间隔时间 1分钟
    Int32 MergeTlsMemoryBlockIntervalMs = 5000;

    /** 事件循环Poller配置 */
    // 预设的poller扫描时间间隔
    Int32 MaxPollerScanMilliseconds = 1;

    // 创建自己
    static KernelConfig *CreateNewObj(KernelConfig &&cfg)
    {
        return KernelConfig::New_KernelConfig(std::move(cfg));
    }

    // 释放自己
    void Release()
    {
        KernelConfig::Delete_KernelConfig(this);
    }
};


KERNEL_END


namespace YAML
{
    // 网络单元定义
    template<>
    struct KERNEL_EXPORT convert<KERNEL_NS::NetUnitDefine>
    {
        static Node encode(const KERNEL_NS::NetUnitDefine& rhs)
        {
            Node node;
            node["Id"] = rhs.Id;
            node["Count"] = rhs.Count;
            return node;
        }

        static bool decode(const Node& node,  KERNEL_NS::NetUnitDefine& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }
            
            rhs.Id = node["Id"].as<Int32>();
            {
                auto &&value = node["Count"];
                if(value.IsDefined())
                    rhs.Count = value.as<Int32>();
            }

            return true;
        }
    };

    // 黑白名单
    template<>
    struct KERNEL_EXPORT convert<KERNEL_NS::BlackWhiteListMode>
    {
        static Node encode(const KERNEL_NS::BlackWhiteListMode& rhs)
        {
            Node node;
            node["BlackList"] = rhs.BlackList;
            node["WhiteList"] = rhs.WhiteList;
            return node;
        }

        static bool decode(const Node& node,  KERNEL_NS::BlackWhiteListMode& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            {
                auto &&value = node["BlackList"];
                if(value.IsDefined())
                    rhs.BlackList = value.as<bool>();
            }
            {
                auto &&value = node["WhiteList"];
                if(value.IsDefined())
                    rhs.WhiteList = value.as<bool>();
            }

            return true;
        }
    };

    // 网络配置
    template<>
    struct KERNEL_EXPORT convert<KERNEL_NS::NetConfig>
    {
        static Node encode(const KERNEL_NS::NetConfig& rhs)
        {
            Node node;
            node["Enable"] = rhs.Enable;
            node["Linker"] = rhs.Linker;
            node["DataTransfer"] = rhs.DataTransfer;
            node["BlackWhiteListMode"] = rhs.BlackWhiteListMode;
            node["MaxSessionQuantity"] = rhs.MaxSessionQuantity;
            node["SessionRecvPacketContentLimit"] = rhs.SessionRecvPacketContentLimit;
            node["SessionSendPacketContentLimit"] = rhs.SessionSendPacketContentLimit;
            node["MaxRecvBytesPerFrame"] = rhs.MaxRecvBytesPerFrame;
            node["MaxSendBytesPerFrame"] = rhs.MaxSendBytesPerFrame;
            node["MaxAcceptCountPerFrame"] = rhs.MaxAcceptCountPerFrame;
            return node;
        }

        static bool decode(const Node& node,  KERNEL_NS::NetConfig& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            {
                auto &&value = node["Enable"];
                if(value.IsDefined())
                    rhs.Enable = value.as<bool>();
            }
            {
                auto &&value = node["Linker"];
                if(value.IsMap())
                    rhs.Linker = value.as<KERNEL_NS::NetUnitDefine>();
            }
            {
                auto &&value = node["DataTransfer"];
                if(value.IsMap())
                    rhs.DataTransfer = value.as<KERNEL_NS::NetUnitDefine>();
            }
            {
                auto &&value = node["BlackWhiteListMode"];
                if(value.IsMap())
                    rhs.BlackWhiteListMode = value.as<KERNEL_NS::BlackWhiteListMode>();
            }
            {
                auto &&value = node["MaxSessionQuantity"];
                if(value.IsDefined())
                    rhs.MaxSessionQuantity = value.as<Int32>();
            }
            {
                auto &&value = node["SessionRecvPacketContentLimit"];
                if(value.IsDefined())
                    rhs.SessionRecvPacketContentLimit = value.as<Int32>();
            }
            {
                auto &&value = node["SessionSendPacketContentLimit"];
                if(value.IsDefined())
                    rhs.SessionSendPacketContentLimit = value.as<Int32>();
            }
            {
                auto &&value = node["MaxRecvBytesPerFrame"];
                if(value.IsDefined())
                    rhs.MaxRecvBytesPerFrame = value.as<Int32>();
            }
            {
                auto &&value = node["MaxSendBytesPerFrame"];
                if(value.IsDefined())
                    rhs.MaxSendBytesPerFrame = value.as<Int32>();
            }
            {
                auto &&value = node["MaxAcceptCountPerFrame"];
                if(value.IsDefined())
                    rhs.MaxAcceptCountPerFrame = value.as<Int32>();
            }

            return true;
        }
    };

    // 内核配置
    template<>
    struct KERNEL_EXPORT convert<KERNEL_NS::KernelConfig>
    {
        static Node encode(const KERNEL_NS::KernelConfig& rhs)
        {
            Node node;
            node["NetConfig"] = rhs.NetConfig;
            node["AllMemoryAlloctorTotalUpper"] = rhs.AllMemoryAlloctorTotalUpper;
            node["GCIntervalMs"] = rhs.GCIntervalMs;
            node["CenterCollectorIntervalMs"] = rhs.CenterCollectorIntervalMs;
            node["WakeupCenterCollectorMinBlockNum"] = rhs.WakeupCenterCollectorMinBlockNum;
            node["WakeupCenterCollectorMinMergeBufferInfoNum"] = rhs.WakeupCenterCollectorMinMergeBufferInfoNum;
            node["MergeTlsMemoryBlockIntervalMs"] = rhs.MergeTlsMemoryBlockIntervalMs;
            node["MaxPollerScanMilliseconds"] = rhs.MaxPollerScanMilliseconds;
            
            return node;
        }

        static bool decode(const Node& node,  KERNEL_NS::KernelConfig& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            {
                auto &&value = node["NetConfig"];
                if(value.IsMap())
                    rhs.NetConfig = value.as<KERNEL_NS::NetConfig>();
            }
            {
                auto &&value = node["AllMemoryAlloctorTotalUpper"];
                if(value.IsDefined())
                    rhs.AllMemoryAlloctorTotalUpper = value.as<Int64>();
            }
            {
                auto &&value = node["GCIntervalMs"];
                if(value.IsDefined())
                    rhs.GCIntervalMs = value.as<Int32>();
            }
            {
                auto &&value = node["CenterCollectorIntervalMs"];
                if(value.IsDefined())
                    rhs.CenterCollectorIntervalMs = value.as<Int32>();
            }
            {
                auto &&value = node["WakeupCenterCollectorMinBlockNum"];
                if(value.IsDefined())
                    rhs.WakeupCenterCollectorMinBlockNum = value.as<Int32>();
            }
            {
                auto &&value = node["WakeupCenterCollectorMinMergeBufferInfoNum"];
                if(value.IsDefined())
                    rhs.WakeupCenterCollectorMinMergeBufferInfoNum = value.as<Int32>();
            }
            {
                auto &&value = node["MergeTlsMemoryBlockIntervalMs"];
                if(value.IsDefined())
                    rhs.MergeTlsMemoryBlockIntervalMs = value.as<Int32>();
            }
            {
                auto &&value = node["MaxPollerScanMilliseconds"];
                if(value.IsDefined())
                    rhs.MaxPollerScanMilliseconds = value.as<Int32>();
            }

            return true;
        }
    };
}

#endif