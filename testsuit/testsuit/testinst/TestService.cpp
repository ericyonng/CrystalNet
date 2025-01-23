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
 * 测试结果：
 * 单个service可以承载25wqps, 2000人, 发包频率:100qps 每个人(service 配置:32核64G)
 * 所以框架建议：
 * Gateway需要多个service承载,若要达到1w连接, 需要5个线程
 * GameServer 采用多进程模型, 每个点2000个人
*/

#include <pch.h>
#include <testsuit/testinst/TestService.h>

#ifdef ENABLE_TEST_SERVICE
 #include <service/TestService/service.h>

class TestServiceApplication : public SERVICE_COMMON_NS::Application
{
    POOL_CREATE_OBJ_DEFAULT_P1(Application, TestServiceApplication);

public:
    TestServiceApplication()
    {
        
    }
    
    ~TestServiceApplication() override
    {
        
    }

    void OnRegisterComps() override;
    {
        SERVICE_COMMON_NS::Application::OnRegisterComps();

        // 注册热更监控
        RegisterComp<SERVICE_COMMON_NS::LibraryHotfixMonitorFactory>();
    }

    virtual Int32 _OnCompsCreated() override
    {
        auto err = SERVICE_COMMON_NS::Application::_OnCompsCreated();
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("comps created fail err:%d"), err);
            return err;
        }

        auto hotfixMonitor = GetComp<SERVICE_COMMON_NS::ILibraryHotfixMonitor>();
        // 设置检测文件
        hotfixMonitor->SetDetectionFile(_path + KERNEL_NS::LibString().AppendFormat(".TestPlugin.hotfix_%d", _processId));
        // 设置路径
        hotfixMonitor->SetRootPath(KERNEL_NS::DirectoryUtil::GetFileDirInPath(GetAppPath()).strip());
        // 设置插件集热更回调 TODO:必须为每个监听者添加一个callback, 并设置监听的key
        hotfixMonitor->SetHotFixCallback([this](std::vector<SERVICE_COMMON_NS::HotFixContainerElemType> &hotfixs)
        {
            // TODO:有问题, 这里如果遍历service那么需要每个service一个copy, 每个动态库在每个service的地址不需不能是copy，而是重新加载的
            GetComp<SERVICE_COMMON_NS::ServiceProxy>()->pos
        });
    }

    void Release() override
    {
        TestServiceApplication::Delete_TestServiceApplication(this);
    }
};

POOL_CREATE_OBJ_DEFAULT_IMPL(TestServiceApplication);

#endif

void TestService::Run(int argc, char const *argv[])
{
#ifdef ENABLE_TEST_SERVICE
    KERNEL_NS::SmartPtr<TestServiceApplication, KERNEL_NS::AutoDelMethods::CustomDelete> app = TestServiceApplication::New_TestServiceApplication();
    app.SetClosureDelegate([](void *ptr)
    {
        auto p = KERNEL_NS::KernelCastTo<TestServiceApplication>(ptr);
        TestServiceApplication::Delete_TestServiceApplication(p);
        ptr = NULL;
    });

    SERVICE_COMMON_NS::ApplicationHelper::Start(app.AsSelf(), SERVICE_NS::ServiceFactory::New_ServiceFactory(), argc, argv, "./ini/service.ini");
#endif
}