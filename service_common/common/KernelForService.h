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
 * Date: 2026-09-09 12:36:36
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_COMMON_KERNEL_FOR_SERVICE_H__
#define __CRYSTAL_NET_SERVICE_COMMON_COMMON_KERNEL_FOR_SERVICE_H__

#pragma once

#include <service_common/common/macro.h>
#include <kernel/common/BaseMacro.h>

KERNEL_BEGIN
class YamlMemory;
KERNEL_END

SERVICE_COMMON_BEGIN

class KernelFlags
{
public:
    enum ENUMS : Int32
    {
        // 切换工作路径
        CHANGE_WORK_DIR = 0,

        // 修改文件描述符限制
        MODIFY_FILE_DESC_LIMIT,

        // 修改系统时区(禁用时, 应该不调用tzset()等方法)
        MODIFY_SYSTEM_TIME_ZONE,

        // 捕获异常信号
        CATCH_ABNORMAL_SIGNAL,

        // 设置信号处理任务
        SET_SIGNAL_PROCESSOR,

        // 设置Crash hook
        MODIFY_CRASH_HOOK,

        // 初始化网络环境
        INIT_SOCKET_ENV,

        // 初始化curl
        INIT_CURL,

        // 初始化tls
        INIT_TLS,
        
        // 最大值
        MAX,
    };

    static constexpr UInt64 DefaultFlags = ~(1LLU << KernelFlags::MAX);
};

class KernelForService
{
public:
    static Int32 Init(int argc, char const *argv[], KERNEL_NS::YamlMemory *yamlMemory = NULL, const char *yamlPartPath = "/Yaml/", const char *logFilaName = "Log.yaml", UInt64 flags = KernelFlags::DefaultFlags, bool needSignalHandle = true, Int64 fileSoftLimit = 1024000, Int64 fileHardLimit = 1024000);
    static void Start();

    static void Destroy();

    static void DefaultOnSignalClose();
};

// 新的全局变量
extern std::atomic<UInt64> s_KernelFlags;

SERVICE_COMMON_END


#endif