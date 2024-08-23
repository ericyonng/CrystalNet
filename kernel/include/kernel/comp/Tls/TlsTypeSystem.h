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
 * Date: 2024-08-21 21:12:14
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_TYPE_SYSTEM_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_TYPE_SYSTEM_H__

#pragma once

#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/CompObject/CompFactory.h>
#include <unordered_map>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class KERNEL_EXPORT TlsTypeSystem : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, TlsTypeSystem);

public:
    TlsTypeSystem();
    ~TlsTypeSystem() {}

    virtual void Release() override;

    bool CheckAddTypeInfo(IObject *obj);

private:
    virtual Int32 _OnCreated() final;

private:
    std::unordered_map<UInt64, KERNEL_NS::LibString> _typeIdRefTypeName;
    std::unordered_map<KERNEL_NS::LibString, UInt64> _typeNameRefTypeId;
};

class KERNEL_EXPORT TlsTypeSystemFactory : public CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate();

    virtual void Release();

    virtual CompObject *Create() const;

    virtual Int32 GetPriorityLevel() const { return CompPriorityLevel::HIGH; }
};

KERNEL_END

#endif
