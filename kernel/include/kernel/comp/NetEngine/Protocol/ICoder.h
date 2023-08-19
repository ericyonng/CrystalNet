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
 * Date: 2022-06-26 22:39:31
 * Author: Eric Yonng
 * Description: ICoder与protobuff/json等配合使用 
 * 例如: 
 *          class LoginReq : public ::google::protobuf::Message, public SERVICE_COMMON_NS::ICoder
 *          {
 *              public:
 *                  virtual bool Encode(LibPacket &packet);
 *                  virtual bool Decode(const LibPacket &packet);
 *          };
 *          class LoginReq : public KERNEL_NS::json, public SERVICE_COMMON_NS::ICoder
 *          {
 *              public:
 *                  virtual bool Encode(LibPacket &packet);
 *                  virtual bool Decode(const LibPacket &packet);
 *          };
 * 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_PROTOCOL_ICODER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_PROTOCOL_ICODER_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibStream.h>

KERNEL_BEGIN

// 编码器
class KERNEL_EXPORT ICoder
{
    POOL_CREATE_OBJ_DEFAULT(ICoder);

public:
    ICoder(){}
    virtual ~ICoder(){}
    virtual void Release() = 0;

    virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) = 0;
    virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) = 0;

    virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) = 0;
    virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) = 0;

    virtual KERNEL_NS::LibString ToJsonString() const { return ""; }
    virtual bool ToJsonString(std::string *data) const { return false; }
    virtual bool FromJsonString(const Byte8 *data, size_t len) { return false; }
};

// 工厂方法
class KERNEL_EXPORT ICoderFactory
{
    POOL_CREATE_OBJ_DEFAULT(ICoderFactory);
    
public:
    ICoderFactory() {}
    virtual ~ICoderFactory() {}
    virtual void Release() = 0;
    
    virtual ICoder *Create() const = 0;
    virtual ICoder *Create(const ICoder *coder) const = 0;
};

KERNEL_END

#endif
