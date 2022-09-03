/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2020-10-11 16:46:15
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_NUM_WIDTH_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_NUM_WIDTH_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/common.h>

KERNEL_BEGIN

class KERNEL_EXPORT NumWidthUtil
{
public:
    // 最大的二进制位宽
    template<typename ObjType>
    static Int32 GetBinaryWidth()
    {
        Int32 cnt = 1;
        auto max = (std::numeric_limits<ObjType>::max)();
        while(max >>= 1)
            ++cnt;

        return cnt;
    }

    // 返回value的二进制位宽
    template<typename ObjType>
    static Int32 GetBinaryWidth(ObjType val)
    {
        Int32 cnt = val?1:0;
        while(val >>= 1)
            ++cnt;

        return cnt;
    }

    // 返回十进制最大位宽
    template<typename ObjType>
    static Int32 GetDecimalWidth()
    {
        Int32 cnt = 1;
        auto max = (std::numeric_limits<ObjType>::max)();
        while(max /= 10)
            ++cnt;

        return cnt;
    }
    
    // 返回value的十进制位宽
    template<typename ObjType>
    static Int32 GetDecimalWidth(ObjType val)
    {
        Int32 cnt = val?1:0;
        while(val /= 10)
            ++cnt;

        return cnt;
    }
};

KERNEL_END

#endif
