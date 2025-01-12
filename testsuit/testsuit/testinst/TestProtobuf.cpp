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
 * Date: 2022-11-17 03:14:50
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestProtobuf.h>
#include <protocols/protocols.h>

void TestProtobuf::Run()
{
    Opcodes::Init();

    Opcodes::CheckOpcode(Opcodes::OpcodeConst::OPCODE_LoginReq);

    g_Log->Custom("OPCODE_LoginReq:%s", Opcodes::GetOpcodeInfo(Opcodes::OpcodeConst::OPCODE_LoginReq)->ToString().c_str());

    ::CRYSTAL_NET::service::LoginReq *req = new ::CRYSTAL_NET::service::LoginReq;

    KERNEL_NS::LibString info;
    std::string data;
    req->SerializeToString(&data);
    info = data;
    req->PrintDebugString();

    ::CRYSTAL_NET::service::LoginReq *req2 = new ::CRYSTAL_NET::service::LoginReq;
    req2->ParseFromString(info.GetRaw());
    req2->PrintDebugString();

    Opcodes::Destroy();
}