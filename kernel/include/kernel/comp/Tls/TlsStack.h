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
 * Date: 2021-01-15 22:36:16
 * Author: Eric Yonng
 * Description: 
 * 1.定位高性能，避免cpu cache频繁失效
 * 2.初始化16MB或者指定尺寸大小的内存
 * 3.需要的对象可以通过get获取
 * 4.通过定义枚举获取对应的对象，复杂度O(1)
 * 5.本对象只支持堆创建，避免栈创建导致爆栈
 * 6.所有的tls对象都必须继承于ITlsObj，以便分配id
 * 7.tls设计是线程局部存储故，仅限于当前线程，不可跨线程使用
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_STACK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_STACK_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Tls/Defs.h>
#include <kernel/comp/Tls/ITlsObj.h>
#include <kernel/comp/Tls/TlsDefaultObj.h>

KERNEL_BEGIN

template<TlsStackSize::SizeType TlsSizeType = TlsStackSize::SIZE_2MB>
class TlsStack
{
public:
    TlsStack();
    virtual ~TlsStack();

public:
    // 新建一个对象
    template<typename ObjType, typename... Args>
    ObjType *New(Args&&... args);
    template<typename ObjType>
    ObjType *New();

    // 外部对idx内存进行重复利用
    template<typename ObjType, typename... Args>
    ObjType *NewByIdx(UInt64 idx, Args&&... args);
    template<typename ObjType>
    ObjType *NewByIdx(UInt64 idx);

    template<typename ObjType = TlsDefaultObj>
    ObjType *Get(UInt64 idx = 0);

    // 默认的tls对象
    TlsDefaultObj *GetDef();

    // 回收并返回idx以便再次利用
    UInt64 Free(ITlsObj *ptr);
    void FreeAll();

    std::string ToString();

private:
    UInt64 _curUseBytes;                                            // 当前已分配的字节数
    UInt64 _maxIdx;                                                 // 当前分配的对象id最大值
    Byte8 *_objAddr[TlsSizeType/TlsStackSize::SIZE_MIN_UNIT];       // 对象索引_mem指针
    Byte8 _mem[TlsSizeType];                                        // 分配对象的内存空间
    // LogType *_log;
};


template<TlsStackSize::SizeType TlsSizeType>
inline TlsStack<TlsSizeType>::TlsStack()
    :_curUseBytes(0)
    ,_maxIdx(0)
{
    // 第一个为默认对象 tlsstack避免调用自身导致死循环
    _objAddr[0] = reinterpret_cast<Byte8 *>(New<TlsDefaultObj>());
}

template<TlsStackSize::SizeType TlsSizeType>
inline TlsStack<TlsSizeType>::~TlsStack()
{
    FreeAll();
}

template<TlsStackSize::SizeType TlsSizeType>
template<typename ObjType, typename... Args>
inline ObjType *TlsStack<TlsSizeType>::New(Args&&... args)
{
    const UInt64 objSize = __MEMORY_ALIGN__(sizeof(ObjType));

    // 内存是否够用
    if(UNLIKELY((_curUseBytes + objSize) > TlsSizeType))
        return NULL;

    // objidx 都是非0的
    Byte8 *ptr = reinterpret_cast<Byte8 *>(_mem + _curUseBytes);
    ObjType *obj = ::new(ptr)ObjType(std::forward<Args>(args)...);
    obj->SetTlsStack(this);
    obj->SetParams(_maxIdx
    , this
    , &TlsStack<TlsSizeType>::Free);

    _objAddr[_maxIdx] = ptr;
    _curUseBytes += objSize;
    ++_maxIdx;

    return obj;
}

template<TlsStackSize::SizeType TlsSizeType>
template<typename ObjType>
inline ObjType *TlsStack<TlsSizeType>::New()
{
    const UInt64 objSize = __MEMORY_ALIGN__(sizeof(ObjType));

    // 内存是否够用
    if(UNLIKELY((_curUseBytes + objSize) > static_cast<UInt64>(TlsSizeType)))
        return NULL;

    Byte8 *ptr = reinterpret_cast<Byte8 *>(_mem + _curUseBytes);
    ObjType *obj = ::new(ptr)ObjType();
    obj->SetTlsStack(this);
    obj->SetParams(_maxIdx
    , this
    , &TlsStack<TlsSizeType>::Free);

    _objAddr[_maxIdx] = ptr;
    _curUseBytes += objSize;
    ++_maxIdx;

    return obj; 
}

template<TlsStackSize::SizeType TlsSizeType>
template<typename ObjType, typename... Args>
inline ObjType *TlsStack<TlsSizeType>::NewByIdx(UInt64 idx, Args&&... args)
{
    Byte8 *ptr = _objAddr[idx];
    ObjType *obj = ::new(ptr)ObjType(std::forward<Args>(args)...);
    obj->SetTlsStack(this);
    obj->SetParams(idx
    , this
    , &TlsStack<TlsSizeType>::Free);

    return obj;
}

template<TlsStackSize::SizeType TlsSizeType>
template<typename ObjType>
inline ObjType *TlsStack<TlsSizeType>::NewByIdx(UInt64 idx)
{
    Byte8 *ptr = _objAddr[idx];
    ObjType *obj = ::new(ptr)ObjType();
    obj->SetTlsStack(this);
    obj->SetParams(idx
    , this
    , &TlsStack<TlsSizeType>::Free);

    return obj;
}

template<TlsStackSize::SizeType TlsSizeType>
template<typename ObjType>
inline ObjType *TlsStack<TlsSizeType>::Get(UInt64 idx)
{
    if(UNLIKELY(_maxIdx < idx))
        return NULL;
        
    return reinterpret_cast<ObjType *>(_objAddr[idx]);
}

template<TlsStackSize::SizeType TlsSizeType>
inline TlsDefaultObj *TlsStack<TlsSizeType>::GetDef()
{
    return reinterpret_cast<TlsDefaultObj *>(_objAddr[0]);
}

template<TlsStackSize::SizeType TlsSizeType>
inline UInt64 TlsStack<TlsSizeType>::Free(ITlsObj *ptr)
{
    UInt64 idx = ptr->GetTlsIdx();
//    CRYSTAL_TRACE("TlsStack FREE OBJ[%s:%p], idx[%llu]", ptr->GetObjTypeName(), ptr, idx);
    ptr->Destoy();
    return idx;
}

template<TlsStackSize::SizeType TlsSizeType>
inline void TlsStack<TlsSizeType>::FreeAll()
{
    // CRYSTAL_TRACE("tls stack size=[%llu] will free all maxidx=[%llu]", TlsSizeType, static_cast<UInt64>(_maxIdx));
    for(Int32 i = static_cast<Int32>(_maxIdx - 1); i >= 0; --i)
    {
        ITlsObj *obj = reinterpret_cast<ITlsObj *>(_objAddr[i]);
        if(UNLIKELY(!obj->IsFree()))
            Free(obj);
    }

    // 打印
    // _log->Info(LIB_LOG_FMT("FreeAll tls objh:\n %s\n, tls free finish."), ToString().c_str());
}

template<TlsStackSize::SizeType TlsSizeType>
inline std::string TlsStack<TlsSizeType>::ToString()
{
    BUFFER512 info;
    sprintf(info, "this stack addr[%p], real obj mem[%p] cur usebytes[%llu], maxIdx[%llu] TlsSizeType[%llu]", this, _mem, _curUseBytes, _maxIdx, static_cast<UInt64>(TlsSizeType));

    return info;
}

KERNEL_END

#endif
