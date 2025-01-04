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
 * Date: 2022-09-19 01:44:15
 * Author: Eric Yonng
 * Description: 创建指针
*/


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_PTR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_PTR_H__

#pragma once

#include <kernel/comp/Tls/ITlsObj.h>
#include <kernel/comp/Tls/Defs.h>
#include <string>

KERNEL_BEGIN

class KERNEL_EXPORT TlsPtr : public ITlsObj
{
public:
    TlsPtr();
    ~TlsPtr(){}
    virtual void OnDestroy() override
    {
        _ptr = NULL;
    }

public:
    virtual const char *GetObjTypeName() const override { return _objTypeName.c_str(); }

    template<typename T>
    T *&CastTo()
    {
        return reinterpret_cast<T *&>(_ptr);
    }
    
public:
    std::string _objTypeName;
    void *_ptr;
};

// t需要有默认构造
template<typename T>
class TlsTargetPtr : public ITlsObj
{
public:
    TlsTargetPtr()
        :_objTypeName(std::string("TlsTargetPtr<") + std::string(typeid(T).name()) + ">")
    {
        _ptr = CRYSTAL_NEW(T);
    }
    ~TlsTargetPtr() override
    {
        TlsTargetPtr::OnDestroy();
    }

    virtual void OnDestroy() override
    {
        CRYSTAL_DELETE_SAFE(_ptr);
    }

    T *Get()
    {
        return _ptr;
    }

    virtual const char *GetObjTypeName() const override { return _objTypeName.c_str(); }

public:
    const std::string _objTypeName;
    T *_ptr;
};

KERNEL_END
#endif