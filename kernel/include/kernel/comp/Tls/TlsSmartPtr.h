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
 * Date: 2024-11-30 21:42:40
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_SMART_PTR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_SMART_PTR_H__

#pragma once

#include <kernel/comp/Tls/ITlsObj.h>
#include <kernel/comp/Tls/Defs.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Utils/RttiUtil.h>

KERNEL_BEGIN

template<typename ObjType, AutoDelMethods::Way delMethod = AutoDelMethods::Delete>
class TlsSmartPtr : public ITlsObj
{
public:
    TlsSmartPtr()
    :_objTypeName(KERNEL_NS::RttiUtil::GetByType<ObjType>())
    {

    }
    ~TlsSmartPtr()
    {
        OnDestroy();
    }

    virtual const char *GetObjTypeName() const override { return _objTypeName.c_str(); }

    virtual void OnDestroy() override 
    {
        _ptr.Release();
    }

    KERNEL_NS::SmartPtr<ObjType, delMethod> &GetPtr()
    {
        return _ptr;
    }

    const KERNEL_NS::SmartPtr<ObjType, delMethod> &GetPtr() const
    {
        return _ptr;
    }
    
private:
    const KERNEL_NS::LibString _objTypeName;
    KERNEL_NS::SmartPtr<ObjType, delMethod> _ptr;
};

KERNEL_END

#endif