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
 * Date: 2021-03-20 22:00:20
 * Author: Eric Yonng
 * Description: 
 *              1.测试io重定向
 *              2.测试守护进程
 *              3.测试进程fork后原进程的对象如log等是否会拷贝到新的进程 fork后原进程的资源会被拷贝到新进程
*/

#include <pch.h>
#include <testsuit/testinst/TestDaemon.h>

#define TEST_DAEMON_TIME 120

void TestDaemon::Run() 
{
    auto curProgPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    CRYSTAL_TRACE("CUR PATH=%s", curProgPath.c_str());

    // auto stdiolog = curProgPath + "stdio.log";
    // KERNEL_NS::SystemUtil::TurnDaemon(stdiolog, curProgPath);
    CRYSTAL_TRACE("hello world!");

    // 写入pid文件
    const auto &progName = KERNEL_NS::SystemUtil::GetCurProgramNameWithoutExt();
    auto pid = KERNEL_NS::SystemUtil::GetCurProcessId();
    KERNEL_NS::LibString pidFile;
    pidFile.AppendFormat("./%s.pid", progName.c_str());
    auto fp = KERNEL_NS::FileUtil::OpenFile(pidFile.c_str(), true, "wb+");
    KERNEL_NS::LibString content;
    content.AppendFormat("%d", pid);
    KERNEL_NS::FileUtil::WriteFile(*fp, content);
    KERNEL_NS::FileUtil::CloseFile(*fp);
    CRYSTAL_TRACE("write pid[%d] to pid file[%s]", pid, pidFile.c_str());
    
    Int64 interval = TEST_DAEMON_TIME;
    while (--interval >= 0)
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        CRYSTAL_TRACE("test daemon interval=%lld", interval);
        g_Log->Sys(LOGFMT_NON_OBJ_TAG(TestDaemon, "process id[%d], process name[%s] is runing background")
        , pid, progName.c_str());
    }

    CRYSTAL_TRACE("will finish test daemon!");
    
}
