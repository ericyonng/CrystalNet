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
#include <ProtoGen/protogen.h>
#include <service/ProtoGenService/service.h>
#include <ProtoGen/ProtogenIni.h>

class ProtoGenApp : public SERVICE_COMMON_NS::Application
{
    POOL_CREATE_OBJ_DEFAULT_P1(Application, ProtoGenApp);

protected:
    virtual void _OnMonitorThreadFrame() override
    {
        // 不打印帧状态
    }
};

// TODO:
// 1.禁用日志显示
// 2.扩展日志打印接口 格式:[PROTOGEN]: cpp gen file:%s, success!
// 3.Csharp导出


POOL_CREATE_OBJ_DEFAULT_IMPL(ProtoGenApp);

Int32 ProtoGen::Run(int argc, char const *argv[])
{
    KERNEL_NS::SmartPtr<ProtoGenApp, KERNEL_NS::AutoDelMethods::CustomDelete> app = ProtoGenApp::New_ProtoGenApp();
    app.SetClosureDelegate([](void *ptr)
    {
        auto p = KERNEL_NS::KernelCastTo<ProtoGenApp>(ptr);
        ProtoGenApp::Delete_ProtoGenApp(p);
        ptr = NULL;
    });

    // 设置配置
    // app->SetIniFile("./ini/protogen.ini");
    app->SetMemoryIniContent(s_appIniContent);

    // 设置传入的参数
    std::vector<KERNEL_NS::LibString> args;
    for(Int32 idx = 0; idx < argc; ++idx)
        args.push_back(KERNEL_NS::LibString(argv[idx]));

    app->SetAppArgs(args);

    // 异常关闭app
    const auto currentTid = KERNEL_NS::SystemUtil::GetCurrentThreadId();
    auto signalCloseLambda = [&app, currentTid]()->void{
        auto threadId = KERNEL_NS::SystemUtil::GetCurrentThreadId();
        g_Log->Info(LOGFMT_NON_OBJ_TAG(ProtoGen, "signal catched, application will close threadId:%llu, application thread id:%llu..."), threadId, currentTid);
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        if (threadId == currentTid)
        {
            app->WillClose();
            app->Close();

            while (app->IsReady())
                KERNEL_NS::SystemUtil::ThreadSleep(0);

            KERNEL_NS::KernelUtil::OnSignalClose();
        }
        else
        {
            KERNEL_NS::KernelUtil::OnAbnormalClose();
        }
#else
        app->WillClose();
        app->Close();

        while (app->IsReady())
            KERNEL_NS::SystemUtil::ThreadSleep(0);

        KERNEL_NS::KernelUtil::OnSignalClose();
#endif
        
    };

    auto closeDelg = KERNEL_CREATE_CLOSURE_DELEGATE(signalCloseLambda, void);
    KERNEL_NS::KernelUtil::InstallSignalCloseHandler(closeDelg);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(ProtoGen, "init protogen..."));

    Int32 errCode = app->Init();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(ProtoGen, "application init fail %s, errCode:%d"), app->IntroduceStr().c_str(), errCode);
        return errCode;
    }

    // 设置服务创建工厂
    auto serviceProxy = app->GetComp<SERVICE_COMMON_NS::ServiceProxy>();
    serviceProxy->SetServiceFactory(SERVICE_NS::ServiceFactory::New_ServiceFactory());

    // 启动app
    g_Log->Info(LOGFMT_NON_OBJ_TAG(ProtoGen, "start..."));
    errCode = app->Start();
    if(errCode != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(ProtoGen, "start fail %s, errCode:%d"), app->IntroduceStr().c_str(), errCode);
        return errCode;
    }

    // app等待结束
    g_Log->Info(LOGFMT_NON_OBJ_TAG(ProtoGen, "wait finish(can send a message to close client.)..."));
    Int32 err = Status::Success;
    app->WaitFinish(err);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(ProtoGen, "wake up and will close..."));
    app->WillClose();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(ProtoGen, "close..."));
    app->Close();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(ProtoGen, "close finish... err:%d"), err);

    return err;
}
