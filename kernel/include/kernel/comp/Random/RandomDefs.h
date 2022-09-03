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
 * Date: 2021-01-24 15:51:36
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_RANDOM_RANDOM_DEFS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_RANDOM_RANDOM_DEFS_H__

#pragma once

#include <kernel/kernel_inc.h>

KERNEL_BEGIN

// 梅森螺旋算法结构类型定义
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    typedef std::mt19937 LibMt19937;
#else
    typedef std::tr1::mt19937 LibMt19937;
#endif

class KERNEL_EXPORT RandDistibuteType
{
public:
    enum ENUMS
    {
        RAND_DIS_TYPE_SMALLINT = 0, // 一个整数集上的离散均匀分布
        RAND_DIS_TYPE_INT,          // 一个整数集上的离散均匀分布
        RAND_DIS_TYPE_01,           // [0, 1)上的连续均匀分布
        RAND_DIS_TYPE_REAL,         // [min, max)上的连续均匀分布
        RAND_DIS_TYPE_BERNOULLI,    // 伯努利分布
        RAND_DIS_TYPE_GEOMETRIC,    // 几何分布
        RAND_DIS_TYPE_TRIANGLE,     // 三角分布
        RAND_DIS_TYPE_EXPONENTIAL,  // 指数分布
        RAND_DIS_TYPE_NORMAL,       // 正态分布
        RAND_DIS_TYPE_LOGNORMAL,    // 对数分布
        RAND_DIS_TYPE_ON_SPHERE,    // 球面上的均匀分布
        RAND_DIS_TYPE_BETA,         // 贝塔分布
        RAND_DIS_TYPE_BINOMIAL,     // 二项分布
        RAND_DIS_TYPE_CAUCHY,       // 柯西分布
        RAND_DIS_TYPE_DISCRETE,     // 离散分布
    };
};

class KERNEL_EXPORT RandSourceAlgorithmType
{
public:
    // 随机数源产生算法 算法速度有高到低，算法质量由低到高
    enum ENUMS
    {
        RAND_GEN_ALGORITHM_TYPE_RAND48 = 0,             // rand48算法随机数发生器
        RAND_GEN_ALGORITHM_TYPE_MT19937,                // mt19937算法随机数发生器
        RAND_GEN_ALGORITHM_TYPE_MT19937_64,             // mt19937-64算法随机数发生器
        RAND_GEN_ALGORITHM_TYPE_LAGGED_FIBONACCI19937,  // lagged_fibonacci19937算法随机数发生器
    };
};

class KERNEL_EXPORT RandDisNumScope
{
public:
    enum ENUMS : Int64
    {
        RAND_DIS_NUM_SCOPE_MIN      = 0LL,
        RAND_DIS_NUM_SCOPE_INT64MAX = 1152921504606846976LL,
        RAND_DIS_NUM_SCOPE_INT32MAX = 1073741824,
    };
};

KERNEL_END

#endif
