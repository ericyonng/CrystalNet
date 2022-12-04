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
 * Date: 2022-06-24 12:38:41
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_APPLICATION_APPLICATION_CONFIG_H__
#define __CRYSTAL_NET_SERVICE_COMMON_APPLICATION_APPLICATION_CONFIG_H__

#pragma once

#include <kernel/kernel.h>
#include <service_common/common/common.h>

SERVICE_COMMON_BEGIN

// 全球id生成机制(63bit 兼容lua,lua实际上最大只能支持到63位最高位是符号位):国家id(8bit) + 16bit machineId（一个进程一个） + 39位序列号（数据库最大值+本地存储最大值保证高可用（数据库挂了还有本地存储引擎）,因为每个实例的序列号最大值不共用）
struct ApplicationConfig
{
    KERNEL_NS::LibString _appAliasName;             // 程序别名:比如gs, ls等
    KERNEL_NS::LibString _projectMainServiceName;   // 项目主服务名 如:Login, Gate等
    std::atomic<UInt16> _machineId = {0};       // 机器id(全球唯一,默认是0,此时应该向中心注册机器,获取机器id)

    // 机器注册信息 machineId没获取成功之前以下参数都会变化
    UInt64 _registerTime = 0;                   // 注册成功的机器注册时间微妙
    KERNEL_NS::LibString _registerPath;         // 注册成功的机器注册时的进程路径
    UInt64 _registerProcessId = 0;              // 注册成功的机器注册时的进程id
    KERNEL_NS::LibString _machineApplyId;       // 程序创建生成的唯一标识符:base64(sha256(进程路径 + 项目类型名 + 时间 + 进程id)) 记录申请machineId时的唯一标识符
};

SERVICE_COMMON_END

#endif
