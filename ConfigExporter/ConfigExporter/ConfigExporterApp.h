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
 * Date: 2023-12-23 18:09:27
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <kernel/comp/CompObject/CompHostObject.h>

class ConfigExporterApp : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, ConfigExporterApp);

public:
    ConfigExporterApp()
    {

    }

    ~ConfigExporterApp()
    {
    }

    virtual Int32 _OnHostInit() override;

    virtual Int32 _OnHostStart() override;

    virtual void Release() override;

    virtual void OnRegisterComps() override;

    const KERNEL_NS::LibString &GetAppName() const;

private:
    KERNEL_NS::LibString _appName;
};
