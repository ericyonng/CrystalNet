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
 * Date: 2021-01-24 23:53:56
 * Author: Eric Yonng
 * Description: 
 *          原理:查表，将整数拆分成n个字节,每个字节查询是O(1)
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_COUNT_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_COUNT_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

// 只统计无符号的1的个数
class KERNEL_EXPORT CountUtil
{
public:
    template <typename NumType>
    static UInt32 Count1InBinary(NumType num);

    // 可以将任意整数拆成n个字节并分别算出1的个数再相加
    static void Generate8BitStatistics1Table(UInt32 array[256]);

    // static bool Comp(UInt32 array[256]);

private:
    static UInt32 *_GetCountTable();    
};

template <typename NumType>
ALWAYS_INLINE UInt32 CountUtil::Count1InBinary(NumType num)
{
    auto countArr = _GetCountTable();
    UInt32 countNum = 0;
    for (;num;)
    {
        countNum += countArr[num & 0xff];
        num >>= 8;
    }
    
    return countNum;
}


// inline bool CountUtil::Comp(UInt32 array[256])
// {
//     auto arr2 = _GetCountTable();
//     for (Int32 i = 0; i < 256; ++i)
//     {
//         if (array[i] != arr2[i])
//             return false;
//     }

//     return true;
// }

KERNEL_END

#endif
