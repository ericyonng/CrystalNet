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
#endif

void TestService::Run(int argc, char const *argv[])
{
#ifdef ENABLE_TEST_SERVICE
    KERNEL_NS::SmartPtr<SERVICE_COMMON_NS::Application, KERNEL_NS::AutoDelMethods::CustomDelete> app = SERVICE_COMMON_NS::Application::New_Application();
    app.SetClosureDelegate([](void *ptr)
    {
        auto p = KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::Application>(ptr);
        SERVICE_COMMON_NS::Application::Delete_Application(p);
        ptr = NULL;
    });

    SERVICE_COMMON_NS::ApplicationHelper::Start(app.AsSelf(), SERVICE_NS::ServiceFactory::New_ServiceFactory(), argc, argv, "./ini/service.ini");
#endif
}