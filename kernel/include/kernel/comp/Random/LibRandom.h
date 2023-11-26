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
 * Date: 2021-01-24 15:45:38
 * Author: Eric Yonng
 * Description: 随机数发生器各个接口线程安全
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_RANDOM_LIB_RANDOM_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_RANDOM_LIB_RANDOM_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Random/RandomDistribute.h>
#include <kernel/comp/Random/RandomSource.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Utils/AllocUtil.h>

KERNEL_BEGIN
class LibRandomInstance
{
public:
    template<typename Rtn, Int64 minValue, Int64 maxValue>
    static Rtn &GetInstance(_Build::MT::Type);
    template<typename Rtn, Int64 minValue, Int64 maxValue>
    static Rtn &GetInstance(_Build::TL::Type);
};

template<typename ValueType = Int64, typename BuildType = _Build::MT, RandDistibuteType::ENUMS distType = RandDistibuteType::RAND_DIS_TYPE_INT, RandSourceAlgorithmType::ENUMS sourceType = RandSourceAlgorithmType::RAND_GEN_ALGORITHM_TYPE_MT19937_64>
class LibRandom
{
public:
    LibRandom(ValueType minValue = (std::numeric_limits<ValueType>::min)(), ValueType maxValue = (std::numeric_limits<ValueType>::max)());
    ~LibRandom(){}

    template<ValueType minValue, ValueType maxValue>
    static LibRandom<ValueType, BuildType, distType, sourceType> &GetInstance();

    // 重置种子/生成新的随机序列
    void ResetSeed();

    // 生成随机数[minValue, maxValue]
    ValueType Gen();

    // 生成某个范围的随机数[startValue, endValue]
    ValueType Gen(ValueType startValue, ValueType endValue);

private:
    LockWrap<BuildType, LockParticleType::Light>  _lck;     // 多线程并发安全
    RandomSource<ValueType, sourceType> _randomSource;      // 随机数源 核心
    Distributor<ValueType, distType> _dist;                 // 随机数分布器 不影响随机数源的序列
}; 


template<typename ValueType, typename BuildType, RandDistibuteType::ENUMS distType, RandSourceAlgorithmType::ENUMS sourceType>
ALWAYS_INLINE LibRandom<ValueType, BuildType, distType, sourceType>::LibRandom(ValueType minValue, ValueType maxValue)
    :_dist(minValue, maxValue)
{

}

template<typename ValueType, typename BuildType, RandDistibuteType::ENUMS distType, RandSourceAlgorithmType::ENUMS sourceType>
template<ValueType minValue, ValueType maxValue>
ALWAYS_INLINE LibRandom<ValueType, BuildType, distType, sourceType> &LibRandom<ValueType, BuildType, distType, sourceType>::GetInstance()
{
    return LibRandomInstance::GetInstance<LibRandom<ValueType, BuildType, distType, sourceType>, minValue, maxValue>(BuildType::V);
}

template<typename ValueType, typename BuildType, RandDistibuteType::ENUMS distType, RandSourceAlgorithmType::ENUMS sourceType>
ALWAYS_INLINE void LibRandom<ValueType, BuildType, distType, sourceType>::ResetSeed()
{
    _lck.Lock();
    _randomSource._generator.seed(std::chrono::system_clock().now().time_since_epoch().count());
    _lck.Unlock();
}

template<typename ValueType, typename BuildType, RandDistibuteType::ENUMS distType, RandSourceAlgorithmType::ENUMS sourceType>
ALWAYS_INLINE ValueType LibRandom<ValueType, BuildType, distType, sourceType>::Gen()
{
    _lck.Lock();
    ValueType rand = _dist._generator(_randomSource._generator);
    _lck.Unlock();
    return rand;
}


template<typename ValueType, typename BuildType, RandDistibuteType::ENUMS distType, RandSourceAlgorithmType::ENUMS sourceType>
ALWAYS_INLINE ValueType LibRandom<ValueType, BuildType, distType, sourceType>::Gen(ValueType startValue, ValueType endValue)
{
    if(LIKELY(endValue >= startValue))
    {
        Distributor<ValueType, distType> dist(startValue, endValue);
        _lck.Lock();
        ValueType rand = dist._generator(_randomSource._generator);
        _lck.Unlock();

        return rand;
    }

    Distributor<ValueType, distType> dist(endValue, startValue);
    _lck.Lock();
    ValueType rand = dist._generator(_randomSource._generator);
    _lck.Unlock();

    return rand;
}

template<typename Rtn, Int64 minValue, Int64 maxValue>
ALWAYS_INLINE Rtn &LibRandomInstance::GetInstance(_Build::MT::Type)
{
    static Rtn *instance = new Rtn(minValue, maxValue);
    return *instance;
}

template<typename Rtn, Int64 minValue, Int64 maxValue>
ALWAYS_INLINE Rtn &LibRandomInstance::GetInstance(_Build::TL::Type)
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR SmartPtr<Rtn> instance;
    if(UNLIKELY(!instance))
    {
        instance = KERNEL_NS::AllocUtil::GetStaticThreadLocalTemplateObjNoFree<Rtn>([]()-> void *{
            return new Rtn(minValue, maxValue);
        });
    }
    return *instance;
}

KERNEL_END

#endif
