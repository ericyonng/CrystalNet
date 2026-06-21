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
// Date: 2026-06-14 19:06:06
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <kernel/comp/Variant/VariantHelper.h>
#include <kernel/comp/Variant/Variant.h>

KERNEL_BEGIN

void VariantHelper::Del(const Variant &var)
{
    if(var.IsBriefData())
    {
        if(var.IsStreamTL())
        {
            KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(var.AsStreamTL());
        }
        else if(var.IsStreamMT())
        {
            KERNEL_NS::LibStreamMT::Delete_LibStream(var.AsStreamMT());
        }
    }
    else if(var.IsDict())
    {
        auto &dict = var.AsDict();
        for(auto &iter : dict)
        {
            Del(iter.second);
        }
    }
    else if(var.IsSeq())
    {
        auto &arr = var.AsSequence();
        const auto count = static_cast<Int32>(arr.size());
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto &item = arr[idx];
            Del(item);
        }
    }
}

size_t VariantHelper::CalculateHash(const Variant &var)
{
    std::hash<Variant> hasher;
    return hasher(var);
}

KERNEL_END