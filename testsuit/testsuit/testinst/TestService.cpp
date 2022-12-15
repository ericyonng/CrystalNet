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
 * Date: 2022-06-27 00:58:21
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestService.h>
#include <service/TestService/service.h>

void TestService::Run(int argc, char const *argv[])
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "test service begin."));
    KERNEL_NS::SmartPtr<SERVICE_COMMON_NS::Application, KERNEL_NS::AutoDelMethods::CustomDelete> app = SERVICE_COMMON_NS::Application::New_Application();
    app.SetClosureDelegate([](void *ptr)
    {
        auto p = KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::Application>(ptr);
        SERVICE_COMMON_NS::Application::Delete_Application(p);
        ptr = NULL;
    });

    // 设置配置
    app->SetIniFile("./ini/service.ini");

    // 设置传入的参数
    std::vector<KERNEL_NS::LibString> args;
    for(Int32 idx = 0; idx < argc; ++idx)
        args.push_back(KERNEL_NS::LibString(argv[idx]));

    app->SetAppArgs(args);

    // 异常关闭app
    const auto currentTid = KERNEL_NS::SystemUtil::GetCurrentThreadId();
    auto signalCloseLambda = [&app, currentTid]()->void{
        auto threadId = KERNEL_NS::SystemUtil::GetCurrentThreadId();
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "signal catched, application will close threadId:%llu, application thread id:%llu..."), threadId, currentTid);
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        if (threadId == currentTid)
        {
            app->WillClose();
            app->Close();

            while (app->IsReady())
                KERNEL_NS::SystemUtil::ThreadSleep(0);

            KERNEL_NS::KernelUtil::OnSignalClose();

            printf("\npress q to quit.\n");
            while(true)
            {
                auto v = getchar();
                if(v == 'q')
                    break;
            }
        }
        else
        {
            KERNEL_NS::KernelUtil::OnAbnormalClose();

            printf("\npress q to quit.\n");
            while(true)
            {
                auto v = getchar();
                if(v == 'q')
                    break;
            }
        }
#else
        app->WillClose();
        app->Close();

        while (app->IsReady())
            KERNEL_NS::SystemUtil::ThreadSleep(0);

        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "application close finished."));

        KERNEL_NS::KernelUtil::OnSignalClose();

        printf("\npress q to quit.\n");
        while(true)
        {
            auto v = getchar();
            if(v == 'q')
                break;
        }
#endif
        
    };

    auto closeDelg = KERNEL_CREATE_CLOSURE_DELEGATE(signalCloseLambda, void);
    KERNEL_NS::KernelUtil::InstallSignalCloseHandler(closeDelg);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "init application..."));

    Int32 errCode = app->Init();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestService, "application init fail %s, errCode:%d"), app->IntroduceStr().c_str(), errCode);
        return;
    }

    // 设置服务创建工厂
    auto serviceProxy = app->GetComp<SERVICE_COMMON_NS::ServiceProxy>();
    serviceProxy->SetServiceFactory(SERVICE_NS::ServiceFactory::New_ServiceFactory());

    // 启动app
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "start application..."));
    errCode = app->Start();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestService, "application start fail %s, errCode:%d"), app->IntroduceStr().c_str(), errCode);
        return;
    }

    // app等待结束
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "application wait finish(client can send a message to close application.)..."));
    Int32 err = Status::Success;
    app->WaitFinish(err);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "application wake up and will close err:%d..."), err);
    app->WillClose();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "application close..."));
    app->Close();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "application close finish..."));
}