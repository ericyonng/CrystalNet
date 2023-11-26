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

#include <kernel/kernel_inc.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/common/procstat.h>

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
    Int32 GetCpuCoreCnt();

private:
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    Int64 _CompareFileTime(FILETIME time1, FILETIME time2) const;
#else
    Int64 _GetTotalCpuTime() const;
    Int64 _GetProcCpuTime(UInt64 pid) const;
#endif

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

inline LibCpuInfo::LibCpuInfo()
    :_isInit{false}
    ,_cpuCoreNum{0}
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    ,_preidleTime{0}
    ,_preKernalTime{0}
    ,_preUserTime{0}
#else
    ,_totalCpuTime(0)
    ,_procCpuTime(0)
#endif
{

}

inline bool LibCpuInfo::Initialize()
{
    if(UNLIKELY(_isInit.exchange(true)))
        return true;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    
    FILETIME preIdle;
    FILETIME preKernal;
    FILETIME preUser;
    if(UNLIKELY(!GetSystemTimes(&preIdle, &preKernal, &preUser)))
    {
        CRYSTAL_TRACE("GetSystemTimes fail when Initialize cpu info.");
        return false;
    }

    _lck.Lock();
    _preidleTime  = preIdle;
    _preKernalTime  = preKernal;
    _preUserTime = preUser;
    _lck.Unlock();

#else
    const auto total = _GetTotalCpuTime();
    const auto proc = _GetProcCpuTime(SystemUtil::GetCurProcessId());

    _lck.Lock();
    _totalCpuTime = total;
    _procCpuTime = proc;
    _lck.Unlock();

#endif

    return true;
}

inline Int64 LibCpuInfo::GetUsage()
{
    if(UNLIKELY(!_isInit))
        return 0;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;
    if(!GetSystemTimes(&idleTime, &kernelTime, &userTime))
        return 0;

    _lck.Lock();
    Int64 diffIdleTime = _CompareFileTime(_preidleTime, idleTime);          // free time
    Int64 diffKernelTime = _CompareFileTime(_preKernalTime, kernelTime);    // kernal time
    Int64 diffUserTime = _CompareFileTime(_preUserTime, userTime);          // user time

    if(diffKernelTime + diffUserTime == 0)
    {
        _lck.Unlock();
        return 0;
    }

    // cpu usage =（total-idle）/total
    Int64 usage = (diffKernelTime + diffUserTime - diffIdleTime) * 10000 / (diffKernelTime + diffUserTime);
    _preidleTime = idleTime;
    _preKernalTime = kernelTime;
    _preUserTime = userTime;
    _lck.Unlock();

    return usage;

#else

	Int64 totalCpuTime = _GetTotalCpuTime();
	Int64 procCpuTime = _GetProcCpuTime(SystemUtil::GetCurProcessId());
 
    _lck.Lock();

    // 占用率 = 进程占用时间 / cpu总时间
    const auto diffTotalCpu = totalCpuTime - _totalCpuTime;
    // CRYSTAL_TRACE("diffTotalCpu =%lld, totalCpuTime=%lld, _totalCpuTime=%lld procCpuTime=%lld, _procCpuTime=%lld"
    // , diffTotalCpu, totalCpuTime, _totalCpuTime, procCpuTime, _procCpuTime);

	if(diffTotalCpu)
    {
        // CRYSTAL_TRACE("diffTotalCpu =%lld", diffTotalCpu);
        Int64 ret = (procCpuTime - _procCpuTime) * 10000 / diffTotalCpu;

        _procCpuTime = procCpuTime;
        _totalCpuTime = totalCpuTime;
        _lck.Unlock();
		return ret;
    }

    _lck.Unlock();

    return 0;

#endif
}

inline Int32 LibCpuInfo::GetCpuCoreCnt()
{
    if(UNLIKELY(!_isInit.load()))
        return 0;

    Int32 count = 1; // at least one
    const auto num = _cpuCoreNum.load();
    if(LIKELY(num))
        return num;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    count = si.dwNumberOfProcessors;
#else
    count = sysconf(_SC_NPROCESSORS_CONF);
#endif  

    _cpuCoreNum = count;
    return count;
}

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

inline Int64 LibCpuInfo::_CompareFileTime(FILETIME time1, FILETIME time2) const
{
    Int64 a = (static_cast<Int64>(time1.dwHighDateTime) << 32) | static_cast<Int64>(time1.dwLowDateTime);
    Int64 b = (static_cast<Int64>(time2.dwHighDateTime) << 32) | static_cast<Int64>(time2.dwLowDateTime);

    return abs(b - a);
}

#else
    
inline Int64 LibCpuInfo::_GetTotalCpuTime() const
{
    // 读取stat第一行即为cpu信息
    auto fp = FileUtil::OpenFile("/proc/stat", false, "r");
    if(!fp)
    {
        CRYSTAL_TRACE("open /proc/stat fail %s", SystemUtil::GetErrString(errno).c_str());
        return 0;
    }
	
    LibString cpuInfoString;
    FileUtil::ReadOneLine(*fp, cpuInfoString);
    cpuInfoString.strip();
    FileUtil::CloseFile(*fp);
    // CRYSTAL_TRACE("/proc/stat:%s", cpuInfoString.c_str());

    // 切割字符串
    auto strArr = cpuInfoString.Split(' ');
    // CRYSTAL_TRACE("total cpu time extract:%s %s %s %s %s", strArr[ProcStat::CPU_NAME].c_str(), strArr[ProcStat::USER].c_str()
    // , strArr[ProcStat::NICE].c_str(), strArr[ProcStat::SYSTEM].c_str(), strArr[ProcStat::IDLE].c_str());
    Int64 user = StringUtil::StringToInt64(strArr[ProcStat::USER].c_str());     // 用户模式
    Int64 nice = StringUtil::StringToInt64(strArr[ProcStat::NICE].c_str());     // 低优先级用户模式
    Int64 system = StringUtil::StringToInt64(strArr[ProcStat::SYSTEM].c_str());   // 内核模式
    Int64 idle = StringUtil::StringToInt64(strArr[ProcStat::IDLE].c_str());     // 空闲处理器时间

    return user + nice + system + idle;
}

inline Int64 LibCpuInfo::_GetProcCpuTime(UInt64 pid) const
{
    LibString fileName;
    fileName.AppendFormat("/proc/%llu/stat", pid);

    auto fp = FileUtil::OpenFile(fileName.c_str(), false, "r");
    if(!fp)
    {
        CRYSTAL_TRACE("open %s fail %s", fileName.c_str(), SystemUtil::GetErrString(errno).c_str());
        return 0;
    }
	
    LibString cpuInfoString;
    FileUtil::ReadOneLine(*fp, cpuInfoString);
    cpuInfoString.strip();
    FileUtil::CloseFile(*fp);
    // CRYSTAL_TRACE("%s:%s", fileName.c_str(), cpuInfoString.c_str());
	
    // 切割字符串 参数见proc pid stat 相关文档
    auto strArr = cpuInfoString.Split(' ');
    // CRYSTAL_TRACE("proc[%llu] cpu time exstact: %s %s %s %s", pid, strArr[ProcPidStat::UTIME].c_str()
    // , strArr[ProcPidStat::STIME].c_str(), strArr[ProcPidStat::CUTIME].c_str(), strArr[ProcPidStat::CSTIME].c_str());
    
    Int64 userTime = StringUtil::StringToInt64(strArr[ProcPidStat::UTIME].c_str());     // utime         user mode jiffies
    Int64 kernelTime = StringUtil::StringToInt64(strArr[ProcPidStat::STIME].c_str());   // stime         kernel mode jiffies
    Int64 alluserTime = StringUtil::StringToInt64(strArr[ProcPidStat::CUTIME].c_str());  // cutime        user mode jiffies with child's
    Int64 alldeadTime = StringUtil::StringToInt64(strArr[ProcPidStat::CSTIME].c_str());  // cstime        kernel mode jiffies with child's

    // CRYSTAL_TRACE("userTime=%lld, kernelTime=[%lld], alluserTime=[%lld], alldeadTime[%lld]"
    // , userTime, kernelTime, alluserTime, alldeadTime);

    return userTime + kernelTime + alluserTime + alldeadTime;
}

#endif

KERNEL_END


#endif
