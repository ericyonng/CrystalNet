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
// Date: 2026-03-20 23:03:46
// Author: Eric Yonng
// Description:序列化反序列化LibString


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIBSTRING_YAML_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIBSTRING_YAML_H__

#pragma once

#include <kernel/comp/LibString.h>
#include <yaml-cpp/yaml.h>

namespace YAML
{
    // LibString
    template<>
    struct KERNEL_EXPORT convert<KERNEL_NS::LibString>
    {
        static Node encode(const KERNEL_NS::LibString& rhs)
        {
            Node node;
            node = rhs.GetRaw();
            return node;
        }
    
        static bool decode(const Node& node,  KERNEL_NS::LibString& rhs)
        {
            if(!node.IsDefined())
                return false;
    
            rhs = node.as<std::string>();
            return true;
        }
    };
}
#endif

