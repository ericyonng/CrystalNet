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
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/DirectoryUtil.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/StringUtil.h>


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
            if(UNLIKELY(!GetModuleFileName(NULL, pathName, MAX_PATH)))
                return Status::SystemUtil_GetModuleFileNameFailed;

            processPath.AppendData(pathName, MAX_PATH);
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
            if(!QueryFullProcessImageName(hProc, 0, pathName, &dwProcPathLen))
                return Status::SystemUtil_QueryFullProcessImageNameFailed;

            processPath.AppendData(pathName, MAX_PATH);
            break;
        }

        // 获取进程带驱动器名的路径（驱动器名：\\Device\\HardwareVolume1）
        if(!::GetProcessImageFileName(hProc, pathName, MAX_PATH))
            return Status::SystemUtil_GetProcessImageFileNameFailed;

        // 遍历确认驱动器名对应的盘符名
        Byte8   volNameDev[MAX_PATH] = {0};
        Byte8   volName[MAX_PATH] = {0};
        _tcscat_s(volName, MAX_PATH, TEXT("A:"));
        bool isFound = false;
        for(; *volName <= _T('Z'); (*volName)++)
        {
            // 获取盘符
            if(!QueryDosDevice(volName, volNameDev, MAX_PATH))
            {
                auto lastError = GetLastError();
                if(lastError == 2)
                    continue;

                return Status::SystemUtil_QueryDosDeviceError;
            }

            // 确认是否驱动器名一样
            if(_tcsncmp(pathName, volNameDev, _tcslen(volNameDev)) == 0)
            {
                isFound = true;
                break;
            }
        }

        if(!isFound)
            return Status::SystemUtil_GetDriveError;

        processPath.AppendData(volName, _tcslen(volName));
        processPath.AppendData(pathName + _tcslen(volNameDev), _tcslen(pathName) - _tcslen(volNameDev));
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

Int32 SystemUtil::CloseProcess(Int32 processId, ULong *lastError)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if(!TerminateProcess(OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, false, processId), 0))
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
            return true;
    }

    return false;
#else
    // TODO:linux
    return false;
#endif
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

KERNEL_END

