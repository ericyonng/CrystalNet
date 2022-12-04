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
 * Date: 2022-10-19 23:26:02
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/ProtoGenService/ServiceCompHeader.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/PbCacheInfoFormat.h>

SERVICE_BEGIN

struct MessageInfo;

struct ProtoContentInfo
{
    POOL_CREATE_OBJ_DEFAULT(ProtoContentInfo);

    ProtoContentInfo();
    ~ProtoContentInfo();

    void Release();
    void MakeMd5();

    KERNEL_NS::LibString ToString() const;
    PbCacheFileInfo ToPbCache() const;
    const MessageInfo *GetMessageInfo(const KERNEL_NS::LibString &messageName) const;
    
    KERNEL_NS::LibString _md5;
    bool _isMd5Change;
    KERNEL_NS::FindFileInfo _protoInfo;
    KERNEL_NS::LibString _fullPathName;                     // 文件全名,带路径的
    std::map<Int32, KERNEL_NS::LibString> _lineRefContent;  // 每行的数据
    std::map<KERNEL_NS::LibString, MessageInfo *> _messageNameRefMessageInfo;   // pb消息相关信息
};

SERVICE_END
