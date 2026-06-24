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
// Date: 2026-06-25 01:06:04
// Author: Eric Yonng
// Description:


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIME_SLICE_YAML_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIME_SLICE_YAML_H__

#pragma once


#include <kernel/comp/TimeSlice.h>
#include <yaml-cpp/yaml.h>

namespace YAML
{
    // LibString
    template<>
    struct convert<KERNEL_NS::TimeSlice>
    {
        static ALWAYS_INLINE Node encode(const KERNEL_NS::TimeSlice& rhs)
        {
            Node node;
            node = rhs.ToString();
            return node;
        }
    
        static ALWAYS_INLINE bool decode(const Node& node,  KERNEL_NS::TimeSlice& rhs)
        {
            if(!node.IsDefined())
                return false;
    
            rhs = KERNEL_NS::TimeSlice::FromFmt(node.as<KERNEL_NS::LibString>());
            return true;
        }
    };
}


#endif
