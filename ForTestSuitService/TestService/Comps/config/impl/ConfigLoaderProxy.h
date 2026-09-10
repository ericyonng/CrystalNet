// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-08-23 15:08:36
// Author: Eric Yonng
// Description:

#pragma once

#include <service/common/macro.h>
#include <kernel/comp/CompObject/CompHostObject.h>
#include <kernel/comp/SmartPtr.h>

SERVICE_BEGIN

class ConfigLoader;

class ConfigLoaderProxy : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, ConfigLoaderProxy);

public:
    ConfigLoaderProxy();
    ~ConfigLoaderProxy();

    void Release() override;

    virtual void OnRegisterComps() override;

    KERNEL_NS::SmartPtr<ConfigLoader, KERNEL_NS::AutoDelMethods::Release> GetConfigLoader() const;

    
protected:
    virtual Int32 _OnHostInit() override;
    virtual Int32 _OnHostStart() override;
    virtual void _OnHostWillClose() override;
    virtual void _OnHostClose() override;

private:
    KERNEL_NS::SmartPtr<ConfigLoader, KERNEL_NS::AutoDelMethods::Release> _configLoader;
};

SERVICE_END
