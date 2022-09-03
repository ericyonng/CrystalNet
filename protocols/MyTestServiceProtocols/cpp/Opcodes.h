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
 * Date: 2022-08-27 23:19:26
 * Author: Eric Yonng
 * Description: 
 * 工具自动生成, Opcodes只提供纯粹的接口，不可与相关业务耦合而降低其通用性
*/

#ifndef __CRYSTAL_NET_PROTOCOLS_MY_TEST_SERVICE_PROTOCOLS_OPCODES_H__
#define __CRYSTAL_NET_PROTOCOLS_MY_TEST_SERVICE_PROTOCOLS_OPCODES_H__

#pragma once

#include <kernel/kernel.h>

struct OpcodeInfo
{
    POOL_CREATE_OBJ_DEFAULT(OpcodeInfo);

    OpcodeInfo()
    {

    }

    OpcodeInfo
    ( Int32 opcode
    , const KERNEL_NS::LibString &opcodeName
    , const KERNEL_NS::LibString &protoFile )
    
    : _opcode(opcode)
    , _opcodeName(opcodeName)
    ,_protoFile(protoFile)
    
    {

    }

    KERNEL_NS::LibString ToString() const
    {
        KERNEL_NS::LibString info;
        info.AppendFormat("opcode:%d, "     , _opcode)
            .AppendFormat("name:%s, "       , _opcodeName.c_str())
            .AppendFormat("from:%s, "       , _from.c_str())
            .AppendFormat("to:%s, "         , _to.c_str())
            .AppendFormat("enable:%s, "     , _enable ? "true":"false")
            .AppendFormat("cd(ms):%lld, "     , _cdInMilliSec)
            .AppendFormat("Count in Cd:%lld, ", _countInCd)
            .AppendFormat("proto file:%s, " , _protoFile.c_str())
            ;

        return info;
    }

    Int32 _opcode = 0;                  // 协议id
    KERNEL_NS::LibString _opcodeName;   // 协议名
    KERNEL_NS::LibString _protoFile;    // 协议所属文件名

    bool _enable = true;                // 是否启用该协议,默认是启用的 用于禁用某些协议订阅,只需要在proto上设置协议开关即可，不用动任何代码
    KERNEL_NS::LibString _from;         // 发送方
    KERNEL_NS::LibString _to;           // 到达方
    
    Int64 _cdInMilliSec = 0;            // 协议cd时间 TODO:需要业务层支持  from 如果是cli则 默认cd时间是1s
    Int64 _countInCd = 0;               // 协议cd时间内 TODO:需要业务层支持 允许的协议次数 from如果是cli 默认是1次            
};

class Opcodes
{
public:
    enum ENUMS
    {
        BEGIN = 0,  
        TestOpcodeReq = 1,      // 测试        
        TestOpcodeRes = 2,      // 测试        
    };

    // 初始化opcode
    static Int32 Init();

    // 结束销毁opcode
    static void Destroy();

    // 转字符串
    static const KERNEL_NS::LibString &ToString(Int32 opcode);

    // 是否存在
    static bool CheckOpcode(Int32 opcode);
    static bool CheckOpcode(const KERNEL_NS::LibString &opcodeName);

    // 获取opcode信息
    static const OpcodeInfo *GetOpcodeInfo(Int32 opcode);
    static const OpcodeInfo *GetOpcodeInfo(const KERNEL_NS::LibString &opcodeName);

    // 获取所有的opcode信息
    static const std::unordered_map<Int32, OpcodeInfo *> &GetOpcodeDict();
    static const std::unordered_map<KERNEL_NS::LibString, OpcodeInfo *> &GetOpcodeNameDict();

private:
    
    // 工具写入opcode信息(工具自动生成)
    static void _ToolInput();

private:
    // opcode字典
    static std::unordered_map<Int32, OpcodeInfo *> _opcodeIdRefInfo;

    // 消息名字典
    static std::unordered_map<KERNEL_NS::LibString, OpcodeInfo *> _opcodeNameRefInfo;
    
    // 所有的opcode信息
    static std::vector<OpcodeInfo> _allOpcodeInfo;

    // 未知的opcode
    static const KERNEL_NS::LibString _unknownOpcode;
};

ALWAYS_INLINE const KERNEL_NS::LibString &Opcodes::ToString(Int32 opcode)
{
    auto iter = _opcodeIdRefInfo.find(opcode);
    if(UNLIKELY(iter == _opcodeIdRefInfo.end()))
    {
        return _unknownOpcode;
    }
    return iter->second->_opcodeName;
}

ALWAYS_INLINE bool Opcodes::CheckOpcode(Int32 opcode)
{
    auto iter = _opcodeIdRefInfo.find(opcode);
    return iter != _opcodeIdRefInfo.end();
}

ALWAYS_INLINE bool Opcodes::CheckOpcode(const KERNEL_NS::LibString &opcodeName)
{
    auto iter = _opcodeNameRefInfo.find(opcodeName);
    return iter != _opcodeNameRefInfo.end();
}

ALWAYS_INLINE const OpcodeInfo *Opcodes::GetOpcodeInfo(Int32 opcode)
{
    auto iter = _opcodeIdRefInfo.find(opcode);
    return iter == _opcodeIdRefInfo.end() ? NULL : iter->second;
}

ALWAYS_INLINE const OpcodeInfo *Opcodes::GetOpcodeInfo(const KERNEL_NS::LibString &opcodeName)
{
    auto iter = _opcodeNameRefInfo.find(opcodeName);
    return iter == _opcodeNameRefInfo.end() ? NULL : iter->second;
}

ALWAYS_INLINE void Opcodes::Destroy()
{
    KERNEL_NS::ContainerUtil::DelContainer(_opcodeIdRefInfo, [](OpcodeInfo *info)
    {
        OpcodeInfo::Delete_OpcodeInfo(info);
    });   

    _opcodeNameRefInfo.clear();
    _allOpcodeInfo.clear();
}

ALWAYS_INLINE const std::unordered_map<Int32, OpcodeInfo *> &Opcodes::GetOpcodeDict()
{
    return _opcodeIdRefInfo;
}

ALWAYS_INLINE const std::unordered_map<KERNEL_NS::LibString, OpcodeInfo *> &Opcodes::GetOpcodeNameDict()
{
    return _opcodeNameRefInfo;
}

#endif
