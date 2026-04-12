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
// Date: 2026-04-12 16:04:15
// Author: Eric Yonng
// Description:


#pragma once

#include <service/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/FileMonitor/FileMonitorMacro.h>

#include <kernel/comp/LibString.h>
#include <kernel/comp/LibStringYaml.h>

#include "service/common/Configs/AddrConfig.h"

SERVICE_BEGIN

struct TestOptions
{
    FILE_MONITOR_DECLARE(TestOptions)

    // 测试数量
    Int32 TestSessionCount = 0;
    // 连接的时间间隔
    Int32 TestConnectIntervalMs = 100;

    // 地址信息
    AddrConfig TestTargetAddr;
    bool HasTargetAddr = false;

    // 1表示等响应包回来继续发送, 其他是间隔发送
    Int32 TestSendMode = 1;
    // 发送时间间隔
    Int32 TestSendIntervalMs = 0;
    // 一次发送多少个包
    Int32 TestSendPackCountOnce = 1;
    // 一次发送的包内容至少多少个字节(pb序列化会多增加数据量)
    Int32 TestSendPackageBytes = 100;
    // 发包超时时间 30s
    Int32 TestSendPackageTimeoutMilliseconds = 30000;
};

SERVICE_END


namespace YAML
{
    template<>
    struct convert<SERVICE_NS::TestOptions>
    {
        static Node encode(const SERVICE_NS::TestOptions& rhs)
        {
            Node node;
            node["TestSessionCount"] = rhs.TestSessionCount;
            node["TestConnectIntervalMs"] = rhs.TestConnectIntervalMs;
            node["TestTargetAddr"] = rhs.TestTargetAddr;
            node["TestSendMode"] = rhs.TestSendMode;
            node["TestSendIntervalMs"] = rhs.TestSendIntervalMs;
            node["TestSendPackCountOnce"] = rhs.TestSendPackCountOnce;
            node["TestSendPackageBytes"] = rhs.TestSendPackageBytes;
            node["TestSendPackageTimeoutMilliseconds"] = rhs.TestSendPackageTimeoutMilliseconds;
            return node;
        }

        static bool decode(const Node& node, SERVICE_NS::TestOptions& rhs)
        {
            if (!node.IsMap())
            {
                return false;
            }

            {
                auto &&value = node["TestSessionCount"];
                if(value.IsDefined())
                {
                    rhs.TestSessionCount = value.as<Int32>();
                }
            }

            {
                auto &&value = node["TestConnectIntervalMs"];
                if(value.IsDefined())
                {
                    rhs.TestConnectIntervalMs = value.as<Int32>();
                }
            }

            {
                auto &&value = node["TestTargetAddr"];
                if(value.IsDefined())
                {
                    rhs.TestTargetAddr = value.as<SERVICE_NS::AddrConfig>();
                    rhs.HasTargetAddr = true;
                }
            }

            {
                auto &&value = node["TestSendMode"];
                if(value.IsDefined())
                {
                    rhs.TestSendMode = value.as<Int32>();
                }
            }

            {
                auto &&value = node["TestSendIntervalMs"];
                if(value.IsDefined())
                {
                    rhs.TestSendIntervalMs = value.as<Int32>();
                }
            }

            {
                auto &&value = node["TestSendPackCountOnce"];
                if(value.IsDefined())
                {
                    rhs.TestSendPackCountOnce = value.as<Int32>();
                }
            }

            {
                auto &&value = node["TestSendPackageBytes"];
                if(value.IsDefined())
                {
                    rhs.TestSendPackageBytes = value.as<Int32>();
                }
            }

            {
                auto &&value = node["TestSendPackageTimeoutMilliseconds"];
                if(value.IsDefined())
                {
                    rhs.TestSendPackageTimeoutMilliseconds = value.as<Int32>();
                }
            }

            return true;
        }
    };
}