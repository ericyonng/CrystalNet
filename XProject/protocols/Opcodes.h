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
            .AppendFormat("nolog:%s, "     , _noLog ?  "true" : "false")
            .AppendFormat("_msgFlags:%x, "     , _msgFlags)
            .AppendFormat("name:%s, "       , _opcodeName.c_str())
            .AppendFormat("proto file:%s, " , _protoFile.c_str())
            ;

        return info;
    }

    Int32 _opcode = 0;                  // 协议id
    bool _noLog = false;                // 消息需不需要打印日志
    bool _enableStorage = false;        // 需不需要存储能力
    UInt32 _msgFlags = 0;                  // MsgFlagsType
    KERNEL_NS::LibString _opcodeName;   // 协议名
    KERNEL_NS::LibString _protoFile;    // 协议所属文件名
};

class Opcodes
{
public:

    // opcode枚举
    #include "OpcodeEnums.h"

    // 初始化opcode
    static Int32 Init();

    // 结束销毁opcode
    static void Destroy();

    // 转字符串
    static const KERNEL_NS::LibString &ToString(Int32 opcode);

    // 是否存在
    static bool CheckOpcode(Int32 opcode);
    static bool CheckOpcode(const KERNEL_NS::LibString &opcodeName);

    // 是否需要日志打印
    static bool IsNeedLog(Int32 opcode);

    // 获取flags
    static UInt32 GetFlags(Int32 opcode);

    // 获取opcode信息
    static const OpcodeInfo *GetOpcodeInfo(Int32 opcode);
    static const OpcodeInfo *GetOpcodeInfo(const KERNEL_NS::LibString &opcodeName);

    // 获取所有的opcode信息
    static const std::unordered_map<Int32, OpcodeInfo *> &GetOpcodeDict();
    static const std::unordered_map<KERNEL_NS::LibString, OpcodeInfo *> &GetOpcodeNameDict();

    // 获取factory
    static KERNEL_NS::ICoderFactory *GetCoderFactory(Int32 opcode);
    static void RegisterCoderFactory(Int32 opcode, KERNEL_NS::ICoderFactory *coderFactory);

private:
    
    // 工具写入opcode信息(工具自动生成)
    static void _ToolInput();

private:
    // opcode字典
    static std::unordered_map<Int32, OpcodeInfo *> _opcodeIdRefInfo;

    // 消息名字典
    static std::unordered_map<KERNEL_NS::LibString, OpcodeInfo *> _opcodeNameRefInfo;

    // opcode对应的factory
    static std::unordered_map<Int32, KERNEL_NS::ICoderFactory *> _opcodeRefCoderFactory;

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

ALWAYS_INLINE bool Opcodes::IsNeedLog(Int32 opcode)
{
    auto opcodeInfo = GetOpcodeInfo(opcode);
    if(UNLIKELY(!opcodeInfo))
        return false;

    return !(opcodeInfo->_noLog);
}

ALWAYS_INLINE UInt32 Opcodes::GetFlags(Int32 opcode)
{
    auto opcodeInfo = GetOpcodeInfo(opcode);
    if(UNLIKELY(!opcodeInfo))
        return 0;

    return opcodeInfo->_msgFlags;
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

    KERNEL_NS::ContainerUtil::DelContainer2(_opcodeRefCoderFactory);

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

ALWAYS_INLINE KERNEL_NS::ICoderFactory *Opcodes::GetCoderFactory(Int32 opcode)
{
    auto iter = _opcodeRefCoderFactory.find(opcode);
    return iter == _opcodeRefCoderFactory.end() ? NULL : iter->second;
}

ALWAYS_INLINE void Opcodes::RegisterCoderFactory(Int32 opcode, KERNEL_NS::ICoderFactory *coderFactory)
{
    auto iter = _opcodeRefCoderFactory.find(opcode);
    if(iter != _opcodeRefCoderFactory.end())
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(Opcodes, "replace old coder factory with new one opcode info:%s."), GetOpcodeInfo(opcode)->ToString().c_str());
        iter->second->Release();
        iter->second = coderFactory;
        return;
    }

    _opcodeRefCoderFactory.insert(std::make_pair(opcode, coderFactory));
}

#endif
