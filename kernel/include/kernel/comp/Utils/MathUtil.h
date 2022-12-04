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
 * Date: 2020-11-21 23:49:40
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_MATH_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_MATH_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>

KERNEL_BEGIN

class KERNEL_EXPORT MathUtil
{
public:
    // base:底数,trueNum:真数 采用乘法逼近 若能够整除则结果是准确的，不能够整除的只能取得近似整数值
    // return 对数
    static UInt64 log(UInt64 base, UInt64 trueNum);
    // 最大公约数（欧几里得算法）
    static Int64 GetGcd(Int64 a, Int64 b);
    // 最小公倍数
    static Int64 GetLcm(Int64 a, Int64 b);
};

inline UInt64 MathUtil::log(UInt64 base, UInt64 trueNum)
{
    UInt64 result = 0;
    const UInt64 multi = base;    
    // 不能使用真数除法，因为不能整除会损失精度，只能用乘法逼近
    for (base = 1; (base*=multi) <= trueNum; )
        ++result;    

    return result;
}

// 最大公约数（欧几里得算法）
inline Int64 MathUtil::GetGcd(Int64 a, Int64 b)
{
    // 当b为0时,得出结果,a既为结果
    Int64 t = 0;
    while (b)
    {
        // 存b
        t = b;
        // b为a对b取模
        b = (a % b);
        // a为上一次的b
        a = t;
    }

    return a;
}

// 最小公倍数
inline Int64 MathUtil::GetLcm(Int64 a, Int64 b)
{
    return a * b / GetGcd(a, b);
}

KERNEL_END

#endif