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
 * Date: 2022-10-19 23:38:55
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/ProtoGenService/ServiceCompHeader.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/PbCacheInfoFormat.h>

SERVICE_BEGIN

struct MessageInfo
{
    POOL_CREATE_OBJ_DEFAULT(MessageInfo);

    MessageInfo();

    void Release();

    void FieldsFromAnnotations(Int32 &maxOpcode);

    PbCaheInfo ToPbCache(const KERNEL_NS::LibString &protoName, const KERNEL_NS::LibString &protoPath) const;

    KERNEL_NS::LibString _messageName;
    Int32 _opcode;
    bool _noLog;
    bool _isXorEncrypt;
    bool _isKeyBase64;
    
    std::map<KERNEL_NS::LibString, KERNEL_NS::LibString> _annotationParamNameRefValue;  // 注解kv
};

SERVICE_END