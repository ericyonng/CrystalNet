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
 * Date: 2022-10-12 12:50:58
 * Author: Eric Yonng
 * Description: 注解参数定义
*/

#include "pch.h"
#include <ProtoGen/Exporter/Defs/Defs.h>

// ///开始参数行
const KERNEL_NS::LibString ProtobufMessageParam::ParamLineBegin = "///";

const KERNEL_NS::LibString ProtobufMessageParam::AsPacket = "@as_acket";
const KERNEL_NS::LibString ProtobufMessageParam::Message = "message";
const KERNEL_NS::LibString ProtobufMessageParam::Oneof = "oneof";
const KERNEL_NS::LibString ProtobufMessageParam::ClassFlag = "class ";

const KERNEL_NS::LibString ProtobufMessageParam::ProtoInfoCacheFile = "ProtoInfoCache.pbcache";

const KERNEL_NS::LibString ProtobufMessageParam::MessageStartFlag = "[";
const KERNEL_NS::LibString ProtobufMessageParam::MessageEndFlag = "]";
const KERNEL_NS::LibString ProtobufMessageParam::ProtoFileStartFlag = "///";
const KERNEL_NS::LibString ProtobufMessageParam::CacheKVSepFlag = ":";
const KERNEL_NS::LibString ProtobufMessageParam::CacheSegSepFlag = ",";

const KERNEL_NS::LibString ProtobufMessageParam::ProtoName = "ProtoName";
const KERNEL_NS::LibString ProtobufMessageParam::ProtoPath = "ProtoPath";
const KERNEL_NS::LibString ProtobufMessageParam::MessageName = "MessageName";
const KERNEL_NS::LibString ProtobufMessageParam::Opcode = "Opcode";
const KERNEL_NS::LibString ProtobufMessageParam::NoLog = "NoLog";
const KERNEL_NS::LibString ProtobufMessageParam::XorEncrypt = "XorEncrypt";
const KERNEL_NS::LibString ProtobufMessageParam::KeyBase64 = "KeyBase64";
const KERNEL_NS::LibString ProtobufMessageParam::EnableStorage = "EnableStorage";

const KERNEL_NS::LibString ProtobufMessageParam::Md5 = "Md5";
const KERNEL_NS::LibString ProtobufMessageParam::ModifyTime = "ModifyTime";