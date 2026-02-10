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
 * Date: 2022-12-08 13:03:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <file_tool/FileTool.h>
#include <file_tool/Ini.h>

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

KERNEL_NS::YamlMemory *g_YamlMemory = NULL;

int main(int argc, char const *argv[])
{
    printf("/*********************************************/!\n");
    printf("/*            Hello File Tool!               */\n");
    printf("/*********************************************/!\n\n");

    LogFactory logFactory;
    KERNEL_NS::LibString programPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    KERNEL_NS::LibString logIniPath;
    logIniPath = programPath + "/ini/";
    KERNEL_NS::SystemUtil::GetProgramPath(true, programPath);
    g_YamlMemory = KERNEL_NS::YamlMemory::From(s_LogYamlContent);
    Int32 err = KERNEL_NS::KernelUtil::Init(&logFactory, "LogCfg.ini", logIniPath.c_str(), g_YamlMemory);
    if(err != Status::Success)
    {
        CRYSTAL_TRACE("kernel init fail err:%d", err);
        return 0;
    }

    KERNEL_NS::KernelUtil::Start();

    FileTool::Run(argc, argv);

    KERNEL_NS::KernelUtil::Destroy();

    return 0;
}