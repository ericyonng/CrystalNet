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
 * Date: 2020-10-18 15:26:21
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/LibStream.h>

KERNEL_BEGIN

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    const Byte8 * LibString::endl = "\n";
#else
    const Byte8 * LibString::endl = "\r\n";
#endif

// 默认需要剔除的符号
const std::string LibString::_defStripChars = DEF_STRIP_CHARS;


KERNEL_END

template<typename T>
KERNEL_NS::LibStream<T> &operator <<(KERNEL_NS::LibStream<T> &o, const KERNEL_NS::LibString &str)
{
    o.Write(str);
    return o;
}

std::string &operator <<(std::string &o, const KERNEL_NS::LibString &str)
{
    o += str.GetRaw();
    return o;
}
