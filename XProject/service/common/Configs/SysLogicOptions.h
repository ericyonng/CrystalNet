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
// Date: 2026-04-11 21:04:28
// Author: Eric Yonng
// Description:

#pragma once

#include <service/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <service/common/Configs/TcpListenInfo.h>

SERVICE_BEGIN

struct SysLogicOptions
{
    POOL_CREATE_OBJ_DEFAULT(SysLogicOptions);
    
    // 创建自己
    static SysLogicOptions *CreateNewObj(SysLogicOptions &&cfg)
    {
        return SysLogicOptions::New_SysLogicOptions(std::move(cfg));
    }

    // 释放自己
    void Release()
    {
        SysLogicOptions::Delete_SysLogicOptions(this);
    }

    // 监听信息
    std::vector<TcpListenInfo> TcpListenList;
};

SERVICE_END

namespace YAML
{
    template<>
    struct convert<SERVICE_NS::SysLogicOptions>
    {
        static Node encode(const SERVICE_NS::SysLogicOptions& rhs)
        {
            Node node;
            node["TcpListenList"] = rhs.TcpListenList;
            
            return node;
        }

        static bool decode(const Node& node, SERVICE_NS::SysLogicOptions& rhs)
        {
            if(!node.IsMap())
            {
                return false;
            }

            {
                auto &&value = node["TcpListenList"];
                if(value.IsSequence())
                {
                    rhs.TcpListenList = value.as<std::vector<SERVICE_NS::TcpListenInfo>>();
                }
            }

            return true;
        }
    };
}