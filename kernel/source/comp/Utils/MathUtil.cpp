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
 * Date: 2024-05-11 21:56:27
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/MathUtil.h>
#include <kernel/comp/Utils/Defs/Math.h>

KERNEL_BEGIN

LibString MathUtil::ToFmtDataSize(Int64 bytes)
{
    // 小于1MB大于1KB的使用KB
    // 大于1MB小于1GB的使用MB
    // 大于1GB的使用GB
    // 精度保留3位小数
    LibString info;
    if(bytes < __LIB_DATA_1KB__)
    {// 小于1KB的使用B/s
        info.AppendFormat("%lld B", bytes);
        return info;
    }
    else if(bytes >= __LIB_DATA_1KB__ && bytes < __LIB_DATA_1MB__)
    {
        double valueData = static_cast<double>(bytes);
        valueData = valueData / __LIB_DATA_1KB__;
        info.AppendFormat("%.3lf KB", valueData);
        return info;
    }
    else if(bytes >= __LIB_DATA_1MB__ && bytes < __LIB_DATA_1GB__)
    {
        double valueData = static_cast<double>(bytes);
        valueData = valueData / __LIB_DATA_1MB__;
        info.AppendFormat("%.3lf MB", valueData);
        return info;
    }
    else if(bytes >= __LIB_DATA_1GB__)
    {
        double valueData = static_cast<double>(bytes);
        valueData = valueData / __LIB_DATA_1GB__;
        info.AppendFormat("%.3lf GB", valueData);
        return info;
    }

    return "";
}
KERNEL_END
