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

#include <kernel/kernel_export.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/Defs/SystemUtilDef.h>
#include <kernel/common/timedefs.h>
#include <kernel/common/macro.h>
#include <map>

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
    static UInt64 GetCurProcessMainThreadId();

    // 找进程id，isLikely:是否模糊匹配
    static bool GetProcessIdList(const LibString &processName, std::map<UInt64, LibString> &processIdRefNames, bool isLikely = true, bool isMatchPath = false);
    // 遍历进程判断某进程是否在进程列表
    static bool IsProcessExist(const LibString &processName);
    static bool IsProcessExist(UInt64 processId);
    
    // 设置线程名
    static bool SetCurrentThreadName(const LibString &threadName, LibString &err);
    static bool GetCurrentThreadName(LibString &threadName, LibString &err);


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

KERNEL_END


#endif
