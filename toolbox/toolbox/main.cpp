/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2024-05-11 14:51:29
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <toolbox/ToolboxIni.h>

#include <toolbox/scan_reason/ScanReason.h>
#include <toolbox/scan_player_count/ScanPlayerCount.h>

class LibTestLog : public KERNEL_NS::LibLog
{
public:
    LibTestLog() {}
    ~LibTestLog() {}
};

class LogFactory : public KERNEL_NS::ILogFactory
{
public:
    virtual KERNEL_NS::ILog *Create()
    {
        return new LibTestLog();
    }
};

int main(int argc, char const *argv[])
{
    // 1.初始化内核
    printf("/*********************************************/!\n");
    printf("/*           Hello Crystal Net Toolbox!      */\n");
    printf("/*********************************************/!\n\n");

    LogFactory logFactory;
    KERNEL_NS::LibString programPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    KERNEL_NS::LibString logIniPath;
    logIniPath = programPath + "/ini/";
    KERNEL_NS::SystemUtil::GetProgramPath(true, programPath);
    Int32 err = KERNEL_NS::KernelUtil::Init(&logFactory, "LogCfg.ini", NULL, s_logIniContent, s_consoleIniContent);
    if(err != Status::Success)
    {
        CRYSTAL_TRACE("kernel init fail err:%d", err);
        return err;
    }

    KERNEL_NS::KernelUtil::Start();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "kernel started."));

    // ScanReason::Run(argc, argv);
    ScanPlayerCount::Run(argc, argv);

    getchar();

    KERNEL_NS::KernelUtil::Destroy();

    return err;
}
