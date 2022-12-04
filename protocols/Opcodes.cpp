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
 * Date: 2022-08-27 23:44:57
 * Author: Eric Yonng
 * Description: 
 * 工具自动生成
*/

#include <pch.h>
#include <protocols/Opcodes.h>
#include <protocols/AllPbs.h>

std::unordered_map<Int32, OpcodeInfo *> Opcodes::_opcodeIdRefInfo;
std::unordered_map<KERNEL_NS::LibString, OpcodeInfo *> Opcodes::_opcodeNameRefInfo;
std::vector<OpcodeInfo> Opcodes::_allOpcodeInfo;
std::unordered_map<Int32, KERNEL_NS::ICoderFactory *> Opcodes::_opcodeRefCoderFactory;

const KERNEL_NS::LibString Opcodes::_unknownOpcode = "UNKNOWN_OPCOE";

POOL_CREATE_OBJ_DEFAULT_IMPL(OpcodeInfo);

Int32 Opcodes::Init()
{
    if(!_opcodeIdRefInfo.empty())
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(Opcodes, "redo opcodes init."));
        return Status::Success;
    }

    // 1. 输入所有opcode信息
    _ToolInput();

    // 2.所有opcode信息转换成字段并检查错误
    for(auto &opcodeInfo : _allOpcodeInfo)
    {
        {// 2.1 opcode字典校验
            auto iter = _opcodeIdRefInfo.find(opcodeInfo._opcode);
            if(iter != _opcodeIdRefInfo.end())
            {// 重复
                auto oldInfo = iter->second;
                g_Log->Error(LOGFMT_NON_OBJ_TAG(Opcodes, "repeate opcode info: opcode id:%d, old opcode info:%s, new opcode info:%s")
                            , oldInfo->_opcode, oldInfo->ToString().c_str(), opcodeInfo.ToString().c_str());
                return Status::Repeat;
            }
        }

        {// 2.2 opcode名字字典校验
            auto iter = _opcodeNameRefInfo.find(opcodeInfo._opcode);
            if(iter != _opcodeNameRefInfo.end())
            {// 重复
                auto oldInfo = iter->second;
                g_Log->Error(LOGFMT_NON_OBJ_TAG(Opcodes, "repeate opcode info: opcode id:%d, old opcode info:%s, new opcode info:%s")
                            , oldInfo->_opcode, oldInfo->ToString().c_str(), opcodeInfo.ToString().c_str());
                return Status::Repeat;
            }
        }

        // 2.3 新建OpcodeInfo
        auto newInfo = OpcodeInfo::New_OpcodeInfo(opcodeInfo._opcode
                                                , opcodeInfo._opcodeName
                                                , opcodeInfo._protoFile);

        // 2.4 建立字典
        _opcodeIdRefInfo.insert(std::make_pair(newInfo->_opcode, newInfo));
        _opcodeNameRefInfo.insert(std::make_pair(newInfo->_opcodeName, newInfo));
    }
    
    return Status::Success;
}

void Opcodes::_ToolInput()
{
    #include "OpcodeInfo.h"
}

