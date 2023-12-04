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
 * Date: 2021-01-24 15:45:28
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_RANDOM_RANDOM_SOURCE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_RANDOM_RANDOM_SOURCE_H__

#pragma once

#include <kernel/comp/Random/RandomDefs.h>
#include <chrono>

KERNEL_BEGIN

// 随机数源
template<typename RandValType, RandSourceAlgorithmType::ENUMS>
struct RandomSource
{

};

template<typename RandValType>
struct RandomSource<RandValType, RandSourceAlgorithmType::RAND_GEN_ALGORITHM_TYPE_MT19937>
{
    LibMt19937 _generator;
    RandomSource(const RandValType srandVal = static_cast<RandValType>(std::chrono::system_clock().now().time_since_epoch().count()))
        :_generator(srandVal)
    {

    }
};

template<typename RandValType>
struct RandomSource<RandValType, RandSourceAlgorithmType::RAND_GEN_ALGORITHM_TYPE_MT19937_64>
{
    std::mt19937_64 _generator;
    RandomSource(const RandValType srandVal = static_cast<RandValType>(std::chrono::system_clock().now().time_since_epoch().count()))
        :_generator(srandVal)
    {

    }
};

// 64与32在不同编译器上的性能各不相同，一般64比32会快一点 所以建议使用MT19937-64
typedef RandomSource<Int64, RandSourceAlgorithmType::RAND_GEN_ALGORITHM_TYPE_MT19937_64> MT1993764RandSrc;
typedef RandomSource<Int64, RandSourceAlgorithmType::RAND_GEN_ALGORITHM_TYPE_MT19937> MT19937RandSrc;


KERNEL_END

#endif
