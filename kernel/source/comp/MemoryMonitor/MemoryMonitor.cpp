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
 * Date: 2021-03-19 22:23:58
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <memory>

#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/common/macro.h>
#include <kernel/comp/memory/CenterMemoryCollector.h>
#include <kernel/comp/Log/log.h>

#include <kernel/comp/MemoryMonitor/MemoryMonitor.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Cpu/LibCpuCounter.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/MemoryMonitor/Statistics.h>

KERNEL_BEGIN

MemoryMonitor::MemoryMonitor()
:_init{false}
,_isStart{false}
,_milliSecInterval(60 * 1000)   // 默认1分钟一次
{

}

MemoryMonitor::~MemoryMonitor()
{
    Close();
}

MemoryMonitor *MemoryMonitor::GetInstance()
{
    static std::shared_ptr<MemoryMonitor> s_Monitor(CRYSTAL_NEW(MemoryMonitor));

    return s_Monitor.get();
}

Int32 MemoryMonitor::Init(Int64 milliSecInterval)
{
    if(_init.exchange(true))
    {
        CRYSTAL_TRACE("MemoryMonitor has init before.");
        return Status::Success;
    }

    _milliSecInterval = milliSecInterval;

    return Status::Success;
}

void MemoryMonitor::Start()
{
    if(!_init)
        return;

    if(_isStart.exchange(true))
        return;
}

void MemoryMonitor::Close()
{
    if(!_isStart.exchange(false))
        return;
    
    _init = false;
}

Statistics *MemoryMonitor::GetStatistics()
{
    static std::shared_ptr<Statistics> s_statistics(new Statistics);

    return s_statistics.get();
}

IDelegate<void> *MemoryMonitor::MakeWorkTask()
{
    return DelegateFactory::Create(this, &MemoryMonitor::_DoWork);
}

void MemoryMonitor::_DoWork()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR UInt64 batchNum = 0;
    auto statics = GetStatistics();
    LibCpuCounter cpuCounter, startCounter;
    startCounter.Update();

    const auto pid = SystemUtil::GetCurProcessId();
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    UInt64 sysAvail = SystemUtil::GetAvailPhysMemSize();
    UInt64 totalSystemSize = SystemUtil::GetTotalPhysMemSize();
    #else
    std::map<LibString, LibString> memInfo;
    SystemUtil::ReadMemInfoDict(memInfo);
    // 单位KB
    UInt64 sysAvail = SystemUtil::GetAvailableMem(memInfo) * 1024;
    UInt64 totalSystemSize = SystemUtil::GetTotalMem(memInfo) * 1024;

    KERNEL_NS::LinuxProcInfo procInfo;
    KERNEL_NS::SystemUtil::GetLinuxProcessProcInfo(procInfo);

    #endif

    auto centerMemoryCollector = CenterMemoryCollector::GetInstance();

    g_Log->MemMonitor("Begin Proc:%d memory monitor log batchNum:%llu, System Total Mem Size:%llu, System Available Mem Size:%llu\n", pid, ++batchNum, totalSystemSize, sysAvail);

    // linux 进程信息
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        g_Log->MemMonitor("Proc Info:\n[FDSize:%llu, Peak Memory Occupied:%llu KB, Current Occupied:%llu KB]\n"
                        "[Peak Malloc Phisic Mem:%llu KB, Current Used Physic Mem:%llu KB]\n"
                        "[Data Segment:%llu KB, Stack Segment::%llu KB, Exe Code:%llu KB]\n"
                        "[Libarary:%llu KB, Swap::%llu KB, PageTable:%llu KB]\n"
                        , procInfo._fdSize, procInfo._vmPeak, procInfo._vmSize
                        , procInfo._vmHwm, procInfo._vmRss
                        , procInfo._vmData, procInfo._vmStk, procInfo._vmExe
                        , procInfo._vmLib, procInfo._vmSwap, procInfo._vmPTE
                        );
    #endif

    // 已打印的不打,未打印的append上去,一旦发生变化,需要重新便利
    auto &dict = statics->GetDict();
    UInt64 idx = 0;
    UInt64 totalBufferBytes = 0;
    while (true)
    {
        statics->Lock();
        if(idx >= dict.size())
        {
            statics->Unlock();
            break;
        }
        
        LibString cache;
        auto deleg = dict.at(idx);
        totalBufferBytes += deleg->Invoke(cache);
        ++idx;
        statics->Unlock();

        g_Log->MemMonitor("%s\n", cache.c_str());
    }

    const auto &centerCollectorInfo = centerMemoryCollector->ToString();
    g_Log->MemMonitor("centerCollectorInfo:%s", centerCollectorInfo.c_str());

    g_Log->MemMonitor("End Proc:%d memory monitor log batchNum:%llu Total Pool Alloc Buffer Bytes:%llu, Memory monitor Use Time: %llu(micro seconds) one frame.\n"
                    , pid, batchNum, totalBufferBytes, cpuCounter.Update().ElapseMicroseconds(startCounter));
}

KERNEL_END
