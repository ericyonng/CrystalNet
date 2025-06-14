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
 * Date: 2025-06-08 18:00:30
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestSort.h>

static int partition(std::vector<int>& arr, int low, int high) 
{
    auto random = KERNEL_NS::LibRandom<Int32, KERNEL_NS::_Build::TL>::GetInstance<(std::numeric_limits<Int32>::min)(), (std::numeric_limits<Int32>::max)()>();

    // 随机选择基准
    int randomIndex = low + random.Gen() % (high - low + 1);
    std::swap(arr[randomIndex], arr[high]);

    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (arr[j] < pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return i + 1;
}

// 快速排序递归函数
static void quickSort(std::vector<int>& arr, int low, int high) {
    if (low < high) {
        // 获取分区索引
        int pivotIndex = partition(arr, low, high);

        // 递归排序左半部分
        quickSort(arr, low, pivotIndex - 1);
        // 递归排序右半部分
        quickSort(arr, pivotIndex + 1, high);
    }
}

void TestSort::Run()
{
    std::vector<int> arr;
    arr.push_back(1);
    arr.push_back(10);
    arr.push_back(5);
    arr.push_back(7);
    arr.push_back(16);
    arr.push_back(2);
    arr.push_back(3);

    KERNEL_NS::SortUtil::QuickSort(arr);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSort, "arr:%s"), KERNEL_NS::StringUtil::ToString(arr, ',').c_str());
}