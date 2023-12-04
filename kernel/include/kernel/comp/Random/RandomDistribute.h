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
 * Date: 2021-01-24 15:45:18
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_RANDOM_RANDOM_DISTRIBUTE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_RANDOM_RANDOM_DISTRIBUTE_H__

#pragma once

#include <kernel/comp/Random/RandomDefs.h>

KERNEL_BEGIN

// 随机数分布器
template<typename ValType, RandDistibuteType::ENUMS>
struct Distributor
{

};


template<typename RandValType>
struct Distributor<RandValType, RandDistibuteType::RAND_DIS_TYPE_SMALLINT>
{
    std::uniform_int_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, RandDistibuteType::RAND_DIS_TYPE_INT>
{
    std::uniform_int_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, RandDistibuteType::RAND_DIS_TYPE_REAL>
{
    std::uniform_real_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, RandDistibuteType::RAND_DIS_TYPE_BERNOULLI>
{
    std::bernoulli_distribution _generator;
    Distributor(const RandValType minVal = RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, RandDistibuteType::RAND_DIS_TYPE_GEOMETRIC>
{
    std::geometric_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, RandDistibuteType::RAND_DIS_TYPE_EXPONENTIAL>
{
    std::exponential_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, RandDistibuteType::RAND_DIS_TYPE_NORMAL>
{
    std::normal_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, RandDistibuteType::RAND_DIS_TYPE_LOGNORMAL>
{
    std::lognormal_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, RandDistibuteType::RAND_DIS_TYPE_BINOMIAL>
{
    std::binomial_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, RandDistibuteType::RAND_DIS_TYPE_CAUCHY>
{
    std::cauchy_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

template<typename RandValType>
struct Distributor<RandValType, RandDistibuteType::RAND_DIS_TYPE_DISCRETE>
{
    std::discrete_distribution<RandValType> _generator;
    Distributor(const RandValType minVal = RandDisNumScope::RAND_DIS_NUM_SCOPE_MIN, const RandValType maxVal = ((std::numeric_limits<RandValType>::max)()))
        :_generator(minVal, maxVal)
    {

    }
};

KERNEL_END

#endif
