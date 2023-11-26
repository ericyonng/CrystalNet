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
 * Date: 2021-01-24 18:19:16
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_CYPHER_GENERATOR_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_CYPHER_GENERATOR_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Random/Random.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class KERNEL_EXPORT CypherGeneratorUtil
{
public:
    enum Defs:Int32
    {
        CYPHER_128BIT = 16,            // 128位 共16字节
    };

public:

    template<typename BuildType = _Build::MT>
    static void Gen(Byte8 *cypher, Int32 cypherBytes);
    template<typename BuildType = _Build::MT>
    static void Gen(LibString &cypher, Int32 cypherBytes);

    template<typename BuildType = _Build::MT>
    static void SpeedGen(Byte8 *cypher, Int32 cypherBytes)
    {
        auto &randomEngine = _GetRandom(BuildType::V);

        const Int32 loopCount = cypherBytes / 4;
        UInt32 *p = (UInt32 *)(cypher);
        // 每次产生4个字节的数据
        for(Int32 i = 0; i < loopCount; ++i)
        {
            *p = static_cast<UInt32>(randomEngine.Gen());
            p += 1;
        }

        const Int32 leftBytes = cypherBytes % 4;
        if(UNLIKELY(leftBytes != 0))
        {
            Byte8 *b = (Byte8 *)(p);
            for(Int32 i = 0; i < leftBytes; ++i)
            {
                *b = static_cast<U8>(randomEngine.Gen());
                ++b;
            }
        }
    }
    template<typename BuildType = _Build::MT>
    static void SpeedGen(LibString &cypher, Int32 cypherBytes)
    {
        auto &randomEngine = _GetRandom(BuildType::V);
        cypher.resize(static_cast<UInt64>(cypherBytes));
        auto &raw = cypher.GetRaw();

        // 每次产生4个字节的数据
        const Int32 loopCount = cypherBytes / 4;
        UInt32 *p = (UInt32 *)(raw.data());
        for(Int32 i = 0; i < loopCount; ++i)
        {
            *p = static_cast<UInt32>(randomEngine.Gen());
            p += 1;
        }

        const Int32 leftBytes = cypherBytes % 4;
        if(UNLIKELY(leftBytes != 0))
        {
            Byte8 *b = (Byte8 *)(p);
            for(Int32 i = 0; i < leftBytes; ++i)
            {
                *b = static_cast<U8>(randomEngine.Gen());
                ++b;
            }
        }
    }

private:
    static LibInt64Random<_Build::MT> &_GetRandom(_Build::MT::Type);
    static LibInt64Random<_Build::TL> &_GetRandom(_Build::TL::Type);

    static LibInt64Random<_Build::MT> &_GetViewRandom(_Build::MT::Type);
    static LibInt64Random<_Build::TL> &_GetViewRandom(_Build::TL::Type);
};

ALWAYS_INLINE LibInt64Random<_Build::MT> &CypherGeneratorUtil::_GetRandom(_Build::MT::Type)
{
    return LibInt64Random<_Build::MT>::GetInstance<1, 4294967295>();
}

ALWAYS_INLINE LibInt64Random<_Build::TL> &CypherGeneratorUtil::_GetRandom(_Build::TL::Type)
{
    return LibInt64Random<_Build::TL>::GetInstance<1, 4294967295>();
}

ALWAYS_INLINE LibInt64Random<_Build::MT> &CypherGeneratorUtil::_GetViewRandom(_Build::MT::Type)
{
    return LibInt64Random<_Build::MT>::GetInstance<1, 127>();
}

ALWAYS_INLINE LibInt64Random<_Build::TL> &CypherGeneratorUtil::_GetViewRandom(_Build::TL::Type)
{
    return LibInt64Random<_Build::TL>::GetInstance<1, 127>();
}

template<typename BuildType>
ALWAYS_INLINE void CypherGeneratorUtil::Gen(Byte8 *cypher, Int32 cypherBytes)
{
    auto &randomEngine = _GetViewRandom(BuildType::V);
    for(Int32 i = 0; i < cypherBytes; ++i)
        cypher[i] = static_cast<Int32>(randomEngine.Gen());
}

template<typename BuildType>
ALWAYS_INLINE void CypherGeneratorUtil::Gen(LibString &cypher, Int32 cypherBytes)
{
    auto &randomEngine = _GetViewRandom(BuildType::V);
    cypher.resize(static_cast<UInt64>(cypherBytes));
    auto &raw = cypher.GetRaw();
    for(Int32 i = 0; i < cypherBytes; ++i)
        raw[i] = static_cast<Int32>(randomEngine.Gen());
}

KERNEL_END

#endif
