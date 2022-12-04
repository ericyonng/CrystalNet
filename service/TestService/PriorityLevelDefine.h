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
 * Date: 2022-08-23 23:58:35
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/common.h>

SERVICE_BEGIN

class PriorityLevelDefine
{
public:
    enum ENUMS : UInt32
    {
        INNER = 0,      // 内部节点
        DB = 1,         // db节点

        // 3个外部可以负载均衡
        OUTER_LEVEL_BEGIN = 2,  // 外部
        OUTER1 = 2,       // 外部1 
        OUTER2 = 3,       // 外部2
        OUTER3 = 4,       // 外部3
        OUTER_LEVEL_END = 4,    // 外部
    };

    enum TYPE_ENUMS
    {
        INNER_TYPE = 0,      // 内部类型
        DB_TYPE,            // 数据库类型
        OUTER_TYPE,         // 面向外部类型
    };

    static UInt32 StringToPriorityLevel(const KERNEL_NS::LibString &levelStr)
    {
        if(levelStr == "INNER")
            return PriorityLevelDefine::INNER;
        if(levelStr == "DB")
            return PriorityLevelDefine::DB;
        if(levelStr == "OUTER1")
            return PriorityLevelDefine::OUTER1;
        if(levelStr == "OUTER2")
            return PriorityLevelDefine::OUTER2;
        if(levelStr == "OUTER3")
            return PriorityLevelDefine::OUTER3;
        
        return PriorityLevelDefine::OUTER1;
    }
};

SERVICE_END

