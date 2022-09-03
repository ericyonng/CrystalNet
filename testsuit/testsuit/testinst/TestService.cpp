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
#include <service/service.h>

void TestService::Run()
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "test service begin."));
    KERNEL_NS::SmartPtr<SERVICE_COMMON_NS::Application, KERNEL_NS::_Build::MT, KERNEL_NS::AutoDelMethods::CustomDelete> app = SERVICE_COMMON_NS::Application::New_Application();
    app.SetClosureDelegate([](void *ptr)
    {
        auto p = KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::Application>(ptr);
        SERVICE_COMMON_NS::Application::Delete_Application(p);
        ptr = NULL;
    });

    // 设置配置
    app->SetIniFile("./service.ini");

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
    app->WaitFinish();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "application wake up and will close..."));
    app->WillClose();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "application close..."));
    app->Close();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestService, "application close finish..."));
}