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
// Date: 2026-08-21 19:24:02
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <kernel/comp/FileMonitor/SourceWrap.h>
#include <kernel/comp/Utils/HashUtil.h>

KERNEL_BEGIN

LibString SourceWrap::ToString() const
{
    return LibString().AppendFormat("Path:%s, FromMemory:%p", Path.c_str(), FromMemory);
}
    
LibString &SourceWrap::MakeKey() const
{
    if(LIKELY(!GenKey.empty()))
        return GenKey;
    
    GenKey.AppendFormat("%s-%p", Path.c_str(), FromMemory);
    return GenKey;
}
    
UInt64 SourceWrap::HashCode() const
{
    if (LIKELY(MakeKeyHashCode != 0))
        return MakeKeyHashCode;
    
    auto &&key = MakeKey();
    MakeKeyHashCode = HashUtil::Hash64(key.data(), key.size());
    
    return MakeKeyHashCode;
}
KERNEL_END