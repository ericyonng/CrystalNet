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
 * Date: 2022-12-16 22:25:46
 * Author: Eric Yonng
 * Description: 拷贝器
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_COPY_ADAPTER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_COPY_ADAPTER_H__

#pragma once

#include <kernel/common/macro.h>
#include <kernel/common/DataTypeAdapter.h>

KERNEL_BEGIN


// 流输出必须区分,指针,pod类型
template<typename _Ty, LibDataType::ENUMS _DataType>
class KernelCopyAdapter
{
public:
    static void Invoke(_Ty &target, _Ty &source);
};

template<typename _Ty, LibDataType::ENUMS _DataType>
ALWAYS_INLINE void KernelCopyAdapter<_Ty, _DataType>::Invoke(_Ty &target, _Ty &source)
{
    target = source;
}

// ARRAY
template<typename _Ty>
class KernelCopyAdapter<_Ty, LibDataType::ARRAY_TYPE>
{
public:
    static void Invoke(_Ty target, _Ty source);
};

template<typename _Ty>
ALWAYS_INLINE void KernelCopyAdapter<_Ty, LibDataType::ARRAY_TYPE>::Invoke(_Ty target, _Ty source)
{
    const UInt64 sz = static_cast<UInt64>(sizeof(_Ty));
    ::memcpy(target, source, sz);
}

template<typename _Ty>
class CopyAdapter : public KernelCopyAdapter<_Ty, LibTraitsDataType<_Ty>::value>
{

};

KERNEL_END

#endif
