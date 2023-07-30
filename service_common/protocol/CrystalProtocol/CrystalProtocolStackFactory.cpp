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
 * Date: 2022-08-27 15:13:02
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include "service_common/protocol/CrystalProtocol/CrystalProtocolStackFactory.h"
#include "service_common/protocol/CrystalProtocol/CrystalProtocolStackType.h"
#include "service_common/protocol/CrystalProtocol/CrystalProtocolStack.h"
#include "service_common/protocol/CrystalProtocol/CrystalProtocolJsonStack.h"
#include "service_common/protocol/CrystalProtocol/CrystalMsgHeader.h"

SERVICE_COMMON_BEGIN

KERNEL_NS::IProtocolStack *CrystalProtocolStackFactory::Create(Int32 type, UInt64 recvMsgContentBytesLimit)
{
    switch (type)
    {
    // 无限制上行数据包大小, 一般用于内部节点通信
    case CrystalProtocolStackType::CRYSTAL_PROTOCOL_NO_LIMIT:
        return new CrystalProtocolStack(type);
    // 默认是有限制上行数据包大小
    case CrystalProtocolStackType::CRYSTAL_PROTOCOL:
    {
        auto stack = new CrystalProtocolStack(type);
        stack->SetMaxRecvMsgContentBytes(recvMsgContentBytesLimit);
        return stack;
    }
    break;
    // json解析
    case CrystalProtocolStackType::JSON:
    {
        return new CrystalProtocolJsonStack(type);
    } 
    break;
    default:
        break;
    }

    return NULL;
}


SERVICE_COMMON_END
