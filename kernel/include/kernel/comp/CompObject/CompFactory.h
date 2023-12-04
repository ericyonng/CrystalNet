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
 * Date: 2022-03-15 01:12:36
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COMP_OBJECT_COMP_FACTORY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COMP_OBJECT_COMP_FACTORY_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/common/LibObject.h>

KERNEL_BEGIN

class CompObject;

#undef CREATE_CRYSTAL_COMP
// 创建组件并设置接口类id, 前提是有接口类才能使用否则, 接口类id在IObject中默认是0
#define CREATE_CRYSTAL_COMP(VAR, T)                                 \
auto VAR = T::NewByAdapter_##T(_buildType.V);                       \
VAR->SetInterfaceTypeId(KERNEL_NS::RttiUtil::GetTypeId<I##T>())

#undef CREATE_CRYSTAL_COMP_INS
// 提供接口类的命名空间 INS:接口命名空间
#define CREATE_CRYSTAL_COMP_INS(VAR, T, INTERFACE_TYPE_NS)          \
auto VAR = T::NewByAdapter_##T(_buildType.V);                       \
VAR->SetInterfaceTypeId(KERNEL_NS::RttiUtil::GetTypeId<INTERFACE_TYPE_NS::I##T>())

class KERNEL_EXPORT CompPriorityLevel
{
public:
    enum ENUMS
    {
        NONE = 0,       // 无差别
        HIGH = 1,       // 高优先级
        MIDDLE = 2,     // 次优先级
        LOW = 3,        // 低优先级
    };
};

class KERNEL_EXPORT CompFactory
{
    // 创建factory对象时候使用创建的方法类型
public:
    static constexpr _Build::UNKNOWN _buildType{};
public:
    CompFactory() {}
    virtual ~CompFactory() {  }

    virtual void Release() = 0;

    virtual CompObject *Create() const = 0;

    // 组件优先级
    virtual Int32 GetPriorityLevel() const { return CompPriorityLevel::NONE; }
    // 派生类无需要重写以下静态接口任意一个
    // static KERNEL_NS::CompFactory *FactoryCreate();
    // static std::vector<KERNEL_NS::CompFactory *> FactoryCreate();
};

KERNEL_END

#endif
