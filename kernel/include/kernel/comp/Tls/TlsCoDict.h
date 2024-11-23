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
 * Date: 2024-11-16 19:18:38
 * Author: Eric Yonng
 * Description: 当前线程所有协程管理
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_CODICT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_CODICT_H__

#pragma once

#include <kernel/comp/Tls/ITlsObj.h>
#include <kernel/comp/LibString.h>
#include <map>

KERNEL_BEGIN

struct KernelHandle;

class KERNEL_EXPORT TlsCoDict : public ITlsObj
{
public:
   TlsCoDict();
   ~TlsCoDict();
   virtual void OnDestroy() override;
   virtual const char *GetObjTypeName() const override { return _objTypeName.c_str(); }

    void AddCo(UInt64 id, KernelHandle *handle);
    void RemoveCo(UInt64 id);
    KernelHandle *GetHandle(UInt64 id);
    const KernelHandle *GetHandle(UInt64 id) const;
 
private:
 const std::string _objTypeName;
 std::map<UInt64, KernelHandle *> _idRefHandle;
};

ALWAYS_INLINE void TlsCoDict::AddCo(UInt64 id, KernelHandle *handle)
{
   _idRefHandle.insert(std::make_pair(id, handle));
}

ALWAYS_INLINE void TlsCoDict::RemoveCo(UInt64 id)
{
    _idRefHandle.erase(id);
}

ALWAYS_INLINE KernelHandle *TlsCoDict::GetHandle(UInt64 id)
{
    auto iter = _idRefHandle.find(id);
    return iter == _idRefHandle.end() ? NULL : iter->second;
}

ALWAYS_INLINE const KernelHandle *TlsCoDict::GetHandle(UInt64 id) const
{
    auto iter = _idRefHandle.find(id);
    return iter == _idRefHandle.end() ? NULL : iter->second;
}

KERNEL_END

#endif
