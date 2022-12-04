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
 * Date: 2021-01-15 23:11:08
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_ITLS_OBJ_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_ITLS_OBJ_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Delegate/Delegate.h>

KERNEL_BEGIN

class KERNEL_EXPORT ITlsObj
{
public:
    ITlsObj()
    :_tlsIdx(0)
    ,_tlsStack(NULL)
    , _free(NULL)
    ,_isFree(false)
    {}

    virtual ~ITlsObj()
    {
        ITlsObj::Destoy();
    }

    virtual void Destoy();

    // 设置参数 idx, 释放对象的方法（线程安全与线程不安全释放）
    template<typename TlsStackType>
    void SetParams(UInt64 tlsIdx, TlsStackType *ptr
    , UInt64 (TlsStackType::*FreeHandler)(ITlsObj *ptr));

    // 获取idx
    UInt64 GetTlsIdx() const;
    // 返回idx
    UInt64 Free();
    // 是否释放过内存
    bool IsFree();
    // 获取类型名
    virtual const char *GetObjTypeName() = 0;
    // 主动调用析构
    // virtual void InvokeDestruct() = 0;
    void *GetTlsStack();
    void SetTlsStack(void *tlsStack);

private:
    UInt64 _tlsIdx;             // 在tls中的唯一索引
    void *_tlsStack;            // tlsStack指针

    // 释放对象方法
    IDelegate<UInt64, ITlsObj *> *_free;
    bool _isFree;
};

inline void ITlsObj::Destoy()
{
    if(_isFree)
        return;

    _isFree = true;
    _tlsStack = NULL;
    _tlsIdx = 0;
    CRYSTAL_DELETE_SAFE(_free);
}

template<typename TlsStackType>
inline void ITlsObj::SetParams(UInt64 tlsIdx, TlsStackType *ptr
    , UInt64 (TlsStackType::*FreeHandler)(ITlsObj *ptr))
{
    _tlsIdx = tlsIdx;
    _free = DelegateFactory::Create(ptr, FreeHandler);
}

inline UInt64 ITlsObj::GetTlsIdx() const
{
    return _tlsIdx;
}

inline UInt64 ITlsObj::Free()
{
    // TODO:调用tlsstack free方法回收
    return _free->Invoke(this);
}

inline bool ITlsObj::IsFree()
{
    return _isFree;
}

inline void *ITlsObj::GetTlsStack()
{
    return _tlsStack;
}

inline void ITlsObj::SetTlsStack(void *tlsStack)
{
    _tlsStack = tlsStack;
}

KERNEL_END

#endif
