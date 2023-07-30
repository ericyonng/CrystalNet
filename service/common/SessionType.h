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
 * Date: 2022-08-24 00:21:09
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/macro.h>
#include <service_common/ServiceCommon.h>

SERVICE_BEGIN

// 除非指定，否则默认都是外部session
class SessionType
{
    // 按照位与来判断
public:
    enum ENUMS
    {
        UNKNOWN = -1,       // 未知的
        BEGIN = 0,
        OUTER = BEGIN,      // 外部的，可以以此辨别是否要提高安全性等考虑
        INNER = 1,      // 内部的通信 包括db，各个节点间等
        OUTER_NO_LIMIT = 2,      // 外部的无限制的
        JSON_STACK = 3,     // 使用json协议
        END,
    };

    static Int32 SessionStringToSessionType(const KERNEL_NS::LibString &sessionType)
    {
        if(sessionType == "OUTER")
            return SessionType::OUTER;
        if(sessionType == "OUTER_NO_LIMIT")
            return SessionType::OUTER_NO_LIMIT;
        if(sessionType == "INNER")
            return SessionType::INNER;
        if(sessionType == "JSON_STACK")
            return SessionType::JSON_STACK;

        return SessionType::UNKNOWN;
    }
};

SERVICE_END
