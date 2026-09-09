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
 * Date: 2026-09-09 11:57:25
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <LogicServer/LogicServer.h>
#include <stdio.h>
#include <kernel/comp/Log/log.h>
#include <service_common/common/KernelForService.h>
#include <LogicServer/LogicServerApp.h>
#include <LogicServer/ServiceFactory.h>
#include <service_common/application/ApplicationHelper.h>

LogicServer::LogicServer()
{

}

LogicServer::~LogicServer()
{

}

int LogicServer::Start(int argc, char const *argv[])
{
    printf("/*********************************************/!\n");
    printf("/*           Hello LogicServer!              */\n");
    printf("/*********************************************/!\n\n");

    // 1. 初始化
    Int32 err = SERVICE_COMMON_NS::KernelForService::Init(argc, argv);
    if(err != Status::Success)
    {
        CRYSTAL_TRACE("kernel init fail err:%d", err);
        return err;
    }

    SERVICE_COMMON_NS::KernelForService::Start();

    CLOG_INFO("Kernel started...");

    // TODO: app start
    KERNEL_NS::SmartPtr<LogicServerApp, KERNEL_NS::AutoDelMethods::Release> app = LogicServerApp::New_LogicServerApp();
    SERVICE_COMMON_NS::ApplicationHelper::Start(app.AsSelf(), ServiceFactory::New_ServiceFactory(), argc, argv, "./Yaml/LogicServer.yaml");

    CLOG_INFO("kernel will destroy...");
    KERNEL_NS::SystemUtil::ThreadSleep(5000);
    SERVICE_COMMON_NS::KernelForService::Destroy();
}