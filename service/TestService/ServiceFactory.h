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
 * Date: 2022-06-26 16:19:22
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/common.h>
#include <service_common/service/service.h>
#include <kernel/common/LibObject.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/memory/ObjPoolMacro.h>

SERVICE_BEGIN

class ServiceFactory :public SERVICE_COMMON_NS::IServiceFactory
{
    POOL_CREATE_OBJ_DEFAULT_P1(IServiceFactory, ServiceFactory);

public:
    virtual SERVICE_COMMON_NS::IService *Create(const KERNEL_NS::LibString &serviceName) override;
    virtual void Release() override;

    static constexpr KERNEL_NS::_Build::TL _buildType{};
};

SERVICE_END
