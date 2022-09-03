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
 * Date: 2020-10-11 21:52:44
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_BIT_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_BIT_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/BitWidth.h>

KERNEL_BEGIN

class KERNEL_EXPORT BitUtil
{
public:
    template <typename ObjType, typename PosType>
    static inline bool IsSet(ObjType val, PosType pos)
    {
        return (val & (static_cast<ObjType>(1) << pos)) != 0;
    }

    template <typename ObjType, typename PosType>
    static inline ObjType Set(ObjType val, PosType pos)
    {
        return val | (static_cast<ObjType>(1) << pos);
    }

    template <typename ObjType, typename PosType>
    static inline ObjType Clear(ObjType val, PosType pos)
    {
        return val &= ~(static_cast<ObjType>(1) << pos);
    }
};


// 简易位图算法,基于map,可支持std::map/::google::protobuf::map等接口风格相同的map结构
class KERNEL_EXPORT SimpleBitmapUtil
{
public:    
    template<typename MapType, typename ValueType>
    static inline bool IsSet(const MapType &mapContainer, ValueType val, RemoveReferenceType<ValueType> binaryWidth = BitWidth<RemoveReferenceType<ValueType>, RemoveReferenceType<ValueType>>::_value)
    {
        // 索引位
        ValueType idx = val / binaryWidth;
        // 索引字段的标记位
        ValueType rest = val % binaryWidth;

        // 超出说明并没有设置
        auto iter = mapContainer.find(idx);
        if (iter == mapContainer.end())
            return false;

        return BitUtil::IsSet(iter->second, rest);
    }

    template<typename MapType, typename ValueType>
    static inline void Set(MapType &mapContainer, ValueType val, RemoveReferenceType<ValueType> binaryWidth = BitWidth<RemoveReferenceType<ValueType>, RemoveReferenceType<ValueType>>::_value)
    {
        // 索引位
        ValueType idx = val / binaryWidth;
        // 索引字段的标记位
        ValueType rest = val % binaryWidth;

        auto iter = mapContainer.find(idx);
        if (iter == mapContainer.end())
        {
            iter = mapContainer.insert(std::make_pair(idx, 0)).first;
        }

        iter->second = BitUtil::Set(iter->second, rest);
    }

    template<typename MapType, typename ValueType>
    static inline void Clear(MapType &mapContainer, ValueType val, RemoveReferenceType<ValueType> binaryWidth = BitWidth<RemoveReferenceType<ValueType>, RemoveReferenceType<ValueType>>::_value)
    {
        // 索引位
        ValueType idx = val / binaryWidth;
        // 索引字段的标记位
        ValueType rest = val % binaryWidth;

        auto iter = mapContainer.find(idx);
        if (iter == mapContainer.end())
            return;

        iter->second = BitUtil::Clear(iter->second, rest);
    }

};

KERNEL_END

#endif
