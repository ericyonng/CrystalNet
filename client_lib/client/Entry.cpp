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
 * Date: 2022-10-08 12:57:29
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <client/client.h>
#include <client/Entry.h>

#include <client/Ini.h>

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

KERNEL_NS::LibThread *Entry::EntryThread = NULL;
std::atomic<KERNEL_NS::IApplication *> Entry::Application = {NULL};
std::atomic<SERVICE_COMMON_NS::IService *> Entry::Service = {NULL};

bool Entry::Run()
{
    // 1.初始化内核
    printf("/*********************************************/!\n");
    printf("/*           Hello Crystal Net Client!        */\n");
    printf("/*********************************************/!\n\n");

    KERNEL_NS::ParamsInfo params;
    KERNEL_NS::LibString errParamsInfo;
    KERNEL_NS::LibString sucParamsInfo;
    Int32 paramNum = KERNEL_NS::ParamsHandler::GetParams(0, NULL, params, sucParamsInfo, errParamsInfo);

    LogFactory logFactory;
    KERNEL_NS::LibString programPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    KERNEL_NS::LibString logIniPath;
    logIniPath = programPath + "/ini/";
    KERNEL_NS::SystemUtil::GetProgramPath(true, programPath);
    Int32 err = KERNEL_NS::KernelUtil::Init(&logFactory, "LogCfg.ini", logIniPath.c_str(), s_logIniContent, s_consoleIniContent, false, params._fileSoftLimit, params._fileHardLimit);
    if(err != Status::Success)
    {
        CRYSTAL_TRACE("kernel init fail err:%d", err);
        return false;
    }

    KERNEL_NS::KernelUtil::Start();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "kernel started paramNum:%d. \nsucParamsInfo:\n%s, errParamsInfo:\n%s."), paramNum, sucParamsInfo.c_str(), errParamsInfo.c_str());

    Entry::EntryThread = new KERNEL_NS::LibThread();
    Entry::EntryThread->AddTask2([](KERNEL_NS::LibThread *thread, KERNEL_NS::Variant *var)
    {
        Client::Run(0, NULL);

    }, NULL);
    
    return true;
}