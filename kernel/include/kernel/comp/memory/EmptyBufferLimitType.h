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
 * Date: 2020-11-09 02:25:18
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_EMPTY_BUFFER_LIMIT_TYPE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_EMPTY_BUFFER_LIMIT_TYPE_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/MemoryDefs.h>
#include <kernel/comp/Utils/NumWidthUtil.h>

KERNEL_BEGIN

class KERNEL_EXPORT EmptyBufferLimitType
{
public:
    EmptyBufferLimitType(){}
    ~EmptyBufferLimitType(){}

public:
    // 对象池在程序启动时候初始化此时_bytesRefEmptyBufferLimit对象若没有初始化则会导致访问出错
    static UInt64 GetLimit(UInt64 bytes);

private:
    static void _Init(const UInt64 minEmptyLimit, const UInt64 maxEmptyLimit, std::map<UInt64, UInt64> &bytesRefEmptyBufferLimit);

private:
};


inline UInt64 EmptyBufferLimitType::GetLimit(UInt64 bytes)
{
    static const UInt64 minEmptyLimit = MIN_EMPTY_BUFFER_NUM_LIMIT;
    static const UInt64 maxEmptyLimit = MAX_EMPTY_BUFFER_NUM_LIMIT;

    // 初始化对象
    static std::map<UInt64, UInt64> bytesRefEmptyBufferLimit;
    if(UNLIKELY(bytesRefEmptyBufferLimit.empty()))
        _Init(minEmptyLimit, maxEmptyLimit, bytesRefEmptyBufferLimit);

    if (bytes < LOCK_WHEN_BELOW_SPECIFY_BYTES)
        return minEmptyLimit;
    if (bytes > LOCK_WHEN_OVER_SPECIFY_BYTES)
        return maxEmptyLimit;
    
    UInt64 lastLimit = 1;
    for (auto &iterLimit: bytesRefEmptyBufferLimit)
    {
        lastLimit = iterLimit.second;
        if (iterLimit.first >= bytes)
            break;
    }

    return lastLimit;
}


inline void EmptyBufferLimitType::_Init(const UInt64 minEmptyLimit, const UInt64 maxEmptyLimit, std::map<UInt64, UInt64> &bytesRefEmptyBufferLimit)
{
    // 计算每次多少次循环移位一次
    const Int32 max_width = NumWidthUtil::GetBinaryWidth(LOCK_WHEN_OVER_SPECIFY_BYTES - LOCK_WHEN_BELOW_SPECIFY_BYTES);
    const Int32 max_limit_width = NumWidthUtil::GetBinaryWidth(minEmptyLimit);
    const Int32 loop_to_chg = max_width/max_limit_width ? (max_width/max_limit_width):1;

    Int32 loop_count = 0;
    UInt64 empty_limit = minEmptyLimit;
    for (UInt64 bytes = LOCK_WHEN_BELOW_SPECIFY_BYTES; bytes <= LOCK_WHEN_OVER_SPECIFY_BYTES; bytes<<=1)
    {
        ++loop_count;
        if (loop_count >= loop_to_chg)
        {
            loop_count = 0;
            empty_limit >>= 1;
            empty_limit = (empty_limit > maxEmptyLimit) ? empty_limit :maxEmptyLimit;
        }

        bytesRefEmptyBufferLimit.insert(std::make_pair(bytes, empty_limit));
    }
}

KERNEL_END

#endif
