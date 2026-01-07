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
// Date: 2025-06-08 18:06:55
// Author: Eric Yonng
// Description: 排序

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_SORT_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_SORT_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <utility>
#include <kernel/comp/Random/LibRandom.h>

KERNEL_BEGIN

class KERNEL_EXPORT SortUtil
{
public:
    template<typename T>
    requires requires(T t1, T t2)
    {
        t1 < t2;
    }
    static void QuickSort(std::vector<T> &arr);

private:
    template<typename T>
    requires requires(T t1, T t2)
    {
        t1 < t2;
    }
    static Int32 _Partition(std::vector<T> &arr, Int32 low, Int32 high);

    template<typename T>
    requires requires(T t1, T t2)
    {
        t1 < t2;
    }
    static void _QuickSort(std::vector<T>& arr, Int32 low, Int32 high);
};


template<typename T>
requires requires(T t1, T t2)
{
    t1 < t2;
}
ALWAYS_INLINE void SortUtil::QuickSort(std::vector<T> &arr)
{
    _QuickSort(arr, 0, static_cast<Int32>(arr.size()) - 1);
}

template<typename T>
requires requires(T t1, T t2)
{
    t1 < t2;
}
ALWAYS_INLINE Int32 SortUtil::_Partition(std::vector<T> &arr, Int32 low, Int32 high)
{
    auto &random = KERNEL_NS::LibRandom<Int32, KERNEL_NS::_Build::TL>::GetInstance<0, (std::numeric_limits<Int32>::max)()>();

    // 随机选择基准
    Int32 randomIndex = low + random.Gen() % (high - low + 1);
    std::swap(arr[randomIndex], arr[high]);

    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++)
    {
        if (arr[j] < pivot)
        {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}

// 快速排序递归函数
template<typename T>
requires requires(T t1, T t2)
{
    t1 < t2;
}
ALWAYS_INLINE void SortUtil::_QuickSort(std::vector<T>& arr, Int32 low, Int32 high)
{
    if (low < high)
    {
        // 获取分区索引
        Int32 pivotIndex = _Partition(arr, low, high);

        // 递归排序左半部分
        _QuickSort(arr, low, pivotIndex - 1);
        // 递归排序右半部分
        _QuickSort(arr, pivotIndex + 1, high);
    }
}

KERNEL_END

#endif

