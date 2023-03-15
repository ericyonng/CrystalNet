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
 * Date: 2021-09-11 17:49:11
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_KERNEL_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_KERNEL_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class ILogFactory;
class LibCpuInfo;
class CpuFeature;
class MemoryMonitor;

class KERNEL_EXPORT KernelUtil
{
public:
    // 框架初始化与销毁
    static Int32 Init(ILogFactory * logFactory, const Byte8 *logIniName, const Byte8 *iniPath, const LibString &logContent = LibString(), const LibString &consoleContent = LibString(), bool needSignalHandle = true);
    static void Start();
    static void Destroy();
    static void OnSignalClose();
    static void OnAbnormalClose();

    static void InstallSignalCloseHandler(IDelegate<void> *task);

private:
    static void _OnSinalOccur();
};

KERNEL_END

extern KERNEL_EXPORT KERNEL_NS::LibCpuInfo *g_cpu;
// extern KERNEL_EXPORT KERNEL_NS::CpuFeature *g_cpuFeature;
extern KERNEL_EXPORT KERNEL_NS::MemoryMonitor *g_MemoryMonitor;
extern KERNEL_EXPORT KERNEL_NS::LibString g_LogIniName;
extern KERNEL_EXPORT KERNEL_NS::LibString g_LogIniRootPath;

#endif