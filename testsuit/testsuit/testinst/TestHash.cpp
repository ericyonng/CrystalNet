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
 * Date: 2026-06-18 20:53:42
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include "TestHash.h"

void TestHash::Run()
{
 KERNEL_NS::LibString info = "hello world";
 auto hashValue =  KERNEL_NS::HashUtil::Hash64(info.c_str(), info.length());
 auto hash32 = KERNEL_NS::HashUtil::Hash(info.c_str(), info.length());
 auto hash128 = KERNEL_NS::HashUtil::Hash128(info.c_str(), info.length());

 auto isSame = (hash128.Hi == hashValue);
 CLOG_INFO_GLOBAL(TestHash, "info:%s, hash32:%u, hash64:%llu, hash128%llu%llu, is same:%d", info.c_str(), hash32, hashValue, hash128.Hi, hash128.Lo, isSame ? 1 : 0);
}
