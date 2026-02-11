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
#include <service/TestService/service.h>
#include <client/Entry.h>
#include <client/Ini.h>

bool Client::Run(int argc, char const *argv[])
{
    KERNEL_NS::SmartPtr<SERVICE_COMMON_NS::Application, KERNEL_NS::AutoDelMethods::CustomDelete> app = SERVICE_COMMON_NS::Application::New_Application();
    app.SetClosureDelegate([](void *ptr)
    {
        auto p = KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::Application>(ptr);
        SERVICE_COMMON_NS::Application::Delete_Application(p);
        ptr = NULL;
    });

    Entry::Application.exchange(app.AsSelf(), std::memory_order_release);
    auto err = SERVICE_COMMON_NS::ApplicationHelper::Start(app, SERVICE_NS::ServiceFactory::New_ServiceFactory(), argc, argv, "", s_appIniContent);
    if (err != Status::Success)
    {
        Entry::Application.exchange(NULL, std::memory_order_release);

        CLOG_ERROR_GLOBAL(Client, "app fail err:%d", err);
        return false;
    }

    return true;
}
