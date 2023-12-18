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
 * Date: 2021-01-24 17:19:48
 * Author: Eric Yonng
 * Description: 随机数发生器各个接口线程安全
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_RANDOM_LIB_INT64_RANDOM_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_RANDOM_LIB_INT64_RANDOM_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

#include <kernel/comp/Random/LibRandom.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Utils/AllocUtil.h>


KERNEL_BEGIN

class KERNEL_EXPORT LibInt64RandomInstance
{
public:
    template<typename Rtn, Int64 minValue, Int64 maxValue>
    static Rtn &GetInstance(_Build::MT::Type);
    template<typename Rtn, Int64 minValue, Int64 maxValue>
    static Rtn &GetInstance(_Build::TL::Type);
};

template<typename BuildType = _Build::MT>
class LibInt64Random
{
public:
    using DistType = KERNEL_NS::RandDistibuteType::ENUMS;
    using SrcType = KERNEL_NS::RandSourceAlgorithmType::ENUMS;

    // 随机数生成器只能通过 GetInstance创建保证同一个范围的随机数发生器只有一个对象并被置种子
public:
    LibInt64Random(Int64 minValue = RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, Int64 maxValue = RandDisNumScope::RAND_DIS_NUM_SCOPE_INT64MAX);
    ~LibInt64Random(){}

public:
    // 获取单例的随机数发生器,使全局统一使用统一个随机数发生器
    static LibInt64Random<BuildType> &GetInstance();
    template<Int64 minValue, Int64 maxValue>
    static LibInt64Random<BuildType> &GetInstance();

    // 重置种子
    void ResetSeed();

    // 生成随机数[minValue, maxValue]
    Int64 Gen();
    
    // 生成指定范围内的随机数[startValue, endValue]
    Int64 Gen(Int64 startValue, Int64 endValue);

private:
    LibRandom<Int64, BuildType
    , DistType::RAND_DIS_TYPE_INT
    , SrcType::RAND_GEN_ALGORITHM_TYPE_MT19937_64 > _engine;
};

template<typename BuildType>
ALWAYS_INLINE LibInt64Random<BuildType>::LibInt64Random(Int64 minValue, Int64 maxValue)
    :_engine(minValue, maxValue)
{

}

template<typename BuildType>
ALWAYS_INLINE LibInt64Random<BuildType> &LibInt64Random<BuildType>::GetInstance() 
{
    return GetInstance<RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, RandDisNumScope::RAND_DIS_NUM_SCOPE_INT64MAX>();
}

template<typename BuildType>
template<Int64 minValue, Int64 maxValue>
ALWAYS_INLINE LibInt64Random<BuildType> &LibInt64Random<BuildType>::GetInstance()
{
    return LibInt64RandomInstance::GetInstance<LibInt64Random<BuildType>, minValue, maxValue>(BuildType::V);
}

template<typename BuildType>
ALWAYS_INLINE void LibInt64Random<BuildType>::ResetSeed()
{
    _engine.ResetSeed();
}

template<typename BuildType>
ALWAYS_INLINE Int64 LibInt64Random<BuildType>::Gen()
{
    return _engine.Gen();
}

template<typename BuildType>
ALWAYS_INLINE Int64 LibInt64Random<BuildType>::Gen(Int64 startValue, Int64 endValue)
{
    return _engine.Gen(startValue, endValue);
}

template<typename Rtn, Int64 minValue, Int64 maxValue>
ALWAYS_INLINE Rtn &LibInt64RandomInstance::GetInstance(_Build::MT::Type)
{
    static Rtn *instance = new Rtn(minValue, maxValue);
    return *instance;
}

template<typename Rtn, Int64 minValue, Int64 maxValue>
ALWAYS_INLINE Rtn &LibInt64RandomInstance::GetInstance(_Build::TL::Type)
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR  Rtn *instance = NULL;
    if(UNLIKELY(!instance))
    {
        instance = new Rtn(minValue, maxValue);
    }
    return *instance;
}

KERNEL_END

#endif

