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
 * Date: 2023-07-23 20:09:11
 * Author: Eric Yonng
 * Description:
*/

#pragma once


#include <service/common/macro.h>
#include <service/common/status.h>
#include <kernel/common/LibObject.h>

SERVICE_BEGIN

class IStorageInfo;

class StorageFactory
{
    // 创建factory对象时候使用创建的方法类型
public:
    static constexpr KERNEL_NS::_Build::UNKNOWN _buildType{};

public:
    StorageFactory() {}
    virtual ~StorageFactory() {  }

    virtual void Release() = 0;

    virtual IStorageInfo *Create() const = 0;

    virtual UInt64 GetObjTypeId() const = 0;

    // 派生类无需要重写以下静态接口任意一个
    // static SERVICE_NS::StorageFactory *FactoryCreate();
    // static std::vector<SERVICE_NS::StorageFactory *> FactoryCreate();
};

SERVICE_END
