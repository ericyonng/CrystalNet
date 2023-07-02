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
 * Date: 2020-12-07 00:44:45
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_SYSTEM_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_SYSTEM_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/Defs/SystemUtilDef.h>

KERNEL_BEGIN

class LibString;
struct ProcessMemInfo;

class KERNEL_EXPORT SystemUtil
{
public:
    // 睡眠挂起
    static void ThreadSleep(UInt64 milliSec, UInt64 microSec = 0);

    // 获取线程id
    static UInt64 GetCurrentThreadId();

    /* 进程线程 */
    // 获取程序目录
    static Int32 GetProgramPath(bool isCurrentProcess, LibString &processPath, UInt64 pid = 0);
    // 获取当前进程名
    static LibString GetCurProgramName();   // 带扩展名
    static LibString GetCurProgramNameWithoutExt();  // 不扩展名
    // 获取当前进程根目录
    static LibString GetCurProgRootPath();
    // 获取进程id
    static Int32 GetCurProcessId();
    // 结束进程
    static Int32 CloseProcess(UInt64 processId, ULong *lastError = NULL);
    static Int32 SendCloseMsgToProcess(UInt64 processId, ULong *lastError = NULL);
    static UInt64 GetMainThreadId(UInt64 processId);

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    // 获取可用的内存大小
    static UInt64 GetAvailPhysMemSize();
    // 获取内存大小
    static UInt64 GetTotalPhysMemSize();
    // 内存使用率
    static UInt64 GetMemoryLoad();
    // 进程占用内存信息
    static bool GetProcessMemInfo(HANDLE processHandle, ProcessMemInfo &info);

    // 创建进程快照（遍历进程相关）
    static HANDLE CreateProcessSnapshot();
    // 获取第一个进程id
    static UInt64 GetFirstProcessPid(HANDLE &hSnapshot);
    // 获取下一个进程id
    static UInt64 GetNextProcessPid(HANDLE &hSnapshot);
    // 获取进程句柄
    static HANDLE GetCurProcessHandle();
    // 通过进程ID获取窗口句柄
    static HWND GetWindowHwndByPID(DWORD dwProcessID);
    // 将窗口设置顶层
    static void BringWindowsToTop(HWND curWin);
    // 弹窗
    static void MessageBoxPopup(const LibString &title, const LibString &content);
    // 获取当前调用线程所在的cpu编号信息
    static void GetCallingThreadCpuInfo(UInt16 &cpuGroup, Byte8 &cpuNumber);
    // 遍历进程判断某进程是否在进程列表
    static bool IsProcessExist(const LibString &processName);
    // 找进程id，isLikely:是否模糊匹配
    static bool GetProcessIdList(const LibString &processName, std::map<UInt64, LibString> &processIdRefNames, bool isLikely = true, bool isMatchPath = false);
    #else
    // 可用内存
    static UInt64 GetFreeMemBySysCall();
    static UInt64 GetAvailableMem();
    static UInt64 GetAvailableMem(const std::map<LibString, LibString> &memInfo);
    // 总内存
    static UInt64 GetTotalMem();
    static UInt64 GetTotalMem(const std::map<LibString, LibString> &memInfo);
    // 读取内存信息字典 key:内存信息名, value对应meminfo中的值（tripUnitOfValue:去除值的数量单位）
    static bool ReadMemInfoDict(std::map<LibString, LibString> &memInfo, bool tripUnitOfValue = true);
    #endif
    
    /* 控制台 */
    static void LockConsole();
    static void UnlockConsole();
    // 设置控制台颜色
    static Int32 SetConsoleColor(Int32 color);
    // 获取控制台颜色
    static Int32 GetConsoleColor();
    // 输出给控制台
    static void OutputToConsole(const LibString &outStr);

    /* 杂项 */
    // 获取错误码字符串
    static LibString GetErrString(Int32 err);
    static Int32 GetErrNo(bool fromNet = false);
    // 获取错误码

    // linux后台程序 stdIoRedirect是输入输出错误信息重定向的文件,默认/dev/null, 必须是绝对路径
    // 工作路径设置会对相对路径产生影响
    static void TurnDaemon(const std::string &stdIoRedirect = "", const std::string &workDir = "");
    // 设置工作目录 工作路径设置会对相对路径产生影响
    static void ChgWorkDir(const LibString &workDir);

    static void YieldScheduler();
    // 让出当前cpu调度
    static void RelaxCpu();

    // 执行命令并返回执行过程中产生的输出
    static bool Exec(const LibString &cmd, Int32 &err, LibString &outputInfo);

    #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    // 设置资源信息
    static Int32 SetProcessFileDescriptLimit(Int32 resourceId, Int64 softLimit, Int64 hardLimit, LibString &errInfo);
    
    // 获取资源信息
    static Int32 GetProcessFileDescriptLimit(Int32 resourceId, Int64 &softLimit, Int64 &hardLimit, LibString &errInfo);
    #endif

private:
};

inline void SystemUtil::ThreadSleep(UInt64 milliSec, UInt64 microSec)
{
    std::chrono::microseconds t(milliSec * TimeDefs::MICRO_SECOND_PER_MILLI_SECOND + microSec);
    std::this_thread::sleep_for(t);
}

inline UInt64 SystemUtil::GetCurrentThreadId()
{
    thread_local UInt64 s_currentThreadId = 0;

#if CRYSTAL_TARGET_PLATFORM_LINUX
	// pthread_self是获取的是pthread_create创建的tcb块的首地址，基本一样,不是真正的线程id,应该使用gettid()
	// linux glibc 不提供gettid只能手动调用系统调用 __NR_gettid 224
	// return ::pthread_self();
	// return ::gettid();
    if(UNLIKELY(s_currentThreadId == 0))
        s_currentThreadId = static_cast<UInt64>(::syscall(__NR_gettid));
#else
    if(UNLIKELY(s_currentThreadId == 0))
        s_currentThreadId = ::GetCurrentThreadId();
	// TODO:windows
#endif

	return s_currentThreadId;
}

inline Int32 SystemUtil::GetCurProcessId()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::_getpid();
#else
    return ::getpid();
#endif
}

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS

inline UInt64 SystemUtil::GetFreeMemBySysCall()
{
    struct sysinfo info;
    sysinfo(&info); 

    return info.freeram;
}

inline UInt64 SystemUtil::GetAvailableMem()
{
    std::map<LibString, LibString> memInfo;
    if(!ReadMemInfoDict(memInfo))
    {
        CRYSTAL_TRACE("read mem info fail.");
        return 0;
    }

    return GetAvailableMem(memInfo);
}

inline UInt64 SystemUtil::GetTotalMem()
{
    std::map<LibString, LibString> memInfo;
    if(!ReadMemInfoDict(memInfo))
    {
        CRYSTAL_TRACE("read mem info fail.");
        return 0;
    }

    return GetTotalMem(memInfo);
}

#endif

inline void SystemUtil::LockConsole()
{
    GetConsoleLocker().Lock();
}

inline void SystemUtil::UnlockConsole()
{
    GetConsoleLocker().Unlock();
}

inline void SystemUtil::OutputToConsole(const LibString &outStr)
{
    printf("%s", outStr.c_str());
}

ALWAYS_INLINE Int32 SystemUtil::GetErrNo(bool fromNet)
{
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
     if(fromNet)    // 网络,由于是多线程请调用WSA版本，如socket上的错误
         return ::WSAGetLastError();
 
     return ::GetLastError();
    #else
     return errno;
    #endif
}

ALWAYS_INLINE LibString SystemUtil::GetErrString(Int32 err)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    return strerror(err);
#else
    LibString info;
    HLOCAL hLocal = nullptr;
    const DWORD sysLocale = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    // const DWORD sysLocale = MAKELANGID(LANG_SYSTEM_DEFAULT, SUBLANG_SYS_DEFAULT);
    ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | 
                            FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                        nullptr,
                        err,
                        sysLocale,
                        (PSTR)&hLocal,
                        0,
                        nullptr);
    if (!hLocal)
    {
        HMODULE netDll = LoadLibraryExA("netmsg.dll", nullptr, DONT_RESOLVE_DLL_REFERENCES);
        if (netDll != nullptr)
        {
            ::FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE | 
                                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_IGNORE_INSERTS,
                                netDll,
                                err,
                                sysLocale,
                                (PSTR)&hLocal,
                                0,
                                nullptr);
                                
            ::FreeLibrary(netDll);
        }
    }

    if (hLocal != nullptr)
    {
        PSTR sysErr = (PSTR)::LocalLock(hLocal);

        bool hasCRLF = false;
        const size_t sysErrLen = strlen(sysErr);
        if (sysErrLen >= 2)
            if (sysErr[sysErrLen - 2] == '\r' && 
                sysErr[sysErrLen - 1] == '\n')
                    hasCRLF = true;

        if (hasCRLF)
            sysErr[sysErrLen - 2] = '\0';

        ::LocalUnlock(hLocal);

        info = sysErr;
        ::LocalFree(hLocal);
    }
    else
    {
        info.AppendFormat("Unknown error code:[%d]", err);
    }

    return info;
#endif
}

inline void SystemUtil::ChgWorkDir(const LibString &workDir)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (workDir.length())
        chdir(workDir.c_str());
#endif
}

inline void SystemUtil::YieldScheduler()
{
    std::this_thread::yield();
}

ALWAYS_INLINE void SystemUtil::RelaxCpu()
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
 #if CRYSTAL_TARGET_PLATFORM_LINUX || CRYSTAL_TARGET_PLATFORM_ANDROID || CRYSTAL_TARGET_PLATFORM_MAC
    asm volatile ("rep;nop" : : : "memory");
 #else
    asm volatile ("nop");
 #endif
#else // WINDOWS platform
    YieldProcessor();
#endif // Non-WINDOWS platform
}

KERNEL_END


#endif
