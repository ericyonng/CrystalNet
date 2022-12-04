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
 * 
 *  缓存协议格式：[ProtoName:xxx,// 需要注意是排除ProtoPath后的路径proto名， MessageName: 协议名, Opcode: 协议号]
 * 缓存 proto文件信息格式:/// ProtoName:xxx,// 需要注意是排除ProtoPath后的路径proto名, md5:, ModifyTime:, ProtoPath:
*/

#pragma once

#include <service/ProtoGenService/ServiceCompHeader.h>

SERVICE_BEGIN

class ProtobufMessageParam
{
public:
    enum PARAMS_ENUMS
    {

    };

    // 参数行开始标志
    static const KERNEL_NS::LibString ParamLineBegin;

    // @as_packet()
    static const KERNEL_NS::LibString AsPacket;

    // message 
    static const KERNEL_NS::LibString Message;
    static const KERNEL_NS::LibString ClassFlag;
    
    // proto 协议信息缓存文件
    static const KERNEL_NS::LibString ProtoInfoCacheFile;
    static const KERNEL_NS::LibString MessageStartFlag;
    static const KERNEL_NS::LibString MessageEndFlag;
    static const KERNEL_NS::LibString ProtoFileStartFlag;
    static const KERNEL_NS::LibString CacheKVSepFlag;
    static const KERNEL_NS::LibString CacheSegSepFlag;

    // 字段
    static const KERNEL_NS::LibString ProtoName;
    static const KERNEL_NS::LibString ProtoPath;
    static const KERNEL_NS::LibString MessageName;
    static const KERNEL_NS::LibString Opcode;
    static const KERNEL_NS::LibString Md5;
    static const KERNEL_NS::LibString ModifyTime;
};

SERVICE_END