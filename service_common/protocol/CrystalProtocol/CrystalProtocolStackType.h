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
 * Date: 2022-08-13 23:26:32
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_PROTOCOL_CRYSTAL_PROTOCOL_CRYSTAL_PROTOCOL_STACK_TYPE_H__
#define __CRYSTAL_NET_SERVICE_COMMON_PROTOCOL_CRYSTAL_PROTOCOL_CRYSTAL_PROTOCOL_STACK_TYPE_H__

#pragma once

#include <service_common/common/common.h>
#include <kernel/comp/LibString.h>

SERVICE_COMMON_BEGIN

class CrystalProtocolStackType
{
public:
    enum ENUMS
    {
        UNKNOWN = 0,
        BEGIN,
        CRYSTAL_PROTOCOL = BEGIN, // crystal协议栈类型
        CRYSTAL_PROTOCOL_NO_LIMIT, // 无限制crystal_protocol
        HTTP,                  // http协议
        JSON,                   // json
        WEBSOCKET,              // websocket
        END,
    };

    static Int32 TurnFromString(const KERNEL_NS::LibString &stackTypeName)
    {
        if(stackTypeName == "CRYSTAL_PROTOCOL")
            return CrystalProtocolStackType::CRYSTAL_PROTOCOL;
        if(stackTypeName == "CRYSTAL_PROTOCOL_NO_LIMIT")
            return CrystalProtocolStackType::CRYSTAL_PROTOCOL_NO_LIMIT;
        if(stackTypeName == "HTTP")
            return CrystalProtocolStackType::HTTP;
        if(stackTypeName == "JSON")
            return CrystalProtocolStackType::JSON;   
        if(stackTypeName == "WEBSOCKET")
            return CrystalProtocolStackType::WEBSOCKET;   

        return CrystalProtocolStackType::UNKNOWN;
    }
};

SERVICE_COMMON_END

#endif
