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
 * Date: 2022-06-26 17:51:22
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/Client/ServiceFactory.h>
#include <service/Client/MyTestService.h>

SERVICE_BEGIN

SERVICE_COMMON_NS::IService *ServiceFactory::Create(const KERNEL_NS::LibString &serviceName)
{
    if(serviceName == "Client" || serviceName == "LogReciever")
        return MyTestService::NewByAdapter_MyTestService(_buildType.V);
    
    g_Log->Warn(LOGFMT_OBJ_TAG("unknown service name:%s"), serviceName.c_str());
    return NULL;
}

void ServiceFactory::Release()
{
    ServiceFactory::Delete_ServiceFactory(this);
}

SERVICE_END
