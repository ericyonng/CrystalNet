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
 * Date: 2021-12-28 00:22:46
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_PROTOCOL_TYPE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_PROTOCOL_TYPE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

class KERNEL_EXPORT ProtocolType
{
public:
    enum
    {
        TCP = 1,    // tcp协议
        UDP = 2,    // udp协议  (以后支持)
        QUIC = 3,   // quic协议（以后支持）
    };

    static const Byte8 *ToString(Int32 protocolType)
    {
        switch (protocolType)
        {
        case ProtocolType::TCP: return "TCP";
        case ProtocolType::UDP: return "UDP";
        case ProtocolType::QUIC: return "QUIC";
        default:
            // g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::ProtocolType, "unknown protocoltype:%d"), protocolType);
            break;
        }

        return "unknown protocol";
    }

    static Int32 ToIpProtocol(Int32 protocolType)
    {
        switch (protocolType)
        {
        case ProtocolType::TCP: return SOCK_STREAM;
        case ProtocolType::UDP: return SOCK_DGRAM;
        default:
            // g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::ProtocolType, "unknown protocoltype:%d"), protocolType);
            break;
        }

        return -1;
    }

    static const Byte8 *ToProtocolType(Int32 type)
    {
        switch (type)
        {
        case SOCK_STREAM: return "SOCK_STREAM";
        case SOCK_DGRAM: return "SOCK_DGRAM";
        default:
            // g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::ProtocolType, "unknown type:%d"), type);
            break;
        }

        return "unknown type";
    }
};

KERNEL_END

#endif
