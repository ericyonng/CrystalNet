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
// Date: 2026-06-30 15:04:28
// Author: Eric Yonng
// Description:

#pragma once

#include <service/common/macro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/LibStringYaml.h>

SERVICE_BEGIN

struct StorageOptions
{
    POOL_CREATE_OBJ_DEFAULT(StorageOptions);
    
    // 创建自己
    static StorageOptions *CreateNewObj(StorageOptions &&cfg)
    {
        return StorageOptions::New_StorageOptions(std::move(cfg));
    }

    // 释放自己
    void Release()
    {
        StorageOptions::Delete_StorageOptions(this);
    }
    
    KERNEL_NS::LibString DbName;
};

SERVICE_END


namespace YAML
{
    template<>
    struct convert<SERVICE_NS::StorageOptions>
    {
        static Node encode(const SERVICE_NS::StorageOptions& rhs)
        {
            Node node;
            node["DbName"] = rhs.DbName;
            
            return node;
        }

        static bool decode(const Node& node, SERVICE_NS::StorageOptions& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            {
                auto &&value = node["DbName"];
                if(value.IsDefined())
                {
                    rhs.DbName = value.as<KERNEL_NS::LibString>();
                }
            }

            return true;
        }
    };
}