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
 * Date: 2020-12-08 01:02:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Utils/Defs/FindFileInfo.h>
#include <kernel/comp/SmartPtr.h>

#if CRYSTAL_TARGET_PLATFORM_LINUX
    #include <unistd.h>

    // syscall 系统调用获取硬件时钟,单调递增,不随用户调整而变化,不受ntp影响 系统调用会有上下文切换开销
    #include <sys/syscall.h>

    // 包含sysinfo结构体信息
    #include <linux/kernel.h>
    #include <sys/sysinfo.h>
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include <process.h>
 #include <tchar.h>
 #include "Psapi.h"
 #include "tlhelp32.h"
#endif

#include <kernel/common/Buffer.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/DirectoryUtil.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/TranscoderUtil.h>
#include <kernel/common/func.h>
#include <kernel/common/status.h>
#include <kernel/common/statics.h>
#include <kernel/common/Int128.h>


#include <thread>
#include <chrono>
#include <map>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 typedef size_t ssize_t;
#endif

#if CRYSTAL_TARGET_PLATFORM_LINUX
 #include <sys/prctl.h>
 #include <limits.h> // 含有PATH_MAX
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
// defines
// 获取内存状态函数函数原型指针
// typedef   void(WINAPI *__GlobalMemoryStatusExFunc)(LPMEMORYSTATUSEX);

static inline Int32 GetMemoryStatus(MEMORYSTATUSEX &status)
{
    // 载入动态链接库kernel32.dll，返回它的句柄
//     HMODULE hModule;
//     hModule = LoadLibrary("kernel32.dll");
//     if(UNLIKELY(!hModule))
//         return StatusDefs::SystemUtil_GetKernel32HandleFailed;

    // 在kernel32.dll句柄里查找GlobalMemoryStatusEx函数，获取函数指针
//     __GlobalMemoryStatusExFunc globalMemoryStatusEx = (__GlobalMemoryStatusExFunc)GetProcAddress(hModule, "GlobalMemoryStatusEx");
//     if(UNLIKELY(!globalMemoryStatusEx))
//         return StatusDefs::SystemUtil_GetGlobalMemoryStatusExFuncFailed;

//      globalMemoryStatusEx(&status);
// 
//     // 释放链接库句柄
//     FreeLibrary(hModule);

    // 调用函数取得系统的内存情况
    status.dwLength = sizeof(status);
    if(!GlobalMemoryStatusEx(&status))
        return Status::SystemUtil_GetGlobalMemoryStatusExFailed;

    return Status::Success;
}


#endif

KERNEL_BEGIN

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
// windowsdefines
typedef struct
{
    HWND    hwndWindow;     // 窗口句柄
    DWORD   dwProcessID;    // 进程ID
}EnumWindowsArg;


static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    auto *pArg = (EnumWindowsArg *)lParam;

    // 通过窗口句柄取得进程ID
    DWORD  dwProcessID = 0;
    ::GetWindowThreadProcessId(hwnd, &dwProcessID);
    if(dwProcessID == pArg->dwProcessID)
    {
        pArg->hwndWindow = hwnd;
        // 找到了返回FALSE
        return false;
    }

    // 没找到，继续找，返回TRUE
    return true;
}
#endif

KERNEL_END

KERNEL_BEGIN

Int32 SystemUtil::GetProgramPath(bool isCurrentProcess, LibString &processPath, UInt64 pid) 
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    HMODULE hModule = NULL;
    HANDLE hProc = NULL;

    do
    {
        if(UNLIKELY(!isCurrentProcess && !pid))
            return Status::ParamError;

        // 若是当前进程
        Byte8  pathName[MAX_PATH] = {0};
        if(isCurrentProcess)
        {
            if(UNLIKELY(!GetModuleFileNameA(NULL, pathName, MAX_PATH)))
                return Status::SystemUtil_GetModuleFileNameFailed;

            processPath.AppendData(pathName, MAX_PATH);
            processPath.RemoveZeroTail();
            break;
        }

        hProc = OpenProcess(PROCESS_QUERY_INFORMATION, false, static_cast<DWORD>(pid));
        if(UNLIKELY(!hProc))
            return Status::SystemUtil_OpenProcessQueryInfomationFailed;

        hModule = LoadLibrary(TEXT("Kernel32.dll"));
        if(UNLIKELY(!hModule))
            return Status::SystemUtil_LoadKernel32LibraryFailed;

        // 获取QueryFullProcessImageNameA函数
        if(GetProcAddress(hModule, "QueryFullProcessImageNameA"))
        {
            DWORD dwProcPathLen = MAX_PATH / sizeof(Byte8);
            if(!QueryFullProcessImageNameA(hProc, 0, pathName, &dwProcPathLen))
                return Status::SystemUtil_QueryFullProcessImageNameFailed;

            processPath.AppendData(pathName, MAX_PATH);
            processPath.RemoveZeroTail();
            break;
        }

        // 获取进程带驱动器名的路径（驱动器名：\\Device\\HardwareVolume1）
        if(!::GetProcessImageFileNameA(hProc, pathName, MAX_PATH))
            return Status::SystemUtil_GetProcessImageFileNameFailed;

        // 遍历确认驱动器名对应的盘符名
        Byte8   volNameDev[MAX_PATH] = {0};
        Byte8   volName[MAX_PATH] = {0};
        strcat_s(volName, MAX_PATH, "A:");
        bool isFound = false;
        for(; *volName <= 'Z'; (*volName)++)
        {
            // 获取盘符
            if(!QueryDosDeviceA(volName, volNameDev, MAX_PATH))
            {
                auto lastError = GetLastError();
                if(lastError == 2)
                    continue;

                return Status::SystemUtil_QueryDosDeviceError;
            }

            // 确认是否驱动器名一样
            if(strncmp(pathName, volNameDev, ::strlen(volNameDev)) == 0)
            {
                isFound = true;
                break;
            }
        }

        if(!isFound)
            return Status::SystemUtil_GetDriveError;

        processPath.AppendData(volName, ::strlen(volName));
        processPath.RemoveZeroTail();
        processPath.AppendData(pathName + ::strlen(volNameDev), ::strlen(pathName) - ::strlen(volNameDev));
        processPath.RemoveZeroTail();
    } while(0);

    if(hModule)
        FreeLibrary(hModule);

    if(hProc)
        CloseHandle(hProc);

    return Status::Success;
#else

    ssize_t ret = -1;
    char buf[PATH_MAX + 1];
    if(isCurrentProcess)
    {
        if((ret = readlink("/proc/self/exe", buf, PATH_MAX)) == -1)
            return Status::SystemUtil_GetProcNameFail;
    }
    else
    {
        BUFFER64 proc = {};
        sprintf(proc, "/proc/%llu/exe", pid);
        if((ret = readlink(proc, buf, PATH_MAX)) == -1)
            return Status::SystemUtil_GetProcNameFail;
    }

    buf[ret] = '\0';
    processPath = buf;
    processPath.RemoveZeroTail();
    return Status::Success;
#endif
}

LibString SystemUtil::GetCurProgramName()
{
    LibString path;
    SystemUtil::GetProgramPath(true, path);
    return DirectoryUtil::GetFileNameInPath(path);
}

LibString SystemUtil::GetCurProgramNameWithoutExt()
{
    return FileUtil::ExtractFileWithoutExtension(SystemUtil::GetCurProgramName().RemoveZeroTail());
}

LibString SystemUtil::GetCurProgRootPath()
{
    LibString path;
    SystemUtil::GetProgramPath(true, path);
    return DirectoryUtil::GetFileDirInPath(path);
}

Int32 SystemUtil::CloseProcess(UInt64 processId, ULong *lastError)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if(!TerminateProcess(OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, false, static_cast<DWORD>(processId)), 0))
    {
        if(lastError)
            *lastError = GetLastError();

        return Status::Failed;
    }
#else
    auto ret = kill(processId, SIGKILL);
    if(ret != 0)
    {
        perror("kill process fail");
        return Status::Failed;
    }
#endif

    return Status::Success;
}

Int32 SystemUtil::SendCloseMsgToProcess(UInt64 processId, ULong *lastError)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    auto hwd = GetWindowHwndByPID((DWORD)processId);
    auto ret = ::SendMessage(hwd, WM_CLOSE, 0, 0);
    if(ret != 0)
    {
        if(lastError)
        {
            *lastError = static_cast<ULong>(::GetLastError());
        }
        return Status::Failed;
    }

    // if(!AllocConsole())
    // {
    //     if(lastError)
    //     {
    //         *lastError = ::GetLastError();
    //     }
    //     return Status::Failed;
    // }
    // if(!AttachConsole((DWORD)processId))
    // {
    //     if(lastError)
    //     {
    //         *lastError = ::GetLastError();
    //     }
    //     return Status::Failed;
    // }
    // SetConsoleCtrlHandler(NULL, TRUE);
    // if(!GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0))
    // {
    //     if(lastError)
    //     {
    //         *lastError = ::GetLastError();
    //     }
    //     return Status::Failed;
    // }
    // FreeConsole();

    // auto h = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, (DWORD)processId);
    // if(!h)
    // {
    //     if(lastError)
    //     {
    //         *lastError = static_cast<ULong>(::GetLastError());
    //     }
    //     return  Status::Failed;
    // }

    // if(!GenerateConsoleCtrlEvent(CTRL_C_EVENT, (DWORD)processId))
    // {
    //     if(lastError)
    //     {
    //         *lastError = static_cast<ULong>(::GetLastError());
    //     }

    //     CloseHandle(h);

    //     return Status::Failed;
    // }

    // CloseHandle(h);
#endif

    return Status::Success;
}

UInt64 SystemUtil::GetMainThreadId(UInt64 processId)
{
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
    static UInt64 s_mainThreadId = 0;
    if(UNLIKELY(s_mainThreadId == 0))
    {
        do
        {
            // 获取进程的主线程ID
            THREADENTRY32 te;       // 线程信息
            te.dwSize = sizeof(THREADENTRY32);
            auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); // 系统所有线程快照
            if(!hSnapshot)
                break;

            if(Thread32First(hSnapshot, &te))       // 第一个线程
            {
                do
                {
                    if(processId == te.th32OwnerProcessID)      // 认为找到的第一个该进程的线程为主线程
                    {
                        s_mainThreadId = (UInt64)te.th32ThreadID;
                        break;
                    }
                }while(Thread32Next(hSnapshot, &te));           // 下一个线程
            }

            CloseHandle(hSnapshot); // 删除快照
        } while (false);
        
    }

    return s_mainThreadId;
    #else
        return processId;
    #endif
}

UInt64 SystemUtil::GetCurProcessMainThreadId()
{
    return GetMainThreadId(static_cast<UInt64>(GetCurProcessId()));
}

bool SystemUtil::GetProcessIdList(const LibString &processName, std::map<UInt64, LibString> &processIdRefNames, bool isLikely, bool isMatchPath)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    // 遍历进程
    auto hProcModule = CreateProcessSnapshot();
    auto pid = GetFirstProcessPid(hProcModule);
    bool isFirst = true;
    KERNEL_NS::LibString pachCache;
    for(; isFirst ? isFirst : (pid > 0); pid = GetNextProcessPid(hProcModule))
    {
        isFirst = false;
        pachCache.clear();
        if(GetProgramPath(false, pachCache, pid) != Status::Success)
            continue;

        const auto &dirPart = DirectoryUtil::GetFileDirInPath(processName.c_str());
        const auto &namePart = DirectoryUtil::GetFileNameInPath(processName.c_str());

        const auto &wholePathPart = DirectoryUtil::GetFileDirInPath(pachCache.c_str());
        const auto &wholeNamePart = DirectoryUtil::GetFileNameInPath(pachCache.c_str());

        if(isMatchPath)
        {
            if(wholePathPart.GetRaw().find(dirPart.GetRaw()) == std::string::npos)
                continue;
        }

        if(isLikely)
        {
            auto iterExist = wholeNamePart.GetRaw().find(namePart.GetRaw());
            if(iterExist != std::string::npos)
            {
                processIdRefNames[pid] = pachCache;
            }
        }
        else
        {
            if(namePart == wholeNamePart)
            {
                processIdRefNames[pid] = pachCache;
            }
        }
    }

    if(hProcModule)
        CloseHandle(hProcModule);

    return !processIdRefNames.empty();
#else
    DirectoryUtil::TraverseDirRecursively("/proc", [&processIdRefNames, &processName, isLikely, isMatchPath](const KERNEL_NS::FindFileInfo &fileInfo, bool &isParentDirContinue){

            if(!KERNEL_NS::FileUtil::IsDir(fileInfo))
                return true;

            if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
                g_Log->Info(LOGFMT_NON_OBJ_TAG(SystemUtil, "traval dir:%s, file:%s"), fileInfo._rootPath.c_str(), fileInfo._fileName.c_str());
            if(fileInfo._rootPath != "/proc")
                return true;

            // 匹配数值
            if(!fileInfo._fileName.isdigit())
                return true;
            
            // 读取exe
            ssize_t ret = -1;
            char buf[PATH_MAX + 1];

            BUFFER64 proc = {};
            sprintf(proc, "/proc/%s/exe", fileInfo._fileName.c_str());
            if((ret = readlink(proc, buf, PATH_MAX)) == -1)
                return true;

            buf[ret] = '\0';
            LibString procPath = buf;
            procPath.RemoveZeroTail();

            const auto &dirPart = DirectoryUtil::GetFileDirInPath(processName.c_str());
            const auto &namePart = DirectoryUtil::GetFileNameInPath(processName.c_str());

            const auto &wholePathPart = DirectoryUtil::GetFileDirInPath(procPath.c_str());
            const auto &wholeNamePart = DirectoryUtil::GetFileNameInPath(procPath.c_str());

            if(isMatchPath)
            {
                if(wholePathPart.GetRaw().find(dirPart.GetRaw()) == std::string::npos)
                    return true;
            }

            const UInt64 pid = StringUtil::StringToUInt64(fileInfo._fileName.c_str());
            if(isLikely)
            {
                auto iterExist = wholeNamePart.GetRaw().find(namePart.GetRaw());
                if(iterExist != std::string::npos)
                {
                    processIdRefNames[pid] = procPath;
                }
            }
            else
            {
                if(namePart == wholeNamePart)
                {
                    processIdRefNames[pid] = procPath;
                }
            }

            return true;
    }, 1);

    return !processIdRefNames.empty();
#endif
}

bool SystemUtil::IsProcessExist(const LibString &processName)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    // 遍历进程
    auto hProcModule = CreateProcessSnapshot();
    auto pid = GetFirstProcessPid(hProcModule);
    bool isFirst = true;
    KERNEL_NS::LibString pachCache;
    for(; isFirst ? isFirst : (pid > 0); pid = GetNextProcessPid(hProcModule))
    {
        isFirst = false;
        pachCache.clear();
        if(GetProgramPath(false, pachCache, pid) != Status::Success)
            continue;

        auto iterExist = pachCache.GetRaw().find(processName.GetRaw());
        if(iterExist != std::string::npos)
        {
            if(hProcModule)
                CloseHandle(hProcModule);
            return true;
        }
    }

    if(hProcModule)
        CloseHandle(hProcModule);
    return false;
#else
    bool isFound = false;
    DirectoryUtil::TraverseDirRecursively("/proc", [&processName, &isFound](const KERNEL_NS::FindFileInfo &fileInfo, bool &isParentDirContinue){

            if(!KERNEL_NS::FileUtil::IsDir(fileInfo))
                return true;

            if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
                g_Log->Info(LOGFMT_NON_OBJ_TAG(SystemUtil, "traval dir:%s, file:%s"), fileInfo._rootPath.c_str(), fileInfo._fileName.c_str());
            if(fileInfo._rootPath != "/proc")
                return true;

            // 匹配数值
            if(!fileInfo._fileName.isdigit())
                return true;
            
            // 读取exe
            ssize_t ret = -1;
            char buf[PATH_MAX + 1];

            BUFFER64 proc = {};
            sprintf(proc, "/proc/%s/exe", fileInfo._fileName.c_str());
            if((ret = readlink(proc, buf, PATH_MAX)) == -1)
                return true;

            buf[ret] = '\0';
            LibString procPath = buf;
            procPath.RemoveZeroTail();

            auto iter = procPath.GetRaw().find(processName.GetRaw());
            if(iter != std::string::npos)
            {
                isParentDirContinue = false;
                isFound = true;
                return false;
            }

            return true;
    }, 1);

    return isFound;
#endif
}

bool SystemUtil::IsProcessExist(UInt64 processId)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    // 遍历进程
    auto hProcModule = CreateProcessSnapshot();
    auto pid = GetFirstProcessPid(hProcModule);
    bool isFirst = true;
    KERNEL_NS::LibString pachCache;
    for(; isFirst ? isFirst : (pid > 0); pid = GetNextProcessPid(hProcModule))
    {
        isFirst = false;
        if(pid == processId)
            return true;
    }

    if(hProcModule)
        CloseHandle(hProcModule);
    return false;
#else
    bool isFound = false;
    DirectoryUtil::TraverseDirRecursively("/proc", [&isFound, processId](const KERNEL_NS::FindFileInfo &fileInfo, bool &isParentDirContinue){

            if(!KERNEL_NS::FileUtil::IsDir(fileInfo))
                return true;

            if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
                g_Log->Info(LOGFMT_NON_OBJ_TAG(SystemUtil, "traval dir:%s, file:%s"), fileInfo._rootPath.c_str(), fileInfo._fileName.c_str());
            if(fileInfo._rootPath != "/proc")
                return true;

            // 匹配数值
            if(!fileInfo._fileName.isdigit())
                return true;
            
            // 读取exe
            ssize_t ret = -1;
            char buf[PATH_MAX + 1];

            BUFFER64 proc = {};
            sprintf(proc, "/proc/%s/exe", fileInfo._fileName.c_str());
            if((ret = readlink(proc, buf, PATH_MAX)) == -1)
                return true;

            buf[ret] = '\0';
            if(KERNEL_NS::StringUtil::StringToUInt64(fileInfo._fileName.c_str()) == processId)
            {
                isFound = true;
                isParentDirContinue = false;
                return false;
            }

            return true;
    }, 1);

    return isFound;
#endif
}

bool SystemUtil::SetCurrentThreadName(const LibString &threadName, LibString &err)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    Int32 ret = 0;
    if(threadName.size() > 16)
    {
        ret = prctl(PR_SET_NAME, threadName.GetRaw().substr(0, 16).c_str());
    }
    else
    {
        ret = prctl(PR_SET_NAME, threadName.c_str());
    }
   // auto threadId = SystemUtil::GetCurrentThreadId();
   // auto ret = ::pthread_setname_np(threadId, threadName.c_str());
   if(ret != 0)
   {
      err = GetErrString(ret);
      return false;
   }

   return true;
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    // auto handle = ::GetCurrentThread();
    // KERNEL_NS::LibString wideStr;
    // auto st = TranscoderUtil::MultiByteToWideChar("UTF8", threadName, wideStr);
    // if(st != Status::Success)
    // {
    //     err = GetErrString(GetErrNo());
    //     return false;
    // }

    // auto ret = SetThreadDescription(handle, reinterpret_cast<const wchar *>(wideStr.data()));
    // if(FAILED(ret))
    // {
    //     err = GetErrString(GetErrNo());
    //     return false;
    // }

    return true;
#endif
}

bool SystemUtil::GetCurrentThreadName(LibString &threadName, LibString &err)
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    threadName.resize(256);
    auto ret = prctl(PR_GET_NAME, threadName.data());
   // auto threadId = SystemUtil::GetCurrentThreadId();
   // auto ret = ::pthread_setname_np(threadId, threadName.c_str());
   if(ret != 0)
   {
      err = GetErrString(ret);
      return false;
   }

    threadName.RemoveZeroTail();
   return true;
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    auto handle = ::GetCurrentThread();
    wchar *p = NULL;
    auto ret = GetThreadDescription(handle, &p);
    if(FAILED(ret))
    {
        err = GetErrString(GetErrNo());
        return false;
    }

    if(!p)
    {
        err = GetErrString(GetErrNo());
        return false;
    }

    KERNEL_NS::LibString wideStr;
    wideStr.AppendData(reinterpret_cast<Byte8 *>(p), ::lstrlenW(p) * sizeof(wchar));
    LocalFree(p);
    
    auto st = TranscoderUtil::WideCharToMultiByte("UTF8", wideStr, threadName);
    if(st != Status::Success)
    {
        err = GetErrString(GetErrNo());
        return false;
    }

    threadName.RemoveZeroTail();

    return true;
#endif
}

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

UInt64 SystemUtil::GetAvailPhysMemSize()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    MEMORYSTATUSEX status;
    auto ret = GetMemoryStatus(status);
    if(ret != Status::Success)
        return 0;

    return status.ullAvailPhys;

#else
    // TODO:Linux
#endif
}

UInt64 SystemUtil::GetTotalPhysMemSize()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    MEMORYSTATUSEX status;
    auto ret = GetMemoryStatus(status);
    if(ret != Status::Success)
        return 0;

    return status.ullTotalPhys;
#else
    // TODO:Linux
#endif
}

UInt64 SystemUtil::GetMemoryLoad()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    MEMORYSTATUSEX status;
    auto ret = GetMemoryStatus(status);
    if(ret != Status::Success)
        return 0;

    return status.dwMemoryLoad;
#else
    // TODO:Linux
#endif
}
// 进程占用内存信息
bool SystemUtil::GetProcessMemInfo(HANDLE processHandle, ProcessMemInfo &info)
{
    PROCESS_MEMORY_COUNTERS_EX processInfo = {};
    if(!GetProcessMemoryInfo(processHandle, (PROCESS_MEMORY_COUNTERS *)&processInfo, sizeof(PROCESS_MEMORY_COUNTERS_EX)))
        return false;

    info._maxHistorySetSize = processInfo.PeakWorkingSetSize;
    info._curSetSize = processInfo.WorkingSetSize;
    info._maxHistoryPagedPoolUsage = processInfo.QuotaPeakPagedPoolUsage;
    info._pagedPoolUsage = processInfo.QuotaPagedPoolUsage;
    info._maxHistoryNonPagedPoolUsage = processInfo.QuotaPeakNonPagedPoolUsage;
    info._curNonPagedPoolUsage = processInfo.QuotaNonPagedPoolUsage;
    info._curPageFileUsage = processInfo.PagefileUsage;
    info._maxHistoryPageFileUsage = processInfo.PeakPagefileUsage;
    info._processAllocMemoryUsage = processInfo.PrivateUsage;
    return true;
}

HANDLE SystemUtil::CreateProcessSnapshot()
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(INVALID_HANDLE_VALUE == hSnapshot)
        return NULL;

    return hSnapshot;
}

UInt64 SystemUtil::GetFirstProcessPid(HANDLE &hSnapshot)
{
    PROCESSENTRY32 pe = {0};
    pe.dwSize = sizeof(pe);

    Process32First(hSnapshot, &pe);
    return pe.th32ProcessID;
}

UInt64 SystemUtil::GetNextProcessPid(HANDLE &hSnapshot)
{
    PROCESSENTRY32 pe = {0};
    pe.dwSize = sizeof(pe);
    if(!Process32Next(hSnapshot, &pe))
        return 0;

    return pe.th32ProcessID;
}

// 获取进程句柄
HANDLE SystemUtil::GetCurProcessHandle()
{
    return ::GetCurrentProcess();
}

HWND SystemUtil::GetWindowHwndByPID(DWORD dwProcessID)
{
    HWND hwndRet = NULL;
    EnumWindowsArg ewa;
    ewa.dwProcessID = dwProcessID;
    ewa.hwndWindow = NULL;
    EnumWindows(KERNEL_NS::EnumWindowsProc, (LPARAM)&ewa);
    if(ewa.hwndWindow)
    {
        hwndRet = ewa.hwndWindow;
    }
    return hwndRet;
}

void SystemUtil::BringWindowsToTop(HWND curWin)
{
    ::BringWindowToTop(curWin);
    ::ShowWindow(curWin, SW_SHOW);
}

// 弹窗
void SystemUtil::MessageBoxPopup(const LibString &title, const LibString &content)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    auto hwnd = GetWindowHwndByPID(GetCurProcessId());
    ::MessageBoxA(hwnd, content.c_str(), title.c_str(), MB_ABORTRETRYIGNORE);
#endif
}

void SystemUtil::GetCallingThreadCpuInfo(UInt16 &cpuGroup, Byte8 &cpuNumber)
{
    PROCESSOR_NUMBER processorInfo = {};
    GetCurrentProcessorNumberEx(&processorInfo);
    cpuGroup = processorInfo.Group;
    cpuNumber = processorInfo.Number;
}

#else

 UInt64 SystemUtil::GetAvailableMem(const std::map<LibString, LibString> &memInfo)
 {
    auto iter = memInfo.find("MemAvailable");
    if(iter == memInfo.end())
    {
        CRYSTAL_TRACE("cant find MemAvailable option in meminfo file.");
        return 0;
    }

    return StringUtil::StringToUInt64(iter->second.c_str());
 }

// 总内存
UInt64 SystemUtil::GetTotalMem(const std::map<LibString, LibString> &memInfo)
{
    auto iter = memInfo.find("MemTotal");
    if(iter == memInfo.end())
    {
        CRYSTAL_TRACE("cant find MemTotal option in meminfo file.");
        return 0;
    }

    return StringUtil::StringToUInt64(iter->second.c_str());
}

bool SystemUtil::ReadMemInfoDict(std::map<LibString, LibString> &memInfo, bool tripUnitOfValue)
{
    auto fp = FileUtil::OpenFile("/proc/meminfo", false, "r");
    if(!fp)
    {
        CRYSTAL_TRACE("open /proc/meminfo fail %s", GetErrString(errno).c_str());
        return false;
    }

    // 逐行读取
    LibString line;
    while(FileUtil::ReadOneLine(*fp, line))
    {
        // 分离key value
        line.strip();
        auto arr = line.Split(':');

        // value处理
        auto &value = arr[1];
        value.strip();

        //  映射字典
        if(tripUnitOfValue)
        {
            // 真正的value在第0个
            const auto &valueArr = value.Split(DEF_STRIP_CHARS, -1, true);
            memInfo[arr[0]] = valueArr[0];
            // CRYSTAL_TRACE("key:%s, value:%s", arr[0].c_str(), valueArr[0].c_str());
        }
        else
        {// 包含了单位等
            memInfo[arr[0]] = value;
            // CRYSTAL_TRACE("key:%s, value:%s", arr[0].c_str(), value.c_str());
        }
        
        line.clear();
    }

    FileUtil::CloseFile(*fp);

    return true;
}


void SystemUtil::GetLinuxProcessProcInfo(LinuxProcInfo &info)
{
    ::memset(&info, 0, sizeof(info));

    auto pid = GetCurProcessId();
    KERNEL_NS::LibString path;
    path.AppendFormat("/proc/%d/status", pid);
    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(path.c_str(), false, "r");
    if(!fp)
    {
        CRYSTAL_TRACE("open %s fail %s", path.c_str(), GetErrString(GetErrNo()).c_str());
        return;
    }
    fp.SetClosureDelegate([](void *p){
        KERNEL_NS::FileUtil::CloseFile(*KERNEL_NS::KernelCastTo<FILE>(p));
    });

    // 逐行读取
    LibString line;
    while(FileUtil::ReadOneLine(*fp, line))
    {
        // 分离key value
        line.strip();
        auto arr = line.Split(':');

        // value处理
        auto &key = arr[0];
        auto &value = arr[1];
        value.strip();
        key.strip();

        if(key == "FDSize")
        {
            info._fdSize = KERNEL_NS::StringUtil::StringToUInt64(value.c_str());
        }
        else if(key == "VmPeak")
        {
            info._vmPeak = KERNEL_NS::StringUtil::StringToUInt64(value.c_str());
        }
        else if(key == "VmSize")
        {
            info._vmSize = KERNEL_NS::StringUtil::StringToUInt64(value.c_str());
        }
        else if(key == "VmHWM")
        {
            info._vmHwm = KERNEL_NS::StringUtil::StringToUInt64(value.c_str());
        }
        else if(key == "VmRSS")
        {
            info._vmRss = KERNEL_NS::StringUtil::StringToUInt64(value.c_str());
        }
        else if(key == "VmData")
        {
            info._vmData = KERNEL_NS::StringUtil::StringToUInt64(value.c_str());
        }        
        else if(key == "VmStk")
        {
            info._vmStk = KERNEL_NS::StringUtil::StringToUInt64(value.c_str());
        }    
        else if(key == "VmExe")
        {
            info._vmExe = KERNEL_NS::StringUtil::StringToUInt64(value.c_str());
        }    
        else if(key == "VmLib")
        {
            info._vmLib = KERNEL_NS::StringUtil::StringToUInt64(value.c_str());
        }   
        else if(key == "VmSwap")
        {
            info._vmSwap = KERNEL_NS::StringUtil::StringToUInt64(value.c_str());
        }  
        else if(key == "VmPTE")
        {
            info._vmPTE = KERNEL_NS::StringUtil::StringToUInt64(value.c_str());
        }  

        line.clear();
    }
}
#endif

Int32 SystemUtil::SetConsoleColor(Int32 color)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if(::SetConsoleTextAttribute(handle, color) == 0)
    {
        ASSERT(!"SetConsoleTextAttribute failed");
        return Status::Failed;
    }
#endif

    return Status::Success;
}

Int32 SystemUtil::GetConsoleColor()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    if(::GetConsoleScreenBufferInfo(handle, &info) == 0)
    {
        ASSERT(!"GetConsoleScreenBufferInfo failed");
        return Status::Error;
    }

    return info.wAttributes;
#endif

    return 0;
}

LibString SystemUtil::GetErrString(Int32 err)
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

void SystemUtil::TurnDaemon(const std::string &stdIoRedirect /*= ""*/, const std::string &workDir)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    // 避免僵尸进程
    ::signal(SIGCHLD, SIG_IGN);
    // 杀死会话组组长时进程组所有进程都会收到该信号,该信号默认处理是终止进程
    ::signal(SIGHUP, SIG_IGN);

    // 成为新会话组组长并脱离控制终端,且以后不可再打开控制终端
    if (fork() != 0) {
        exit(0);
    }
    setsid();
    if (fork() != 0) {
        exit(0);
    }

    // 修改默认文件创建权限与工作目录(默认设置到根目录) 改变当前工作路径的作用是 在做相对路径的时候 它会在当前路径下起作用
    umask(0);
    if(workDir.empty())
        chdir("/");
    else
        chdir(workDir.c_str());

    // 关闭打开的文件 当前最大打开的文件描述符
    int maxFd = sysconf(_SC_OPEN_MAX);
    if (maxFd == -1)
        maxFd = BD_MAX_CLOSE;

    // 关闭所有文件描述符
    for (int i = 0; i < maxFd; ++i)
        close(i);

    // 重定向标准输入输出
    close(STDIN_FILENO);
    // 默认重定向到 /dev/null 若有指定则重定向到 stdIoRedirect
    std::string ioRedirect = stdIoRedirect.length() ? stdIoRedirect : "/dev/null";
    // LibString ioRedirect = stdIoRedirect.length() ? (LibString("/") + DirectoryUtil::GetFileNameInPath(stdIoRedirect))
    //     : "/dev/null";

    // 此时描述符一定是0(STDIN_FILENO)
    int fd = open(ioRedirect.c_str(), O_RDWR | O_CREAT | O_TRUNC);
    if (fd != STDIN_FILENO)
        exit(0);
    // 标准输出与ioRedirect文件关联 并返回 STDOUT_FILENO
    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
        exit(0);
    // 标准错误输出与ioRedirect文件关联并返回 STDERR_FILENO
    if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
        exit(0);
#endif
}

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #define LibPopen ::_popen
 #define LibPClose ::_pclose
#else
 #define LibPopen ::popen
 #define LibPClose ::pclose
#endif

bool SystemUtil::Exec(const LibString &cmd, Int32 &err, LibString &outputInfo)
{
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
     auto outFp = LibPopen(cmd.c_str(), "rb");
    #else
     // linux 下必须是"r"
     auto outFp = LibPopen(cmd.c_str(), "r");
    #endif

    if(!outFp)
    {
        err = GetErrNo();
        outputInfo = GetErrString(err);
        return false;
    }

    // 读取输出
    FileUtil::ReadFile(*outFp, outputInfo);

    // 获取退出码
    auto ret = LibPClose(outFp);
    if(ret < 0 )
    {
        err = GetErrNo();
        outputInfo = GetErrString(err);
        return false;
    }

    // 非0表示不成功
    if (ret != 0)
    {
        err = ret;
        outputInfo = GetErrString(err);
        return false;
    }

    return true;
}

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS

Int32 SystemUtil::SetProcessFileDescriptLimit(Int32 resourceId, Int64 softLimit, Int64 hardLimit, LibString &errInfo)
{
    rlimit64 limitInfo;
    ::memset(&limitInfo, 0, sizeof(limitInfo));

    if(softLimit < 0 && hardLimit < 0)
    {
        return Status::Success;
    }
    
    if(softLimit > 0)
    {
        limitInfo.rlim_cur = static_cast<rlim_t>(softLimit);
    }

    if(hardLimit > 0)
    {
        limitInfo.rlim_max = static_cast<rlim_t>(hardLimit);
    }

    Int32 err = ::setrlimit64(resourceId, &limitInfo);
    if(err != 0)
    {
        err = GetErrNo();
        errInfo = GetErrString(err);
        return err;
    }

    return Status::Success;
}

Int32 SystemUtil::GetProcessFileDescriptLimit(Int32 resourceId, Int64 &softLimit, Int64 &hardLimit, LibString &errInfo)
{
    rlimit64 limitInfo;
    ::memset(&limitInfo, 0, sizeof(limitInfo));
    Int32 err = ::getrlimit64(resourceId, &limitInfo);
    if(err != 0)
    {
        err = GetErrNo();
        errInfo = GetErrString(err);
        return err;
    }

    softLimit = static_cast<Int64>(limitInfo.rlim_cur);
    hardLimit = static_cast<Int64>(limitInfo.rlim_max);

    return Status::Success;
}
#endif

void SystemUtil::ThreadSleep(UInt64 milliSec, UInt64 microSec)
{
    std::chrono::microseconds t(milliSec * TimeDefs::MICRO_SECOND_PER_MILLI_SECOND + microSec);
    std::this_thread::sleep_for(t);
}

UInt64 SystemUtil::GetCurrentThreadId()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR UInt64 s_currentThreadId = 0;

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

Int32 SystemUtil::GetCurProcessId()
{
    static Int32 s_pid = 0;
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if(s_pid == 0)
        s_pid =  static_cast<Int32>(::_getpid());
    return s_pid;
#else
    if(s_pid == 0)
        s_pid = static_cast<Int32>(::getpid());
    return s_pid;
#endif
}

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS

UInt64 SystemUtil::GetFreeMemBySysCall()
{
    struct sysinfo info;
    sysinfo(&info); 

    return info.freeram;
}

UInt64 SystemUtil::GetAvailableMem()
{
    std::map<LibString, LibString> memInfo;
    if(!ReadMemInfoDict(memInfo))
    {
        CRYSTAL_TRACE("read mem info fail.");
        return 0;
    }

    return GetAvailableMem(memInfo);
}

UInt64 SystemUtil::GetTotalMem()
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

void SystemUtil::LockConsole()
{
    GetConsoleLocker().Lock();
}

void SystemUtil::UnlockConsole()
{
    GetConsoleLocker().Unlock();
}

void SystemUtil::OutputToConsole(const LibString &outStr)
{
    printf("%s", outStr.c_str());
}

Int32 SystemUtil::GetErrNo(bool fromNet)
{
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
     if(fromNet)    // 网络,由于是多线程请调用WSA版本，如socket上的错误
         return ::WSAGetLastError();
 
     return ::GetLastError();
    #else
     return errno;
    #endif
}

void SystemUtil::ChgWorkDir(const LibString &workDir)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    if (workDir.length())
        chdir(workDir.c_str());
#endif
}

void SystemUtil::YieldScheduler()
{
    std::this_thread::yield();
}

void SystemUtil::RelaxCpu()
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

