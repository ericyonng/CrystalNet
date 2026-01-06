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
 * Author: Eric Yonng
 * Date: 2021-03-18 11:42:28
 * Description: 
 *              1.提供cpu信息
 *              2.会读取文件, 比较消耗性能
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CPU_CPU_INFO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CPU_CPU_INFO_H__

#pragma once

#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <atomic>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include <minwinbase.h>
#endif

KERNEL_BEGIN

class KERNEL_EXPORT LibCpuInfo
{
public:
    static LibCpuInfo *GetInstance();
    LibCpuInfo();

    // 接口均线程安全
public:
    // 初始化
    bool Initialize();
    // 获取万分数 linux下不准
    Int64 GetUsage();
    Int32 GetCpuCoreCnt() const;

private:
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    Int64 _CompareFileTime(FILETIME time1, FILETIME time2) const;
#else
    Int64 _GetTotalCpuTime() const;
    Int64 _GetProcCpuTime(UInt64 pid) const;
#endif

    void _InitCorNumber();

private:
    std::atomic_bool _isInit;
    std::atomic<Int32> _cpuCoreNum;      // cpu核心数量
    SpinLock _lck;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    FILETIME _preidleTime;
    FILETIME _preKernalTime;
    FILETIME _preUserTime;
#else
    Int64 _totalCpuTime;    // 总cpu时间
    Int64 _procCpuTime;     // 进程时间
#endif

};

ALWAYS_INLINE Int32 LibCpuInfo::GetCpuCoreCnt() const
{
    return _cpuCoreNum.load(std::memory_order_relaxed);
}

KERNEL_END


#endif
