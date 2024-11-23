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
 * Date: 2022-09-30 13:15:51
 * Author: Eric Yonng
 * Description: 
*/


#include "pch.h"
#include "service_common/protocol/CrystalProtocol/CrystalMsgHeader.h"

#ifndef DISABLE_OPCODES
 #include <protocols/protocols.h>
#endif

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(CrystalMsgHeader);

static ALWAYS_INLINE KERNEL_NS::LibString StackOpcodeToString(Int32 opcode)
{
    KERNEL_NS::LibString opcodeName;

    #ifndef DISABLE_OPCODES
    auto opcodeInfo = Opcodes::GetOpcodeInfo(opcode);
    if(LIKELY(opcodeInfo))
        opcodeName.AppendFormat("opcode name:%s", opcodeInfo->_opcodeName.c_str());
    else
        opcodeName.AppendFormat("unknown opcode");
    #endif

    return opcodeName;
}

KERNEL_NS::LibString CrystalMsgHeader::ToString() const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("opcode:%d,%s flag:%x, packetId:%lld, len:%u, protocol version:%llu, _keyLen:%u"
    , _opcodeId, StackOpcodeToString(_opcodeId).c_str(), _flags, _packetId, _len, _protocolVersion, _keyLen);

    return info;
}

SERVICE_COMMON_END
