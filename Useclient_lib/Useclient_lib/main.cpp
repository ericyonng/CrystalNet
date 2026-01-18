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
#include <client/client_api.h>

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
    printf("/*           Hello Crystal Net Client!        */\n");
    printf("/*********************************************/!\n\n");

    KERNEL_NS::ParamsInfo params;
    KERNEL_NS::LibString errParamsInfo;
    KERNEL_NS::LibString sucParamsInfo;
    Int32 paramNum = KERNEL_NS::ParamsHandler::GetParams(argc, argv, params, sucParamsInfo, errParamsInfo);

    LogFactory logFactory;
    KERNEL_NS::LibString programPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    KERNEL_NS::LibString logIniPath;
    logIniPath = programPath + "/ini/";
    KERNEL_NS::SystemUtil::GetProgramPath(true, programPath);
    Int32 err = KERNEL_NS::KernelUtil::Init(&logFactory, "LogCfg.ini", logIniPath.c_str(), KERNEL_NS::LibString(), KERNEL_NS::LibString(), true, params._fileSoftLimit, params._fileHardLimit);
    if(err != Status::Success)
    {
        CRYSTAL_TRACE("kernel init fail err:%d", err);
        return 0;
    }

    KERNEL_NS::KernelUtil::Start();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "kernel started paramNum:%d. \nsucParamsInfo:\n%s, errParamsInfo:\n%s."), paramNum, sucParamsInfo.c_str(), errParamsInfo.c_str());

    auto shareLib = KERNEL_NS::ShareLibraryLoader::NewThreadLocal_ShareLibraryLoader();
    const auto path = KERNEL_NS::SystemUtil::GetCurProgRootPath() + "/libclient_lib_debug.so";
    shareLib->SetLibraryPath(path);
    shareLib->Init();
    shareLib->Start();

    auto initPtr = shareLib->LoadSym<ClientInitPtr>(KERNEL_NS::LibString("ClientInit"));
    auto clientSet = shareLib->LoadSym<ClientSetPtr>(KERNEL_NS::LibString("ClientSet"));
    auto clientStart = shareLib->LoadSym<ClientStartPtr>(KERNEL_NS::LibString("ClientStart"));
    auto clientClose = shareLib->LoadSym<ClientClosePtr>(KERNEL_NS::LibString("ClientClose"));
    auto sendLog = shareLib->LoadSym<SendLogPtr>(KERNEL_NS::LibString("SendLog"));

    g_Log->Info(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "initPtr:%p, clientSet:%p, clientStart:%p, clientClose:%p, sendLog:%p, path:%s")
    , initPtr, clientSet, clientStart, clientClose, sendLog, path.c_str());

    initPtr();
    KERNEL_NS::LibString accountName = "test_role_ce";
    KERNEL_NS::LibString pwd = "1586ddk?R7'6s";
    clientSet("127.0.0.1", 9, 3900, accountName.data(), accountName.length(), pwd.data(), pwd.length());
    clientStart();

    auto sendTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    sendTimer->SetTimeOutHandler([sendLog](KERNEL_NS::LibTimer *timer)
    {
        auto idGen = KERNEL_NS::TlsUtil::GetIdGenerator()->NewId();
        KERNEL_NS::LibString str;
        str.AppendFormat("hello id:%llu", idGen);
        sendLog(str.data(), str.length(), static_cast<Int64>(idGen));
    });
    sendTimer->Schedule(KERNEL_NS::TimeSlice::FromSeconds(1));

    auto endTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    endTimer->SetTimeOutHandler([](KERNEL_NS::LibTimer *timer)
    {
        KERNEL_NS::TlsUtil::GetPoller()->QuicklyLoop();

        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(timer);
    });
    endTimer->Schedule(KERNEL_NS::TimeSlice::FromMinutes(1));

    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    poller->PrepareLoop();
    poller->EventLoop();
    poller->OnLoopEnd();

    clientClose();

    shareLib->Close();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(KERNEL_NS::KernelUtil, "unload so path:%s")
    , path.c_str());
    
    KERNEL_NS::KernelUtil::Destroy();

    return 0;
}