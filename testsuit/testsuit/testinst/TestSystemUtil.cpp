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
 * Date: 2021-02-01 00:32:48
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestSystemUtil.h>


void TestSystemUtil::Run() 
{
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        // 内存情况
        std::cout << "cur process avail mem size:" << KERNEL_NS::SystemUtil::GetAvailPhysMemSize() << std::endl;
        std::cout << "cur process total mem size: " << KERNEL_NS::SystemUtil::GetTotalPhysMemSize() << std::endl;
        std::cout << "cur process mem in used: " << KERNEL_NS::SystemUtil::GetMemoryLoad() << std::endl;
        // 当前进程
        KERNEL_NS::LibString path;
        KERNEL_NS::SystemUtil::GetProgramPath(true,path);
        std::cout <<"current process id:"<<KERNEL_NS::SystemUtil::GetCurProcessId()<< " current process path:" << path.c_str() << std::endl;

        // 遍历进程
        auto hProcModule = KERNEL_NS::SystemUtil::CreateProcessSnapshot();
        auto nPid = KERNEL_NS::SystemUtil::GetFirstProcessPid(hProcModule);
        bool bFirst = true;
        KERNEL_NS::LibString pachCache;
        for(; bFirst ? bFirst : (nPid > 0); nPid = KERNEL_NS::SystemUtil::GetNextProcessPid(hProcModule))
        {
            bFirst = false;
            pachCache.clear();
            Int32 err = KERNEL_NS::SystemUtil::GetProgramPath(false, pachCache, nPid);
            if(err != Status::Success)
            {
                //CRYSTAL_TRACE("GetProgramPath fail pid=[%llu], err[%d]", nPid, err);
                continue;
            }

            std::cout << "process id:" << nPid << " path:" << pachCache.c_str() << std::endl;
        }

        // 内存信息
        KERNEL_NS::ProcessMemInfo memInfo;
        KERNEL_NS::SystemUtil::GetProcessMemInfo(KERNEL_NS::SystemUtil::GetCurProcessHandle(), memInfo);

        #ifdef _DEBUG
            std::cout << "IsProcessExist testsuit_debug.exe:" << KERNEL_NS::SystemUtil::IsProcessExist(KERNEL_NS::LibString("testsuit_debug.exe")) << std::endl;
        #else
            std::cout << "IsProcessExist testsuit.exe:" << KERNEL_NS::SystemUtil::IsProcessExist(KERNEL_NS::LibString("testsuit.exe")) << std::endl;
        #endif

        #else

        // linux 下内存
        std::map<KERNEL_NS::LibString, KERNEL_NS::LibString> memInfo;
        KERNEL_NS::SystemUtil::ReadMemInfoDict(memInfo);
        // 打印内存信息
        for(auto &iterMem:memInfo)
        {
            g_Log->Custom("%s:%s", iterMem.first.c_str(), iterMem.second.c_str());
        }

        g_Log->Custom("no tripUnitOfValue");
        
        memInfo.clear();
        KERNEL_NS::SystemUtil::ReadMemInfoDict(memInfo, false);
        // 打印内存信息
        for(auto &iterMem:memInfo)
            g_Log->Custom("%s:%s", iterMem.first.c_str(), iterMem.second.c_str());

        g_Log->Custom("total mem :%llu\n, available mem :%llu, free mem by sys call:%llu"
        , KERNEL_NS::SystemUtil::GetTotalMem()
        , KERNEL_NS::SystemUtil::GetAvailableMem()
        , KERNEL_NS::SystemUtil::GetFreeMemBySysCall());

        #endif

    std::cout << "cur thread id = " << KERNEL_NS::SystemUtil::GetCurrentThreadId() << std::endl;
    std::cout << "GetCurProgramName = " << KERNEL_NS::SystemUtil::GetCurProgramName() << std::endl;
    std::cout << "GetCurProgramNameWithoutExt = " << KERNEL_NS::SystemUtil::GetCurProgramNameWithoutExt() << std::endl;
    std::cout << "GetCurProcessId = " << KERNEL_NS::SystemUtil::GetCurProcessId() << std::endl;
    std::cout << "LocalMachineEndianType = " << KERNEL_NS::LibEndianType::ToString(KERNEL_NS::LibEndian::GetLocalMachineEndianType()) << std::endl;
    std::cout << "GetErrString = " << KERNEL_NS::SystemUtil::GetErrString(11) << std::endl;

    // 测试环境变量
    std::cout << KERNEL_NS::SystemUtil::GetEnv("TestEnv") << std::endl;
}
