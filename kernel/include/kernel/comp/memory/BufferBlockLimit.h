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
 * Date: 2020-11-21 22:53:19
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_BUFFER_BLOCK_LIMIT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_BUFFER_BLOCK_LIMIT_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/MemoryDefs.h>
#include <kernel/comp/Utils/MathUtil.h>
#include <kernel/comp/Utils/NumWidthUtil.h>

KERNEL_BEGIN

class KERNEL_EXPORT BufferBlockLimit
{
public:
    BufferBlockLimit(){}
    ~BufferBlockLimit(){}

public:
    static UInt64 GetLimit(UInt64 bytes);

private:
    static void _Init(UInt64 maxBlockLimit, UInt64 minBlockLimit, std::map<UInt64, UInt64> &bytesRefBlockNum);

private:
};


inline UInt64 BufferBlockLimit::GetLimit(UInt64 bytes)
{
    static const UInt64 maxBlockLimit = MAX_BLOCK_NUM_PER_BUFF_DEF;
    static const UInt64 minBlockLimit = MIN_BLOCK_NUM_PER_BUFF_DEF;

    // ??????????????????????????????block??????
    static std::map<UInt64, UInt64> bytesRefBlockNum;  
    if (bytesRefBlockNum.empty())
        _Init(maxBlockLimit, minBlockLimit, bytesRefBlockNum);

    if (bytes < LOCK_WHEN_BELOW_SPECIFY_BYTES)
        return maxBlockLimit;
    if (bytes > LOCK_WHEN_OVER_SPECIFY_BYTES)
        return minBlockLimit;

    // ??????????????????
    UInt64 lastLimit = 1;
    for (auto &iterLimit:bytesRefBlockNum)
    {
        lastLimit = iterLimit.second;
        if (iterLimit.first >= bytes)
            break;
    }

    return lastLimit;
}


inline void BufferBlockLimit::_Init(UInt64 maxBlockLimit, UInt64 minBlockLimit, std::map<UInt64, UInt64> &bytesRefBlockNum)
{
    // ???????????????????????????????????????
    const Int32 max_width = NumWidthUtil::GetBinaryWidth(LOCK_WHEN_OVER_SPECIFY_BYTES - LOCK_WHEN_BELOW_SPECIFY_BYTES);
    const Int32 max_limit_width = NumWidthUtil::GetBinaryWidth(maxBlockLimit);
    const Int32 loop_to_chg = max_width / max_limit_width ? (max_width / max_limit_width) : 1;

    // _startChangeBytes * _maxBlockLimit =MEMORY_POOL_MAX_BLOCK_BYTES ??????????????? START_CHG_MEMROY_BLOCK_BYTES
    // _startChangeBytes = std::max<UInt64>(MEMORY_POOL_MAX_BLOCK_BYTES/_maxBlockLimit, START_CHG_MEMROY_BLOCK_BYTES);

    Int32 loop_count = 0;
    UInt64 blockNum = maxBlockLimit;
    for (UInt64 bytes = LOCK_WHEN_BELOW_SPECIFY_BYTES; bytes <= LOCK_WHEN_OVER_SPECIFY_BYTES; bytes<<=1)
    {
        blockNum = (blockNum > minBlockLimit) ? blockNum : minBlockLimit;
        bytesRefBlockNum.insert(std::make_pair(bytes, blockNum));

        ++loop_count;
        if (loop_count >= loop_to_chg)
        {
            loop_count = 0;
            blockNum >>= 1;
        }
    }
}

KERNEL_END

#endif
