// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-07-15 00:07:26
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <kernel/comp/Utils/RandomUtil.h>
#include <kernel/comp/Random/LibRandom.h>

namespace 
{
    static KERNEL_NS::LibRandom<Int64, KERNEL_NS::_Build::TL> *KernelInnerGetInt64Random()
    {
        DEF_STATIC_THREAD_LOCAL_DECLEAR KERNEL_NS::LibRandom<Int64, KERNEL_NS::_Build::TL> *random = NULL;
        if(UNLIKELY(!random))
        {
            random = &(KERNEL_NS::LibRandom<Int64, KERNEL_NS::_Build::TL>::GetInstance<(std::numeric_limits<Int64>::min)(), (std::numeric_limits<Int64>::max)()>());
            random->ResetSeed();
        }

        return random;
    }

    static KERNEL_NS::LibRandom<Int32, KERNEL_NS::_Build::TL> *KernelInnerGetInt32Random()
    {
        DEF_STATIC_THREAD_LOCAL_DECLEAR KERNEL_NS::LibRandom<Int32, KERNEL_NS::_Build::TL> *random = NULL;
        if(UNLIKELY(!random))
        {
            random = &(KERNEL_NS::LibRandom<Int32, KERNEL_NS::_Build::TL>::GetInstance<(std::numeric_limits<Int32>::min)(), (std::numeric_limits<Int32>::max)()>());
            random->ResetSeed();
        }

        return random;
    }

    static KERNEL_NS::LibRandom<UInt64, KERNEL_NS::_Build::TL> *KernelInnerGetUInt64Random()
    {
        DEF_STATIC_THREAD_LOCAL_DECLEAR KERNEL_NS::LibRandom<UInt64, KERNEL_NS::_Build::TL> *random = NULL;
        if(UNLIKELY(!random))
        {
            random = &(KERNEL_NS::LibRandom<UInt64, KERNEL_NS::_Build::TL>::GetInstance<(std::numeric_limits<UInt64>::min)(), (std::numeric_limits<UInt64>::max)()>());
            random->ResetSeed();
        }

        return random;
    }

    static KERNEL_NS::LibRandom<UInt32, KERNEL_NS::_Build::TL> *KernelInnerGetUInt32Random()
    {
        DEF_STATIC_THREAD_LOCAL_DECLEAR KERNEL_NS::LibRandom<UInt32, KERNEL_NS::_Build::TL> *random = NULL;
        if(UNLIKELY(!random))
        {
            random = &(KERNEL_NS::LibRandom<UInt32, KERNEL_NS::_Build::TL>::GetInstance<(std::numeric_limits<UInt32>::min)(), (std::numeric_limits<UInt32>::max)()>());
            random->ResetSeed();
        }

        return random;
    }
}

KERNEL_BEGIN

Int64 RandomUtil::RandomInt64()
{
    auto random = KernelInnerGetInt64Random();
    return random->Gen();
}

// [start, endValue]:左闭, 右闭
Int64 RandomUtil::RandomInt64(Int64 start, Int64 endValue)
{
    auto random = KernelInnerGetInt64Random();
    return random->Gen(start, endValue);
}

// 整个Int32整数域
Int32 RandomUtil::RandomInt32()
{
    auto random = KernelInnerGetInt32Random();
    return random->Gen();
}

// [start, endValue]:左闭, 右闭
Int32 RandomUtil::RandomInt32(Int32 start, Int32 endValue)
{
    auto random = KernelInnerGetInt32Random();
    return random->Gen(start, endValue);
}

UInt64 RandomUtil::RandomUInt64()
{
    auto random = KernelInnerGetUInt64Random();
    return random->Gen();
}

// [start, endValue]:左闭, 右闭
UInt64 RandomUtil::RandomUInt64(UInt64 start, UInt64 endValue)
{
    auto random = KernelInnerGetUInt64Random();
    return random->Gen(start, endValue);
}

// 整个Int32整数域
UInt32 RandomUtil::RandomUInt32()
{
    auto random = KernelInnerGetUInt32Random();
    return random->Gen();
}

// [start, endValue]:左闭, 右闭
UInt32 RandomUtil::RandomUInt32(UInt32 start, UInt32 endValue)
{
    auto random = KernelInnerGetUInt32Random();
    return random->Gen(start, endValue);
}

KERNEL_END
