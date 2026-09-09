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
 * Date: 2023-08-06 19:43:39
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/macro.h>
#include <service_common/config/IConfigLoder.h>

SERVICE_BEGIN

class ConfigLoader : public SERVICE_COMMON_NS::IConfigLoader
{
    POOL_CREATE_OBJ_DEFAULT_P1(IConfigLoader, ConfigLoader);

public:
    ConfigLoader();
    ~ConfigLoader();

    void Release() override;

    virtual void OnRegisterComps() override;

protected:
    virtual Int32 _OnHostInit() override;
    virtual Int32 _OnHostStart() override;
    virtual void _OnHostClose() override;
};

SERVICE_END
