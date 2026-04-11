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
// Date: 2026-04-11 23:04:33
// Author: Eric Yonng
// Description:


#pragma once

#include <service/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>

SERVICE_BEGIN

struct UserOptions
{
    POOL_CREATE_OBJ_DEFAULT(UserOptions);
    
    // 创建自己
    static UserOptions *CreateNewObj(UserOptions &&cfg)
    {
        return UserOptions::New_UserOptions(std::move(cfg));
    }

    // 释放自己
    void Release()
    {
        UserOptions::Delete_UserOptions(this);
    }

    // 用户lru限制数量 usermgr
    Int32 UserLruCapacityLimit = 1000;
};

SERVICE_END


namespace YAML
{
    template<>
    struct convert<SERVICE_NS::UserOptions>
    {
        static Node encode(const SERVICE_NS::UserOptions& rhs)
        {
            Node node;
            node["UserLruCapacityLimit"] = rhs.UserLruCapacityLimit;
            return node;
        }

        static bool decode(const Node& node, SERVICE_NS::UserOptions& rhs)
        {
            if (!node.IsMap())
            {
                return false;
            }

            {
                auto &&value = node["UserLruCapacityLimit"];
                if(value.IsDefined())
                {
                    rhs.UserLruCapacityLimit = value.as<Int32>();
                }
            }

            return true;
        }
    };
}