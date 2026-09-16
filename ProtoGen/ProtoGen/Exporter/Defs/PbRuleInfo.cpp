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
 * Date: 2026-09-16 10:50:48
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <ProtoGen/Exporter/Defs/PbRuleInfo.h>

#include "Defs.h"

KERNEL_NS::LibString PbRuleInfo::GetAnnotationValue(const KERNEL_NS::LibString &annotationKey) const
{
  KERNEL_NS::LibString value;
  if(annotationKey == ProtobufMessageParam::Opcode)
  {
   value.AppendFormat("%d", _opcode);
   return value;
  }

  if(annotationKey == ProtobufMessageParam::NoLog)
  {
   value.AppendFormat("%s", _noLog ? "true":"false");
   return value;
  }

  if(annotationKey == ProtobufMessageParam::XorEncrypt)
  {
   value.AppendFormat("%s", _isXorEncrypt ? "true":"false");
   return value;
  }

  if(annotationKey == ProtobufMessageParam::KeyBase64)
  {
   value.AppendFormat("%s", _isKeyBase64 ? "true":"false");
   return value;
  }

  if(annotationKey == ProtobufMessageParam::EnableStorage)
  {
   value.AppendFormat("%s", _enableStorage ? "true":"false");
   return value;
  }

  return value;
}

void PbRuleInfo::From(const std::pair<KERNEL_NS::LibString, KERNEL_NS::LibString> &kv, bool genOpcodeMode, Int32 &maxOpcode)
{
  if(kv.first == ProtobufMessageParam::MessageName)
  {
    _messageName = kv.second;
  }
  else if(kv.first == ProtobufMessageParam::Opcode)
  {
      if(kv.second.length() != 0)
      {
          _opcode = KERNEL_NS::StringUtil::StringToInt32(kv.second.c_str());
      }
      
      if (genOpcodeMode)
      {
          if(_opcode == 0)
              _opcode = ++maxOpcode;
      }
      
      maxOpcode = std::max<Int32>(maxOpcode, _opcode);
  }
  else if(kv.first == ProtobufMessageParam::NoLog)
  {
      auto value = kv.second.strip().tolower();
      if(value.length() != 0)
      {
          _noLog = value == "true";
      } 
  }
  else if(kv.first == ProtobufMessageParam::XorEncrypt)
  {
      auto value = kv.second.strip().tolower();
      if (value.length() != 0)
      {
          _isXorEncrypt = value == "true";
      }
  }
  else if(kv.first == ProtobufMessageParam::KeyBase64)
  {
      auto value = kv.second.strip().tolower();
      if (value.length() != 0)
      {
          _isKeyBase64 = value == "true";
      }
  }
  else if(kv.first == ProtobufMessageParam::EnableStorage)
  {
      auto value = kv.second.strip().tolower();
      if (value.length() != 0)
      {
          _enableStorage = value == "true";
      }
  }
}

KERNEL_NS::LibString PbRuleInfo::ToPbChacheString() const
{
    KERNEL_NS::LibString info;
    info
        .AppendFormat("%s%s%s%s", ProtobufMessageParam::MessageName.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _messageName.c_str(), ProtobufMessageParam::CacheSegSepFlag.c_str())
        .AppendFormat("%s%s%d%s", ProtobufMessageParam::Opcode.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _opcode, ProtobufMessageParam::CacheSegSepFlag.c_str())
        .AppendFormat("%s%s%s%s", ProtobufMessageParam::NoLog.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _noLog?"true":"false", ProtobufMessageParam::CacheSegSepFlag.c_str())
        .AppendFormat("%s%s%s%s", ProtobufMessageParam::XorEncrypt.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _isXorEncrypt?"true":"false", ProtobufMessageParam::CacheSegSepFlag.c_str())
        .AppendFormat("%s%s%s%s", ProtobufMessageParam::KeyBase64.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _isKeyBase64?"true":"false", ProtobufMessageParam::CacheSegSepFlag.c_str())
        .AppendFormat("%s%s%s%s", ProtobufMessageParam::EnableStorage.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _enableStorage?"true":"false", ProtobufMessageParam::CacheSegSepFlag.c_str())
        ;
    
    return info;
}

KERNEL_NS::LibString PbRuleInfo::GenAnnotationInfo() const
{
    const auto annotationInfo = KERNEL_NS::LibString().AppendFormat("// AnnotaionInfo[opcode(%d), nolog(%s), XorEncrypt(%s), KeyBase64(%s), EnableStorage:(%s)]"
             , _opcode, _noLog ? "true" : "false", _isXorEncrypt ? "true" : "false"
             , _isKeyBase64 ? "true" : "false", _enableStorage ? "true" : "false");
    
    return annotationInfo;
}

void PbRuleInfo::GenOpCodeInfo(std::vector<KERNEL_NS::LibString> &lines) const
{
    lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._opcode = %d;", _opcode));
    lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._noLog = %s;", _noLog ? "true" : "false"));
    lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._enableStorage = %s;", _enableStorage ? "true" : "false"));

    // 加密
    if(_isXorEncrypt)
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._msgFlags |= SERVICE_COMMON_NS::MsgFlagsType::XOR_ENCRYPT_FLAG;"));
    if(_isKeyBase64)
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._msgFlags |= SERVICE_COMMON_NS::MsgFlagsType::KEY_IN_BASE64_FLAG;"));

    lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._opcodeName = \"%s\";", _messageName.c_str()));
}

void PbRuleInfo::GenTsOpCodeInfo(std::vector<KERNEL_NS::LibString> &lines) const
{
    lines.push_back(KERNEL_NS::LibString().AppendFormat("      getOpcode():number {return %s.OPCODE; }", _messageName.c_str()));
    lines.push_back(KERNEL_NS::LibString().AppendFormat("      getIsXorEncrypt():boolean {return %s.XorEncrypt; }", _messageName.c_str()));
    lines.push_back(KERNEL_NS::LibString().AppendFormat("      getIsKeyBase64():boolean {return %s.KeyBase64; }", _messageName.c_str()));
    lines.push_back(KERNEL_NS::LibString().AppendFormat("      getOpcodeName():string {return %s.OPCODE_NAME; }", _messageName.c_str()));

    lines.push_back(KERNEL_NS::LibString().AppendFormat("      static OPCODE:number = %d;", _opcode));
    lines.push_back(KERNEL_NS::LibString().AppendFormat("      static OPCODE_NAME:string = \"%s\";", _messageName.c_str()));
    lines.push_back(KERNEL_NS::LibString().AppendFormat("      static XorEncrypt:boolean = %s;", _isXorEncrypt ? "true" : "false"));
    lines.push_back(KERNEL_NS::LibString().AppendFormat("      static KeyBase64:boolean = %s;", _isKeyBase64 ? "true" : "false"));
        
}



